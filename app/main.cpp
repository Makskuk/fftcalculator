#include <QApplication>
#include <QDateTime>
#include <QFileInfo>
#include <QDir>
#include "filereader.h"
#include "audiodevice.h"
#include "mainwindow.h"

#include <QDebug>

/*
Точка входа в программу, открывает окно, запускает цикл обработки событий
*/
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("FFT calculator");

    MainWindow w;
    w.show();

    return a.exec();
}
