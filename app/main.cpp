#include <QCoreApplication>
#include "filereader.h"
#include "filewriter.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    FileReader freader("sample.txt");
    FileWriter fwriter("fft_out.txt");

    QObject::connect(&fwriter, &FileWriter::done, &a, &QCoreApplication::quit, Qt::QueuedConnection);

    QObject::connect(&freader, &FileReader::done, [&](bool success){
        if (!success) {
            emit fwriter.done();
            return;
        }

        qDebug("All done. Write result to file %s", qPrintable(fwriter.fileName()));
        QVector<float> data = freader.getFftResult();
        fwriter.writeVector(data);
        qDebug("Write complete.");
    });

    freader.readFile();

    return a.exec();
}
