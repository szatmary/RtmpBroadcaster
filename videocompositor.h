#ifndef VIDEOCOMPOSITOR_H
#define VIDEOCOMPOSITOR_H

#include "flvtag.h"
#include "videocap.h"

#include <QUuid>
#include <QSharedPointer>

extern "C"
{
#include <x264.h>
#include <libswscale/swscale.h>
}


class VideoCompositor : public VideoCap
{
Q_OBJECT
public:
    explicit VideoCompositor(QSize resolution, double fps = DEFAULT_FPS, QObject *parent = 0);

    virtual void setFps(double fps);
    void setResolution(QSize resolution);

    void setBackgroundImage(QImage);
    void setForegroundImage(QImage);

    // VideoCompositor takes possession of 'source'. and frees it when done
    QUuid addSource(VideoCap *source, QRect position = QRect(), int zorder = 0, double opacity = 1.0);
    bool modifySource(QUuid id, QRect position, int zorder, double opacity);
    bool removeSource(QUuid id);
    bool removeAllSources();
signals:
    void flvTag(FlvTag);
private slots:
    virtual void doGrabFrame(QImage &frame);
private:
    QSize       m_resolution;
    QMutex      m_mutex;
public:
    struct CompositeInfo
    {
        QSharedPointer<VideoCap> m_capSource;
        QUuid                    m_id;
        QRect                    m_position;
        int                      m_zorder;
        double                   m_opacity;
    };
private:
    QList<CompositeInfo> m_capSource;
};

#endif // VIDEOCOMPOSITOR_H
