#include "videocap.h"

#include <QTime>
#include <QTimer>
#include <QDebug>
#include <QMutexLocker>

VideoCap::VideoCap(double fps, QObject *parent)
: QObject( parent )
, m_delay( 1000 / fps )
{
    moveToThread( &m_thread );
    m_thread.start();
    QMetaObject::invokeMethod(this,"nextFrame");
}

VideoCap::~VideoCap()
{
    setFps(0);
    m_thread.quit();
    m_thread.wait();
}

void VideoCap::setFps(double fps)
{
    QMutexLocker locker(&m_mutex);
    m_delay = 1000 / fps;
}

void VideoCap::nextFrame()
{
    if ( 0 == m_delay ) { return; }
    m_timer.restart();

    QImage frame;
    doGrabFrame( frame );

    if ( ! frame.isNull() )
    {
        m_mutex.lock();
        m_frame = frame;
        m_mutex.unlock();
        emit readyRead();
    }

    if ( 0 != m_delay )
    {
        QTimer::singleShot( qMax( 0.0, m_delay - m_timer.elapsed() ), this, SLOT(nextFrame()) );
    }
}

QImage VideoCap::grabFrame()
{
    QMutexLocker locker(&m_mutex);
    return m_frame.copy();
}
