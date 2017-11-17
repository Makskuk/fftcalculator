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

    int channelsCount()  const { return m_channelsCount; }
    int samplesCount()   const { return m_samplesCount; }
    int sampleRate()     const { return m_sampleRate; }
    int bytesPerSample() const { return m_bytesPerSample; }


signals:
    void buffrRead();
    void done();
    void outputPathChanged(QString path);
    void error(QString msg);

public slots:
    void setOutputPath(QString absOutputPath);
    virtual void start();
    virtual void stop();

protected slots:
    void splitChannels(QByteArray &buffer);
    void onFftFinished(int workerId);
    virtual void readBuffer();
    virtual void onBufferRead();
    qreal pcmToReal(qint16 pcm);

protected:
    QString     m_outputPath;
    QIODevice*  m_dataInput;
    QVector<FftCalculator::DataVector*> *m_inputChannelVector;
    QVector<Worker*> *m_workers;

    int m_finishedWorkers;

    int m_samplesCount;
    int m_channelsCount;
    int m_bytesPerSample;
    int m_sampleRate;
    int m_readPos;
    bool m_lastBufferRead;
};

#endif // BASEDATAREADER_H
