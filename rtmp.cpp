#include "rtmp.h"
#include "url.h"

Rtmp::Rtmp(QUrl url, QObject *parent)
: QObject(parent)
{
    m_rtmp = RTMP_Alloc();
    RTMP_Init(m_rtmp);
    qDebug() << "Connecting to" << url;
    RTMP_SetupURL(m_rtmp, MY_URL );
    RTMP_EnableWrite(m_rtmp);

    RTMP_Connect(m_rtmp, NULL);
    RTMP_ConnectStream(m_rtmp, 0);
    memset(&m_rtmpPacket, 0, sizeof(RTMPPacket));
    qDebug() << RTMP_IsConnected(m_rtmp);



}

void Rtmp::sendTag(FlvTag &tag)
{
     if( ! RTMP_IsConnected(m_rtmp) || RTMP_IsTimedout(m_rtmp) )
     {
        qDebug() << "RTMP error";
        exit(0);
     }

     RTMP_Write(m_rtmp,tag.constData(),tag.size());

     // Handle RTMP ping and such
     fd_set sockset; struct timeval timeout = {0,0};
     FD_ZERO(&sockset); FD_SET(RTMP_Socket(m_rtmp), &sockset);
     register int result = select(RTMP_Socket(m_rtmp) + 1, &sockset, NULL, NULL, &timeout);
     if (result == 1 && FD_ISSET(RTMP_Socket(m_rtmp), &sockset) )
     {
//         qDebug() << "RTMP_ReadPacket";
         RTMP_ReadPacket(m_rtmp, &m_rtmpPacket);
        if( ! RTMPPacket_IsReady(&m_rtmpPacket) )
        {
            qDebug() << "Received RTMP packet";
            RTMP_ClientPacket(m_rtmp,&m_rtmpPacket);
            RTMPPacket_Free(&m_rtmpPacket);
        }
    }
}
