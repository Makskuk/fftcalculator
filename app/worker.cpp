#include "worker.h"
#include <qmath.h>

Worker::Worker(int num, QObject *parent) : QObject(parent),
    m_thread(new QThread()),
    m_fftCalculator(new FftCalculator(this)),
    m_outputFile_real(new QFile()),
    m_outputFile_imagine(new QFile()),
    m_outputFile_amp(new QFile()),
    m_outputDir(QDir::current()),
    m_outputFileName("fft_out"),
    m_workerId(num),
    m_avgBuffersCounter(0), // в именах файлов нумерация будет с 1
    m_busy(false)
{
    connect(m_thread, &QThread::finished, this, &QObject::deleteLater);

    connect(m_fftCalculator, &FftCalculator::fftReady, this, &Worker::writeResult);
    connect(this, &Worker::bufferChanged, this, &Worker::doFft);
    connect(this, &Worker::bufAccumulatorFull, this, &Worker::calcAvgBuffer);
    connect(this, &Worker::done, [&]{
        m_buffer = 0;
        m_busy = false;
    });

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
    if (m_outputFile_amp->isOpen())
        m_outputFile_amp->close();
    delete m_outputFile_real;
    delete m_outputFile_imagine;
    delete m_outputFile_amp;
}

void Worker::stop()
{
    if (m_busy) {
        qWarning("Work in progress, can't stop worker %d", m_workerId);
        return;
    }

    m_outputFile_real->close();
    m_outputFile_imagine->close();
    m_outputFile_amp->close();

    m_thread->quit();
//    m_thread->wait();
}

void Worker::setOutputDir(QString absDirPath)
{
    m_outputDir.setPath(absDirPath);
    if (!m_outputDir.exists()) {
        m_outputDir.mkpath(absDirPath);
    }
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


/**
 * @brief Worker::calcAvgBuffer
 *
 * Вычисление и запись в файл усредненных значений амплитуд.
 */
void Worker::calcAvgBuffer()
{
    QString fileName = m_outputDir.absolutePath() + "/"
                        + m_outputFileName
                        + QString::number(m_workerId+1)             // в именах файлов нумерация будет с 1
                        + QString::number(m_avgBuffersCounter+1)    // в именах файлов нумерация будет с 1
                        + ".txt";
    QFile outputFile(fileName);
    int count = FftCalculator::fftWindowLength()/2;
    QVector<qreal> avgBuffer(count, 0);

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
        output.append(QByteArray::number(value, 'f', 8).append("\r\n"));
    }
    outputFile.open(QIODevice::WriteOnly | QIODevice::Text);
    outputFile.write(output);
    outputFile.close();

    m_avgBuffersCounter++;
    m_bufAccumulator.clear();

    emit done(m_workerId);
}

/**
 * @brief Worker::writeResult
 * @param data - результат работы БПФ - массив комплексных чисел
 *
 * Открываем файлы, если они еще не открыты. Разбираем входные данные,
 * вычисляем амплитуду, записываем результаты. Если накопилось достаточное
 * количество буферов - вычисляем и записываем "средний буфер".
 */
void Worker::writeResult(FftCalculator::DataVector data)
{
    qreal real, imag;
    int numSamples = FftCalculator::fftWindowLength()/2;
    QVector<qreal> result;

    QString filename_real(m_outputFileName), filename_imagine(m_outputFileName),
            filename_amp(m_outputFileName);
    if (!m_outputFile_real->isOpen()) {
        filename_real.prepend(m_outputDir.absolutePath() + "/")
                .append("_"+QString::number(m_workerId)+"_real.txt");
        m_outputFile_real->setFileName(filename_real);
        m_outputFile_real->open(QIODevice::WriteOnly | QIODevice::Text);
    }
    if (!m_outputFile_imagine->isOpen()) {
        filename_imagine.prepend(m_outputDir.absolutePath() + "/")
                .append("_"+QString::number(m_workerId)+"_imagine.txt");
        m_outputFile_imagine->setFileName(filename_imagine);
        m_outputFile_imagine->open(QIODevice::WriteOnly | QIODevice::Text);
    }
    if (!m_outputFile_amp->isOpen()) {
        filename_amp.prepend(m_outputDir.absolutePath() + "/")
                .append("_"+QString::number(m_workerId)+"_amp.txt");
        m_outputFile_amp->setFileName(filename_amp);
        m_outputFile_amp->open(QIODevice::WriteOnly | QIODevice::Text);
    }

    QByteArray output_r, output_i, output_amp;
    FftCalculator::DataVector::iterator iterator_r = data.begin();
    FftCalculator::DataVector::iterator iterator_i = data.begin();

    real = *iterator_r;
    output_r.append(QByteArray::number(real, 'f', 8).append("\r\n"));
    output_i.append("0\r\n"); // первая мнимая часть - 0
    result.append(qAbs(real)); // первая мнимая часть - ноль, sqrt(x*x + 0*0) = |x|
    iterator_r++;
    iterator_i += numSamples+1; // мнимые части начинаются с length/2 + 1
    int i=1;
    while (i < numSamples) {
        real = *iterator_r;
        imag = *iterator_i;
        // Получаем из результатов БПФ значения амплитуд
//        const qreal magnitude = qSqrt(real*real + imag*imag);

        const qreal magnitude = real*real + imag*imag; // из последних требований - нужна только сумма квадратов
        result.append(magnitude);
        output_amp.append(QByteArray::number(magnitude, 'f', 8).append("\r\n"));
        output_r.append(QByteArray::number(real, 'f', 8).append("\r\n"));
        output_i.append(QByteArray::number(imag, 'f', 8).append("\r\n"));
        iterator_i++;
        iterator_r++;
        i++;
    }

    m_outputFile_real->write(output_r);
    m_outputFile_imagine->write(output_i);
    m_outputFile_amp->write(output_amp);

    m_bufAccumulator.append(result);
    if (m_bufAccumulator.size() == BUFFERS_COUNT) {
        emit bufAccumulatorFull();
    } else {
        emit done(m_workerId);
    }
}
