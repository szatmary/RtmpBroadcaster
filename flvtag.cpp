#include "flvtag.h"

FlvTag FlvTag::flvHeader(bool hasAudio, bool hasVideo)
{
    FlvTag tag;
    tag.reserve( 13 );
    tag.append( 'F' );
    tag.append( 'L' );
    tag.append( 'V' );
    tag.append( 0x01 ); // Version
    tag.append( ( hasAudio ? 0x04 : 0x00 ) | ( hasVideo ? 0x01 : 0x00 ) ); // Flags
    tag.append( char(0) ); // DataOffset
    tag.append( char(0) ); // DataOffset
    tag.append( char(0) ); // DataOffset
    tag.append( 0x09 );    // DataOffset
    tag.append( char(0) ); // PreviousTagSize
    tag.append( char(0) ); // PreviousTagSize
    tag.append( char(0) ); // PreviousTagSize
    tag.append( char(0) ); // PreviousTagSize
    return tag;
}

FlvTag FlvTag::flvTag(quint8 tagType, qint64 pts, qint64 dts, bool keyframe, quint8 *data, int size, bool sequenceHeader)
{
    FlvTag tag;
    tag.reserve( size + 32); // 32 sounds good. Bigest is probablly 20
    tag.append( tagType );
    quint32 dataSize = size;
    switch( tagType )
    {
        case 9:
        {
            dataSize += 5; // 5 is video heaer size
            tag.append( quint8( (dataSize >> 16) & 0xFF ) );
            tag.append( quint8( (dataSize >>  8) & 0xFF ) );
            tag.append( quint8( (dataSize >>  0) & 0xFF ) );

            tag.append( quint8( (dts >> 16) & 0xFF ) );
            tag.append( quint8( (dts >>  8) & 0xFF ) );
            tag.append( quint8( (dts >>  0) & 0xFF ) );
            tag.append( quint8( (dts >> 24) & 0xFF ) );

            tag.append( char(0) ); // StreamId
            tag.append( char(0) );
            tag.append( char(0) );

            tag.append( ( keyframe ? 0x10 : 0x20 ) | 0x07 ); // FrameType + CodecID
            tag.append( sequenceHeader ? 0x00 : 0x01 );

            pts = sequenceHeader ? 0 : (pts - dts);
            tag.append( ( pts >> 16 ) & 0xFF );
            tag.append( ( pts >>  8 ) & 0xFF );
            tag.append( ( pts >>  0 ) & 0xFF );
        } break;
        case 8:
        {
            dataSize += 2; // a is aac heaer size
            tag.append( quint8( (dataSize >> 16) & 0xFF ) );
            tag.append( quint8( (dataSize >>  8) & 0xFF ) );
            tag.append( quint8( (dataSize >>  0) & 0xFF ) );

            tag.append( quint8( (dts >> 16) & 0xFF ) );
            tag.append( quint8( (dts >>  8) & 0xFF ) );
            tag.append( quint8( (dts >>  0) & 0xFF ) );
            tag.append( quint8( (dts >> 24) & 0xFF ) );

            tag.append( char(0) ); // StreamId
            tag.append( char(0) );
            tag.append( char(0) );

            tag.append( 0xA0 | 0x0F ); // CodecID + SoundFormat
            tag.append( sequenceHeader ? 0x00 : 0x01 );
        } break;
        default:
            qDebug() << "Unknown tagType" << tagType;
            return FlvTag();
    }

    tag.append( (const char*)data, size );

    dataSize += 11;
    tag.append( (dataSize >> 24) & 0xFF );
    tag.append( (dataSize >> 16) & 0xFF );
    tag.append( (dataSize >>  8) & 0xFF );
    tag.append( (dataSize >>  0) & 0xFF );
    return tag;
}
