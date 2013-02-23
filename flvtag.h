#ifndef FLVTAG_H
#define FLVTAG_H

#include <QByteArray>
#include <QDebug>
class FlvTag : public QByteArray
{
public:
    FlvTag() {}
    quint32 flvTimeStamp() {return (quint8(at(7))<<24)|(quint8(at(4))<<16)|(quint8(at(5))<<8)|(quint8(at(6)));}
    static FlvTag flvHeader(bool hasAudio = true, bool hasVideo = true);
    static FlvTag flvTag(quint8 tagType, qint64 pts, qint64 dts, bool keyframe, quint8 *data, int size, bool sequenceHeader = false);
};

#endif // FLVTAG_H
