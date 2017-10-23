#include "worker.h"

Worker::Worker(int num, QObject *parent) : QObject(parent),
    m_thread(new QThread()),
    m_fftCalculator(new FftCalculator(this)),
    m_outputFile(new QFile()),
    m_outputDir(QDir::current()),
    m_outputFileName("fft_out.txt"),
    m_workerId(num),
    m_busy(false)
{
    // moveToThread() cannot be called on a QObject with a parent
    setParent(0);
    moveToThread(m_thread);

    m_outputFileName.prepend(QString::number(m_workerId) + "_");

    connect(m_thread, &QThread::finished, this, &QObject::deleteLater);

    connect(m_fftCalculator, &FftCalculator::fftReady, this, &Worker::writeResult);
    connect(this, &Worker::bufferChanged, this, &Worker::doFft);

    m_thread->start();
}

Worker::~Worker()
{
    if (m_outputFile->isOpen())
        m_outputFile->close();
    delete m_outputFile;
}

void Worker::stop()
{
    if (m_busy) {
        qWarning("Work in progress, can't stop worker %d", m_workerId);
        return;
    }

    m_outputFile->close();

    m_thread->quit();
    m_thread->wait();
}

void Worker::setOutputDir(QString absDirPath)
{
    m_outputDir.cd(absDirPath);
    emit outputDirChanged(absDirPath);
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

    m_fftCalculator->doFFT(m_buffer);
}

void Worker::writeResult(FftCalculator::DataVector *data)
{
    if (!m_outputFile->isOpen()) {
        qDebug("Worker %d set path to file %s", m_workerId, qPrintable(m_outputFileName));
        m_outputFileName.prepend(m_outputDir.absolutePath() + "/");
        m_outputFile->setFileName(m_outputFileName);
        qDebug("Worker %d open file %s", m_workerId, qPrintable(m_outputFile->fileName()));
        m_outputFile->open(QIODevice::WriteOnly);
    }

    while (!data->isEmpty()) {
        m_outputFile->write(QByteArray::number(data->takeFirst()).append("\r\n"));
    }

    m_busy = false;

    emit done(m_workerId);
}
