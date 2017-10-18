#include <QCoreApplication>
#include "filereader.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    FileReader freader("sample.txt");

    QFile outputFile("fft_out.txt");

//    QObject::connect(&freader, &FileReader::done, &a, &QCoreApplication::quit);
    QObject::connect(&freader, &FileReader::done, [&](bool success){
        if (!success) {
            a.exit(-1);
            return;
        }

        qDebug("All done. Write result to file %s", outputFile.fileName().toLocal8Bit().data());
        outputFile.open(QIODevice::WriteOnly);
        outputFile.flush();
        QVector<float> data = freader.getFftResult();
        while (!data.isEmpty()) {
            outputFile.write(QByteArray::number(data.takeFirst()).append("\r\n"));
        }
        outputFile.close();
        qDebug("Write complete");
        a.exit(0);
    });

    QObject::connect(&a, &QCoreApplication::aboutToQuit, [&]{
        qDebug("app quit...");
    });

    freader.readFile();

    qDebug("start event loop...");
    a.exec();

    qDebug("asdasdasdasd");
    return 0;
}
