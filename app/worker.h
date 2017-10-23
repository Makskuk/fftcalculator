#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QFile>
#include <QDir>

#include "fftcalculator.h"

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(int num, QObject *parent = 0);
    ~Worker();

    void stop();

    static int samplesCount() {return FftCalculator::fftWindowLength();}

signals:
    void done(int workerId);
    void outputDirChanged(QString path);
    void bufferChanged();

public slots:
    void setOutputDir(QString absDirPath);
    void setBuffer(FftCalculator::DataVector *buffer);

protected slots:
    void doFft();
    void writeResult(FftCalculator::DataVector data);

protected:
    QThread *m_thread;
    FftCalculator *m_fftCalculator;
    FftCalculator::DataVector *m_buffer;
    QFile *m_outputFile;
    QDir m_outputDir;
    QString m_outputFileName;
    int m_workerId;
    bool m_busy;
};

#endif // WORKER_H
