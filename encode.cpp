#include "encode.h"

#include <QDir>
#include <QScreen>
#include <QGuiApplication>
#include <QAbstractEventDispatcher>
#include <QDebug>
#include <QStandardPaths>
#include <QDateTime>

#include "url.h"

#define RNDTOEVEN(X) (2*(int(X)/2))

Encode::Encode(QSize resolution, QObject *parent)
: QObject(parent)
, m_resolution( (resolution/2)*2 )
, m_swCtx( NULL )
, m_rtmp (QString(MY_URL))
{
    // Write FLV Header to file
    QDir movieDir( QStandardPaths::writableLocation( QStandardPaths::MoviesLocation ) );
    m_file.setFileName( movieDir.filePath( QDateTime::currentDateTime().toString("yyyy MM dd - hh mm ss'.flv'") ) );
    m_file.open( QIODevice::ReadWrite | QIODevice::Truncate );
    m_file.write( FlvTag::flvHeader() );

    // TODO write onMetaData script tag here!

    x264_param_t param;
    x264_param_default_preset(&param, "fast", "zerolatency" );
    param.i_threads        = QThread::idealThreadCount();
    param.b_sliced_threads = true;
    param.i_width          = m_resolution.width();
    param.i_height         = m_resolution.height();
    param.b_vfr_input      = true;
    param.i_timebase_num   = 1;
    param.i_timebase_den   = 1000;

    //Rate control:
    param.rc.i_rc_method = X264_RC_CRF;
    param.rc.f_rf_constant = 25;
    param.rc.f_rf_constant_max = 35;

//    param.b_
    // Bitstream format
    param.b_annexb = 0;
    param.b_repeat_headers = 0;
    param.b_aud = 0;

    x264_param_apply_profile(&param, "baseline");
    m_264enc = x264_encoder_open(&param);

    // Create Flv Sequence header
    int i_nals; x264_nal_t* nals;
    x264_encoder_headers( m_264enc, &nals, &i_nals );
    QVector<QByteArray> sps, pps;
    for(int i = 0 ; i < i_nals ; ++i )
    {
        switch( nals[i].i_type )
        { // +4, -4, removes the 4 byte size at teh begining for the NALU
        case 7: sps.append( QByteArray((const char*)nals[i].p_payload + 4, nals[i].i_payload - 4) ); break;
        case 8: pps.append( QByteArray((const char*)nals[i].p_payload + 4, nals[i].i_payload - 4) ); break;
        }
    }

    // TODO make sure we have sps and pps
    QByteArray sequenceHeader;
    sequenceHeader.append( 0x01 );  // version
    sequenceHeader.append( sps[0][1] ); // profile
    sequenceHeader.append( sps[0][2] ); // compatibility
    sequenceHeader.append( sps[0][3] ); // level
    sequenceHeader.append(0xFC | 3); // reserved (6 bits), NULA length size - 1 (2 bits)
    sequenceHeader.append(0xE0 | sps.size() ); // reserved (3 bits), num of SPS (5 bits)
    for(int i = 0 ; i < sps.size() ; ++i )
    {
        quint16 size = sps[i].size();
        sequenceHeader.append( 0xFF & ( size >> 8 ) ); // 2 bytes for length of SPS
        sequenceHeader.append( 0xFF & ( size >> 0 ) ); // 2 bytes for length of SPS
        sequenceHeader.append( sps[i] );
    }

    sequenceHeader.append( pps.size() );
    for(int i = 0 ; i < pps.size() ; ++i )
    {
        quint16 size = pps[i].size();
        sequenceHeader.append( 0xFF & ( size >> 8 ) ); // 2 bytes for length of SPS
        sequenceHeader.append( 0xFF & ( size >> 0 ) ); // 2 bytes for length of SPS
        sequenceHeader.append( pps[i].data() );
    }

    // Write Sequence Header
    writeFlvTag(9, 0, 0, true, (quint8*)sequenceHeader.data(), sequenceHeader.size(), true);


    // TODO check return codes
    AACENC_InfoStruct info;
    aacEncOpen(&m_aacenc, 0, 0);
    // TODO should we set SBR? v2?
//    qDebug() << aacEncoder_SetParam(m_aacenc,AACENC_SIGNALING_MODE,0);
    aacEncoder_SetParam(m_aacenc,AACENC_TRANSMUX,0);
    aacEncoder_SetParam(m_aacenc,AACENC_AFTERBURNER,1);
    aacEncoder_SetParam(m_aacenc,AACENC_BITRATE,128000);
    aacEncoder_SetParam(m_aacenc,AACENC_SAMPLERATE,44100);
    aacEncoder_SetParam(m_aacenc,AACENC_CHANNELMODE,2);
    aacEncEncode(m_aacenc, NULL, NULL, NULL, NULL); // Initilize
    aacEncInfo(m_aacenc, &info);

    // info.encoderDelay; This may be usefull!
    writeFlvTag(8, 0, 0, true, (quint8*)info.confBuf,info.confSize, true);

    moveToThread( &m_thread );
    m_thread.start();
}

Encode::~Encode()
{
    // shutdown the encode thread
    qDebug() << "Closing the encoder";
    m_thread.quit();
    m_thread.wait();

    while( x264_encoder_delayed_frames( m_264enc ) )
    {
        int i_nals; x264_nal_t* nals; x264_picture_t pic_out;
        int frame_size = x264_encoder_encode(m_264enc, &nals, &i_nals, 0, &pic_out);
        if ( i_nals > 0 )
            writeFlvTag(9, pic_out.i_pts, pic_out.i_dts, pic_out.b_keyframe, nals->p_payload, frame_size );
    }

//    while ( m_video.size() && m_audio.size() )
//    {
//        if ( m_audio.front().timeStamp() <= m_video.front().timeStamp() )
//            m_file.write( m_audio.takeFirst() );
//        else
//            m_file.write( m_video.takeFirst() );
//    }

//    while ( m_video.size() )
//        m_file.write( m_video.takeFirst() );

//    while ( m_audio.size() )
//        m_file.write( m_audio.takeFirst() );


    x264_encoder_close( m_264enc );
    sws_freeContext( m_swCtx );

    m_swCtx   = 0;
    m_264enc = 0;
    m_file.close();
}

void Encode::encodeFrame(QImage frame, quint32 pts)
{
    const uint8_t *srcSlice[] = { frame.bits() };
    int srcStride = frame.width() * 4 ;

    x264_picture_t pic_in, pic_out;
    x264_picture_alloc(&pic_in, X264_CSP_I420, m_resolution.width(), m_resolution.height() );
    pic_in.i_pts = pts;

    // TODO rebuild this context if (when) screen geomorty changes!
    m_swCtx = sws_getCachedContext( m_swCtx,
        frame.width(), frame.height(), AV_PIX_FMT_RGB32,
        m_resolution.width(), m_resolution.height(), AV_PIX_FMT_YUV420P,
        SWS_LANCZOS | SWS_ACCURATE_RND, NULL, NULL, NULL );
    //  SWS_BICUBIC

    sws_scale(m_swCtx, (const uint8_t * const *)&srcSlice, &srcStride, 0, frame.height(), pic_in.img.plane, pic_in.img.i_stride);

    int i_nals; x264_nal_t* nals;
    int frame_size = x264_encoder_encode(m_264enc, &nals, &i_nals, &pic_in, &pic_out);
    if ( frame_size > 0 )
    {
//        qDebug() << pic_in.i_pts << pic_out.i_pts << pic_out.i_dts;
        writeFlvTag(9, pic_out.i_pts, pic_out.i_dts, pic_out.b_keyframe, nals->p_payload, frame_size );
    }

    x264_picture_clean( &pic_in );
}

void Encode::encodeFrame(QByteArray frame, quint32 pts)
{
    AACENC_BufDesc in_buf, out_buf;
    AACENC_InArgs  in_args ;
    AACENC_OutArgs out_args;

    void *in_ptr                 = frame.data();
    int in_buffer_size           = 2 * 2 * 1024; // TODO un hard code shannels value
    int in_buffer_element_size   = 2;
    int in_buffer_identifier     = IN_AUDIO_DATA;
    int out_buffer_identifier    = OUT_BITSTREAM_DATA;

    in_args.numInSamples     = 2 * 1024; // TODO un hard code shannels value
    in_buf.numBufs           = 1;
    in_buf.bufs              = &in_ptr;
    in_buf.bufferIdentifiers = &in_buffer_identifier;
    in_buf.bufSizes          = &in_buffer_size;
    in_buf.bufElSizes        = &in_buffer_element_size;

    /* The maximum packet size is 6144 bits aka 768 bytes per channel. */
    QByteArray aacData;  aacData.resize( 6144 );
    void *out_ptr               = aacData.data();
    int out_buffer_size         = aacData.size();
    int out_buffer_element_size = 1;
    out_buf.numBufs             = 1;
    out_buf.bufs                = &out_ptr;
    out_buf.bufferIdentifiers   = &out_buffer_identifier;
    out_buf.bufSizes            = &out_buffer_size;
    out_buf.bufElSizes          = &out_buffer_element_size;

    AACENC_ERROR err;
    if ( (err = aacEncEncode(m_aacenc, &in_buf, &out_buf, &in_args, &out_args)) != AACENC_OK)
    {
            qDebug() << "aacEncEncode error" << err;
    }
    else
    {
        // TODO do w need to check pts? do we need to make sure all bytes are consumed?
//        qDebug() << "Writing audio frame" << out_args.numOutBytes;
        writeFlvTag(8, pts, pts, true, (quint8 *)aacData.data(), out_args.numOutBytes );
    }
}


void Encode::writeFlvTag(quint8 TagType, qint64 Pts, qint64 Dts, bool Keyframe, quint8 *data, int size, bool SequenceHeader)
{
    FlvTag tag = FlvTag::flvTag(TagType, Pts, Dts, Keyframe, data, size, SequenceHeader );

    switch( TagType )
    {
    case 9: m_video.append( tag ); break;
    case 8: m_audio.append( tag ); break;
    }

//    if ( ! m_file.isOpen() || 0 == m_rtmp )
//        return;

    while ( m_video.size() && m_audio.size() )
    {
        // pop the tag with the lower timestamp;
        if ( m_audio.first().flvTimeStamp() <= m_video.first().flvTimeStamp() )
             tag = m_audio.takeFirst();
        else
            tag = m_video.takeFirst();

         m_file.write( tag );
         m_rtmp.sendTag( tag );
    }
}
