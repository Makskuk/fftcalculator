#include "mainwindow.h"
#include "ui_mainwidget.h"
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    m_modeLoadFile(true),
    ui(new Ui::MainWidget),
    m_input(""),
    m_outputPath(QDir::currentPath()),
    m_timer(new QTimer(this)),
    m_secondsRec(0)
{
    ui->setupUi(this);
    m_timer->setSingleShot(true);
    createUi();
    connectUi();
}

MainWindow::~MainWindow()
{
    stopRecord();
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
        m_input = fileName;
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

void MainWindow::showError(QString error)
{
    QMessageBox::warning(this, "Error", error, QMessageBox::Close);
    stopRecord();
}

void MainWindow::startRecord(bool toggled)
{
    if (!toggled) return; // при отпускании кнопки ничего не делаем
    qDebug("Start processing...");
    disableUi();
    m_outputPath = ui->lineEditOutPath->text();

    if (m_modeLoadFile) {
        m_input = ui->lineEditFileName->text();
        if (m_input.isEmpty()) {
            ui->btnStart->setChecked(false);
            showError("Input file not specified!");
            return;
        }
        m_fileReader = new FileReader(this);
        connect(m_fileReader, &FileReader::error, this, &MainWindow::showError);
        connect(m_fileReader, &FileReader::stopped, this, &MainWindow::stopRecord);
        connect(m_fileReader, &FileReader::stopped,
                m_fileReader, &FileReader::deleteLater);
        m_fileReader->setInputFile(m_input);
        m_fileReader->setOutputPath(m_outputPath);
        m_fileReader->setOutputFileName("b"); // ну так хотел заказчик :)
        m_fileReader->start();
    } else {
        m_input = ui->comboBoxSource->currentText();
        m_audioDeviceReader = new AudioDeviceReader(this);
        connect(m_audioDeviceReader, &AudioDeviceReader::stopped,
                m_audioDeviceReader, &AudioDeviceReader::deleteLater);
        connect(m_audioDeviceReader, &AudioDeviceReader::error,
                this, &MainWindow::showError);
        connect(m_audioDeviceReader, &AudioDeviceReader::audioNotify,
                this, &MainWindow::incrementSecCounter);
        m_secondsRec = 0;
        ui->lblSecondsRecordered->setText("0");
        m_audioDeviceReader->setInputDevice(m_input);
        m_audioDeviceReader->setOutputPath(m_outputPath);
        m_audioDeviceReader->setOutputFileName("b"); // ну так хотел заказчик :)
        m_audioDeviceReader->start();
    }

    if (ui->checkBoxRecTimer->isChecked()) {
        int timeout = 1000 * ui->spinBoxTimeToRec->value();
        m_timer->setInterval(timeout);
        connect(m_timer, &QTimer::timeout, this, &MainWindow::stopRecord);
        m_timer->start();
    }
}

void MainWindow::stopRecord()
{
    if (!ui->btnStart->isChecked()) return; // ничего не делаем, если процесс не запущен

    qDebug("Stop processing...");
    if (m_modeLoadFile) {
        m_fileReader->stop();
    } else {
        m_audioDeviceReader->stop();
    }

    if (m_timer->isActive())
        m_timer->stop();

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
            this, SLOT(startRecord(bool)));
    connect(ui->btnStop, SIGNAL(clicked(bool)),
            this, SLOT(stopRecord()));

    connect(this, SIGNAL(m_modeLoadFileChanged(bool)),
            this, SLOT(onModeChanged(bool)));

    connect(ui->checkBoxRecTimer, SIGNAL(clicked(bool)),
            ui->spinBoxTimeToRec, SLOT(setEnabled(bool)));
}

void MainWindow::enableUi()
{
    qDebug("enable UI...");
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
    qDebug("Disable UI...");
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

void MainWindow::incrementSecCounter()
{
    m_secondsRec++;
    ui->lblSecondsRecordered->setText(QString::number(m_secondsRec));
}
