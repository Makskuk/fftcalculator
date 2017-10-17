#include "fftcalculator.h"

FftCalculator::FftCalculator(QObject *parent) : QObject(parent),
    m_fft(new FFTRealWrapper),
    m_lengthToAvg(10),
    m_calculatedWindowsCount(0)
{
    connect(this, &FftCalculator::calculatedWindow, this, &FftCalculator::onWindowCalculated);
    connect(this, &FftCalculator::calculatedLastWindow, this, &FftCalculator::calcAvgSpectrumCounts);
}

void FftCalculator::calculateOneWindow(FftCalculator::DataVector inputVector, bool lastWindow)
{
    FftCalculator::DataVector outputVector;

    //рассчет БПФ
    m_fft->calculateFFT(inputVector.data(), outputVector.data());

    //накопление результатов в аккумуляторе
    m_accumulator.push_back(outputVector);
    m_calculatedWindowsCount++;

    //сигнал о готовности следующего окна
    if (!lastWindow)
        emit calculatedWindow(m_calculatedWindowsCount);
    else
        emit calculatedLastWindow();
}

void FftCalculator::calcAvgSpectrumCounts()
{
    FftCalculator::DataVector outputVector;
    int i, j, count;

    // берем первый массив данных (и удаляем его из аккумулятора)
    outputVector = m_accumulator.takeFirst();

    for(i=0; i<m_lengthToAvg-1; i++) {
        // суммируем элементы выходного массива с элементами первого массива в аккумуляторе
        if(!m_accumulator.isEmpty()) {
            for(j=0; j<m_fft->windowLength(); j++) {
                outputVector[j] += m_accumulator.first()[j];
            }
            // удаляем прибавленный массив из аккумулятора
            m_accumulator.removeFirst();
        } else {
            break;
        }
    }

    count = i+1; // количество слагаемых (векторов, извлеченных из аккумулятора)
                // их может быть меньше, чем m_lengthToAvg при обработке последнего окна

    for(j=0; j<m_fft->windowLength(); j++) {
        outputVector[j] /= count;
    }

    // дописывам результаты в выходной усредненный вектор
    m_avgSpectrumCounts += outputVector;
}

void FftCalculator::onWindowCalculated(int count)
{
    if (count == m_lengthToAvg) {
        m_calculatedWindowsCount = 0;
        calcAvgSpectrumCounts();
    }
}
