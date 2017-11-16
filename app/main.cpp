#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>
#include "filereader.h"
#include "audiodevice.h"

#include <QDebug>

typedef enum _ArgType{
    INPUT_FILE,
    OUTPUT_PATH,
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
    stdOutStream << "Read WAV file, calculate FFT and write results to text file\n"
            << "Usage: " << qPrintable(programName) << " -i file [-o path] [-l] \n"
            << "  -i   input WAV file\n"
            << "  -o   path to directory for output text files. One file for one input channel\n"
            << "       By default - current directory\n"
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
    }

    if (result.isEmpty()) {
        qWarning() << "No arguments set!";
        printHelp(progName);
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

#ifdef Q_OS_WIN32
//    QTextCodec::setCodecForLocale(QTextCodec::codecForName("IBM866"));
    setlocale((LC_ALL, ".ACP");
#endif

    QList<Argument> arguments;
    QList<Argument>::iterator  argIterator;
    QFileInfo inputFileInfo;
    QDir outputPath;

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
        case LIST:
            qDebug() << "Available audio input devices:";
            foreach (QString device, AudioDevice::enumerateDevices()) {
                qDebug() << device;
            }
            return 0;
        case HELP:
            printHelp(argIterator->value);
            return 0;
        default:
            qWarning() << "Unexpected argument: " << argIterator->value;
        }
        argIterator++;
    }

    if (inputFileInfo.fileName().isEmpty()) {
        qWarning() << "Input file does not set.";
        printHelp(a.applicationName());
        return -1;
    }

    if (outputPath.absolutePath().isEmpty()) {
        outputPath.setPath(QDir::currentPath());
    }

    qDebug() << "Save results to" << outputPath.absolutePath();

    FileReader freader(inputFileInfo.absoluteFilePath());
    freader.setOutputPath(outputPath.absolutePath());

    QObject::connect(&freader, &FileReader::done, [&](bool success){
        if (success)
            a.exit(0);
        else
            a.exit(-1);
    });

    freader.readFile();

    return a.exec();
}
