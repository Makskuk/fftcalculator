#ifndef BASEDATAREADER_H
#define BASEDATAREADER_H

#include <QObject>
#include "worker.h"

class BaseDataReader : public QObject
{
    Q_OBJECT
public:
    explicit BaseDataReader(QObject *parent = 0);
    ~BaseDataReader();

    bool init();
    void uninit();

    int channelsCount()  const { return m_channelsCount; }
    int samplesCount()   const { return m_samplesCount; }
    int sampleRate()     const { return m_sampleRate; }
    int bytesPerSample() const { return m_bytesPerSample; }

signals:
    void bufferRead();
    void bufferProcessed();
    void stopped();
    void outputPathChanged(QString path);
    void error(QString msg);

public slots:
    void setOutputPath(QString absOutputPath);
    void setOutputFileName(QString fileName);
    virtual void start();
    virtual void stop();

protected slots:
    qreal pcmToReal(const qint16 pcm);
    void splitChannels(QByteArray &buffer);
    void onFftFinished(int workerId);
    void onBufferRead();
    virtual void readBuffer();

protected:
    QString     m_outputPath;
    QString     m_outputFileName;
    QVector<FftCalculator::DataVector*> *m_inputChannelVector;
    QVector<Worker*> *m_workers;

    int m_finishedWorkers;

    int m_samplesCount;
    int m_channelsCount;
    int m_bytesPerSample;
    int m_sampleRate;
    int m_readPos;
    int m_internalBufferLength;

    bool m_inited;
};

#endif // BASEDATAREADER_H
