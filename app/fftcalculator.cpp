#include "fftcalculator.h"

FftCalculator::FftCalculator(QObject *parent) : QObject(parent),
    m_fft(new FFTRealWrapper),
    m_lengthToAvg(10),
    m_calculatedWindowsCount(0),
    m_lastWindow(false)
{
    connect(this, &FftCalculator::calculatedWindow, this, &FftCalculator::onWindowCalculated);
    connect(this, &FftCalculator::calculatedLastWindow, this, &FftCalculator::calcAvgSpectrumCounts);
}

FftCalculator::~FftCalculator()
{
    delete m_fft;
}

void FftCalculator::calculateOneWindow(FftCalculator::DataVector *inputVector, bool lastWindow)
{
    FftCalculator::DataVector outputVector(1024);

    //рассчет БПФ
    m_fft->calculateFFT(inputVector->data(), outputVector.data());

    //накопление результатов в аккумуляторе
    m_accumulator.push_back(outputVector);
    m_calculatedWindowsCount++;

    //сигнал о готовности следующего окна
    if (!lastWindow) {
        emit calculatedWindow(m_calculatedWindowsCount);
    } else {
        m_lastWindow = true;
        emit calculatedLastWindow();
    }
}

void FftCalculator::calcAvgSpectrumCounts()
{
//    FftCalculator::DataVector outputVector;
//    FftCalculator::DataVector inputVector;
//    int i, j, count;

//    // берем первый массив данных (и удаляем его из аккумулятора)
//    outputVector = m_accumulator.takeFirst();

//    for(i=0; i<m_lengthToAvg-1; i++) {
//        // суммируем элементы выходного массива с элементами первого массива в аккумуляторе
//        if(!m_accumulator.isEmpty()) {
//            inputVector = m_accumulator.takeFirst();
//            for(j=0; j<m_fft->windowLength(); j++) {
//                outputVector[j] += inputVector[j];
//            }
//        } else {
//            break;
//        }
//    }

//    count = i+1; // количество слагаемых (векторов, извлеченных из аккумулятора)
//                // их может быть меньше, чем m_lengthToAvg при обработке последнего окна

//    for(j=0; j<m_fft->windowLength(); j++) {
//        outputVector[j] /= count;
//    }

//    // дописывам результаты в выходной усредненный вектор
//    m_avgSpectrumCounts += outputVector;

    // просто записываем выход БПФ
    for(int i=0; i<m_lengthToAvg; i++) {
        if(!m_accumulator.isEmpty()) {
            m_avgSpectrumCounts += m_accumulator.takeFirst();
        } else {
            break;
        }
    }

    if (m_lastWindow)
        emit noMoreData();
}

void FftCalculator::onWindowCalculated(int count)
{
    if (count == m_lengthToAvg) {
        m_calculatedWindowsCount = 0;
        calcAvgSpectrumCounts();
    }
}
