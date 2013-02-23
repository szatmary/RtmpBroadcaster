#include "audiocap.h"

#include <QAudioDeviceInfo>
#include <QDebug>

QStringList AudioCap::deviceList()
{
    QStringList list;
    QList<QAudioDeviceInfo>	devices = QAudioDeviceInfo::availableDevices( QAudio::AudioInput );
    foreach ( QAudioDeviceInfo device, devices )
    {
        list.append( device.deviceName() );
    }

    return list;
}


AudioCap::AudioCap(QString deviceName, QObject *parent)
: QObject(parent)
{
    QList<QAudioDeviceInfo>	devices = QAudioDeviceInfo::availableDevices( QAudio::AudioInput );
    foreach ( QAudioDeviceInfo device, devices )
    {
        if ( device.deviceName() == deviceName )
        {
            QAudioFormat format;
            format.setSampleRate(44100);
            format.setChannelCount(2);
            format.setSampleSize(16);
            format.setCodec("audio/pcm");
            format.setByteOrder(QAudioFormat::LittleEndian);
            format.setSampleType(QAudioFormat::SignedInt);
            format = device.nearestFormat( format );

            m_audioInput = new QAudioInput(device, format);
            goto config_sound_device;
        }
    }

    qDebug() << "Unknown device:" << deviceName;
    qDebug() << AudioCap::deviceList();
return;
config_sound_device:
    // TODO validate AudioFormat
    m_msPerFrame    = 1000 / ( m_audioInput->format().sampleRate() / 1024.0 );
    m_bytesPerFrame = m_audioInput->format().bytesPerFrame() * 1024;
//    qDebug() << "ms per frame" << m_msPerFrame << "bytes per frame" << m_bytesPerFrame;
    m_audioInput->setNotifyInterval( m_msPerFrame );
    connect(m_audioInput,SIGNAL(notify()),this,SIGNAL(readyRead()));

//    qDebug() << m_audioInput->format() << m_msPerFrame;
    m_audioDevice.open( QIODevice::ReadWrite );
    m_audioInput->start( &m_audioDevice );
}

//void AudioCap::readyRead()
//{
//    while( m_bytesPerFrame <= m_audioDevice.bytesAvailable() )
//    {
//        QByteArray frame = m_audioDevice.read( m_bytesPerFrame );
//        quint32 frameTime = m_frameCount * m_msPerFrame;
//        ++m_frameCount;
//    }
//}

QByteArray AudioCap::grabFrame()
{
    if ( m_bytesPerFrame > m_audioDevice.bytesAvailable() )
        return QByteArray();

    return m_audioDevice.read( m_bytesPerFrame );
}

