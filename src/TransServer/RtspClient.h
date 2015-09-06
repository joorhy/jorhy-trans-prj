///////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2009, xx科技有限公司(版权声明) 
/// All rights reserved. 
/// 
/// @file      Client.h 
/// @brief     客户端模块
///
/// @version   1.0 (版本声明)
/// @author    Jorhy (joorhy@gmail.com) 
/// @date      2015/04/24 15:11 
///
///
/// 修订说明：最初版本
///////////////////////////////////////////////////////////////////////////  
#ifndef __RTSP_CLIENT_H_
#define __RTSP_CLIENT_H_

#include "j_includes.h"
#include "x_socket.h"
#include "x_thread.h"
#include "RTPStream.h"
#include "RtspHelper.h"

/// 本类的功能:  客户端业务处理类
class CRTSPClient : public J_Client
{
public:
	CRTSPClient(j_socket_t nSock);
	~CRTSPClient();

public:
	///J_Client
	j_boolean_t IsReady() { return m_bIsReady; }
	j_result_t OnHandleRead(J_AsioDataBase *pAsioData);
	j_result_t OnHandleWrite(J_AsioDataBase *pAsioData);
	j_result_t OnResponse(const CXlClientRespData &respData) { return J_OK; }
	j_result_t OnMessage(j_string_t strHostId, const CXlClientRespData &respData) { return J_OK; }
	j_result_t OnBroken();

private:
	j_result_t Play();
	j_result_t Stop();

	j_uint16_t GetServerPort(j_int32_t setUpType);
	j_result_t GetSDPInfo();

	void StartWork();

private:
	static unsigned X_JO_API WorkThread(void *param)
	{
		(static_cast<CRTSPClient *>(param))->OnWork();
		return 0;
	}
	void OnWork();

private:
	j_boolean_t m_bIsReady;
	CRTSPHelper m_helper;

	j_uint32_t m_mode;
	j_uint16_t m_v_server_port;
	j_uint16_t m_a_server_port;
	SOCKET m_videoRTPSock;
	SOCKET m_videoRTCPSock;
	SOCKET m_audioRTPSock;
	SOCKET m_audioRTCPSock;
	CJoThread m_workThread;
	j_boolean_t m_bStarted;
	HANDLE m_hEvent;

	j_char_t m_pReadBuff[1024];
	j_int32_t m_nReadLen;
	j_char_t m_pWriteBuff[2048];
	j_boolean_t m_bDataReady;
	j_boolean_t m_bCmdReady;

	J_Obj *m_pChannel;
	CRingBuffer m_ringBuffer;
	CRingBuffer m_cmdRingBuffer;
	J_StreamHeader m_streamHeader;
	j_char_t m_cmdBuff[2048];
	j_char_t *m_dataBuff;
	j_char_t *m_rtpBuff;

	CRTPStream m_rtpStream;

	static volatile j_uint16_t s_server_port;
};

#endif // ~__RTSP_CLIENT_H_
