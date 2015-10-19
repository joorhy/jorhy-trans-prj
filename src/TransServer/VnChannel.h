///////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2009, xx科技有限公司(版权声明) 
/// All rights reserved. 
/// 
/// @file      VnChannel.h 
/// @brief     VN_DVR通道模块
///
/// @version   1.0 (版本声明)
/// @author    Jorhy (joorhy@gmail.com) 
/// @date      2015/04/17 20:47 
///
///
/// 修订说明：最初版本
///////////////////////////////////////////////////////////////////////////  
#ifndef __VnCHANNEL_H_
#define __VnCHANNEL_H_
#include "j_includes.h"
#include "AudioConvert.h"
/// 本类的功能:  流管理类
class CVnChannel : public J_Channel
{
	struct RingBufferInfo
	{
		CRingBuffer *pRingBuffer;
		j_boolean_t isNeedIFrame;
	};
public:
	CVnChannel(J_Obj *pOwner, j_int32_t nChannel);
	~CVnChannel();

public:
	///J_Channel
	virtual j_result_t OpenStream(const CXlDataBusInfo &cmdData);
	virtual j_result_t CloseStream(const CXlDataBusInfo &cmdData);

private:
	static void CALLBACK FuncRealMsg(int nMsg, void *pUser)
	{
		(static_cast<CVnChannel *>(pUser))->DoRealMsgProc(nMsg);
	}
	void DoRealMsgProc(int nMsg);
	
	static void CALLBACK FuncRealData(const char *pFrameBuffer, int nFrameLen, int nFrameType, unsigned __int64 llFrameStamp, void *pUser)
	{
		(static_cast<CVnChannel *>(pUser))->DoRealDataProc(pFrameBuffer, nFrameLen, nFrameType, llFrameStamp);
	}
	void DoRealDataProc(const char *pFrameBuffer, int nFrameLen, int nFrameType, unsigned __int64 llFrameStamp);

	void Broken();

private:
	int AddRingBuffer(CRingBuffer *pRingBuffer)
	{
		TLock(m_vecLocker);
		RingBufferInfo info;
		info.pRingBuffer = pRingBuffer;
		info.isNeedIFrame = true;
		m_vecRingBuffer.push_back(info);
		TUnlock(m_vecLocker);

		return J_OK;
	}
	int DelRingBuffer(CRingBuffer *pRingBuffer)
	{
		TLock(m_vecLocker);
		std::vector<RingBufferInfo>::iterator it = m_vecRingBuffer.begin();
		for (; it != m_vecRingBuffer.end(); it++)
		{
			if (it->pRingBuffer == pRingBuffer)
			{
				m_vecRingBuffer.erase(it);
				TUnlock(m_vecLocker);

				return J_OK;
			}
		}
		TUnlock(m_vecLocker);
		return J_NOT_EXIST;
	}

private:
	j_string_t m_strDeviceID;
	j_int32_t m_nChannelNum;
	j_int32_t m_nStreamType;
	J_OS::TLocker_t m_vecLocker;
	std::vector<RingBufferInfo> m_vecRingBuffer;
	J_OS::TLocker_t m_mapLocker;
	std::map<j_guid_t, CRingBuffer *> m_mapRingBuffer;

	long m_lStreamHandle;
	long m_lAudioHandle;
	long long m_lastAudioTimeStamp;
	long long m_lastVideoTimeStamp;

	CAudioConvert m_audioConvert;
	j_int32_t m_nAudioLen;
	j_char_t m_pAudioBuff[2048];
};

#endif //~__VnCHANNEL_H_
