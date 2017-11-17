#include "basedatareader.h"

BaseDataReader::BaseDataReader(QObject *parent) : QObject(parent),
    m_outputPath(QDir::currentPath()),
    m_inputChannelVector(new QVector<FftCalculator::DataVector*>()),
    m_workers(new QVector<Worker*>()),
    m_finishedWorkers(0),
    m_samplesCount(Worker::samplesCount()),
    m_channelsCount(0),
    m_bytesPerSample(0),
    m_sampleRate(0),
    m_readPos(0),
    m_internalBufferLength(0)
{
    /* В дочерних классах нужно определить параметры входного сигнала
     * m_sampleRate, m_bytesPerSample, m_channelsCount.
     * Их нужно взять либо из заголовка WAV-файла, либо из параметров входного устройства.
     */
    connect(this, &BaseDataReader::bufferRead, this, &BaseDataReader::onBufferRead);

    // уничтожить процессы-воркеры по завершении обработки
//    connect(this, &BaseDataReader::bufferProcessed, [&]{
//        for (int i=0; i < m_channelsCount; i++) {
//            m_workers->at(i)->stop();
//            delete m_inputChannelVector->at(i);
//        }
//    });
}

BaseDataReader::~BaseDataReader()
{
    // остановить воркеры
    for (int i=0; i < m_channelsCount; i++) {
        m_workers->at(i)->stop();
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

    m_internalBufferLength = m_samplesCount * m_bytesPerSample * m_channelsCount;

    // Создаем дочерние процессы-воркеры - по одному на канал
    for (int i=0; i < m_channelsCount; i++) {
        m_inputChannelVector->append(new FftCalculator::DataVector(m_samplesCount));

        Worker *worker = new Worker(i);
        worker->setOutputDir(m_outputPath);
        m_workers->append(worker);
        connect(worker, &Worker::done, this, &BaseDataReader::onFftFinished);
    }

    return true;
}

void BaseDataReader::setOutputPath(QString absOutputPath)
{
    if (absOutputPath == m_outputPath)
        return;

    m_outputPath = absOutputPath;
    emit outputPathChanged(m_outputPath);
}

void BaseDataReader::start()
{
    if (init())
        readBuffer();
}

void BaseDataReader::stop()
{
    for (int i=0; i < m_channelsCount; i++) {
        m_workers->at(i)->stop();
        delete m_inputChannelVector->at(i);
    }
    emit stopped();
}

void BaseDataReader::splitChannels(QByteArray &buffer)
{
    qint16* buffer_ptr = reinterpret_cast<qint16*>(buffer.data());
    qreal sample;

    // заполняем векторы нулями (на случай, если данных меньше, чем нужно)
    for (int i=0; i < m_channelsCount; i++) {
        m_inputChannelVector->at(i)->fill(0);
    }

    for (int i=0; i < m_samplesCount; i++) {
        for (int j=0; j < m_channelsCount; j++) {
            sample = pcmToReal(*buffer_ptr); // масштабируем значение отсчета к диапазону [-1.0, 1.0]
            m_inputChannelVector->at(j)->replace(i, sample); // пишем данные в отдельный вектор для каждого канала
            buffer_ptr++;
        }
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
qreal BaseDataReader::pcmToReal(qint16 pcm)
{
    return qreal(pcm) / PCMS16MaxAmplitude;
}
