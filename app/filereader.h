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
    explicit FileReader(QObject *parent = 0);

    int timeToRead() const;

signals:
    void timeToReadChanged(int ms);

public slots:
    void setInputFile(QString filename);
    void setTimeToRead(int ms);
    void printFileInfo();
    void start() override;

protected slots:
    void readBuffer() override;

protected:
    WavFile    *m_file;
    QString     m_inputFile;

    int m_timeToRead; // milliseconds
    int m_dataToRead;
};

#endif // FILEREADER_H
