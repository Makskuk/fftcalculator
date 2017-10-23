#include "fftcalculator.h"

FftCalculator::FftCalculator(QObject *parent) : QObject(parent),
    m_fft(new FFTRealWrapper)
{
    m_rawFFTResults.resize(m_fft->windowLength());
}

FftCalculator::~FftCalculator()
{
    delete m_fft;
}

void FftCalculator::doFFT(FftCalculator::DataVector *inputVector)
{
    //рассчет БПФ
    m_fft->calculateFFT(inputVector->data(), m_rawFFTResults.data());

    //сигнал о готовности
    emit fftReady(m_rawFFTResults);
}

