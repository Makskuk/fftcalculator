#include <QCoreApplication>
#include <QDateTime>
#include <QFileInfo>
#include <QDir>
#include "filereader.h"
#include "audiodevice.h"

#ifndef Q_OS_WIN32
#include "unix-signal-wrapper.h"
#endif

#include <QDebug>

typedef enum _ArgType{
    INPUT_FILE,
    OUTPUT_PATH,
    OUTPUT_PREFIX,
    LIST,
    HELP
} ArgType;

typedef struct _Argument {
    ArgType key;
    QString value;
} Argument;

static void printHelp(QString programName)
{
    QTextStream stdOutStream(stdout);
    stdOutStream << "Calculate FFT and write results to text file.\n"
                 << "If -i argument provided - read data from WAV file.\n"
                 << "If -i NOT provided - read data from default audio input device. To stop reading - type Ctrl+c\n"
            << "Usage: " << qPrintable(programName) << " [-i file] [-o path] [-l] \n"
            << "  -i   input WAV file\n"
            << "  -o   path to directory for output text files. One file for one input channel\n"
            << "       By default - current directory\n"
            << "  -p   prefix for output files. By default - current date and time\n"
            << "  -l   print list of available input devices\n";
}

static int parseCommandLine(int argc, char *argv[], QList<Argument> &result)
{
    QString progName(QFileInfo(argv[0]).baseName());
    QString arg;

    for (int i=1; i < argc; i++) {
        arg = QString(argv[i]);
        if (arg == "-h" || arg == "--help") {
            Argument a = {HELP, progName};
            result.clear();
            result.append(a);
            return 0;
        }
        if (arg == "-l") {
            Argument a = {LIST, ""};
            result.clear();
            result.append(a);
            return 0;
        }
        if (arg == "-i") {
            Argument a = {INPUT_FILE, QString(argv[++i])};
            result.append(a);
            continue;
        }
        if (arg == "-o") {
            Argument a = {OUTPUT_PATH, QString(argv[++i])};
            result.append(a);
            continue;
        }
        if (arg == "-p") {
            Argument a = {OUTPUT_PREFIX, QString(argv[++i])};
            result.append(a);
            continue;
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

#ifdef Q_OS_WIN32
    // перехват Ctrl+c
    signal(SIGINT, a.exit);
    signal(SIGABRT, a.exit);
    signal(SIGTERM, a.exit);

    // установка системной локали, кодировок и тп
    setlocale(LC_ALL, ".ACP");
#else
    // перехват Unix-сигнала для корректного завершения
    UnixSignalWrapper unixSignalWrapper;
    QObject::connect(&unixSignalWrapper, &UnixSignalWrapper::unixSignalReceived,
                     &a, &QCoreApplication::quit);
#endif

    QList<Argument> arguments;
    QList<Argument>::iterator  argIterator;
    QFileInfo inputFileInfo;
    QDir outputPath;
    QString fileName;

    int retval = parseCommandLine(argc, argv, arguments);
    if (retval != 0)
        return retval;

    argIterator = arguments.begin();
    while (argIterator != arguments.end()) {
        switch (argIterator->key) {
        case INPUT_FILE:
            inputFileInfo.setFile(argIterator->value);
            if (!inputFileInfo.exists()) {
                qWarning() << "File" << inputFileInfo.fileName() << "does not exist";
                return -1;
            }
            break;
        case OUTPUT_PATH:
            outputPath.setPath(argIterator->value);
            if (!outputPath.exists()) {
                qWarning() << "Directory" << outputPath.path() << "does not exist";
                return -1;
            }
            break;
        case OUTPUT_PREFIX:
            fileName = argIterator->value;
            break;
        case LIST:
            qDebug() << "Available audio input devices:";
            foreach (QString device, AudioDeviceReader::enumerateDevices()) {
                qDebug() << device;
            }
            qDebug() << "\nDefault device: " << AudioDeviceReader::defaultDevice();
            return 0;
        case HELP:
            printHelp(argIterator->value);
            return 0;
        default:
            qWarning() << "Unexpected argument: " << argIterator->value;
        }
        argIterator++;
    }

    // Путь по умолчанию
    if (outputPath.absolutePath().isEmpty()) {
        outputPath.setPath(QDir::currentPath());
    }

    // Префикс по умолчанию
    if (fileName.isEmpty()) {
        fileName = QDateTime::currentDateTime().toString("ddMMyyyy_HHmmss");
    }

    qDebug() << "Save results to" << outputPath.absolutePath();

    FileReader*  freader;
    AudioDeviceReader* audioDevice;

    // Если задан входной файл - читаем его. Иначе - пытаемся читать из микрофона
    if (!inputFileInfo.fileName().isEmpty()) {
        freader = new FileReader(inputFileInfo.absoluteFilePath(), &a);
        freader->setOutputPath(outputPath.absolutePath());
        freader->setOutputFileName(fileName);

        QObject::connect(freader, &FileReader::stopped, &a, &QCoreApplication::quit);

        freader->start();
    }
    else {
        audioDevice = new AudioDeviceReader(&a);
        audioDevice->setOutputPath(outputPath.absolutePath());
        audioDevice->setOutputFileName(fileName);

        qDebug() << "Read from input device " << audioDevice->currentAudioDeviceInfo().deviceName();
        audioDevice->start();

        QObject::connect(&a, &QCoreApplication::aboutToQuit, audioDevice, &AudioDeviceReader::stop);
    }

    return a.exec();
}
