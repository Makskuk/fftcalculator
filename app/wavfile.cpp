/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <qendian.h>
#include <QVector>
#include <QDebug>
//#include "utils.h"
#include "wavfile.h"

struct chunk
{
    char        id[4];
    quint32     size;
};

struct RIFFHeader
{
    chunk       descriptor;     // "RIFF"
    char        type[4];        // "WAVE"
};

struct WAVEHeader
{
    chunk       descriptor;
    quint16     audioFormat;
    quint16     numChannels;
    quint32     sampleRate;
    quint32     byteRate;
    quint16     blockAlign;
    quint16     bitsPerSample;
};

struct DATAHeader
{
    chunk       descriptor;
};

struct CombinedHeader
{
    RIFFHeader  riff;
    WAVEHeader  wave;
};

WavFile::WavFile(QObject *parent)
    : QFile(parent)
    , m_headerLength(0)
{

}

bool WavFile::open(const QString &fileName)
{
    close();
    setFileName(fileName);
    return QFile::open(QIODevice::ReadOnly) && readHeader();
}

bool WavFile::create(const QString &fileName, QAudioFormat &fileFormat)
{
    close();
    m_fileFormat = fileFormat;
    setFileName(fileName);
    bool ret = QFile::open(QIODevice::WriteOnly);
    if (ret) writeHeader();
    return ret;
}

void WavFile::finalize()
{
    qint64 recordedSize = size() - m_headerLength;
    qDebug() << "Recorded" << recordedSize << "bytes";
    writeHeader(recordedSize);
    close();
}

const QAudioFormat &WavFile::fileFormat() const
{
    return m_fileFormat;
}

qint64 WavFile::headerLength() const
{
return m_headerLength;
}

bool WavFile::readHeader()
{
    seek(0);
    CombinedHeader header;
    bool result = read(reinterpret_cast<char *>(&header), sizeof(CombinedHeader)) == sizeof(CombinedHeader);
    if (result) {
        if ((memcmp(&header.riff.descriptor.id, "RIFF", 4) == 0
            || memcmp(&header.riff.descriptor.id, "RIFX", 4) == 0)
            && memcmp(&header.riff.type, "WAVE", 4) == 0
            && memcmp(&header.wave.descriptor.id, "fmt ", 4) == 0
            && (header.wave.audioFormat == 1 || header.wave.audioFormat == 0)) {

            // Read off remaining header information
            DATAHeader dataHeader;

            if (qFromLittleEndian<quint32>(header.wave.descriptor.size) > sizeof(WAVEHeader)) {
                // Extended data available
                quint16 extraFormatBytes;
                if (peek((char*)&extraFormatBytes, sizeof(quint16)) != sizeof(quint16))
                    return false;
                const qint64 throwAwayBytes = sizeof(quint16) + qFromLittleEndian<quint16>(extraFormatBytes);
                if (read(throwAwayBytes).size() != throwAwayBytes)
                    return false;
            }

            // skip additional metadata (if exists) - go to DATA header
            seek(sizeof(RIFFHeader) + sizeof(chunk)+header.wave.descriptor.size);

            if (read((char*)&dataHeader, sizeof(DATAHeader)) != sizeof(DATAHeader))
                return false;

            // Establish format
            if (memcmp(&header.riff.descriptor.id, "RIFF", 4) == 0)
                m_fileFormat.setByteOrder(QAudioFormat::LittleEndian);
            else
                m_fileFormat.setByteOrder(QAudioFormat::BigEndian);

            int bps = qFromLittleEndian<quint16>(header.wave.bitsPerSample);
            m_fileFormat.setChannelCount(qFromLittleEndian<quint16>(header.wave.numChannels));
            m_fileFormat.setCodec("audio/pcm");
            m_fileFormat.setSampleRate(qFromLittleEndian<quint32>(header.wave.sampleRate));
            m_fileFormat.setSampleSize(qFromLittleEndian<quint16>(header.wave.bitsPerSample));
            m_fileFormat.setSampleType(bps == 8 ? QAudioFormat::UnSignedInt : QAudioFormat::SignedInt);
        } else {
            result = false;
        }
    }
    m_headerLength = pos();
    return result;
}

/**
 * @brief WavFile::writeHeader
 * @param fileFormat
 * @param dataSize - bytes of audio data (without header)
 */
void WavFile::writeHeader(quint32 dataSize)
{
    seek(0);
    RIFFHeader riffHeader = {
        .descriptor = {
            .id = {'R', 'I', 'F', 'F'},
            .size = 36 + dataSize, // must be file size (in bytes) - 8
        },
        .type = {'W', 'A', 'V', 'E'}
    };
    WAVEHeader waveHeader = {
        .descriptor = {
            .id = {'f', 'm', 't', ' '},
            .size = 16, // PCM header size
        },
        .audioFormat = 1, // 1 = PCM, else - with compression
        .numChannels = qToLittleEndian<quint16>(m_fileFormat.channelCount()),
        .sampleRate = qToLittleEndian<quint32>(m_fileFormat.sampleRate()),
        .byteRate = qToLittleEndian<quint32>(m_fileFormat.sampleRate() * m_fileFormat.channelCount() * m_fileFormat.sampleSize()/8),
        .blockAlign = qToLittleEndian<quint16>(m_fileFormat.channelCount() * m_fileFormat.sampleSize()/8),
        .bitsPerSample = qToLittleEndian<quint16>(m_fileFormat.sampleSize()),
    };
    CombinedHeader header = {
        riffHeader,
        waveHeader
    };
    DATAHeader dataHeader = {
        .descriptor = {
            .id = {'d', 'a', 't', 'a'},
            .size = dataSize, // must be size of data in bytes (NumSamples * NumChannels * BitsPerSample/8)
        }
    };

    write(reinterpret_cast<char *>(&header), sizeof(CombinedHeader));
    write(reinterpret_cast<char *>(&dataHeader), sizeof(DATAHeader));
    m_headerLength = pos();
}
