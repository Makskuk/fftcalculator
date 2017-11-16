#include "audiodevice.h"
#include <QDebug>

AudioDevice::AudioDevice(QObject *parent) : QObject(parent),
    m_audioDevInfo(QAudioDeviceInfo::defaultInputDevice()),
    m_audioFormat(m_audioDevInfo.preferredFormat()),
    m_audioInput(new QAudioInput(m_audioDevInfo, m_audioFormat)),
    m_isActive(false)
{

}

AudioDevice::~AudioDevice()
{
    stop();
}

QAudioDeviceInfo AudioDevice::currentAudioDeviceInfo() const
{
    return m_audioDevInfo;
}

QStringList AudioDevice::enumerateDevices()
{
    QStringList list;
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioInput)) {
        list << deviceInfo.deviceName();
    }
    return list;
}

QString AudioDevice::defaultDevice()
{
    return QAudioDeviceInfo::defaultInputDevice().deviceName();
}

void AudioDevice::setInputDevice(QString devName)
{
    QList<QAudioDeviceInfo> devList = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    QList<QAudioDeviceInfo>::iterator devListIterator = devList.begin();
    while (devListIterator != devList.end()) {
        if (devListIterator->deviceName() == devName) {
            setInputDevice(*devListIterator);
            return;
        }
        devListIterator++;
    }
    qWarning() << "Device" << devName << "not found!";
}

void AudioDevice::setInputDevice(QAudioDeviceInfo &device)
{
    if (device.deviceName() == m_audioDevInfo.deviceName())
        return;

    if (m_isActive)
        stop();

    m_audioDevInfo = device;
    m_audioFormat = device.preferredFormat();
}

void AudioDevice::start()
{
    m_audioInput = new QAudioInput(m_audioDevInfo, m_audioFormat);
    connect(m_audioInput, &QAudioInput::stateChanged, this, &AudioDevice::handleDeviceState);

    m_buffer.resize(1024);
    m_inputDevice = m_audioInput->start();    
    connect(m_inputDevice, &QIODevice::readyRead, this, &AudioDevice::audioDataReady);

    m_isActive = true;
}

void AudioDevice::stop()
{
    m_audioInput->stop();
    delete m_audioInput;
    m_audioInput = nullptr;
    m_isActive = false;
}

void AudioDevice::handleDeviceState(QAudio::State state)
{
    qDebug() << "AudioDevice: New device state:" << state;
    if (QAudio::StoppedState == state) {
        // Check error
        QAudio::Error error = m_audioInput->error();
        if (error != QAudio::NoError) {
            qWarning() << "Audio input error:" << error;
        }
    }
}

void AudioDevice::audioDataReady()
{
    const qint64 bytesReady = m_audioInput->bytesReady();
    const qint64 bytesSpace = m_buffer.size() - m_dataLength;
    const qint64 bytesToRead = qMin(bytesReady, bytesSpace);

    const qint64 bytesRead = m_inputDevice->read(
                                       m_buffer.data() + m_dataLength,
                                       bytesToRead);

    if (bytesRead) {
        m_dataLength += bytesRead;
        qDebug() << "Data length changed";
//        emit dataLengthChanged(dataLength());
    }

    if (m_buffer.size() == m_dataLength)
        qDebug() << "Stop recording!";
//        stopRecording();
}
