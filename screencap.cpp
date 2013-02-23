#include "screencap.h"

#include <QDebug>
#include <QTime>
#include <QScreen>
#include <QPixmap>
#include <QGuiApplication>
#include <QElapsedTimer>

ScreenCap::ScreenCap(double fps, QObject *parent)
: VideoCap(fps, parent)
{}

void ScreenCap::doGrabFrame(QImage &frame)
{
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect    rect   = screen->geometry();
    frame  = screen->grabWindow( 0, rect.x(), rect.y(), rect.width(), rect.height() ).toImage();
}
