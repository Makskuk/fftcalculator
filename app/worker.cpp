#include "worker.h"
#include <qmath.h>

Worker::Worker(int num, QObject *parent) : QObject(parent),
    m_thread(new QThread()),
    m_fftCalculator(new FftCalculator(this)),
    m_outputFile_real(new QFile()),
    m_outputFile_imagine(new QFile()),
    m_outputDir(QDir::current()),
    m_outputFileName("fft_out"),
    m_workerId(num),
    m_avgBuffersCounter(0), // в именах файлов нумерация будет с 1
    m_busy(false)
{
    connect(m_thread, &QThread::finished, this, &QObject::deleteLater);

    connect(m_fftCalculator, &FftCalculator::fftReady, this, &Worker::writeResult);
    connect(this, &Worker::bufferChanged, this, &Worker::doFft);

    // moveToThread() cannot be called on a QObject with a parent
    setParent(0);
    moveToThread(m_thread);
    m_thread->start();
}

Worker::~Worker()
{
    if (m_outputFile_real->isOpen())
        m_outputFile_real->close();
    if (m_outputFile_imagine->isOpen())
        m_outputFile_imagine->close();
    delete m_outputFile_real;
    delete m_outputFile_imagine;
}

void Worker::stop()
{
    if (m_busy) {
        qWarning("Work in progress, can't stop worker %d", m_workerId);
        return;
    }

    m_outputFile_real->close();
    m_outputFile_imagine->close();

    m_thread->quit();
    m_thread->wait();
}

void Worker::setOutputDir(QString absDirPath)
{
    m_outputDir.cd(absDirPath);
    emit outputDirChanged(absDirPath);
}

void Worker::setOutputName(QString name)
{
    m_outputFileName = name;
}

void Worker::setBuffer(FftCalculator::DataVector *buffer)
{
    if (m_busy) {
        return;
    }

    if (m_buffer == buffer) {
        return;
    }

    m_buffer = buffer;
    emit bufferChanged();
}

void Worker::doFft()
{
    m_busy = true;

    // формула ДПФ из документации:
    // f(k) = sum (p = 0, N-1, x(p) * exp (+j*2*pi*k*p/N))
    m_fftCalculator->doFFT(m_buffer);
}

void Worker::calcAvgBuffer()
{
    QString fileName = m_outputDir.absolutePath() + "/"
                        + m_outputFileName
                        + QString::number(m_workerId+1)             // в именах файлов нумерация будет с 1
                        + QString::number(m_avgBuffersCounter+1)    // в именах файлов нумерация будет с 1
                        + ".txt";
    QFile outputFile(fileName);
    QVector<qreal> avgBuffer;
    int count = FftCalculator::fftWindowLength()/2;

    avgBuffer.resize(count);

    // Вычисляем усреднённый буфер
    for (int i=0; i < count; i++) {
        foreach (QVector<qreal> buffer, m_bufAccumulator) {
            avgBuffer[i] += buffer[i];
        }
        avgBuffer[i] /= BUFFERS_COUNT;
    }

    // формируем выходные строки и пишем их в файл
    QByteArray output;
    foreach (qreal value, avgBuffer) {
        output.append(QByteArray::number(value).append("\r\n"));
    }
    outputFile.open(QIODevice::WriteOnly | QIODevice::Text);
    outputFile.write(output);
    outputFile.close();

    m_avgBuffersCounter++;
    m_bufAccumulator.clear();
}

void Worker::writeResult(FftCalculator::DataVector data)
{
    qreal real, imag;
    int numSamples = FftCalculator::fftWindowLength()/2;
    QVector<qreal> result;

    // Получаем из результатов БПФ значения амплитуд
    for (int i=0; i<numSamples; ++i) {
        real = data[i];
        if (i == 0)
            imag = 0.0;
        else
            imag = data[numSamples + i];

        const qreal magnitude = qSqrt(real*real + imag*imag);
        result.append(magnitude);
    }

    m_bufAccumulator.append(result);
    if (m_bufAccumulator.size() == BUFFERS_COUNT) {
        calcAvgBuffer();
    }

    m_buffer = 0;
    m_busy = false;

    emit done(m_workerId);
}
