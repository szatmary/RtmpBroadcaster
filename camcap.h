#ifndef CAMCAP_H
#define CAMCAP_H

#include <videocap.h>
#include <opencv2/highgui/highgui.hpp>

class CamCap : public VideoCap
{
Q_OBJECT
public:
    explicit CamCap(int device, double fps = DEFAULT_FPS, QObject *parent = 0);
private:
    virtual void doGrabFrame(QImage &frame);
private:
    cv::VideoCapture m_device;
};

#endif // CAMCAP_H
