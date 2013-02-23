#ifndef RTMP_H
#define RTMP_H

#include "flvtag.h"
#include <QUrl>
#include <QObject>
extern "C"
{
#include <librtmp/rtmp.h>
}

class Rtmp : public QObject
{
Q_OBJECT
public:
    explicit Rtmp(QUrl url, QObject *parent = 0);

    // TODO make this a slot and put it in a thread
    // Use queue back pressure to addjust the vbv
    void sendTag(FlvTag &tag);
signals:    
//    void stateChanged();
public slots:
private:
    RTMP       *m_rtmp;
    RTMPPacket m_rtmpPacket;
};

#endif // RTMP_H
