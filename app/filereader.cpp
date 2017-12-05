#include "filereader.h"

#include <QDebug>
#include <QFileInfo>

FileReader::FileReader(QObject *parent) : BaseDataReader(parent)
{
    connect(this, &FileReader::bufferProcessed, this, &FileReader::readBuffer);
}

void FileReader::setInputFile(QString filename)
{
    m_inputFile = filename;
}

void FileReader::printFileInfo()
{
    qInfo() << "file name:" << m_file->fileName() << "\r\n"
            << "file size:" << m_file->size() << "\r\n"
            << "sample size:" << m_bytesPerSample << "\r\n"
            << "sample rate:" << m_sampleRate << "\r\n"
            << "sample type:" << m_file->fileFormat().sampleType() << "\r\n"
            << "bytes order:" << m_file->fileFormat().byteOrder() << "\r\n"
            << "channel count:" << m_channelsCount << "\r\n"
            << "header length:" << m_file->headerLength();
}

void FileReader::start()
{
    QFileInfo info(m_inputFile);
    if (!info.exists()) {
        emit BaseDataReader::error("Failed to open! File "+m_inputFile+" does not exist.");
        return;
    }

    m_file = new WavFile(this);

    m_file->open(m_inputFile);
    m_sampleRate = m_file->fileFormat().sampleRate();
    m_bytesPerSample = m_file->fileFormat().sampleSize() / 8;
    m_channelsCount = m_file->fileFormat().channelCount();

    m_readPos = m_file->headerLength(); // отступ на длину заголовка к секции данных

    printFileInfo();
    BaseDataReader::start();
}

void FileReader::readBuffer()
{
    if (m_file->atEnd()) {
        stop();
        return;
    }

    qint64 readLen = m_samplesCount * m_bytesPerSample * m_channelsCount; // сколько нужно прочитать
    qint64 readEnd = m_file->size() - m_readPos; // сколько осталось непрочитано
    QByteArray rawBuffer;

    readLen = qMin(readLen, readEnd);

    rawBuffer.resize(readLen); // ресайзим входной буфер (на случай, если данных меньше, чем нужно)
    m_file->seek(m_readPos); // отступаем на прочитанное
    m_file->read(rawBuffer.data(), readLen);     // читаем буфер

    splitChannels(rawBuffer); // раскладываем данные каналов по массивам для воркеров

    // запоминаем, где остановились
    m_readPos = m_file->pos();

    emit bufferRead();
}
