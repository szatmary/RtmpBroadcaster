#ifndef AUDIOCAPDEVICE_H
#define AUDIOCAPDEVICE_H

#include <QByteArray>
#include <QIODevice>
class AudioCapDevice : public QIODevice
{
Q_OBJECT
public:
    AudioCapDevice();
    virtual bool atEnd() const { return 0 == bytesAvailable(); }
    virtual bool isSequential() const { return true; }
    virtual qint64 bytesAvailable() const { return ( m_readBuffer.size() - m_position ) + m_writeBuffer.size() + QIODevice::bytesAvailable(); }
protected:
    qint64 readData(char * data, qint64 maxSize);
    qint64 writeData(const char * data, qint64 maxSize);
private:
    qint64     m_position;
    QByteArray m_readBuffer;
    QByteArray m_writeBuffer;
};

#endif // AUDIOCAPDEVICE_H
