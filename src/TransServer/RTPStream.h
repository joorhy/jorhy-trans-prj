#ifndef __RTPSTREAM_H_
#define __RTPSTREAM_H_
#include "j_includes.h"
#include "x_time.h"

class rtp_helper
{
public:
	rtp_helper();
	~rtp_helper();

public:
	void set_avtime_inc(int vtime_inc, int atime_inc)
	{
		m_video_time_inc = vtime_inc;
		m_audio_time_inc = atime_inc;
	}
	int get_rtp_head(char *head, int i_length, int payload_type, j_uint64_t timeStamp, j_boolean_t is_video, j_boolean_t is_mark);

	typedef struct rtp_head_s
	{
		unsigned short rtsp_head;
		unsigned short length;

		char csrc_count : 4;
		char x : 1;
		char p : 1;
		char vertion : 2;

		char payload_type : 7;
		char m : 1;

		unsigned short seq;
		unsigned int timestamp;
		unsigned int ssrc;
	} rtp_head_t;

	typedef struct rtcp_head_s
	{
		char sc : 5;
		char p : 1;
		char vertion : 2;

		char pt;
		short length;
		int ssrc;
	} rtcp_head_t;

	int m_video_seq_num;
	int m_audio_seq_num;
	int m_video_time;
	int m_audio_time;
	int m_video_time_inc;
	int m_audio_time_inc;
	unsigned int m_video_ssrc;
	unsigned int m_audio_ssrc;
};

enum PackFlag
{
	full_pack = 0,
	begin_pack = 1,
	middle_pack = 2,
	end_pack = 3,
};

class CRTPStream
{
public:
	CRTPStream() {}
	~CRTPStream() {}

	static CRTPStream *createNew()
	{
		return new CRTPStream();;
	}

	int Convert(const char *pInputData, J_StreamHeader &streamHeader, char *pOutputData, int &nOutLen, void *pExtdata = 0);	
	int ConvertToPack(char nalType, const char *pData, int nLen, j_uint64_t timeStamp, char *pOutputData, int &nOutLen, int nFlag);
	void Analyze(const char *pData, int nLen);
	void SetAVTimeStampInc(int nVTimeStampInc, int nATimeStampInc)
	{
		helper.set_avtime_inc(nVTimeStampInc, nATimeStampInc);
	}

private:
	int PrepareAudioData(const char *pData, J_StreamHeader &streamHeader, char *pOutputData, int &nOutLen);
	int PrepareVideoData(const char *pData, J_StreamHeader &streamHeader, char *pOutputData, int &nOutLen);

public:
	typedef struct  nalu_header_s
	{
		//byte 0
		unsigned char TYPE : 5;
		unsigned char NRI : 2;
		unsigned char F : 1;
	} nalu_header_t; /**//* 1 BYTES */

	typedef struct fu_indicator_s
	{
		//byte 0
		unsigned char TYPE : 5;
		unsigned char NRI : 2;
		unsigned char F : 1;
	} fu_indicator_t; /**//* 1 BYTES */

	typedef struct fu_header_s
	{
		//byte 0
		unsigned char TYPE : 5;
		unsigned char R : 1;
		unsigned char E : 1;
		unsigned char S : 1;
	} fu_header_t; /**//* 1 BYTES */

private:
	rtp_helper helper;
};

#endif //~__RTPSTREAM_H_
