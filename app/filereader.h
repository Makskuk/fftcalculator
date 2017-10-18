#ifndef FILEREADER_H
#define FILEREADER_H

#include <QObject>
#include <QFile>

#include "fftcalculator.h"

/**
 * @brief The FileReader class
 *
 * Класс для чтения данных из файла. Построчно читает файл, прочитав
 * заданное количество строк, вызывает вычисление БПФ на прочитанных данных
 */
class FileReader : public QObject
{
    Q_OBJECT
public:
    explicit FileReader(QString filename, QObject *parent = 0);

    FftCalculator::DataVector getFftResult() const;

signals:
    void fftWindowRead(FftCalculator::DataVector vector, bool lastWindow);
    void done(bool success);

public slots:
    void readFile();

protected slots:
    void readDataSet();
    void onFftFinished();

protected:
    QFile *m_file;
    FftCalculator *m_fftCalculator;

    int m_samplesCount;
};

#endif // FILEREADER_H
