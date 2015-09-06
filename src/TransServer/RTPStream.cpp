#include "RTPStream.h"

#define MAX_RTP_DATA_LENGTH	1400

rtp_helper::rtp_helper()
{
	m_video_seq_num = 0;
	m_audio_seq_num = 0;
	m_audio_time = 0;
	m_video_time = 0;
	m_video_time_inc = 22500; //3600;
	m_audio_time_inc = 1024;  //5760;
	m_video_ssrc = 10000UL;
	m_audio_ssrc = 10001UL;
}

rtp_helper::~rtp_helper()
{

}

int rtp_helper::get_rtp_head(char *p_head, int i_length, int payload_type, j_uint64_t timeStamp, j_boolean_t is_video, j_boolean_t is_mark)
{
	rtp_head_t *head = (rtp_head_t *)p_head;
	head->vertion = 2;
	head->p = 0;
	head->x = 0;
	head->csrc_count = 0;

	if (is_mark)
		head->m = 1;
	else
		head->m = 0;

	head->payload_type = (payload_type & 0x7f);
	if (is_video)
	{
		head->seq = htons(m_video_seq_num);
		head->timestamp = htonl(m_video_time);
		//head->timestamp = htonl(CTime::Instance()->GetLocalTime(0) * 90);
		//printf("%d\n", timeStamp);
		//head->timestamp = htonl(timeStamp * 90);
		if (is_mark)
			m_video_time += m_video_time_inc;

		head->ssrc = htonl(m_video_ssrc);
		head->rtsp_head = htons(0x2400);
		m_video_seq_num++;
	}
	else
	{
		head->seq = htons(m_audio_seq_num);
		head->timestamp = htonl(m_audio_time);
		//CTime time;
		//head->timestamp = htonl(time.GetLocalTime(0) * 16);
		//head->timestamp = htonl(timeStamp * 16);
		if (is_mark)
			m_audio_time += m_audio_time_inc;
		head->ssrc = htonl(m_audio_ssrc);
		head->rtsp_head = htons(0x2402);
		m_audio_seq_num++;
	}
	head->length = htons((unsigned short)i_length);

	return 0;
}

int CRTPStream::Convert(const char *pInputData, J_StreamHeader &streamHeader, char *pOutputData, int &nOutLen, void *pExtdata)
{
	if (streamHeader.frameType == jo_audio_frame)
	{
		PrepareAudioData(pInputData, streamHeader, pOutputData, nOutLen);
	}
	else
	{
		PrepareVideoData(pInputData, streamHeader, pOutputData, nOutLen);
	}

	return J_OK;
}

int CRTPStream::ConvertToPack(char nalType, const char *pData, int nLen, j_uint64_t timeStamp, char *pOutputData, int &nOutLen, int nFlag)
{
	CRTPStream::nalu_header_t *nalu_hdr;
	CRTPStream::fu_indicator_t *fu_ind;
	CRTPStream::fu_header_t *fu_hdr;

	int forbidden_bit = nalType & 0x80;
	int nal_reference_idc = nalType & 0x60;
	int nal_unit_type = (nalType) & 0x1f;
	int adjust_len = (nFlag == begin_pack) ? 1 : 0;
	if (nFlag == full_pack)
	{
		char *data = pOutputData;
		memset(data, 0, nLen + 16);
		helper.get_rtp_head(data, nLen + 12, 96, timeStamp, true, true);

		nalu_hdr = (nalu_header_t*)&data[16];
		nalu_hdr->F = forbidden_bit;
		nalu_hdr->NRI = nal_reference_idc >> 5;
		nalu_hdr->TYPE = nal_unit_type;

		memcpy(data + 17, pData + 1, nLen - 1);
		nOutLen = nLen + 16;
	}
	else if (nFlag == end_pack)
	{
		char *data = pOutputData;
		memset(data, 0, nLen);
		helper.get_rtp_head(data, nLen + 14, 96, timeStamp, true, true);

		fu_ind = (fu_indicator_t *)&data[16];
		fu_ind->F = forbidden_bit;
		fu_ind->NRI = nal_reference_idc >> 5;
		fu_ind->TYPE = 28;

		//设置FU HEADER,并将这个HEADER填入sendbuf[13]
		fu_hdr = (fu_header_t *)&data[17];
		fu_hdr->R = 0;
		fu_hdr->S = 0;
		fu_hdr->E = 1;
		fu_hdr->TYPE = nal_unit_type;
		memcpy(data + 18, pData, nLen);
		nOutLen = nLen + 18;
	}
	else
	{
		char *data = pOutputData;
		memset(data, 0, nLen + 18 - adjust_len);
		helper.get_rtp_head(data, nLen + 14 - adjust_len, 96, timeStamp, true, false);

		fu_ind = (fu_indicator_t *)&data[16];
		fu_ind->F = forbidden_bit;
		fu_ind->NRI = nal_reference_idc >> 5;
		fu_ind->TYPE = 28;

		//设置FU HEADER,并将这个HEADER填入sendbuf[17]
		fu_hdr = (fu_header_t *)&data[17];
		fu_hdr->E = 0;
		fu_hdr->R = 0;
		if (nFlag == begin_pack)
		{
			fu_hdr->S = 1;
		}
		else fu_hdr->S = 0;
		fu_hdr->TYPE = nal_unit_type;
		memcpy(data + 18, pData + adjust_len, nLen - adjust_len);
		nOutLen = nLen + 18 - adjust_len;
	}

	return J_OK;
}

void CRTPStream::Analyze(const char *pData, int nLen)
{
	rtp_helper::rtcp_head_t *rtcpHead = (rtp_helper::rtcp_head_t *)pData;
	J_OS::LOGDEBUG("rtcp version = %d p = %d sc = %d type = %d, length = %d", rtcpHead->vertion & 0x03, 
		rtcpHead->p &0x01, rtcpHead->sc & 0x1F, rtcpHead->pt & 0xFF, ntohs(rtcpHead->length));
}

int CRTPStream::PrepareAudioData(const char *pData, J_StreamHeader &streamHeader, char *pOutputData, int &nOutLen)
{
	int i_totle_len = streamHeader.dataLen;
	int i_aac_len = 0;
	int i_offset = 0;
	char *pAacData = NULL;

	int nOffsetOut = 0;
	while (i_totle_len > 0)
	{
		i_aac_len = 0;
		pAacData = (char *)(pData + i_offset);
		i_aac_len |= pAacData[3] & 0x03;
		i_aac_len <<= 8;
		i_aac_len |= pAacData[4] & 0xff;
		i_aac_len <<= 3;
		i_aac_len |= (pAacData[5] >> 5) & 0x07;
		i_offset += i_aac_len;
		i_totle_len -= i_aac_len;
		i_aac_len -= 7;

		char *data = pOutputData;//new char[i_aac_len + 20];
		memset(data, 0, i_aac_len + 20);
		helper.get_rtp_head(data, i_aac_len + 16, 97, streamHeader.timeStamp, false, true);
		data[16] = 0x00;
		data[17] = 0x10;
		data[18] = (i_aac_len >> 5) & 0xff;
		data[19] = (i_aac_len << 3) & 0xf8;
		memcpy(data + 20, pAacData + 7, i_aac_len);
		//memcpy(pOutputData + nOffsetOut, data, i_aac_len + 20);
		nOffsetOut += i_aac_len + 20;
		//delete data;
	}
	nOutLen = nOffsetOut;

	return J_OK;
}

int CRTPStream::PrepareVideoData(const char *pData, J_StreamHeader &streamHeader, char *pOutputData, int &nOutLen)
{
	if (streamHeader.dataLen <= 0)
	{
		nOutLen = 0;
		return J_OK;
	}
	int nTotleLen = streamHeader.dataLen;
	int nSendLen = 0;

	nalu_header_t *nalu_hdr;
	fu_indicator_t *fu_ind;
	fu_header_t *fu_hdr;

	int forbidden_bit = pData[0] & 0x80;
	int nal_reference_idc = pData[0] & 0x60;
	int nal_unit_type = (pData[0]) & 0x1f;
	bool b_first_packet = true;
	int adjust_len = 1;

	int nOffsetOut = 0;
	if (nTotleLen <= MAX_RTP_DATA_LENGTH)
	{
		char *data = pOutputData + nOffsetOut;//new char[nLen + 16];
		memset(data, 0, streamHeader.dataLen + 16);
		helper.get_rtp_head(data, streamHeader.dataLen + 12, 96, streamHeader.timeStamp, true, (streamHeader.frameType != jo_video_a_frame));

		nalu_hdr = (nalu_header_t*)&data[16];
		nalu_hdr->F = forbidden_bit;
		nalu_hdr->NRI = nal_reference_idc >> 5;
		nalu_hdr->TYPE = nal_unit_type;

		memcpy(data + 17, pData + 1, streamHeader.dataLen - 1);
		//memcpy(pOutputData + nOffsetOut, data, nLen + 16);
		nOffsetOut += streamHeader.dataLen + 16;
		//delete data;

		nSendLen += streamHeader.dataLen;
		nTotleLen -= streamHeader.dataLen;
	}
	else
	{
		while (nTotleLen > 0)
		{
			if (nTotleLen > MAX_RTP_DATA_LENGTH)
			{
				if (b_first_packet) adjust_len = 1;
				else adjust_len = 0;

				char *data = pOutputData + nOffsetOut;//new char[MAX_RTP_DATA_LENGTH + 18 - adjust_len];
				memset(data, 0, MAX_RTP_DATA_LENGTH + 18 - adjust_len);
				helper.get_rtp_head(data, MAX_RTP_DATA_LENGTH + 14 - adjust_len, 96, streamHeader.timeStamp, true, false);

				fu_ind = (fu_indicator_t *)&data[16];
				fu_ind->F = forbidden_bit;
				fu_ind->NRI = nal_reference_idc >> 5;
				fu_ind->TYPE = 28;

				//设置FU HEADER,并将这个HEADER填入sendbuf[17]
				fu_hdr = (fu_header_t *)&data[17];
				fu_hdr->E = 0;
				fu_hdr->R = 0;
				if (b_first_packet)
				{
					fu_hdr->S = 1;
					b_first_packet = false;
				}
				else fu_hdr->S = 0;
				fu_hdr->TYPE = nal_unit_type;
				memcpy(data + 18, pData + nSendLen + adjust_len, MAX_RTP_DATA_LENGTH - adjust_len);
				//memcpy(pOutputData + nOffsetOut, data, MAX_RTP_DATA_LENGTH + 18 - adjust_len);
				nOffsetOut += MAX_RTP_DATA_LENGTH + 18 - adjust_len;
				//delete data;

				nSendLen += MAX_RTP_DATA_LENGTH;
				nTotleLen -= MAX_RTP_DATA_LENGTH;
			}
			else
			{
				char *data = pOutputData + nOffsetOut;//new char[nTotleLen + 18];
				memset(data, 0, nTotleLen + 18);
				helper.get_rtp_head(data, nTotleLen + 14, 96, streamHeader.timeStamp, true, true);

				fu_ind = (fu_indicator_t *)&data[16];
				fu_ind->F = forbidden_bit;
				fu_ind->NRI = nal_reference_idc >> 5;
				fu_ind->TYPE = 28;

				//设置FU HEADER,并将这个HEADER填入sendbuf[13]
				fu_hdr = (fu_header_t *)&data[17];
				fu_hdr->R = 0;
				fu_hdr->S = 0;
				fu_hdr->E = 1;
				fu_hdr->TYPE = nal_unit_type;
				memcpy(data + 18, pData + nSendLen, nTotleLen);
				//memcpy(pOutputData + nOffsetOut, data, nTotleLen + 18);
				nOffsetOut += nTotleLen + 18;
				//delete data;

				nSendLen += nTotleLen;
				nTotleLen -= nTotleLen;
			}
		}
	}
	nOutLen = nOffsetOut;

	return J_OK;
}