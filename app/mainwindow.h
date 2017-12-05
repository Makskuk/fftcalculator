#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QTimer>
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
    void showError(QString error);
    void startRecord(bool toggled);
    void stopRecord();

private:
    void createUi();
    void connectUi();
    void enableUi();
    void disableUi();
    void setLoadFileMode(bool isLoadFile);
    void incrementSecCounter();

    bool m_modeLoadFile;
    int  m_secondsRec;

    Ui::MainWidget *ui;
    QString         m_input;
    QString         m_outputPath;
    QTimer*         m_timer;

    FileReader          *m_fileReader;
    AudioDeviceReader   *m_audioDeviceReader;
};

#endif // MAINWIDGET_H
