///////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2009, xx科技有限公司(版权声明) 
/// All rights reserved. 
/// 
/// @file      VnChannel.cpp 
/// @brief     
///
/// @version   1.0 (版本声明)
/// @author    Jorhy (joorhy@gmail.com) 
/// @date      2015/04/22 17:41 
///
///
/// 修订说明：最初版本
/////////////////////////////////////////////////////////////////////////// 
#include "VnChannel.h"
#include "netmediaapi.h"
#include "GPSDeviceDef.h"
#include "VnHost.h"
#include "XlDataBusDef.h"

CVnChannel::CVnChannel(J_Obj *pOwner, j_int32_t nChannel)
	: m_lStreamHandle(NULL)
	, m_lAudioHandle(NULL)
{
	m_nChannelNum = nChannel;
	m_nStreamType = 1;//nStreamType;
	m_lastAudioTimeStamp = 0;
	m_lastVideoTimeStamp = 0;
	dynamic_cast<CVnHost *>(pOwner)->GetHostId(m_strDeviceID);

	m_audioConvert.Init();
}

CVnChannel::~CVnChannel()
{
	m_audioConvert.Deinit();
}

j_result_t CVnChannel::OpenStream(const CXlDataBusInfo &cmdData)
{
	AddRingBuffer(cmdData.clientRequest.realPlay.pBuffer);

	if (m_lStreamHandle == NULL)
	{
		if (NETMEDIA_OpenRealPlay(m_strDeviceID.c_str(), m_nChannelNum, m_nStreamType, 0, TRUE, &m_lStreamHandle) != NETMEDIA_OK)
		{
			J_OS::LOGINFO("NETMEDIA_OpenRealPlay error");
		}
		if (NETMEDIA_SetRealMsgCallBack(m_lStreamHandle, this, FuncRealMsg) != NETMEDIA_OK)
		{
			J_OS::LOGINFO("NETMEDIA_SetRealMsgCallBack error");
		}
		if (NETMEDIA_SetRealDataCallBack(m_lStreamHandle, this, FuncRealData) != NETMEDIA_OK)
		{
			J_OS::LOGINFO("NETMEDIA_SetRealDataCallBack error");
		}
		if (NETMEDIA_StartRealPlay(m_lStreamHandle, NULL) != NETMEDIA_OK)
		{
			J_OS::LOGINFO("NETMEDIA_StartRealPlay error");
		}

		NETMEDIA_PlaySound(m_lStreamHandle, 1);
	}

	return J_OK;
}

j_result_t CVnChannel::CloseStream(const CXlDataBusInfo &cmdData)
{
	if (m_lStreamHandle != NULL)
	{
		NETMEDIA_SetRealMsgCallBack(m_lStreamHandle, 0, NULL);
		NETMEDIA_SetRealDataCallBack(m_lStreamHandle, 0, NULL);
		NETMEDIA_StopRealPlay(m_lStreamHandle);
		NETMEDIA_CloseRealPlay(m_lStreamHandle);
		m_lStreamHandle = NULL;
	}
	DelRingBuffer(cmdData.clientRequest.realPlay.pBuffer);

	return J_OK;
}

void CVnChannel::Broken()
{
	J_StreamHeader streamHeader = { 0 };
	streamHeader.frameType = jo_media_broken;
	TLock(m_vecLocker);
	std::vector<RingBufferInfo>::iterator it = m_vecRingBuffer.begin();
	for (; it != m_vecRingBuffer.end(); it++)
	{
		it->pRingBuffer->PushBuffer(NULL, streamHeader);
	}
	TUnlock(m_vecLocker);
}

void CVnChannel::DoRealMsgProc(int nMsg)
{
	switch (nMsg)
	{
	case NETMEDIA_REAL_MSG_SUCCESS:
		J_OS::LOGINFO("NETMEDIA_REAL_MSG_SUCCESS");
		break;
	default:
		Broken();
		J_OS::LOGINFO("default n_msg = %d", nMsg);
		break;
	}
}

void CVnChannel::DoRealDataProc(const char *pFrameBuffer, int nFrameLen, int nFrameType, unsigned __int64 llFrameStamp)
{
	if (nFrameType == GPS_FRM_TYPE_HEAD)
	{
	}
	else if (nFrameType == GPS_FRM_TYPE_I || nFrameType == GPS_FRM_TYPE_P)
	{
		int nOffset = 4;
		J_StreamHeader streamHeader = { 0 };
		if (m_lastVideoTimeStamp == 0)
		{
			streamHeader.timeStamp = 0;
			m_lastVideoTimeStamp = llFrameStamp;
		}
		else
		{
			streamHeader.timeStamp = ((llFrameStamp - m_lastVideoTimeStamp) / 1000) + 1;
		}

		TLock(m_vecLocker);
		std::vector<RingBufferInfo>::iterator it = m_vecRingBuffer.begin();
		for (; it != m_vecRingBuffer.end(); it++)
		{
			if (nFrameType == GPS_FRM_TYPE_I)
			{
				streamHeader.frameType = jo_video_a_frame;
				streamHeader.dataLen = 8;
				it->pRingBuffer->PushBuffer(pFrameBuffer + nOffset, streamHeader);
				nOffset += (8 + 4);

				streamHeader.frameType = jo_video_a_frame;
				streamHeader.dataLen = 5;
				it->pRingBuffer->PushBuffer(pFrameBuffer + nOffset, streamHeader);
				nOffset += (5 + 4);

				streamHeader.frameType = jo_video_a_frame;
				streamHeader.dataLen = 5;
				it->pRingBuffer->PushBuffer(pFrameBuffer + nOffset, streamHeader);
				nOffset += (5 + 4);

				it->isNeedIFrame = false;
			}

			if (it->isNeedIFrame == false)
			{
				streamHeader.frameType = (nFrameType == GPS_FRM_TYPE_I) ? jo_video_i_frame : jo_video_p_frame;
				streamHeader.dataLen = nFrameLen - nOffset;
				it->pRingBuffer->PushBuffer(pFrameBuffer + nOffset, streamHeader);
			}
		}
		TUnlock(m_vecLocker);
	}
	else if (nFrameType == GPS_FRM_TYPE_A)
	{
		m_nAudioLen = 0;
		memset(m_pAudioBuff, 0, sizeof(m_pAudioBuff));
		if (m_audioConvert.Convert(pFrameBuffer, nFrameLen, m_pAudioBuff, m_nAudioLen) == J_OK)
		{
			J_StreamHeader streamHeader = { 0 };
			if (m_lastAudioTimeStamp == 0)
			{
				streamHeader.timeStamp = 0;
				m_lastAudioTimeStamp = llFrameStamp;
			}
			else
			{
				streamHeader.timeStamp = ((llFrameStamp - m_lastAudioTimeStamp) / 1000) + 1;
				//J_OS::LOGINFO("audio timestamp = %d", streamHeader.timeStamp);
			}
			streamHeader.frameType = jo_audio_frame;

			TLock(m_vecLocker);
			std::vector<RingBufferInfo>::iterator it = m_vecRingBuffer.begin();
			for (; it != m_vecRingBuffer.end(); it++)
			{
				if (it->isNeedIFrame == false)
				{
					streamHeader.dataLen = m_nAudioLen;
					it->pRingBuffer->PushBuffer(m_pAudioBuff, streamHeader);
				}
			}
			TUnlock(m_vecLocker);
		}
	}
}
