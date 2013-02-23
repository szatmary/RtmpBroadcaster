#include "main.h"


#include <QTimer>
#include <QDir>
#include <QMenu>
#include <QApplication>
#include <QStandardPaths>
#include "camcap.h"
#include "videocompositor.h"

Main::Main()
//: m_systray( QIcon(":/icon/tray_icon.png") )
: m_start( QTime::currentTime() )
, m_videoCompositor( QSize(1080,720) )
, m_audioCompositor( "Soundflower (2ch)" )
, m_encode( QSize(1080,720) )
{
    // QSysTrayIcon menus are not working on my mac. Why?
//    m_systray.setContextMenu( menu );
//    m_systray.show();

    //    QDir pictureDir( QStandardPaths::writableLocation( QStandardPaths::PicturesLocation )  );

    m_videoCompositor.addSource( new ScreenCap() );
    m_videoCompositor.addSource( new CamCap(0), QRect(540,360,540,360), 0, 0.4 );

    m_audioPts = m_videoPts = 0;
    m_timer.setInterval( 1000 / DEFAULT_FPS );
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(encode()));
    m_timer.start();
    qDebug() << "HERE";
}

void Main::encode()
{
    // TODO the PTS valuse here are SOOOOOO wrong!
    while( true )
    {
        QByteArray audio = m_audioCompositor.grabFrame();
        if ( audio.isEmpty() ) break;
        m_audioPts += 1000 / ( 44100 / 1024.0 );
        if ( m_audioPts - m_videoPts > (1000/25) )
        {
            m_videoPts = m_audioPts;
            QImage video = m_videoCompositor.grabFrame();
            m_encode.encodeFrame( video, m_videoPts );
        }

        m_encode.encodeFrame( audio, m_audioPts );
    }
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Main m;
    return a.exec();
}
