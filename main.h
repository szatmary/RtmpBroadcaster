#ifndef MAIN_H
#define MAIN_H

#include <QObject>
#include <QTimer>
#include <QSystemTrayIcon>

#include "encode.h"
#include "audiocap.h"
#include "videocompositor.h"
#include "audiocompositor.h"

class Main : public QObject
{
Q_OBJECT
public:
    Main();
public slots:
    void encode();
private:
    QTime m_start;
    QTimer m_timer;
//    QSystemTrayIcon m_systray;
    VideoCompositor m_videoCompositor;
//    AudioCompositor m_audioCompositor;
    AudioCap m_audioCompositor;

    double m_audioPts;
    double m_videoPts;
    Encode m_encode;
};

#endif // MAIN_H
