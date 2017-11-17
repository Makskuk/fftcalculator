#include "basedatareader.h"

BaseDataReader::BaseDataReader(QObject *parent) : QObject(parent),
    m_outputPath(QDir::currentPath()),
    m_inputChannelVector(new QVector<FftCalculator::DataVector*>()),
    m_workers(new QVector<Worker*>()),
    m_finishedWorkers(0),
    m_samplesCount(Worker::samplesCount()),
    m_channelsCount(0),
    m_lastBufferRead(false)
{
    /* В дочерних классах нужно определить параметры входного сигнала
     * m_sampleRate, m_bytesPerSample, m_channelsCount; а так же входное устройство m_dataInput.
     * Их нужно взять либо из заголовка WAV-файла, либо из параметров входного устройства.
     */
    connect(this, &BaseDataReader::buffrRead, this, &BaseDataReader::onBufferRead);
}

BaseDataReader::~BaseDataReader()
{
    for (int i=0; i < m_channelsCount; i++) {
        m_workers->at(i)->stop();
        delete m_inputChannelVector->at(i);
    }
    delete m_inputChannelVector;
    delete m_workers;
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
    if (!m_channelsCount) {
        emit error("Channels count not set!");
        return;
    }

    // Создаем дочерние процессы-воркеры - по одному на канал
    for (int i=0; i < m_channelsCount; i++) {
        m_inputChannelVector->append(new FftCalculator::DataVector(m_samplesCount));

        Worker *worker = new Worker(i);
        worker->setOutputDir(m_outputPath);
        m_workers->append(worker);
        connect(worker, &Worker::done, this, &BaseDataReader::onFftFinished);
    }

    readBuffer();
}

void BaseDataReader::stop()
{
    m_lastBufferRead = true;
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
        if (!m_lastBufferRead)
            readBuffer();
        else
            emit done(); // завершаем если данных больше нет
    }
}

void BaseDataReader::readBuffer()
{
    QByteArray rawBuffer;
    qint64 readLen = m_samplesCount * m_bytesPerSample * m_channelsCount; // сколько нужно прочитать
    qint64 readEnd = m_dataInput->size() - m_readPos; // сколько осталось непрочитано

    readLen = qMin(readLen, readEnd);

    rawBuffer.resize(readLen); // ресайзим входной буфер (на случай, если данных меньше, чем нужно)
    m_dataInput->seek(m_readPos); // отступаем на прочитанное
    m_dataInput->read(rawBuffer.data(), readLen);     // читаем буфер

    splitChannels(rawBuffer); // раскладываем данные поканально в массивы

    // если флаг еще не поставлен - проверяем, не закончились ли данные
    if (!m_lastBufferRead) {
        m_lastBufferRead = m_dataInput->atEnd();
    }

    emit buffrRead();
}

void BaseDataReader::onBufferRead()
{
    for (int i=0; i < m_channelsCount; i++) {
        m_workers->at(i)->setBuffer(m_inputChannelVector->at(i));
    }
}

const quint16 PCMS16MaxAmplitude =  32768; // because minimum int is -32768
qreal BaseDataReader::pcmToReal(qint16 pcm)
{
    return qreal(pcm) / PCMS16MaxAmplitude;
}
