#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QThread>
#include <QFile>
#include <QDir>

#include "fftcalculator.h"

#define BUFFERS_COUNT   10

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
    void setOutputName(QString name);
    void setBuffer(FftCalculator::DataVector *buffer);

protected slots:
    void doFft();
    void calcAvgBuffer();
    void writeResult(FftCalculator::DataVector data);

protected:
    QThread *m_thread;
    FftCalculator *m_fftCalculator;
    FftCalculator::DataVector *m_buffer;
    QFile *m_outputFile_real;
    QFile *m_outputFile_imagine;
    QDir m_outputDir;
    QString m_outputFileName;
    QVector<QVector<qreal>> m_bufAccumulator;

    // результат по каждому буферу будет писаться в свой файл.
    // Файлы именуются по шаблону: "b" + номер канала + номер усреднённого буфера
    // номер канала - m_workerId + 1
    // номер усреднённого буфера - m_avgBuffersCounter
    // усреднённый буфер - среднее арифметическое блока из BUFFERS_COUNT буферов
    int m_workerId;
    int m_avgBuffersCounter;

    bool m_busy;
};

#endif // WORKER_H
