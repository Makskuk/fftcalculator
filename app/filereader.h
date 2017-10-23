#ifndef FILEREADER_H
#define FILEREADER_H

#include <QObject>

#include "wavfile.h"
#include "worker.h"

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

signals:
    void bufferRead(bool lastBuffer);
    void done(bool success);
    void outputPathChanged(QString path);

public slots:
    void setOutputPath(QString absOutputPath);
    void printFileInfo();
    void readFile();

protected slots:
    void readBuffer();
    void onBufferRead(bool lastBuffer);
    void onFftFinished(int workerId);
    qreal pcmToReal(qint16 pcm);

protected:
    WavFile    *m_file;
    QByteArray *m_rawBuffer;
    QVector<FftCalculator::DataVector*> *m_inputChannelVector;
    QList<Worker*> *m_workers;
    QString    m_outputPath;

    int m_samplesCount;
    int m_channelsCount;
    int m_bytesPerSample;
    int m_sampleRate;
    int m_readPos;
};

#endif // FILEREADER_H
