#include <QApplication>
#include <QDateTime>
#include <QFileInfo>
#include <QDir>
#include "filereader.h"
#include "audiodevice.h"
#include "mainwindow.h"

#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("FFT calculator");

    MainWindow w;
    w.show();

    return a.exec();

//    // Если задан входной файл - читаем его. Иначе - пытаемся читать из микрофона
//    if (!inputFileInfo.fileName().isEmpty()) {
//        freader = new FileReader(inputFileInfo.absoluteFilePath(), &a);
//        freader->setOutputPath(outputPath.absolutePath());
//        freader->setOutputFileName(fileName);

//        QObject::connect(freader, &FileReader::stopped, &a, &QCoreApplication::quit);

//        freader->start();
//    }
//    else {
//        audioDevice = new AudioDeviceReader(&a);
//        audioDevice->setOutputPath(outputPath.absolutePath());
//        audioDevice->setOutputFileName(fileName);

//        qDebug() << "Read from input device " << audioDevice->currentAudioDeviceInfo().deviceName();
//        audioDevice->start();

//        QObject::connect(&a, &QCoreApplication::aboutToQuit, audioDevice, &AudioDeviceReader::stop);
//    }
}
