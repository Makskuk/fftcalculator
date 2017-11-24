#ifndef AUDIODEVICE_H
#define AUDIODEVICE_H

#include <QObject>
#include <QAudioDeviceInfo>
#include <QAudioFormat>
#include <QAudioInput>
#include <QIODevice>

#include "basedatareader.h"
#include "worker.h"
#include "wavfile.h"

class AudioDeviceReader : public BaseDataReader
{
    Q_OBJECT
public:
    explicit AudioDeviceReader(QObject *parent = 0);
    ~AudioDeviceReader();

    QAudioDeviceInfo currentAudioDeviceInfo() const;

    static QStringList enumerateDevices();
    static QString     defaultDevice();

signals:
    void audioBufferReady(int size);

public slots:
    void setInputDevice(QString devName);
    void setInputDevice(QAudioDeviceInfo &device);
    void start() override;
    void stop() override;

protected slots:
    void handleDeviceState(QAudio::State state);
    void audioDataReady();
    void onBufferProcessed();
    void readBuffer() override;

protected:
    QAudioDeviceInfo    m_audioDevInfo;
    QAudioFormat        m_audioFormat;
    QAudioInput*        m_audioInput;
    QIODevice*          m_inputDevice;
    QByteArray*         m_audioBuffer;

    WavFile*            m_wavFile;

    bool    m_isActive;
    bool    m_idBufferProcessed;
    int     m_bufReadPos;
};

#endif // AUDIODEVICE_H
