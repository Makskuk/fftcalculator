#include "filewriter.h"

FileWriter::FileWriter(QString filename, QObject *parent) : QObject(parent)
{
    m_file = new QFile(filename, this);
}

void FileWriter::writeVector(QVector<float> data)
{
    m_file->open(QIODevice::WriteOnly);
    m_file->flush();
    while (!data.isEmpty()) {
        m_file->write(QByteArray::number(data.takeFirst()).append("\r\n"));
    }
    m_file->close();
    emit done();
}
