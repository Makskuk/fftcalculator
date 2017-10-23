#include "filereader.h"

#include <QDebug>

FileReader::FileReader(QString filename, QObject *parent) : QObject(parent),
    m_lastBufferRead(false),
    m_finishedWorkers(0)
{
    m_file = new WavFile(this);
    m_workers = new QList<Worker*>();
    m_outputPath = QDir::currentPath();

    m_samplesCount = Worker::samplesCount();

    m_file->open(filename);
    m_sampleRate = m_file->fileFormat().sampleRate();
    m_bytesPerSample = m_file->fileFormat().sampleSize() / 8;
    m_channelsCount = m_file->fileFormat().channelCount();

    m_rawBuffer = new QByteArray();
    m_inputChannelVector = new QVector<FftCalculator::DataVector*>;

    m_readPos = m_file->headerLength(); // отступ на длину заголовка к секции данных

    connect(this, &FileReader::bufferRead, this, &FileReader::onBufferRead);
    connect(this, &FileReader::done, []{
        qInfo() << "Done!";
    });
}

FileReader::~FileReader()
{
    delete m_rawBuffer;
    for (int i=0; i < m_channelsCount; i++) {
        m_workers->at(i)->stop();
        delete m_inputChannelVector->at(i);
    }
    delete m_inputChannelVector;
    delete m_workers;
}

void FileReader::printFileInfo()
{
    qInfo() << "file name:" << m_file->fileName() << "\r\n"
            << "file size:" << m_file->size() << "\r\n"
            << "sample size:" << m_bytesPerSample << "\r\n"
            << "sample rate:" << m_sampleRate << "\r\n"
            << "sample type:" << m_file->fileFormat().sampleType() << "\r\n"
            << "channel count:" << m_channelsCount << "\r\n"
            << "header length:" << m_file->headerLength();
}

void FileReader::readFile()
{

    for (int i=0; i < m_channelsCount; i++) {
        m_inputChannelVector->append(new FftCalculator::DataVector(m_samplesCount));

        Worker *worker = new Worker(i);
        worker->setOutputDir(m_outputPath);
        m_workers->append(worker);
        connect(worker, &Worker::done, this, &FileReader::onFftFinished);
    }

    printFileInfo();
    readBuffer();
}

void FileReader::setOutputPath(QString absOutputPath)
{
    if (absOutputPath == m_outputPath)
        return;

    m_outputPath = absOutputPath;
    emit outputPathChanged(m_outputPath);
}

void FileReader::readBuffer()
{
    qint64 readLen = m_samplesCount * m_bytesPerSample * m_channelsCount; // сколько нужно прочитать
    qint64 readEnd = m_file->size() - m_readPos; // сколько осталось непрочитано
    qint16 *buffer_ptr;
    qint16 i, j;
    qreal sample;

    readLen = qMin(readLen, readEnd);

    // заполняем векторы нулями (на случай, если данных меньше, чем нужно)
    for (j=0; j < m_channelsCount; j++) {
        m_inputChannelVector->at(j)->fill(0);
    }

    m_rawBuffer->resize(readLen); // ресайзим входной буфер (на случай, если данных меньше, чем нужно)
    m_file->seek(m_readPos); // отступаем на прочитанное
    m_file->read(m_rawBuffer->data(), readLen);     // читаем буфер
    buffer_ptr = reinterpret_cast<qint16*>(m_rawBuffer->data());
    for (i=0; i < m_samplesCount; i++) {
        for (j=0; j < m_channelsCount; j++) {
            sample = pcmToReal(*buffer_ptr); // масштабируем значение отсчета к диапазону [-1.0, 1.0]
            m_inputChannelVector->at(j)->replace(i, sample); // пишем данные в отдельный вектор для каждого канала
            buffer_ptr++;
        }
    }

    // запоминаем, где остановились
    m_readPos = m_file->pos();

    emit bufferRead(m_file->atEnd());
}

void FileReader::onBufferRead(bool lastBuffer)
{
    m_lastBufferRead = lastBuffer;
    for (int i=0; i < m_channelsCount; i++) {
        m_workers->at(i)->setBuffer(m_inputChannelVector->at(i));
    }
}

void FileReader::onFftFinished(int workerId)
{
    Q_UNUSED(workerId) // чтобы компилятор не ругался

    // ждем, пока закончат все потоки и читаем новый буфер
    m_finishedWorkers++;
    if (m_finishedWorkers == m_channelsCount) {
        m_finishedWorkers = 0;
        if (!m_lastBufferRead)
            readBuffer();
        else
            emit done(true);
    }
}

const quint16 PCMS16MaxAmplitude =  32768; // because minimum is -32768

qreal FileReader::pcmToReal(qint16 pcm)
{
    return qreal(pcm) / PCMS16MaxAmplitude;
}
