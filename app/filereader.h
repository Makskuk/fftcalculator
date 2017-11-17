#ifndef FILEREADER_H
#define FILEREADER_H

#include <QObject>

#include "basedatareader.h"
#include "wavfile.h"
#include "worker.h"

/**
 * @brief The FileReader class
 *
 * Класс для чтения данных из файла. Построчно читает файл, прочитав
 * заданное количество строк, вызывает вычисление БПФ на прочитанных данных
 */
class FileReader : public BaseDataReader
{
    Q_OBJECT
public:
    explicit FileReader(QString filename, QObject *parent = 0);

signals:

public slots:
    void printFileInfo();
    void start() override;

protected slots:
    void readBuffer() override;

protected:
    WavFile    *m_file;
};

#endif // FILEREADER_H
