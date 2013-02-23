#ifndef VIDEOCAP_H
#define VIDEOCAP_H

#include <QMutex>
#include <QImage>
#include <QThread>
#include <QElapsedTimer>

#define DEFAULT_FPS (25.0)

struct VideoFrame
{
    qint64 m_pts;
    QImage m_frame;
};

class VideoCap : public QObject
{
Q_OBJECT
public:
    explicit VideoCap(double fps = DEFAULT_FPS, QObject *parent = 0);
    virtual ~VideoCap();
    QImage grabFrame();
    virtual void setFps(double fps);
signals:
    void readyRead();
private slots:
    void nextFrame();
private:
    virtual void doGrabFrame(QImage &frame) = 0;
    double        m_delay;
    QElapsedTimer m_timer;
    QThread       m_thread;
    QMutex        m_mutex;
    QImage        m_frame;
};

#endif // VIDEOCAP_H
