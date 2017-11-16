#ifndef AUDIODEVICE_H
#define AUDIODEVICE_H

#include <QObject>
#include <QAudioDeviceInfo>
#include <QAudioFormat>
#include <QAudioInput>
#include <QIODevice>

class AudioDevice : public QObject
{
    Q_OBJECT
public:
    explicit AudioDevice(QObject *parent = 0);
    ~AudioDevice();

    static QStringList enumerateDevices();

signals:

public slots:
    void setInputDevice(QString devName);
    void setInputDevice(QAudioDeviceInfo &device);
    void start();
    void stop();

protected slots:
    void handleDeviceState(QAudio::State state);
    void audioDataReady();

protected:
    QAudioDeviceInfo    m_audioDevInfo;
    QAudioFormat        m_audioFormat;
    QAudioInput*        m_audioInput;
    QIODevice*          m_inputDevice;
    qint64              m_recordPosition;

    QByteArray          m_buffer;
    qint64              m_bufferPosition;
    qint64              m_bufferLength;
    qint64              m_dataLength;
    bool                m_isActive;
};

#endif // AUDIODEVICE_H
