///////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2009, xx科技有限公司(版权声明) 
/// All rights reserved. 
/// 
/// @file      MobileClient.cpp 
/// @brief     客户端处理模块实现
///
/// @version   1.0 (版本声明)
/// @author    Jorhy (joorhy@gmail.com) 
/// @date      2015/04/24 15:13 
///
///
/// 修订说明：最初版本
///////////////////////////////////////////////////////////////////////////  
#include "RtspClient.h"
#include "RtspDef.h"
#include "DeviceManager.h"
#include "XlDataBusDef.h"

#include "VnHost.h"
#include "VnChannel.h"

#include <iostream>
#include <strstream>

#define MAX_RTP_DATA_LENGTH 1200
#define BUFFER_SIZE (1024 * 1024)

//#define __SPORT_UDP__

volatile j_uint16_t CRTSPClient::s_server_port = 20000;

CRTSPClient::CRTSPClient(j_socket_t nSock)
	: m_cmdRingBuffer(100 * 1024)
{
	m_pChannel = NULL;

	m_dataBuff = (j_char_t *)malloc(BUFFER_SIZE);
	m_rtpBuff = (j_char_t *)malloc(BUFFER_SIZE);

	memset(m_pReadBuff, 0, sizeof(m_pReadBuff));
	m_nReadLen = 0;

	m_mode = CRTSPHelper::RTSP_SETUP_TCP;

	m_v_server_port = 0;
	m_a_server_port = 0;

	m_videoRTPSock = INVALID_SOCKET;
	m_videoRTCPSock = INVALID_SOCKET;
	m_audioRTPSock = INVALID_SOCKET;
	m_audioRTCPSock = INVALID_SOCKET;

	m_bStarted = false;
	m_bDataReady = false;
	m_bCmdReady = false;
	m_bIsReady = true;
}

CRTSPClient::~CRTSPClient()
{
	OnBroken();

	if (m_bStarted)
	{
		m_bStarted = false;
		WaitForSingleObject(m_hEvent, 1000);
		m_workThread.Release();
		CloseHandle(m_hEvent);
	}

	if (m_dataBuff != NULL)
	{
		free(m_dataBuff);
		m_dataBuff = NULL;
	}

	if (m_rtpBuff != NULL)
	{
		free(m_rtpBuff);
		m_rtpBuff = NULL;
	}
}

j_result_t CRTSPClient::OnHandleRead(J_AsioDataBase *pAsioData)
{
	if (strstr(m_pReadBuff, rtsp_end) == NULL)
	{
		pAsioData->ioRead.buf = m_pReadBuff;
		pAsioData->ioRead.bufLen = 1024;
		pAsioData->ioRead.finishedLen = 0;
		pAsioData->ioRead.whole = false;
		pAsioData->ioRead.shared = true;
		pAsioData->ioCall = J_AsioDataBase::j_read_e;
		strcpy(pAsioData->ioRead.until_buf, rtsp_end);

		return J_NOT_COMPLATE;
	}

	J_OS::LOGDEBUG(m_pReadBuff);

	j_result_t nResult = J_OK;
	memset(m_pWriteBuff, 0, sizeof(m_pWriteBuff));
	if (m_helper.ParserUri(m_pReadBuff) == J_OK)
	{
		GetSDPInfo();
	}
	
	switch (m_helper.ParserHeaders(m_pReadBuff))
	{
	case CRTSPHelper::RTSP_OPTIONS:
		sprintf(m_pWriteBuff, ret_options, m_helper.GetCseq());
		break;
	case CRTSPHelper::RTSP_DESCRIBE:
		sprintf(m_pWriteBuff, ret_describ, m_helper.GetCseq(), this, strlen(m_helper.GetSDP()), m_helper.GetContent());
		memcpy(m_pWriteBuff + strlen(m_pWriteBuff), m_helper.GetSDP(), strlen(m_helper.GetSDP()));
		break;
	case CRTSPHelper::RTSP_SETUP_TCP:
		m_mode = CRTSPHelper::RTSP_SETUP_TCP;
		sprintf(m_pWriteBuff, ret_setup_tcp, m_helper.GetCseq(), this, m_helper.GetInterleaved(), m_helper.GetSSRC());
		break;
	case CRTSPHelper::RTSP_SETUP_UDP_VIDEO:
		StartWork();
		m_mode = CRTSPHelper::RTSP_SETUP_UDP_VIDEO;
		m_v_server_port = GetServerPort(CRTSPHelper::RTSP_SETUP_UDP_VIDEO);
		sprintf(m_pWriteBuff, ret_setup_udp, m_helper.GetCseq(), m_helper.GetVideoPort(), m_helper.GetVideoPort() + 1, m_v_server_port, m_v_server_port + 1, this);
		break;
	case CRTSPHelper::RTSP_SETUP_UDP_AUDIO:
		StartWork();
		m_mode = CRTSPHelper::RTSP_SETUP_UDP_AUDIO;
		m_a_server_port = GetServerPort(CRTSPHelper::RTSP_SETUP_UDP_AUDIO);
		sprintf(m_pWriteBuff, ret_setup_udp, m_helper.GetCseq(), m_helper.GetAudioPort(), m_helper.GetAudioPort() + 1, m_a_server_port, m_a_server_port + 1, this);
		break;
	case CRTSPHelper::RTSP_SETUP_UDP_UNSPORT:
		memcpy(m_pWriteBuff, ret_setup_unsport, strlen(ret_setup_unsport));
		break;
	case CRTSPHelper::RTSP_PAUSE:
		sprintf(m_pWriteBuff, ret_pause, m_helper.GetCseq(), this);
		break;
	case CRTSPHelper::RTSP_TEARDOWN:
		m_bIsReady = false;
		nResult = Stop();
		sprintf(m_pWriteBuff, ret_teardown, m_helper.GetCseq(), this);
		break;
	case CRTSPHelper::RTSP_PLAY:
		nResult = Play();
		sprintf(m_pWriteBuff, ret_play, m_helper.GetCseq(), this, m_helper.GetContent(), m_helper.GetContent());
		break;
	}

	if (nResult != J_OK)
	{
		m_bIsReady = false;
		return J_SOCKET_ERROR;
	}
	else
	{
		memset(m_pReadBuff, 0, sizeof(m_pReadBuff));
		J_StreamHeader streamHeader = { 0 };
		streamHeader.dataLen = strlen(m_pWriteBuff);
		m_cmdRingBuffer.PushBuffer(m_pWriteBuff, streamHeader);

		J_OS::LOGDEBUG(m_pWriteBuff);

		pAsioData->ioRead.buf = m_pReadBuff;
		pAsioData->ioRead.bufLen = 1024;
		pAsioData->ioRead.finishedLen = 0;
		pAsioData->ioRead.whole = false;
		pAsioData->ioRead.shared = true;
		pAsioData->ioCall =  J_AsioDataBase::j_read_e;
	}

	return J_OK;
}

j_result_t CRTSPClient::OnHandleWrite(J_AsioDataBase *pAsioData)
{
	memset(&m_streamHeader, 0, sizeof(J_StreamHeader));
	memset(m_cmdBuff, 0, sizeof(m_cmdBuff));
	pAsioData->ioCall =  J_AsioDataBase::j_write_e;
	pAsioData->ioWrite.bufLen = 0;
	if (m_cmdRingBuffer.PopBuffer(m_cmdBuff, m_streamHeader) == J_OK)
	{
		pAsioData->ioWrite.buf = m_cmdBuff;
		pAsioData->ioWrite.bufLen = m_streamHeader.dataLen;
		pAsioData->ioWrite.finishedLen = 0;
		pAsioData->ioWrite.whole = true;
		pAsioData->ioWrite.shared = true;
		if (strstr(m_cmdBuff, "RTP-Info:") != NULL)
		{
			m_bCmdReady = true;
		}
	}
	else if (m_mode == CRTSPHelper::RTSP_SETUP_TCP)
	{
		memset(m_dataBuff, 0, BUFFER_SIZE);
		if (m_ringBuffer.PopBuffer(m_dataBuff, m_streamHeader) != J_OK)
		{
			if ((m_bDataReady == false) && m_bCmdReady)
			{
				m_streamHeader.dataLen = 256;
				j_sleep(250);
			}
		}
		else
		{
			m_bDataReady = true;
		}

		int nRtpLen = 0;
		if (m_streamHeader.frameType == jo_media_broken)
		{
			m_bIsReady = false;
		}
		else
		{
			if (m_streamHeader.dataLen > 0)
			{
				m_rtpStream.Convert(m_dataBuff, m_streamHeader, m_rtpBuff, nRtpLen);
			}
		}
		pAsioData->ioWrite.buf = m_rtpBuff;
		pAsioData->ioWrite.bufLen = nRtpLen;
		pAsioData->ioWrite.finishedLen = 0;
		pAsioData->ioWrite.whole = true;
		pAsioData->ioWrite.shared = true;

		if (nRtpLen > 0)
		{
			J_OS::LOGDEBUG("DataLen = %d", nRtpLen);
		}
	}
	else
	{
	}

	return J_OK;
}

j_result_t CRTSPClient::OnBroken()
{
	Stop();

	if (m_videoRTPSock != INVALID_SOCKET)
	{
		closesocket(m_videoRTPSock);
		m_videoRTPSock = INVALID_SOCKET;
	}

	if (m_videoRTCPSock != INVALID_SOCKET)
	{
		closesocket(m_videoRTCPSock);
		m_videoRTCPSock = INVALID_SOCKET;
	}

	if (m_audioRTPSock != INVALID_SOCKET)
	{
		closesocket(m_audioRTPSock);
		m_audioRTPSock = INVALID_SOCKET;
	}

	if (m_audioRTCPSock != INVALID_SOCKET)
	{
		closesocket(m_audioRTCPSock);
		m_audioRTCPSock = INVALID_SOCKET;
	}

	J_OS::LOGINFO("CRTSPClient::OnBroken()");

	return J_OK;
}

j_result_t CRTSPClient::Play()
{
	int nResult = J_UNKNOW;
	J_Host *pHost = JoDeviceManager->GetDeviceObj(m_helper.GetHostID());
	if (pHost != NULL)
	{
		CXlDataBusInfo cmdData = { 0 };
		cmdData.header.cmd = CXlProtocol::xlc_real_play;
		cmdData.header.flag = CXlProtocol::xl_ctrl_start;
		strcpy(cmdData.clientRequest.realPlay.hostId, m_helper.GetHostID().c_str());
		cmdData.clientRequest.realPlay.channel = m_helper.GetChannel();
		cmdData.clientRequest.realPlay.pBuffer = &m_ringBuffer;
		nResult = pHost->OnRequest(cmdData);
	}

	return nResult;
}

j_result_t CRTSPClient::Stop()
{
	int nResult = J_UNKNOW;
	J_Host *pHost = JoDeviceManager->GetDeviceObj(m_helper.GetHostID());
	if (pHost != NULL)
	{
		CXlDataBusInfo cmdData = { 0 };
		cmdData.header.cmd = CXlProtocol::xlc_real_play;
		cmdData.header.flag = CXlProtocol::xl_ctrl_stop;
		strcpy(cmdData.clientRequest.realPlay.hostId, m_helper.GetHostID().c_str());
		cmdData.clientRequest.realPlay.channel = m_helper.GetChannel();
		cmdData.clientRequest.realPlay.pBuffer = &m_ringBuffer;
		nResult = pHost->OnRequest(cmdData);
	}

	return nResult;
}

j_uint16_t CRTSPClient::GetServerPort(j_int32_t setUpType)
{
	if (setUpType == CRTSPHelper::RTSP_SETUP_UDP_VIDEO)
	{
		m_videoRTPSock = socket(AF_INET, SOCK_DGRAM, 0);
		SOCKADDR_IN addrRTPSrv;
		addrRTPSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
		addrRTPSrv.sin_family = AF_INET;
		addrRTPSrv.sin_port = htons(s_server_port);
		while (bind(m_videoRTPSock, (SOCKADDR*)&addrRTPSrv, sizeof(SOCKADDR)) != 0)
		{
			s_server_port += 2;
			if (s_server_port >= 65535)
			{
				s_server_port = 20000;
			}
			addrRTPSrv.sin_port = htons(s_server_port);
		}
		
		J_OS::LOGDEBUG("Video Server port = %d-%d", s_server_port, s_server_port + 1);

		m_videoRTCPSock = socket(AF_INET, SOCK_DGRAM, 0);
		SOCKADDR_IN addrRTCPSrv;
		addrRTCPSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
		addrRTCPSrv.sin_family = AF_INET;
		addrRTCPSrv.sin_port = htons(s_server_port + 1);
		if (bind(m_videoRTCPSock, (SOCKADDR*)&addrRTCPSrv, sizeof(SOCKADDR)) != 0)
		{
			J_OS::LOGDEBUG("Video RTCP bind Error");
		}
	}
	else
	{
		m_audioRTPSock = socket(AF_INET, SOCK_DGRAM, 0);
		SOCKADDR_IN addrRTPSrv;
		addrRTPSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
		addrRTPSrv.sin_family = AF_INET;
		addrRTPSrv.sin_port = htons(s_server_port);
		while (bind(m_audioRTPSock, (SOCKADDR*)&addrRTPSrv, sizeof(SOCKADDR)) != 0)
		{
			s_server_port += 2;
			if (s_server_port >= 65535)
			{
				s_server_port = 20000;
			}
			addrRTPSrv.sin_port = htons(s_server_port);
		}

		J_OS::LOGDEBUG("Audio Server port = %d-%d", s_server_port, s_server_port + 1);

		m_audioRTCPSock = socket(AF_INET, SOCK_DGRAM, 0);
		SOCKADDR_IN addrRTCPSrv;
		addrRTCPSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
		addrRTCPSrv.sin_family = AF_INET;
		addrRTCPSrv.sin_port = htons(s_server_port + 1);
		if (bind(m_audioRTCPSock, (SOCKADDR*)&addrRTCPSrv, sizeof(SOCKADDR)) != 0)
		{
			J_OS::LOGDEBUG("Audio RTCP bind Error");
		}
	}

	return s_server_port;
}

j_result_t CRTSPClient::GetSDPInfo()
{
	if (m_helper.GetHostType() == "vdms")
	{
		m_helper.GetPSForVdms();
		m_helper.GetConfigForVdms();
		m_rtpStream.SetAVTimeStampInc(22500, 1024);
	}
	else if (m_helper.GetHostType() == "xlms")
	{
		m_helper.GetPSForXlms();
		m_helper.GetConfigForXlms();
		m_rtpStream.SetAVTimeStampInc(3600, 5760);
	}
	else if (m_helper.GetHostType() == "dhms")
	{
		m_helper.GetPSForDhms();
		m_helper.GetConfigForDhms();
		m_rtpStream.SetAVTimeStampInc(3600, 5760);
	}

	return J_OK;
}

void CRTSPClient::StartWork()
{
	if (m_bStarted == false)
	{
		m_bStarted = true;
		m_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		j_thread_parm parm = { 0 };
		parm.entry = CRTSPClient::WorkThread;
		parm.data = this;
		m_workThread.Create(parm);
	}
}

void CRTSPClient::OnWork()
{
	fd_set rfd;						// 读描述符集  
	fd_set wfd;						// 写描述符集 
	timeval timeout;				// 定时变量 
	sockaddr_in addrVideoRTP;		// 告诉sock 应该在什么地方licence  
	sockaddr_in addrAudioRTP;		// 告诉sock 应该在什么地方licence  
	char recv_buf[1024];			// 接收缓冲区 
	int nRet;						// select返回值 

	int nReadLen = 0;
	int nRecLen;					// 客户端地址长度
	sockaddr_in cli;				// 客户端地址  

	j_boolean_t bVideoSocketReady = false;
	j_boolean_t bAudioSocketReady = false;
	J_StreamHeader streamHeader = {0};

	// 设置超时时间为100ms  
	timeout.tv_sec = 0;
	timeout.tv_usec = 10;
	while (m_bStarted)
	{
		memset(recv_buf, 0, sizeof(recv_buf));
		FD_ZERO(&rfd);				// 在使用之前总是要清空  
		FD_ZERO(&wfd);				// 在使用之前总是要清空  

		// 开始使用select 
		if (m_videoRTPSock != INVALID_SOCKET)
		{
			FD_SET(m_videoRTPSock, &rfd);	
			FD_SET(m_videoRTPSock, &wfd);
		}
		 
		if (m_videoRTCPSock != INVALID_SOCKET)
		{
			FD_SET(m_videoRTCPSock, &rfd);
		}
		
		if (m_audioRTPSock != INVALID_SOCKET)
		{
			FD_SET(m_audioRTPSock, &rfd);
			FD_SET(m_audioRTPSock, &wfd);
		}

		if (m_audioRTCPSock != INVALID_SOCKET)
		{
			FD_SET(m_audioRTCPSock, &rfd);
		}

		nRet = select(5, &rfd, &wfd, NULL, &timeout);		// 检测是否有套接口是否可读
		if (nRet == SOCKET_ERROR)
		{
			J_OS::LOGDEBUG("CRTSPClient::OnWork() select error");
		}
		else if (nRet == 0) // 超时  
		{
			J_OS::LOGDEBUG("CRTSPClient::OnWork() select time out");
		}
		else    // 检测到有套接口可读  
		{
			if (FD_ISSET(m_videoRTPSock, &rfd))  
			{
				nRecLen = sizeof(addrVideoRTP);
				if (recvfrom(m_videoRTPSock, recv_buf, sizeof(recv_buf), 0, (sockaddr*)&addrVideoRTP, &nRecLen) == INVALID_SOCKET)
				{
					J_OS::LOGDEBUG("Video RTP recvfrom Error");
				}
				else
				{
					J_OS::LOGDEBUG("Video RTP port = %d (%d)", m_helper.GetVideoPort(), ntohs(addrVideoRTP.sin_port));
					bVideoSocketReady = true;
				}
			}

			if (FD_ISSET(m_videoRTCPSock, &rfd))
			{
				nRecLen = sizeof(cli);
				if ((nReadLen = recvfrom(m_videoRTCPSock, recv_buf, sizeof(recv_buf), 0, (sockaddr*)&cli, &nRecLen)) == INVALID_SOCKET)
				{
					J_OS::LOGDEBUG("Video RTCP recvfrom Error");
				}
				else
				{
					J_OS::LOGDEBUG("Video RTCP port = %d (%d) readLen = %d", m_helper.GetVideoPort() + 1, ntohs(cli.sin_port), nReadLen);
					m_rtpStream.Analyze(recv_buf, nReadLen);
				}
			}

			if (FD_ISSET(m_audioRTPSock, &rfd))
			{
				nRecLen = sizeof(addrAudioRTP);
				if (recvfrom(m_audioRTPSock, recv_buf, sizeof(recv_buf), 0, (sockaddr*)&addrAudioRTP, &nRecLen) == INVALID_SOCKET)
				{
					J_OS::LOGDEBUG("Audio RTP recvfrom Error");
				}
				else
				{
					J_OS::LOGDEBUG("Audio RTP port = %d (%d)", m_helper.GetAudioPort(), ntohs(addrAudioRTP.sin_port));
					bAudioSocketReady = true;
				}
			}

			if (FD_ISSET(m_audioRTCPSock, &rfd))
			{
				nRecLen = sizeof(cli);
				if ((nReadLen = recvfrom(m_audioRTCPSock, recv_buf, sizeof(recv_buf), 0, (sockaddr*)&cli, &nRecLen)) == INVALID_SOCKET)
				{
					J_OS::LOGDEBUG("Audio RTCP recvfrom Error");
				}
				else
				{
					J_OS::LOGDEBUG("Audio RTCP port = %d (%d) readLen = %d", m_helper.GetAudioPort() + 1, ntohs(cli.sin_port), nReadLen);
					m_rtpStream.Analyze(recv_buf, nReadLen);
				}
			}

			if (m_bCmdReady && (FD_ISSET(m_videoRTPSock, &wfd) || FD_ISSET(m_audioRTPSock, &wfd)))
			{
				memset(m_dataBuff, 0, sizeof(BUFFER_SIZE));
				memset(&streamHeader, 0, sizeof(J_StreamHeader));
				if (m_ringBuffer.PopBuffer(m_dataBuff, streamHeader) != J_OK)
				{
					if (m_bDataReady == false)
					{
						streamHeader.dataLen = 256;
						streamHeader.frameType = jo_video_i_frame;
						j_sleep(250);
					}
				}
				else
				{
					m_bDataReady = true;
				}

				if (streamHeader.frameType == jo_media_broken)
				{
					m_bIsReady = false;
				}
				else
				{
					if (streamHeader.dataLen > 0)
					{
						int nRtpLen = 0;
						if (bAudioSocketReady && (streamHeader.frameType == jo_audio_frame))
						{
							m_rtpStream.Convert(m_dataBuff, streamHeader, m_rtpBuff, nRtpLen);
							sendto(m_audioRTPSock, m_rtpBuff + 4, nRtpLen - 4, 0, (sockaddr *)&addrAudioRTP, sizeof(addrAudioRTP));
						}
						else if (bVideoSocketReady && (streamHeader.frameType == jo_video_i_frame || 
							streamHeader.frameType == jo_video_p_frame || streamHeader.frameType == jo_video_b_frame))
						{
							if (streamHeader.dataLen <= MAX_RTP_DATA_LENGTH)
							{
								nRtpLen = 0;
								m_rtpStream.ConvertToPack((m_dataBuff[0] & 0xFF), m_dataBuff, streamHeader.dataLen, streamHeader.timeStamp, m_rtpBuff, nRtpLen, full_pack);
								sendto(m_videoRTPSock, m_rtpBuff + 4, nRtpLen - 4, 0, (sockaddr *)&addrVideoRTP, sizeof(addrVideoRTP));
							}
							else
							{
								int nTotleLen = streamHeader.dataLen;
								int nSendLen = 0;
								bool b_first_packet = true;

								while (m_bStarted && nTotleLen > 0)
								{
									nRtpLen = 0;
									memset(m_rtpBuff, 0, BUFFER_SIZE);
									if (nTotleLen > MAX_RTP_DATA_LENGTH)
									{
										if (b_first_packet)
										{
											m_rtpStream.ConvertToPack((m_dataBuff[0] & 0xFF), m_dataBuff + nSendLen, MAX_RTP_DATA_LENGTH, streamHeader.timeStamp, m_rtpBuff, nRtpLen, begin_pack);
											b_first_packet = false;
										}
										else
										{
											m_rtpStream.ConvertToPack((m_dataBuff[0] & 0xFF), m_dataBuff + nSendLen, MAX_RTP_DATA_LENGTH, streamHeader.timeStamp, m_rtpBuff, nRtpLen, middle_pack);
										}
										nSendLen += MAX_RTP_DATA_LENGTH;
										nTotleLen -= MAX_RTP_DATA_LENGTH;
									}
									else
									{
										m_rtpStream.ConvertToPack((m_dataBuff[0] & 0xFF), m_dataBuff + nSendLen, nTotleLen, streamHeader.timeStamp, m_rtpBuff, nRtpLen, end_pack);
										nSendLen += nTotleLen;
										nTotleLen -= nTotleLen;
									}

									if (nRtpLen > 0)
									{
										sendto(m_videoRTPSock, m_rtpBuff + 4, nRtpLen - 4, 0, (sockaddr *)&addrVideoRTP, sizeof(addrVideoRTP));
									}
								}
							}

							if (streamHeader.dataLen > 0)
							{
								J_OS::LOGDEBUG("UDP DataLen = %d", streamHeader.dataLen);
							}
						}					
					}
				}
			}
		}
	}

	SetEvent(m_hEvent);
}