///////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2009, xx科技有限公司(版权声明) 
/// All rights reserved. 
/// 
/// @file      DahuaChannel.h 
/// @brief     VN_DVR通道模块
///
/// @version   1.0 (版本声明)
/// @author    Jorhy (joorhy@gmail.com) 
/// @date      2015/07/17 20:47 
///
///
/// 修订说明：最初版本
///////////////////////////////////////////////////////////////////////////  
#ifndef __VnCHANNEL_H_
#define __VnCHANNEL_H_
#include "j_includes.h"
#include "dhnetsdk.h"
#include "StreamAnalyzer.h"

/// 本类的功能:  流管理类
class CDahuaChannel : public J_Channel
{
	struct RingBufferInfo
	{
		CRingBuffer *pRingBuffer;
		j_boolean_t isNeedIFrame;
	};
public:
	CDahuaChannel(J_Obj *pOwner, j_int32_t nChannel);
	~CDahuaChannel();

public:
	///J_Channel
	virtual j_result_t OpenStream(const CXlDataBusInfo &cmdData);
	virtual j_result_t CloseStream(const CXlDataBusInfo &cmdData);

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
	static void CALLBACK OnRealDataCallBackEx(LLONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, LONG param, LDWORD dwUser)
	{
		CDahuaChannel *pThis = reinterpret_cast<CDahuaChannel *>(dwUser);
		pThis->DoRealDataCallBack(lRealHandle, dwDataType, pBuffer, dwBufSize, param);
	}
	void DoRealDataCallBack(LLONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, LONG param);
	static void CALLBACK OnRealPlayDisConnect(LLONG lOperateHandle, EM_REALPLAY_DISCONNECT_EVENT_TYPE dwEventType, void* param, LDWORD dwUser)
	{
		CDahuaChannel *pThis = reinterpret_cast<CDahuaChannel *>(dwUser);
		pThis->DoRealPlayDisConnect(lOperateHandle, dwEventType, param);
	}
	void DoRealPlayDisConnect(LLONG lOperateHandle, EM_REALPLAY_DISCONNECT_EVENT_TYPE dwEventType, void* param);

private:
	J_Obj *m_pHost;
	J_OS::TLocker_t m_vecLocker;
	std::vector<RingBufferInfo> m_vecRingBuffer;

	j_int64_t m_llStreamHandle;
	j_int32_t m_nChannel;

	HANDLE m_parser;
	ANA_FRAME_INFO m_frame;
};

#endif //~__VnCHANNEL_H_
