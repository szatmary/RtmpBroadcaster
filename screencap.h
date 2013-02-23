#ifndef SCREENCAP_H
#define SCREENCAP_H

#include "videocap.h"


// TODO Alow to specify cap reagon
class ScreenCap : public VideoCap
{
Q_OBJECT
public:
    explicit ScreenCap(double fps = DEFAULT_FPS, QObject *parent = 0);
private:
    virtual void doGrabFrame(QImage &frame);
};

#endif // SCREENCAP_H
