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
    int fftWindowLength() const { return m_fft->windowLength(); }

    // количество окон для усреднения выходных значений
    int lengthToAvg() const { return m_lengthToAvg; }

    //результаты вычислений - усредненные спектральные отсчеты
    //содержит набор комплексных чисел:
    //   первая половина вектора - действительные части (0...length/2)
    //   вторая половина вектора - мнимые части (length/2+1...length-1)
    DataVector getAvgSpectrumCounts() const {return m_avgSpectrumCounts; }

signals:
    void calculatedWindow(int count); // по готовности очередного окна
    void calculatedLastWindow(); // по готовности последнего окна
    void noMoreData(); // после последнего усреднения

public slots:
    /**
     * @brief calculateOneWindow
     * @param inputVector
     * Вычисляет БПФ для одной порции данных - длины окна БПФ
     */
    void calculateOneWindow(DataVector *inputVector, bool lastWindow=false);

protected slots:
    /**
     * @brief calcAvgSpectrumCounts
     * Вычисляет усредненный вектор спектральных отсчетов
     */
    void calcAvgSpectrumCounts();

    /**
    * @brief onWindowCalculated
    * @param count
    * если количество обработанных окон равно m_lengthToAvg, то
    * запускается вычисление усреднения calcAvgSpectrumCounts()
    */
    void onWindowCalculated(int count);

protected:
    FFTRealWrapper *m_fft;
    QVector<DataVector> m_accumulator;
    DataVector m_avgSpectrumCounts;
    int m_lengthToAvg;
    int m_calculatedWindowsCount;
    bool m_lastWindow;
};

#endif // FFTCALCULATOR_H
