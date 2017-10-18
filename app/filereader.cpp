#include "filereader.h"
#include <QDataStream>

#include <QDebug>

FileReader::FileReader(QString filename, QObject *parent) : QObject(parent)
{
    m_file = new QFile(filename, this);
    m_fftCalculator = new FftCalculator(this);

    m_samplesCount = m_fftCalculator->fftWindowLength();

    connect(this, &FileReader::fftWindowRead, m_fftCalculator, &FftCalculator::calculateOneWindow);
    connect(m_fftCalculator, &FftCalculator::noMoreData, this, &FileReader::onFftFinished);
    connect(m_fftCalculator, &FftCalculator::calculatedWindow, this, &FileReader::readDataSet);
}

FftCalculator::DataVector FileReader::getFftResult() const
{
    return m_fftCalculator->getAvgSpectrumCounts();
}

void FileReader::readFile()
{
    m_file->open(QIODevice::ReadOnly);

    if (!m_file->isOpen()) {
        qWarning()<<"Failed to open file" << m_file->fileName();
        emit done(false);
        return;
    }

    qDebug() << "File"<<m_file->fileName()<<"opened, read data:"
             << m_samplesCount << "lines for one fft window";

    readDataSet();
}

void FileReader::readDataSet()
{
    FftCalculator::DataVector inputVector;
    QByteArray buffer;

    for (int i=0; i < m_samplesCount; i++) {
        if (!m_file->atEnd()) {
            buffer = m_file->readLine().trimmed();
            inputVector.push_back(buffer.toFloat());
        } else {
            // дополним вектор нулями до длины m_samplesCount
            inputVector.push_back(0.0);
        }
    }

    emit fftWindowRead(inputVector, m_file->atEnd());
}

void FileReader::onFftFinished()
{
    qDebug()<<"Close file";
    m_file->close();
    emit done(true);
}
