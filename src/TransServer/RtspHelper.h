#ifndef __RTSPHELPER_H__
#define __RTSPHELPER_H__
#include "j_includes.h"

class CRTSPHelper
{
public:
	enum RTSP_METHORD
	{
		RTSP_OPTIONS = 1,
		RTSP_DESCRIBE,
		RTSP_SETUP_TCP,
		RTSP_SETUP_UDP_VIDEO,
		RTSP_SETUP_UDP_AUDIO,
		RTSP_SETUP_UDP_UNSPORT,
		RTSP_PLAY,
		RTSP_PAUSE,
		RTSP_TEARDOWN
	};

	CRTSPHelper();
	~CRTSPHelper();

public:
	j_int32_t GetCseq() const { return m_nCseq; }
	j_char_t *const GetSSRC() { return m_ssrc; }
	j_string_t& const GetHostType() { return m_strHostType; }
	j_string_t& const GetHostID() { return m_strHostID; }
	j_int32_t GetChannel() const { return m_nChannel; }
	j_char_t *const GetSDP() { return m_sdp; }
	j_char_t *const GetContent() { return m_content; }
	j_char_t *const GetInterleaved() { return m_interleaved; }
	j_int32_t GetVideoPort() const { return m_video_port; }
	j_int32_t GetAudioPort() const { return m_audio_port; }

public:
	j_result_t ParserUri(const char *request);
	j_result_t ParserHeaders(const char *request);

	j_boolean_t GetPSForVdms();
	j_boolean_t GetConfigForVdms();
	j_boolean_t GetPSForXlms();
	j_boolean_t GetConfigForXlms();
	j_boolean_t GetPSForDhms();
	j_boolean_t GetConfigForDhms();

private:
	j_int32_t m_nCseq;

	j_char_t m_sps[32];
	j_char_t m_pps[32];
	j_char_t m_config[16];
	j_char_t m_profile_id[4];
	j_char_t m_ssrc[16];
	j_char_t m_client_ip[16];

	j_char_t m_content[128];
	j_char_t m_sdp[1024];

	j_string_t m_strHostType;
	j_string_t m_strHostID;
	j_int32_t m_nChannel;

	j_char_t m_interleaved[10];
	j_uint32_t m_video_ssrc;
	j_uint32_t m_audio_ssrc;

	j_uint16_t m_video_port;
	j_uint16_t m_audio_port;
};

#endif //~__RTSPHELPER_H__