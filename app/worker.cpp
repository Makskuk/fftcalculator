#include "worker.h"

Worker::Worker(int num, QObject *parent) : QObject(parent),
    m_thread(new QThread()),
    m_fftCalculator(new FftCalculator(this)),
    m_outputFile_real(new QFile()),
    m_outputFile_imagine(new QFile()),
    m_outputDir(QDir::current()),
    m_outputFileName("fft_out"),
    m_workerId(num),
    m_busy(false)
{
    m_outputFileName.prepend(QString::number(m_workerId) + "_");

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

void Worker::writeResult(FftCalculator::DataVector data)
{
    QString filename_real(m_outputFileName), filename_imagine(m_outputFileName);
    if (!m_outputFile_real->isOpen()) {
        filename_real.prepend(m_outputDir.absolutePath() + "/").append("_real.txt");
        m_outputFile_real->setFileName(filename_real);
        qDebug("Open file %s for channel %d", qPrintable(filename_real), m_workerId);
        m_outputFile_real->open(QIODevice::WriteOnly | QIODevice::Text);
    }
    if (!m_outputFile_imagine->isOpen()) {
        filename_imagine.prepend(m_outputDir.absolutePath() + "/").append("_imagine.txt");
        m_outputFile_imagine->setFileName(filename_imagine);
        qDebug("Open file %s for channel %d", qPrintable(filename_imagine), m_workerId);
        m_outputFile_imagine->open(QIODevice::WriteOnly | QIODevice::Text);
    }

    QByteArray output_r, output_i;
    FftCalculator::DataVector::iterator iterator_r = data.begin();
    FftCalculator::DataVector::reverse_iterator iterator_i = data.rbegin();
    int i=0, count=FftCalculator::fftWindowLength()/2;

    while (i < count) {
        output_r.append(QByteArray::number(*iterator_r).append("\r\n"));
        output_i.prepend(QByteArray::number(*iterator_i).append("\r\n"));
        iterator_i++;
        iterator_r++;
        i++;
    }

    m_outputFile_real->write(output_r);
    m_outputFile_imagine->write(output_i);

    m_buffer = 0;
    m_busy = false;

    emit done(m_workerId);
}
