#ifndef ENCODE_H
#define ENCODE_H

#include "flvtag.h"
#include "rtmp.h"
#include "screencap.h"

#include <QFile>

extern "C"
{
#include <x264.h>
#include <libswscale/swscale.h>
#include <fdk-aac/aacenc_lib.h>
}

class Encode : public QObject
{
Q_OBJECT
public:
    explicit Encode(QSize resolution, QObject *parent = 0);
    ~Encode();
 //private slots:
    void encodeFrame(QImage,quint32);
    void encodeFrame(QByteArray,quint32);
private:
    QThread    m_thread;
    QSize      m_resolution;

    // video encoder
    SwsContext *m_swCtx;
    x264_t     *m_264enc;

    // audio encoder
    HANDLE_AACENCODER m_aacenc;

    // flv writer
    void writeFlvTag(quint8 TagType, qint64 Pts, qint64 Dts, bool Keyframe, quint8 *data, int size, bool SequenceHeader = false);
    QFile         m_file;
    QList<FlvTag> m_video;
    QList<FlvTag> m_audio;

    Rtmp m_rtmp;
};

#endif // ENCODE_H
