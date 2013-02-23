#include "videocompositor.h"

#include <QPainter>
#include <QDebug>

#include <QTime>
VideoCompositor::VideoCompositor(QSize resolution, double fps, QObject *parent)
: VideoCap( fps, parent )
, m_resolution( resolution )
{}

bool CompositeInfoLessThan(const VideoCompositor::CompositeInfo &ci1, const VideoCompositor::CompositeInfo &ci2) { return ci1.m_zorder < ci2.m_zorder; }
QUuid VideoCompositor::addSource(VideoCap *source, QRect position, int zorder, double opacity)
{
    CompositeInfo ci;
    ci.m_capSource = QSharedPointer<VideoCap>( source );
    ci.m_id        = QUuid::createUuid();
    ci.m_position  = position.isNull() ? QRect( 0, 0, m_resolution.width(), m_resolution.height() ) : position;
    ci.m_zorder    = zorder;
    ci.m_opacity   = opacity;

    // TODO set FPS of source
    QMutexLocker locker(&m_mutex);
    m_capSource.append( ci );
    qSort(m_capSource.begin(), m_capSource.end(), CompositeInfoLessThan); // put in z order
    return ci.m_id;
}

bool VideoCompositor::modifySource(QUuid id, QRect position, int zorder, double opacity)
{
    QMutexLocker locker(&m_mutex);
    foreach(CompositeInfo ci, m_capSource)
    {
        if ( ci.m_id == id )
        {
            ci.m_position  = position.isNull() ? QRect( 0, 0, m_resolution.width(), m_resolution.height() ) : position;
            ci.m_zorder    = zorder;
            ci.m_opacity   = opacity;
            return true;
        }
    }

    return false;
}

bool VideoCompositor::removeSource(QUuid id)
{
    return false;
}

bool VideoCompositor::removeAllSources()
{
    QMutexLocker locker(&m_mutex);
    m_capSource.clear();
    return true;
}


void VideoCompositor::setFps(double fps)
{
    VideoCap::setFps(fps);
    foreach(CompositeInfo info, m_capSource)
    {
        info.m_capSource->setFps( fps );
    }
}

void VideoCompositor::setResolution(QSize resolution)
{
    QMutexLocker locker(&m_mutex);
    m_resolution = resolution;
}

void VideoCompositor::doGrabFrame(QImage &frame)
{
    QMutexLocker locker(&m_mutex);
    frame = QImage( m_resolution, QImage::Format_ARGB32_Premultiplied );
    QPainter painter(&frame);

    // TODO Render background image here
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(frame.rect(), Qt::transparent); // Why is this here?

    foreach(CompositeInfo info, m_capSource)
    {
        painter.setCompositionMode( QPainter::CompositionMode_SourceOver );
        painter.setOpacity(info.m_opacity);
        painter.setRenderHint( QPainter::SmoothPixmapTransform );

        QImage srcImage = info.m_capSource->grabFrame();
        QRect srcRect = QRect( 0, 0, srcImage.width(), srcImage.height() ); // not scaled
        QSize srcSize = ( srcImage.size().scaled( info.m_position.size(), Qt::KeepAspectRatio ) / 2 ) * 2; // Scaled
        QRect dtsRect = QRect( info.m_position.left(), info.m_position.top(), srcSize.width(), srcSize.height() );

        painter.drawImage( dtsRect, srcImage, srcRect );
    }

    // TODO render forground image here
    painter.end();
}




//QImage VideoCompositor::swscale(CompositeInfo *ci)
//{
//    QImage srcImage = ci->m_capSource->grabFrame();
//    QSize  destSize = ( srcImage.size().scaled( ci->m_position.size(), Qt::KeepAspectRatio ) / 2 ) * 2;
//    QImage dstImage( destSize, QImage::Format_RGB32 );
//    qDebug() << "destSize" << ci->m_position.size() << dstImage.size();

//    const uint8_t *srcSlice[] = { srcImage.bits() };
//    uint8_t *dstSlice[] = { dstImage.bits() };
//    int dstStride = dstImage.width() * 4;

//    int srcStride;
//    AVPixelFormat srcFormat;
//    switch( srcImage.format() )
//    {
//    default: qDebug() << "Unsupported format" << srcImage.format(); exit(0); break;
//    case QImage::Format_ARGB32_Premultiplied: srcFormat = AV_PIX_FMT_BGRA; srcStride = srcImage.width() * 4; break;
//    case QImage::Format_RGB32: srcFormat = AV_PIX_FMT_RGB32; srcStride = srcImage.width() * 4; break;
//    case QImage::Format_RGB888: srcFormat = AV_PIX_FMT_RGB24; srcStride = srcImage.width() * 3; break;
//    }

//    ci->m_swCtx = sws_getCachedContext( ci->m_swCtx,
//        srcImage.width(), srcImage.height(), srcFormat,
//        dstImage.width(), dstImage.height(), AV_PIX_FMT_RGB32,
//        SWS_BICUBIC | SWS_ACCURATE_RND, NULL, NULL, NULL );

//    sws_scale(ci->m_swCtx, (const uint8_t * const *)&srcSlice, &srcStride, 0, srcImage.height(), dstSlice, &dstStride);
//    return dstImage;
//}

