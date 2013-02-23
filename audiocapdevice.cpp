#include "audiocapdevice.h"

AudioCapDevice::AudioCapDevice()
: QIODevice()
, m_position( 0 )
{
}

qint64 AudioCapDevice::readData(char * data, qint64 maxSize)
{
    if ( 0 >= maxSize )
        return 0;

    // If read buffer is empty, swap for write buffer
    if ( m_position >= m_readBuffer.size() )
    {
        if( 0 == m_writeBuffer.size() )
            return 0;

        m_position   = 0;
        m_readBuffer = m_writeBuffer;
        m_writeBuffer.clear();
    }

    qint64 bytesRead = qMin( m_readBuffer.size() - m_position, maxSize );
    memcpy( data, m_readBuffer.data() + m_position, bytesRead );
    m_position += bytesRead;

    return bytesRead + readData( data + bytesRead, maxSize - bytesRead );
}

qint64 AudioCapDevice::writeData(const char * data, qint64 maxSize)
{
    // TODO start droping data if the buffer gets too big
    m_writeBuffer.append( data, maxSize );
    return maxSize;
}
