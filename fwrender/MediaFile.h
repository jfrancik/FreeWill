#pragma once

#include <stdint.h>
#include <string>

#include <libavutil/pixfmt.h>

struct AVStream;
struct AVFrame;
struct AVFormatContext;
struct AVRational;
struct AVStream;
struct AVPacket;
struct AVCodec;
struct AVDictionary;

struct MEDIA_ERROR
{
	std::string msg;
	MEDIA_ERROR(std::string msg)	{ this->msg = msg; }
};

class CMediaFile
{
	std::string m_filename;
	AVStream *m_pStream;
	AVFormatContext *m_pContext;
	AVFrame *m_pFrame, *m_pFrame2;
	struct SwsContext *m_psws_ctx;

	/* video output */
	AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height);
	void open_video(AVCodec *codec, AVDictionary *opt_arg);
	void close_stream();


public:
	CMediaFile(void);
	virtual ~CMediaFile(void);

	void open(const char *filename);
	void add_video_stream(int width, int height, int frameRate, int64_t bitRate = 0, AVPixelFormat pix_fmt = AV_PIX_FMT_YUV420P);
	void disable_audio_stream();

	bool get_video_frame(uint8_t **&ppData, int *&pLinesize, int64_t time = 0);
	bool write_video_frame();
	bool flush_video_frame();

	void finish_and_close();

	
	void dump();
};

