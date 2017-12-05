#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include "filereader.h"
#include "audiodevice.h"

namespace Ui {
class MainWidget;
}

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void m_modeLoadFileChanged(bool isLoadFile);

private slots:
    void onInputDeviceChanged(int index);
    void onModeChanged(bool isLoadFile);
    void showFileDialog();
    void showDirDialog();
    void startRecord();
    void stopRecord();

private:
    void createUi();
    void connectUi();
    void enableUi();
    void disableUi();
    void setLoadFileMode(bool isLoadFile);

    bool m_modeLoadFile;

    Ui::MainWidget *ui;
    QString         m_inputFileName;
    QString         m_outputPath;

    FileReader          *m_fileReader;
    AudioDeviceReader   *m_audioDeviceReader;
};

#endif // MAINWIDGET_H
