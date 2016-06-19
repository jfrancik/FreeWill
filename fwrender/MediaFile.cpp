#include "StdAfx.h"
#include "MediaFile.h"

extern "C"
{
#include <libavutil/opt.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

CMediaFile::CMediaFile() : m_pStream(NULL), m_pContext(NULL), m_psws_ctx(NULL), m_pFrame(NULL), m_pFrame2(NULL)
{
}

CMediaFile::~CMediaFile(void)
{
}

/**************************************************************/
/* video output */

AVFrame *CMediaFile::alloc_picture(enum AVPixelFormat pix_fmt, int width, int height)
{
    AVFrame *picture;

    picture = av_frame_alloc();
    if (!picture)
        return NULL;

    picture->format = pix_fmt;
    picture->width  = width;
    picture->height = height;

    /* allocate the buffers for the frame data */
    if (av_frame_get_buffer(picture, 32) < 0)
        throw MEDIA_ERROR("Could not allocate frame data.");

    return picture;
}

void CMediaFile::open_video(AVCodec *codec, AVDictionary *opt_arg)
{
    AVDictionary *opt = NULL;

    av_dict_copy(&opt, opt_arg, 0);

    /* open the codec */
    int h = avcodec_open2(m_pStream->codec, codec, &opt);
    av_dict_free(&opt);
    if (h < 0)
        throw MEDIA_ERROR("Could not open video codec");

    /* allocate and init a re-usable frame */
    m_pFrame = alloc_picture(m_pStream->codec->pix_fmt, m_pStream->codec->width, m_pStream->codec->height);
	m_pFrame->pts = 0;
    if (!m_pFrame)
        throw MEDIA_ERROR("Could not allocate video frame");

    /* If the output format is not YUV420P, then a temporary YUV420P
     * picture is needed too. It is then converted to the required
     * output format. */
    if (m_pStream->codec->pix_fmt != AV_PIX_FMT_YUV420P) 
	{
        m_pFrame2 = alloc_picture(AV_PIX_FMT_YUV420P, m_pStream->codec->width, m_pStream->codec->height);
        if (!m_pFrame2)
            throw MEDIA_ERROR("Could not allocate temporary picture");
    }
}

bool CMediaFile::get_video_frame(uint8_t **&ppData, int *&pLinesize, int64_t time)
{
	ppData = NULL;
	pLinesize = NULL;
	static AVRational AVRATIONAL_11 = { 1, 1 };
    if (time > 0 && av_compare_ts(m_pFrame->pts, m_pStream->codec->time_base, time, AVRATIONAL_11) >= 0)
		return false;

    if (m_pStream->codec->pix_fmt != AV_PIX_FMT_YUV420P) 
	{
        /* as we only generate a YUV420P picture, we must convert it to the codec pixel format if needed */
        if (!m_psws_ctx) 
		{
            m_psws_ctx = sws_getContext(m_pStream->codec->width, m_pStream->codec->height, AV_PIX_FMT_YUV420P, m_pStream->codec->width, m_pStream->codec->height, m_pStream->codec->pix_fmt, SWS_BICUBIC, NULL, NULL, NULL);
            if (!m_psws_ctx)
				throw MEDIA_ERROR("Could not initialize the conversion context");
        }
		if (av_frame_make_writable(m_pFrame2) < 0)
			throw MEDIA_ERROR("Frame write error (av_frame_make_writable)");

		ppData = m_pFrame2->data;
		pLinesize = m_pFrame2->linesize;
    } 
	else 
	{
		if (av_frame_make_writable(m_pFrame) < 0)
			throw MEDIA_ERROR("Frame write error (av_frame_make_writable)");
		ppData = m_pFrame->data;
		pLinesize = m_pFrame->linesize;
    }

    m_pFrame->pts++;

    return true;
}

//AVFrame *CMediaFile::get_video_frame()
//{
//    if (m_pStream->codec->pix_fmt != AV_PIX_FMT_YUV420P) 
//	{
//        /* as we only generate a YUV420P picture, we must convert it to the codec pixel format if needed */
//        if (!m_psws_ctx) 
//		{
//            m_psws_ctx = sws_getContext(m_pStream->codec->width, m_pStream->codec->height, AV_PIX_FMT_YUV420P, m_pStream->codec->width, m_pStream->codec->height, m_pStream->codec->pix_fmt, SWS_BICUBIC, NULL, NULL, NULL);
//            if (!m_psws_ctx)
//				throw MEDIA_ERROR("Could not initialize the conversion context");
//        }
//		if (av_frame_make_writable(m_pFrame2) < 0)
//			throw MEDIA_ERROR("Frame write error (av_frame_make_writable)");
//		fill_yuv_image(m_pFrame2->data, m_pFrame2->linesize, (int)m_pFrame->pts, m_pStream->codec->width, m_pStream->codec->height);
//        sws_scale(m_psws_ctx,(const uint8_t * const *)m_pFrame2->data, m_pFrame2->linesize, 0, m_pStream->codec->height, m_pFrame->data, m_pFrame->linesize);
//    } 
//	else 
//	{
//		if (av_frame_make_writable(m_pFrame) < 0)
//			throw MEDIA_ERROR("Frame write error (av_frame_make_writable)");
//        fill_yuv_image(m_pFrame->data, m_pFrame->linesize, (int)m_pFrame->pts, m_pStream->codec->width, m_pStream->codec->height);
//    }
//
//    m_pFrame->pts++;
//
//    return m_pFrame;
//}


// encode one video frame and send it to the muxer; return 1 if got packet
bool CMediaFile::write_video_frame()
{
	// just finish the cycle started in get_video_frame
    if (m_pStream->codec->pix_fmt != AV_PIX_FMT_YUV420P) 
        sws_scale(m_psws_ctx,(const uint8_t * const *)m_pFrame2->data, m_pFrame2->linesize, 0, m_pStream->codec->height, m_pFrame->data, m_pFrame->linesize);

    AVPacket pkt = { 0 };
    av_init_packet(&pkt);

    /* encode the image */
    int got_packet = 0;
    if (avcodec_encode_video2(m_pStream->codec, &pkt, m_pFrame, &got_packet) < 0)
        throw MEDIA_ERROR("Error encoding video frame");

	if (!got_packet) return false;

	/* rescale output packet timestamp values from codec to stream timebase */
	av_packet_rescale_ts(&pkt, m_pStream->codec->time_base, m_pStream->time_base);
	pkt.stream_index = m_pStream->index;

	/* Write the compressed frame to the media file. */
	if (av_interleaved_write_frame(m_pContext, &pkt) < 0)
	    throw MEDIA_ERROR("Error while writing video frame");

    return true;
}

// encode one video frame and send it to the muxer; return 1 if got packet
bool CMediaFile::flush_video_frame()
{
    AVPacket pkt = { 0 };
    av_init_packet(&pkt);

    /* encode the image */
    int got_packet = 0;
    if (avcodec_encode_video2(m_pStream->codec, &pkt, NULL, &got_packet) < 0)
        throw MEDIA_ERROR("Error encoding video frame");

	if (!got_packet) return false;

	/* rescale output packet timestamp values from codec to stream timebase */
	av_packet_rescale_ts(&pkt, m_pStream->codec->time_base, m_pStream->time_base);
	pkt.stream_index = m_pStream->index;

	/* Write the compressed frame to the media file. */
	if (av_interleaved_write_frame(m_pContext, &pkt) < 0)
	    throw MEDIA_ERROR("Error while writing video frame");

    return true;
}

void CMediaFile::close_stream()
{
    avcodec_close(m_pStream->codec);
    av_frame_free(&m_pFrame);
    av_frame_free(&m_pFrame2);
    sws_freeContext(m_psws_ctx);
}



void CMediaFile::open(const char *filename)
{
	m_filename = filename;

	// Initialize libavcodec, and register all codecs and formats
	av_register_all();

	// allocate the output media context
	avformat_alloc_output_context2(&m_pContext, NULL, NULL, m_filename.c_str());
	if (!m_pContext || (m_pContext->oformat->flags & AVFMT_NOFILE) != 0)
		throw MEDIA_ERROR("Failed to create the ouput media context");

	// open the output file, if needed
	if (avio_open(&m_pContext->pb, m_filename.c_str(), AVIO_FLAG_WRITE) < 0)
		throw MEDIA_ERROR("Failed to open the output file");
}


void CMediaFile::add_video_stream(int width, int height, int frameRate, int64_t bitRate, AVPixelFormat pix_fmt)
{
	// Add the video stream using the default format codecs

    /* find the encoder */
    AVCodec *pCodec = avcodec_find_encoder(m_pContext->oformat->video_codec);
    if (!pCodec) 
		throw MEDIA_ERROR(std::string("Could not find encoder for ") + avcodec_get_name(m_pContext->oformat->video_codec));

    m_pStream = avformat_new_stream(m_pContext, pCodec);
    if (!m_pStream)
		throw MEDIA_ERROR("Could not allocate stream");
    
	m_pStream->id = m_pContext->nb_streams-1;

    if (pCodec->type != AVMEDIA_TYPE_VIDEO) 
		throw MEDIA_ERROR("Wrong type of codec (video codec expected)");
    
		
	// Initialisation of the VIDEO codec
	m_pStream->codec->codec_id = m_pContext->oformat->video_codec;
    m_pStream->codec->bit_rate = bitRate;
    m_pStream->codec->width    = width;		// Resolution must be a multiple of two
    m_pStream->codec->height   = height;
    /* timebase: This is the fundamental unit of time (in seconds) in terms
        * of which frame timestamps are represented. For fixed-fps content,
        * timebase should be 1/framerate and timestamp increments should be
        * identical to 1. */
	m_pStream->time_base.num = 1;
	m_pStream->time_base.den = frameRate;
    m_pStream->codec->time_base = m_pStream->time_base;

    m_pStream->codec->gop_size      = 12; /* emit one intra frame every twelve frames at most */
    m_pStream->codec->pix_fmt       = pix_fmt;
    if (m_pStream->codec->codec_id == AV_CODEC_ID_MPEG2VIDEO)
        /* just for testing, we also add B frames */
        m_pStream->codec->max_b_frames = 2;
    if (m_pStream->codec->codec_id == AV_CODEC_ID_MPEG1VIDEO)
        /* Needed to avoid using macroblocks in which some coeffs overflow.
            * This does not happen with normal video, it just happens here as
            * the motion of the chroma plane does not match the luma plane. */
        m_pStream->codec->mb_decision = 2;

    /* Some formats want stream headers to be separate. */
    if (m_pContext->oformat->flags & AVFMT_GLOBALHEADER)
        m_pStream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;



	// Open the video codec and allocate the necessary encode buffers
	AVDictionary *opt = NULL;
	open_video(pCodec, opt);
	
	// Write the stream header, if any
	if (avformat_write_header(m_pContext, &opt) < 0) 
		throw MEDIA_ERROR("Failed to write the header");
}

void CMediaFile::disable_audio_stream()
{
	m_pContext->oformat->audio_codec = AV_CODEC_ID_NONE;
}

void CMediaFile::finish_and_close()
{
	// Flush write cycles...
	while (flush_video_frame())
		;

	// Write the trailer, if any. 
	// The trailer must be written before you close the CodecContexts open when you wrote the header; 
	// otherwise av_write_trailer() may try to use memory that was freed on av_codec_close().
	av_write_trailer(m_pContext);

	// Close the codec & the output file
	close_stream();
	avio_closep(&m_pContext->pb);

	// free the stream
	avformat_free_context(m_pContext);
}

void CMediaFile::dump()
{
	av_dump_format(m_pContext, 0, m_filename.c_str(), 1);
}
