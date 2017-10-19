#include "filereader.h"

#include <QDebug>

FileReader::FileReader(QString filename, QObject *parent) : QObject(parent)
{
    m_file = new WavFile(this);
    m_fftCalculator = new FftCalculator(this);

    m_samplesCount = m_fftCalculator->fftWindowLength();

    m_file->open(filename);
    m_sampleRate = m_file->fileFormat().sampleRate();
    m_bytesPerSample = m_file->fileFormat().sampleSize() / 8;
    m_channelsCount = m_file->fileFormat().channelCount();

    m_rawBuffer = new QByteArray();
    m_inputChannelVector = new QVector<FftCalculator::DataVector*>;
    for (int i=0; i < m_channelsCount; i++) {
        m_inputChannelVector->append(new FftCalculator::DataVector(m_samplesCount));
    }
    m_readPos = m_file->headerLength(); // отступ на длину заголовка к секции данных

    connect(this, &FileReader::bufferRead, this, &FileReader::onBufferRead);
    connect(m_fftCalculator, &FftCalculator::noMoreData, this, &FileReader::onFftFinished);
    connect(m_fftCalculator, &FftCalculator::calculatedWindow, this, &FileReader::readBuffer);
}

FileReader::~FileReader()
{
    delete m_rawBuffer;
    for (int i=0; i < m_channelsCount; i++) {
        delete m_inputChannelVector->at(i);
    }
    delete m_inputChannelVector;
}

FftCalculator::DataVector FileReader::getFftResult() const
{
    return m_fftCalculator->getAvgSpectrumCounts();
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
    printFileInfo();
    readBuffer();
}

void FileReader::readBuffer()
{
    qint64 readLen = m_samplesCount * m_bytesPerSample * m_channelsCount; // сколько нужно прочитать
    qint64 readEnd = m_file->size() - m_readPos; // сколько осталось непрочитано
    qint16 *buffer_ptr;
    qint16 i, j;
    qreal sample;

    readLen = qMin(readLen, readEnd);

//    qDebug() << "readLen =" << readLen
//             << "readEnd =" << readEnd
//             << "readPos =" << m_readPos;

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
    m_fftCalculator->calculateOneWindow(m_inputChannelVector->at(0), lastBuffer);
}

void FileReader::onFftFinished()
{
    qDebug()<<"Close input file";
    m_file->close();
    emit done(true);
}

const quint16 PCMS16MaxAmplitude =  32768; // because minimum is -32768

qreal FileReader::pcmToReal(qint16 pcm)
{
    return qreal(pcm) / PCMS16MaxAmplitude;
}
