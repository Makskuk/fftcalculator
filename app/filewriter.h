#ifndef FILEWRITER_H
#define FILEWRITER_H

#include <QObject>
#include <QFile>
#include <QVector>

class FileWriter : public QObject
{
    Q_OBJECT
public:
    explicit FileWriter(QString filename, QObject *parent = 0);

    QString fileName() const { return m_file->fileName(); }
signals:
    void done();

public slots:
    void writeVector(QVector<float> data);

protected:
    QFile *m_file;
};

#endif // FILEWRITER_H
