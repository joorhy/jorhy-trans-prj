#include "RtspHelper.h"
#include "x_string.h"
#include "RtspDef.h"
#include "x_base64.c"

CRTSPHelper::CRTSPHelper()
{
	m_strHostType = "";

	memset(m_sps, 0, sizeof(m_sps));
	memset(m_pps, 0, sizeof(m_pps));
	memset(m_config, 0, sizeof(m_config));
	memset(m_profile_id, 0, sizeof(m_profile_id));
	memset(m_ssrc, 0, sizeof(m_ssrc));
	memset(m_client_ip, 0, sizeof(m_client_ip));

	m_video_ssrc = 10000UL;
	m_audio_ssrc = 20000UL;

	m_video_port = 0;
	m_audio_port = 0;
}

CRTSPHelper::~CRTSPHelper()
{

}

j_result_t CRTSPHelper::ParserUri(const char *request)
{
	if (m_strHostType != "")
	{
		return J_EXIST;
	}

	char *p = (char *)request;
	char *p2 = (char *)strstr(p, "rtsp://");
	if (p2 == NULL)
	{
		return J_PARAM_ERROR;
	}

	CXString x_string(p2 + strlen("rtsp://"));

	CXChar_n str_host_type("/", "?");
	x_string >> str_host_type;

	CXChar_n str_host_id("ID=", "&");
	x_string >> str_host_id;

	CXInteger32 i_channel("chn=", " ");
	x_string >> i_channel;

	m_strHostType = str_host_type();
	m_strHostID = str_host_id();
	m_nChannel = i_channel();

	return J_OK;
}

j_result_t CRTSPHelper::ParserHeaders(const char *request)
{
	char *p = (char *)request;
	char *p2 = (char *)strstr(p, " ");
	if (p2 == NULL)
	{
		return J_PARAM_ERROR;
	}

	char methord[20] = { 0 };
	memcpy(methord, p, p2 - p);

	m_nCseq = 0;
	p = ++p2;
	p2 = (char *)strstr(p, "CSeq:");
	if (p2 == NULL)
	{
		return J_PARAM_ERROR;
	}

	p = p2 + strlen("CSeq:");
	p2 = (char *)strstr(p, "\r\n");
	if (p2 == NULL)
	{
		return J_PARAM_ERROR;
	}

	char cseq[10] = { 0 };
	memcpy(cseq, p, p2 - p);
	m_nCseq = atoi(cseq);

	if (memcmp(methord, "OPTIONS", strlen("OPTIONS")) == 0)
	{
		return RTSP_OPTIONS;
	}
	else if (memcmp(methord, "DESCRIBE", strlen("DESCRIBR")) == 0)
	{
		p = (char *)request;
		p2 = (char *)strstr(p, " ");
		if (p2 == NULL)
		{
			return J_PARAM_ERROR;
		}

		p = ++p2;
		p2 = (char *)strstr(p, " ");
		if (p2 == NULL)
		{
			return J_PARAM_ERROR;
		}

		memset(m_content, 0, sizeof(m_content));
		memcpy(m_content, p, p2 - p);
		p = m_content;
		p2 = (char *)strstr(p, "//");
		if (p2 == NULL)
		{
			return J_PARAM_ERROR;
		}

		p = p2 + strlen("//");
		p2 = (char *)strstr(p, ":");
		if (p2 == NULL)
		{
			p2 = (char *)strstr(p, "/");
			if (p2 == NULL)
			{
				return J_PARAM_ERROR;
			}
		}

		char str_ip[16] = { 0 };
		memcpy(str_ip, p, p2 - p);
		sprintf(m_sdp, live_sdp, "127.0.0.1", m_profile_id[0] & 0xFF, m_profile_id[1] & 0xFF
			, m_profile_id[2] & 0xFF, m_sps, m_pps, m_config);

		return RTSP_DESCRIBE;
	}
	else if (memcmp(methord, "SETUP", strlen("SETUP")) == 0)
	{
		p = (char *)request;
		p2 = (char *)strstr(p, "trackID=");
		if (p2 == NULL)
		{
			return J_PARAM_ERROR;
		}

		p = p2 + strlen("trackID=");
		p2 = (char *)strstr(p, " ");
		if (p2 == NULL)
		{
			return J_PARAM_ERROR;
		}

		char track_id[4] = { 0 };
		memcpy(track_id, p, p2 - p);
		int nTrackId = atoi(track_id);
		if (nTrackId == VIDEO_TRACK_ID)
		{
			sprintf(m_ssrc, "%X", m_video_ssrc);
		}
		else
		{
			sprintf(m_ssrc, "%X", m_audio_ssrc);
		}

		j_result_t nResult = RTSP_SETUP_UDP_UNSPORT;
		if (strstr(request, "TCP") == NULL)
		{
			p2 = (char *)strstr(p, "client_port=");
			if (p2 == NULL)
			{
				return J_PARAM_ERROR;
			}

			p = p2 + strlen("client_port=");
			p2 = (char *)strstr(p, "-");
			if (p2 == NULL)
			{
				return J_PARAM_ERROR;
			}

			char client_port[10] = { 0 };
			memcpy(client_port, p, p2 - p);
			int nPort = atoi(client_port);
			if (atoi(track_id) == VIDEO_TRACK_ID)
			{
				m_video_port = nPort;
#ifdef __SPORT_UDP__
				nResult = RTSP_SETUP_UDP_VIDEO;
#endif
			}
			else
			{
				m_audio_port = nPort;
#ifdef __SPORT_UDP__
				nResult = RTSP_SETUP_UDP_AUDIO;
#endif
			}
		}
		else
		{
			p2 = (char *)strstr(p, "interleaved=");
			if (p2 == NULL)
			{
				return J_PARAM_ERROR;
			}

			p = p2 + strlen("interleaved=");
			p2 = strstr(p, "\r\n");
			if (p2 == NULL)
			{
				return J_PARAM_ERROR;
			}

			memset(m_interleaved, 0, sizeof(m_interleaved));
			memcpy(m_interleaved, p, p2 - p);

			nResult = RTSP_SETUP_TCP;
		}
		return nResult;
	}
	else if (memcmp(methord, "PLAY", strlen("PLAY")) == 0)
	{
		p = (char *)request;
		p2 = (char *)strstr(p, " ");
		if (p2 == NULL)
		{
			return J_PARAM_ERROR;
		}

		p = ++p2;
		p2 = (char *)strstr(p, " ");
		if (p2 == NULL)
		{
			return J_PARAM_ERROR;
		}

		memset(m_content, 0, sizeof(m_content));
		memcpy(m_content, p, p2 - p);

		return RTSP_PLAY;
	}
	else if (memcmp(methord, "PAUSE", strlen("PAUSE")) == 0)
	{
		return RTSP_PAUSE;
	}
	else if (memcmp(methord, "TEARDOWN", strlen("TEARDOWN")) == 0)
	{
		return RTSP_TEARDOWN;
	}

	return J_OK;
}

j_boolean_t CRTSPHelper::GetPSForVdms()
{
	m_profile_id[0] = 0x42;
	m_profile_id[1] = 0xE0;
	m_profile_id[2] = 0x14;
	char sps[] = { 0x00, 0x00, 0x00, 0x01, 0x67, 0x42, 0xE0, 0x14,
		0xDB, 0x05, 0x82, 0x51 };
	char pps[] = { 0x00, 0x00, 0x00, 0x01, 0x68, 0xCE, 0x30, 0xA4, 0x80 };
	base64_in((unsigned char *)(sps + 4), m_sps, 8);
	base64_in((unsigned char *)(pps + 4), m_pps, 5);

	return true;
}

bool CRTSPHelper::GetConfigForVdms()
{
	char header[] = { 0xFF, 0xF1, 0x2C, 0x40, 0x11, 0xBF, 0xFC };
	char *fixedHeader = header;

	if (!(((fixedHeader[0] & 0xFF) == 0xFF) && ((fixedHeader[1] & 0xF0) == 0xF0)))
		return false;

	j_uint8_t profile = (fixedHeader[2] & 0xC0) >> 6;
	if (profile == 3)
	{
		J_OS::LOGINFO("Bad profile: 3 in first frame of ADTS");
		return false;
	}

	j_uint8_t sampling_frequency_index = (fixedHeader[2] & 0x3C) >> 2;
	if (samplingFrequencyTable[sampling_frequency_index] == 0)
	{
		J_OS::LOGINFO("Bad sampling_frequency_index: in first frame of ADTS");
		return false;
	}

	j_uint8_t channel_configuration = ((fixedHeader[2] & 0x01) << 2) | ((fixedHeader[3] & 0xC0) >> 6);

	j_uint8_t audioSpecificConfig[2] = { 0 };
	j_uint8_t audioObjectType = profile + 1;
	audioSpecificConfig[0] = (audioObjectType << 3) | (sampling_frequency_index >> 1);
	audioSpecificConfig[1] = (sampling_frequency_index << 7) | (channel_configuration << 3);
	sprintf(m_config, "%02X%02X", audioSpecificConfig[0] & 0xFF, audioSpecificConfig[1] & 0xFF);

	return true;
}

j_boolean_t CRTSPHelper::GetPSForXlms()
{
	m_profile_id[0] = 0x4D;
	m_profile_id[1] = 0x00;
	m_profile_id[2] = 0x1F;
	char sps[] = { 0x00, 0x00, 0x00, 0x01, 0x67, 0x4D, 0x00, 0x1F,
		0x95, 0xA8, 0x14, 0x01, 0x6E, 0x84, 0x00, 0x00, 0x1C, 0x20, 0x00, 0x05, 0x7E, 0x40, 0x10 };
	char pps[] = { 0x00, 0x00, 0x00, 0x01, 0x68, 0xEE, 0x3C, 0x80 };
	base64_in((unsigned char *)(sps + 4), m_sps, 19);
	base64_in((unsigned char *)(pps + 4), m_pps, 4);

	return true;
}

j_boolean_t CRTSPHelper::GetConfigForXlms()
{
	return true;
}

j_boolean_t CRTSPHelper::GetPSForDhms()
{
	m_profile_id[0] = 0x4D;
	m_profile_id[1] = 0x00;
	m_profile_id[2] = 0x1E;
	char sps[] = { 0x00, 0x00, 0x00, 0x01, 0x67, 0x4D, 0x00, 0x1E,
		0x95, 0xA8, 0x2C, 0x04, 0x99 };
	char pps[] = { 0x00, 0x00, 0x00, 0x01, 0x68, 0xCE, 0x3C, 0x80 };
	base64_in((unsigned char *)(sps + 4), m_sps, 9);
	base64_in((unsigned char *)(pps + 4), m_pps, 4);

	return true;
}

j_boolean_t CRTSPHelper::GetConfigForDhms()
{
	return true;
}