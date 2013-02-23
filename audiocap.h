#ifndef AUDIOCAP_H
#define AUDIOCAP_H

#include <QTime>
#include <QBuffer>
#include <QObject>
#include <QAudioInput>

#include "audiocapdevice.h"

class AudioCap : public QObject
{
Q_OBJECT
public:
    explicit AudioCap(QString deviceName, QObject *parent = 0);
    QByteArray grabFrame();
    static QStringList deviceList();
signals:
    void readyRead();
private:
    double         m_msPerFrame;
    qint32         m_bytesPerFrame;
    QAudioInput   *m_audioInput;
    AudioCapDevice m_audioDevice;
};

#endif // AUDIOCAP_H
