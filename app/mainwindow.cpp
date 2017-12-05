#include "mainwindow.h"
#include "ui_mainwidget.h"
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    m_modeLoadFile(true),
    ui(new Ui::MainWidget),
    m_inputFileName(""),
    m_outputPath(QDir::currentPath())
{
    ui->setupUi(this);
    createUi();
    connectUi();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onInputDeviceChanged(int index)
{
    if (index == 0) {
        setLoadFileMode(true);
    } else {
        setLoadFileMode(false);
    }
}

void MainWindow::onModeChanged(bool isLoadFile)
{
    if (isLoadFile) {
        ui->btnSelectFile->setEnabled(true);
        ui->lineEditFileName->setEnabled(true);
    } else {
        ui->btnSelectFile->setEnabled(false);
        ui->lineEditFileName->setEnabled(false);
    }
}

void MainWindow::showFileDialog()
{
    const QString fileName = QFileDialog::getOpenFileName(this, "Open WAV file",
                                                          m_outputPath, "*.wav");
    if (!fileName.isNull()) {
        m_inputFileName = fileName;
        ui->lineEditFileName->setText(fileName);
    }
}

void MainWindow::showDirDialog()
{
    const QString dir = QFileDialog::getExistingDirectory(this, "Open Output Directory",
                                                    m_outputPath,
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        m_outputPath = dir;
        ui->lineEditOutPath->setText(dir);
    }
}

void MainWindow::startRecord()
{
    disableUi();

//    if (m_modeLoadFile) {
//        if (m_inputFileName.isEmpty()) {
//            return;
//        }
//        m_fileReader = new FileReader(m_inputFileName, this);
//        m_fileReader->setOutputPath(m_outputPath);
//        disableUi();
//    } else {

//    }
}

void MainWindow::stopRecord()
{
    enableUi();
}

void MainWindow::createUi()
{
    ui->comboBoxSource->addItems(AudioDeviceReader::enumerateDevices());
    ui->lineEditOutPath->setText(m_outputPath);
}

void MainWindow::connectUi()
{
    connect(ui->comboBoxSource, SIGNAL(activated(int)),
            this, SLOT(onInputDeviceChanged(int)));
    connect(ui->btnSelectFile, SIGNAL(pressed()),
            this, SLOT(showFileDialog()));
    connect(ui->btnSelectOutPath, SIGNAL(pressed()),
            this, SLOT(showDirDialog()));
    connect(ui->btnStart, SIGNAL(toggled(bool)),
            this, SLOT(startRecord()));
    connect(ui->btnStop, SIGNAL(clicked(bool)),
            this, SLOT(stopRecord()));

    connect(this, SIGNAL(m_modeLoadFileChanged(bool)),
            this, SLOT(onModeChanged(bool)));

    connect(ui->checkBoxRecTimer, SIGNAL(clicked(bool)),
            ui->spinBoxTimeToRec, SLOT(setEnabled(bool)));
}

void MainWindow::enableUi()
{
    ui->btnStart->setChecked(false);
    ui->btnStart->setDown(false);
    ui->comboBoxSource->setEnabled(true);
    if (ui->comboBoxSource->currentIndex() == 0) {
        ui->btnSelectFile->setEnabled(true);
        ui->lineEditFileName->setReadOnly(false);
    }
    ui->btnSelectOutPath->setEnabled(true);
    ui->lineEditOutPath->setReadOnly(false);
    ui->checkBoxRecTimer->setEnabled(true);
    if (ui->checkBoxRecTimer->isChecked())
        ui->spinBoxTimeToRec->setEnabled(true);
}

void MainWindow::disableUi()
{
    ui->btnStart->setDown(true);
    ui->comboBoxSource->setEnabled(false);
    ui->btnSelectFile->setEnabled(false);
    ui->lineEditFileName->setReadOnly(true);
    ui->btnSelectOutPath->setEnabled(false);
    ui->lineEditOutPath->setReadOnly(true);
    ui->checkBoxRecTimer->setEnabled(false);
    ui->spinBoxTimeToRec->setEnabled(false);
}

void MainWindow::setLoadFileMode(bool isLoadFile)
{
    if (isLoadFile == m_modeLoadFile)
        return;

    m_modeLoadFile = isLoadFile;
    emit m_modeLoadFileChanged(m_modeLoadFile);
}
