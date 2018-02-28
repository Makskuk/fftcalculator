#include "basedatareader.h"

BaseDataReader::BaseDataReader(QObject *parent) : QObject(parent),
    m_outputPath(QDir::currentPath()),
    m_outputFileName(""),
    m_inputChannelVector(new QVector<FftCalculator::DataVector*>()),
    m_workers(new QVector<Worker*>()),
    m_finishedWorkers(0),
    m_samplesCount(Worker::samplesCount()),
    m_channelsCount(0),
    m_bytesPerSample(0),
    m_sampleRate(0),
    m_readPos(0),
    m_internalBufferLength(0),
    m_inited(false)
{
    /* В дочерних классах нужно определить параметры входного сигнала
     * m_sampleRate, m_bytesPerSample, m_channelsCount.
     * Их нужно взять либо из заголовка WAV-файла, либо из параметров входного устройства.
     */
    connect(this, &BaseDataReader::bufferRead, this, &BaseDataReader::onBufferRead);
    m_inputChannelVector->fill(nullptr);
}

BaseDataReader::~BaseDataReader()
{
    uninit();
    for (int i=0; i < m_channelsCount; i++) {
        if (m_inputChannelVector->at(i))
            delete m_inputChannelVector->at(i);
    }
    delete m_inputChannelVector;
    delete m_workers;
}

bool BaseDataReader::init()
{
    if (!m_channelsCount) {
        emit error("Channels count not set!");
        return false;
    }

    if (m_inited) {
        return true;
    }

    m_internalBufferLength = m_samplesCount * m_bytesPerSample * m_channelsCount;

    m_inputChannelVector->resize(m_channelsCount);
    // Создаем дочерние процессы-воркеры - по одному на канал
    for (int i=0; i < m_channelsCount; i++) {
        if (m_inputChannelVector->at(i) == nullptr)
            m_inputChannelVector->replace(i, new FftCalculator::DataVector(m_samplesCount));

        Worker *worker = new Worker(i);
        worker->setOutputDir(m_outputPath);
        worker->setOutputName(m_outputFileName);
        m_workers->append(worker);
        connect(worker, &Worker::done, this, &BaseDataReader::onFftFinished);
    }

    m_inited = true;
    return true;
}

void BaseDataReader::uninit()
{
    if (!m_inited) return;
    for (int i=0; i < m_channelsCount; i++) {
        if (m_workers->at(i))
            m_workers->at(i)->stop();
//        if (m_inputChannelVector->at(i))
//            delete m_inputChannelVector->at(i);
    }
    m_inited = false;
}

void BaseDataReader::setOutputPath(QString absOutputPath)
{
    if (absOutputPath == m_outputPath)
        return;

    m_outputPath = absOutputPath;
    emit outputPathChanged(m_outputPath);
}

void BaseDataReader::setOutputFileName(QString fileName)
{
    m_outputFileName = fileName;
}

void BaseDataReader::start()
{
    if (init())
        readBuffer();
}

void BaseDataReader::stop()
{
    if (!m_inited) return; // нечего останавливать

    uninit();
    emit stopped();
}

void BaseDataReader::splitChannels(QByteArray &buffer)
{
    qreal sample;
    int bufSamplesCount = buffer.length()/m_bytesPerSample/m_channelsCount;

    // заполняем векторы нулями (на случай, если данных меньше, чем нужно)
    for (int i=0; i < m_channelsCount; i++) {
        m_inputChannelVector->at(i)->fill(0);
    }

    if (bufSamplesCount < m_samplesCount) {
        qWarning("Not enaught data in buffer! Will be appended by zeros.");
    }

    bufSamplesCount = qMin(bufSamplesCount, m_samplesCount);
    // масштабируем значение отсчета к диапазону [-1.0, 1.0] и пишем в отдельный вектор для каждого канала

    qint8 *ptr8 = reinterpret_cast<qint8*>(buffer.data());
    qint16 *ptr16 = reinterpret_cast<qint16*>(buffer.data());
    qint32 *ptr32 = reinterpret_cast<qint32*>(buffer.data());
    qint32 psm24 = 0;
    static const quint32 max32 = 0x80000000;
    switch(m_bytesPerSample) {
    case 1:
        for (int i=0; i < bufSamplesCount; i++) {
            for (int j=0; j < m_channelsCount; j++) {
                sample = pcmToReal(qint16(*ptr8));
                m_inputChannelVector->at(j)->replace(i, sample);
                ptr8++;
            }
        }
        break;
    case 2:
        for (int i=0; i < bufSamplesCount; i++) {
            for (int j=0; j < m_channelsCount; j++) {
                sample = pcmToReal(*ptr16);
                m_inputChannelVector->at(j)->replace(i, sample);
                ptr16++;
            }
        }
        break;
    case 3:
        for (int i=0; i < bufSamplesCount; i++) {
            for (int j=0; j < m_channelsCount; j++) {
                const quint8 sgn = *(ptr8+2) & 0x80;
                const quint8 hi  = *(ptr8+2) & 0x7f;
                const quint8 mid = *(ptr8+1);
                const quint8 lo  = *ptr8;
                psm24 = 0 | (sgn << 24) | (hi << 16) | (mid << 8) | lo;
                sample = qreal(psm24) / max32;
                m_inputChannelVector->at(j)->replace(i, sample);
                ptr8 += 3;
            }
        }
        break;
    case 4:
        for (int i=0; i < bufSamplesCount; i++) {
            for (int j=0; j < m_channelsCount; j++) {
                sample = qreal(*ptr32) / max32;
                m_inputChannelVector->at(j)->replace(i, sample);
                ptr32++;
            }
        }
        break;
    default:
        qWarning("Unsupported format!");
    }
}

void BaseDataReader::onFftFinished(int workerId)
{
    Q_UNUSED(workerId) // чтобы не ругался компилятор

    // ждем, пока закончат все потоки и читаем новый буфер
    m_finishedWorkers++;
    if (m_finishedWorkers == m_channelsCount) {
        m_finishedWorkers = 0;
        emit bufferProcessed(); // завершаем если данных больше нет
    }
}

void BaseDataReader::readBuffer()
{
    // Нужно переопределить в дочернем классе
    emit bufferRead();
}

void BaseDataReader::onBufferRead()
{
    for (int i=0; i < m_channelsCount; i++) {
        m_workers->at(i)->setBuffer(m_inputChannelVector->at(i));
    }
}

static const quint16 PCMS16MaxAmplitude =  32768; // because minimum int is -32768
qreal BaseDataReader::pcmToReal(const qint16 pcm)
{
    return qreal(pcm) / PCMS16MaxAmplitude;
}
