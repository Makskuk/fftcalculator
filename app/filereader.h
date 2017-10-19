#ifndef FILEREADER_H
#define FILEREADER_H

#include <QObject>

#include "fftcalculator.h"
#include "wavfile.h"

/**
 * @brief The FileReader class
 *
 * Класс для чтения данных из файла. Построчно читает файл, прочитав
 * заданное количество строк, вызывает вычисление БПФ на прочитанных данных
 */
class FileReader : public QObject
{
    Q_OBJECT
public:
    explicit FileReader(QString filename, QObject *parent = 0);
    ~FileReader();

    FftCalculator::DataVector getFftResult() const;

signals:
    void bufferRead(bool lastBuffer);
    void done(bool success);

public slots:
    void printFileInfo();
    void readFile();

protected slots:
    void readBuffer();
    void onBufferRead(bool lastBuffer);
    void onFftFinished();
    qreal pcmToReal(qint16 pcm);

protected:
    WavFile *m_file;
    FftCalculator *m_fftCalculator;
    QByteArray *m_rawBuffer;
    QVector<FftCalculator::DataVector*> *m_inputChannelVector;

    int m_samplesCount;
    int m_channelsCount;
    int m_bytesPerSample;
    int m_sampleRate;
    int m_readPos;
};

#endif // FILEREADER_H
