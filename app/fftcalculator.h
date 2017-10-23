#ifndef FFTCALCULATOR_H
#define FFTCALCULATOR_H

#include <QObject>
#include <QVector>

#include "fftreal_wrapper.h"

class FftCalculator : public QObject
{
    Q_OBJECT
public:
    explicit FftCalculator(QObject *parent = 0);
    ~FftCalculator();

    typedef QVector<FFTRealWrapper::DataType>   DataVector;

    //длина окна для БПФ
    static int fftWindowLength() { return FFTRealWrapper::windowLength(); }

    //результаты вычислений - усредненные спектральные отсчеты
    //содержит набор комплексных чисел:
    //   первая половина вектора - действительные части (0...length/2)
    //   вторая половина вектора - мнимые части (length/2+1...length-1)
    DataVector getRawResults() const {return m_rawFFTResults; }

signals:
    void fftReady(DataVector *result); // по готовности преобразования

public slots:
    /**
     * @brief calculateOneWindow
     * @param inputVector
     * Вычисляет БПФ для одной порции данных - длины окна БПФ
     */
    void doFFT(DataVector *inputVector);

protected:
    FFTRealWrapper *m_fft;
    DataVector m_rawFFTResults;
};

#endif // FFTCALCULATOR_H
