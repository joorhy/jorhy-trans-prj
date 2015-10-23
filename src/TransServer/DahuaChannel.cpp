#include "DahuaChannel.h"
#include "DahuaHost.h"
#include "XlDataBusDef.h"

CDahuaChannel::CDahuaChannel(J_Obj *pOwner, j_int32_t nChannel)
{
	m_pHost = pOwner;
	m_nChannel = nChannel;
	m_llStreamHandle = 0;
	ANA_CreateStream(2 * 1024 * 1024, &m_parser);
}

CDahuaChannel::~CDahuaChannel()
{
	ANA_Destroy(m_parser);
}

j_result_t CDahuaChannel::OpenStream(const CXlDataBusInfo &cmdData)
{
	AddRingBuffer(cmdData.clientRequest.realPlay.pBuffer);
	CDahuaHost *pHost = dynamic_cast<CDahuaHost *>(m_pHost);
	if (pHost != NULL)
	{
		m_llStreamHandle = CLIENT_StartRealPlay(pHost->GetLoginHandle(), m_nChannel, NULL, DH_RType_Realplay_0, OnRealDataCallBackEx, OnRealPlayDisConnect, (DWORD)this);
	}

	return J_OK;
}

j_result_t CDahuaChannel::CloseStream(const CXlDataBusInfo &cmdData)
{
	if (m_llStreamHandle != 0)
	{
		CLIENT_StopRealPlay(m_llStreamHandle);
		m_llStreamHandle = 0;
	}
	DelRingBuffer(cmdData.clientRequest.realPlay.pBuffer);

	return J_OK;
}

void CDahuaChannel::DoRealPlayDisConnect(LLONG lOperateHandle, EM_REALPLAY_DISCONNECT_EVENT_TYPE dwEventType, void* param)
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

	CLIENT_StopRealPlay(lOperateHandle);
	m_llStreamHandle = 0;
}

void CDahuaChannel::DoRealDataCallBack(LLONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, LONG param)
{
	if (dwDataType == 0)
	{
		ANA_InputData(m_parser, (unsigned char *)pBuffer, dwBufSize);
		if (ANA_GetNextFrame(m_parser, &m_frame) == 0)
		{
			if (m_frame.nType == FRAME_TYPE_VIDEO)
			{
				int nOffset = 4;

				J_StreamHeader streamHeader = { 0 };
				TLock(m_vecLocker);
				std::vector<RingBufferInfo>::iterator it = m_vecRingBuffer.begin();
				for (; it != m_vecRingBuffer.end(); it++)
				{
					if (m_frame.nSubType == TYPE_VIDEO_I_FRAME)
					{
						streamHeader.frameType = jo_video_a_frame;
						streamHeader.dataLen = 9;
						it->pRingBuffer->PushBuffer((const char *)m_frame.pFrameBody + nOffset, streamHeader);
						nOffset += (9 + 4);

						streamHeader.frameType = jo_video_a_frame;
						streamHeader.dataLen = 4;
						it->pRingBuffer->PushBuffer((const char *)m_frame.pFrameBody + nOffset, streamHeader);
						nOffset += (4 + 4);

						streamHeader.frameType = jo_video_a_frame;
						streamHeader.dataLen = 5;
						it->pRingBuffer->PushBuffer((const char *)m_frame.pFrameBody + nOffset, streamHeader);
						nOffset += (5 + 4);

						it->isNeedIFrame = false;
					}
					
					if (it->isNeedIFrame == false)
					{
						streamHeader.frameType = (m_frame.nSubType == TYPE_VIDEO_I_FRAME) ? jo_video_i_frame : jo_video_p_frame;
						streamHeader.dataLen = m_frame.nBodyLength - nOffset;
						it->pRingBuffer->PushBuffer((const char *)m_frame.pFrameBody + nOffset, streamHeader);
					}
				}
				TUnlock(m_vecLocker);
			}
			else if (m_frame.nType == FRAME_TYPE_AUDIO)
			{

			}
			else
			{

			}
		}
	}
	else if (dwDataType == 1)// video
	{
		tagVideoFrameParam *frameHeader = (tagVideoFrameParam *)param;
		J_StreamHeader streamHeader = { 0 };
		streamHeader.frameType = (frameHeader->frametype == 0) ? jo_video_i_frame : jo_video_p_frame;
		streamHeader.dataLen = dwBufSize;
		TLock(m_vecLocker);
		std::vector<RingBufferInfo>::iterator it = m_vecRingBuffer.begin();
		for (; it != m_vecRingBuffer.end(); it++)
		{
			it->pRingBuffer->PushBuffer((const char *)pBuffer, streamHeader);
		}
		TUnlock(m_vecLocker);
	}
	else if (dwDataType == 3)//audio
	{
		tagCBPCMDataParam *pcmHeader = (tagCBPCMDataParam *)param;
	}
}