#include "filereader.h"

#include <QDebug>

FileReader::FileReader(QString filename, QObject *parent) : BaseDataReader(parent)
{
    m_file = new WavFile(this);

    m_file->open(filename);
    m_sampleRate = m_file->fileFormat().sampleRate();
    m_bytesPerSample = m_file->fileFormat().sampleSize() / 8;
    m_channelsCount = m_file->fileFormat().channelCount();

    m_readPos = m_file->headerLength(); // отступ на длину заголовка к секции данных

    connect(this, &FileReader::done, []{
        qInfo() << "Done!";
    });
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
    printFileInfo();
    BaseDataReader::start();
}

void FileReader::readBuffer()
{
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

    // если флаг еще не поставлен - проверяем, не закончились ли данные
    if (!m_lastBufferRead) {
        m_lastBufferRead = m_file->atEnd();
    }

    emit bufferRead();
}
