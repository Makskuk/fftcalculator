#include <QCoreApplication>
#include <QFileInfo>
#include "filereader.h"
#include "filewriter.h"

#include <QDebug>

typedef enum _ArgType{
    INPUT_FILE,
    OUTPUT_PATH,
    HELP
} ArgType;

typedef struct _Argument {
    ArgType key;
    QString value;
} Argument;

static void printHelp(QString programName)
{
    qInfo() << "Read WAV file, calculate FFT and write results to text file\n"
            << "Usage:" << qPrintable(programName) << "-i file [-o path]\n"
            << "  -i   input WAV file\n"
            << "  -o   path to directory for output text files. One file for one input channel\n"
            << "       By default - current directory";
}

static int parseCommandLine(int argc, char *argv[], QList<Argument> &result)
{
    QString progName(QFileInfo(argv[0]).baseName());
    QString arg;

    for (int i=1; i < argc; i++) {
        arg = QString(argv[i]);
        if (arg == "-h" || arg == "--help") {
            printHelp(progName);
            return 1;
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
//    QCoreApplication a(argc, argv);
    QList<Argument> arguments;
    int retval = parseCommandLine(argc, argv, arguments);
    if (retval != 0)
        return retval;

    for(int i=0; i<arguments.count(); i++) {
        qDebug() << arguments[i].key << arguments[i].value;
    }

    return 0;

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

//    freader.readFile();

//    return a.exec();
    return 0;
}
