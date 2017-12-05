#include "audiodevice.h"
#include <QDebug>

AudioDeviceReader::AudioDeviceReader(QObject *parent) : BaseDataReader(parent),
    m_audioDevInfo(QAudioDeviceInfo::defaultInputDevice()),
    m_audioFormat(m_audioDevInfo.preferredFormat()),
    m_audioInput(new QAudioInput(m_audioDevInfo, m_audioFormat)),
    m_audioBuffer(new QByteArray()),
    m_isActive(false),
    m_idBufferProcessed(true),
    m_bufReadPos(0)
{
    connect(this, &AudioDeviceReader::audioBufferReady, this, &AudioDeviceReader::readBuffer);
    connect(this, &AudioDeviceReader::bufferProcessed, this, &AudioDeviceReader::onBufferProcessed);
}

AudioDeviceReader::~AudioDeviceReader()
{
    if (m_isActive)
        stop();
    delete m_audioBuffer;
}

QAudioDeviceInfo AudioDeviceReader::currentAudioDeviceInfo() const
{
    return m_audioDevInfo;
}

QStringList AudioDeviceReader::enumerateDevices()
{
    QStringList list;
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioInput)) {
        list << deviceInfo.deviceName();
    }
    return list;
}

QString AudioDeviceReader::defaultDevice()
{
    return QAudioDeviceInfo::defaultInputDevice().deviceName();
}

void AudioDeviceReader::setInputDevice(QString devName)
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
    emit BaseDataReader::error("Device " + devName + " not found!");
}

void AudioDeviceReader::setInputDevice(QAudioDeviceInfo &device)
{
    if (device.deviceName() == m_audioDevInfo.deviceName())
        return;

    if (m_isActive)
        stop();

    m_audioDevInfo = device;
    m_audioFormat = device.preferredFormat();
}

void AudioDeviceReader::start()
{
    qDebug() << "Input format: " << m_audioFormat;
    m_audioInput = new QAudioInput(m_audioDevInfo, m_audioFormat);
    m_audioInput->setNotifyInterval(1000);
    m_sampleRate = m_audioFormat.sampleRate();
    m_bytesPerSample = m_audioFormat.sampleSize() / 8;
    m_channelsCount = m_audioFormat.channelCount();
    connect(m_audioInput, &QAudioInput::stateChanged, this, &AudioDeviceReader::handleDeviceState);
    connect(m_audioInput, &QAudioInput::notify, this, &AudioDeviceReader::audioNotify);

    QString filename = m_outputPath + "/" + m_outputFileName + "_rec.wav";

    m_wavFile = new WavFile(this);
    if (m_wavFile->create(filename, m_audioFormat) == false) {
        qWarning() << "Failed to create file" << filename;
    } else {
        qDebug() << "Write audio data to" << filename;
    }

    if (init()) {
        m_audioBuffer->resize(m_internalBufferLength * 10);
        m_inputDevice = m_audioInput->start();
        if (!m_inputDevice) {
            emit BaseDataReader::error("Failed to open input device!");
            return;
        }
        connect(m_inputDevice, &QIODevice::readyRead, this, &AudioDeviceReader::audioDataReady);

        m_isActive = true;
    }
}

void AudioDeviceReader::stop()
{
    m_wavFile->finalize();
    m_audioInput->stop();
    delete m_audioInput;
    m_audioInput = nullptr;
    m_inputDevice = nullptr;
    m_isActive = false;

    BaseDataReader::stop();
}

void AudioDeviceReader::handleDeviceState(QAudio::State state)
{
    qDebug() << "AudioDevice: Change state to" << state;
    if (QAudio::StoppedState == state) {
        // Check error
        QAudio::Error error = m_audioInput->error();
        if (error != QAudio::NoError) {
            qWarning() << "Audio input error:" << error;
            emit BaseDataReader::error("Audio input error");
        }
    }
}

void AudioDeviceReader::audioDataReady()
{
    const qint64 bytesReady = m_audioInput->bytesReady();
    const qint64 bytesSpace = m_audioBuffer->size() - m_readPos;
    const qint64 bytesToRead = qMin(bytesReady, bytesSpace);
    qint64 bytesRead = 0;

//    qDebug() << "Data ready" << bytesReady
//             << "buffer:" << m_audioBuffer->size()
//             << "ReadPos:" << m_readPos
//             << "to read:" << bytesToRead;

    if (m_readPos < m_audioBuffer->size()) {
        bytesRead = m_inputDevice->read(m_audioBuffer->data() + m_readPos, bytesToRead);
//        qDebug() << "read:" << bytesRead;
    }

    if (bytesRead) {
        m_readPos += bytesRead;
        emit audioBufferReady(m_readPos);
    }
}

void AudioDeviceReader::onBufferProcessed()
{
    m_idBufferProcessed = true;
    readBuffer(); // проверим, не пуста ли очередь
//    qDebug() << "    ---> buffer processed";
}

void AudioDeviceReader::readBuffer()
{
    if (!m_idBufferProcessed) {
        // если обработка не закончена - ждём
//        qDebug() << "      ---> processing...";
        return;
    }

    if (m_readPos < m_internalBufferLength) {
        // если данных меньше, чем нужно - ждём
//        qDebug() << "      ---> need mode data...";
        return;
    }

    m_idBufferProcessed = false;

    // возьмем данные слева из буфера
    QByteArray rawBuffer = m_audioBuffer->left(m_internalBufferLength);

    // сдвинем буфер влево
    int bufSize = m_audioBuffer->size();
    QByteArray tmpBuf = m_audioBuffer->right(bufSize - m_internalBufferLength);
    tmpBuf.resize(bufSize);
    delete m_audioBuffer;
    m_audioBuffer = new QByteArray(tmpBuf);
    m_readPos -= m_internalBufferLength;

    m_wavFile->write(rawBuffer); // записать данные с микрофона в WAV-файл
    splitChannels(rawBuffer); // разложим данные каналов
    emit bufferRead();
//    qDebug() << "    ---> buffer read";
}
