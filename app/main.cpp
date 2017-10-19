#include <QCoreApplication>
#include "filereader.h"
#include "filewriter.h"

#include <QDebug>

int main(int argc, char *argv[])
{
//    QCoreApplication a(argc, argv);

    FileReader freader("/home/work/QtCreator_projects/fftcalculator/sine_1kHz.wav");
    FileWriter fwriter("fft_out.txt");

//    QObject::connect(&fwriter, &FileWriter::done, &a, &QCoreApplication::quit, Qt::QueuedConnection);

    QObject::connect(&freader, &FileReader::done, [&](bool success){
        if (!success) {
//            emit fwriter.done();
//            return;
            exit(-1);
        }

        qDebug("All done. Write result to file %s", qPrintable(fwriter.fileName()));
        QVector<float> data = freader.getFftResult();
        fwriter.writeVector(data);
        qDebug("Write complete.");
    });

    freader.readFile();

//    return a.exec();
    return 0;
}
