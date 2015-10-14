///////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2009, xx科技有限公司(版权声明) 
/// All rights reserved. 
/// 
/// @file      XlHost.cpp 
/// @brief     XL_DVR设备模块实现
///
/// @version   1.0 (版本声明)
/// @author    Jorhy (joorhy@gmail.com) 
/// @date      2013/09/22 17:41 
///
///
/// 修订说明：最初版本
/////////////////////////////////////////////////////////////////////////// 
#include "XlHost.h"
#include "XlChannel.h"
#include "MsSqlServer.h"
#include "MySQLAccess.h"
#include "XlHelper.h"
#include "ClientManager.h"
#include "TcpServer4Device.h"
#include "DataBus.h"
#include "XlHostCmdData.h"

#include <iostream>
#include <strstream> 

extern CTcpServer4Device g_deviceServer;

CXlHost::CXlHost(j_string_t strHostId, j_socket_t nSock)
{
	m_socket = nSock;

	m_bReady = true;
	m_hostId = strHostId;
	m_lastBreatTime = time(0);

	m_readBuff = new char[BUFFER_SIZE];			
	m_writeBuff = new char[BUFFER_SIZE];	
	m_dataBuffer = new char[BUFFER_SIZE];

	m_ioState = CXlProtocol::xl_init_state;
}

CXlHost::~CXlHost()
{
	OnBroken();

	if (m_readBuff != NULL)
	{
		delete m_readBuff;
		m_readBuff = NULL;
	}
	if (m_writeBuff != NULL)
	{
		delete m_writeBuff;
		m_writeBuff = NULL;
	}
	if (m_dataBuffer != NULL)
	{
		delete m_dataBuffer;
		m_dataBuffer = NULL;
	}

	CXlHostRespData::RespHostInfo hostInfo = {0};
	strcpy(hostInfo.hostId, m_hostId.c_str());
	JoDataBaseObj->UpdateDevInfo(hostInfo, false);
	JoDataBus->RegisterDevice(m_hostId, NULL);

	J_OS::LOGINFO("CXlHost::~CXlHost() destruct");
}

j_result_t CXlHost::CreateChannel(const j_int32_t nChannelNum, J_Obj *&pObj)
{
	J_Obj *pChannel = NULL;
	ChannelMap::iterator it = m_channelMap.find(nChannelNum);
	if (it != m_channelMap.end())
	{
		pChannel = it->second.pChannel;
	}
	else
	{
		pChannel = new CXlChannel(this, nChannelNum);

		TLock(m_channelLocker);
		ChannelInfo info = {0};
		info.pChannel = pChannel;
		info.nRef = 1;
		m_channelMap[nChannelNum] = info;
		TUnlock(m_channelLocker);
	}
	pObj = pChannel;
	return J_OK;
}

j_boolean_t CXlHost::IsReady()
{
	//if (time(0) - m_lastBreatTime > 120)
	//{
	//	m_bReady = false;
	//	J_OS::LOGINFO("CXlHost::IsReady() TIME_OUT %d - %d", time(0) - m_lastBreatTime);
	//}
	m_bReady = true;
	return m_bReady;
}

j_result_t CXlHost::OnBroken()
{
	if (m_socket.sock != j_invalid_socket_val)
	{
		j_close_socket(m_socket.sock);
		m_socket.sock = j_invalid_socket_val;
	}
	return J_OK;
}

j_result_t CXlHost::GetHostId(j_string_t &strDevId)
{
	strDevId = m_hostId;
	return J_OK;
}

j_result_t CXlHost::OnHandleRead(J_AsioDataBase *pAsioData)
{
	// 收到数据表示连接正常
	m_lastBreatTime = time(0);
	if (m_ioState == CXlProtocol::xl_init_state)
	{
		CXlHelper::MakeRequest(CXlProtocol::xld_get_dvr_info, CXlProtocol::xl_ctrl_start, JoDataBus->GetUniqueSeq(), NULL, 0, (j_char_t *)m_dataBuffer);
		J_StreamHeader streamHeader = { 0 };
		streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + sizeof(CXlProtocol::CmdTail);
		m_ringBuffer.PushBuffer(m_dataBuffer, streamHeader);

		pAsioData->ioCall = J_AsioDataBase::j_read_e;
		CXlHelper::MakeNetData(pAsioData, m_readBuff, sizeof(CXlProtocol::CmdHeader));

		m_ioState = CXlProtocol::xl_read_head_state;
	}
	else if (m_ioState == CXlProtocol::xl_read_head_state)
	{
		CXlProtocol::CmdHeader cmdHeader = *((CXlProtocol::CmdHeader *)m_readBuff);
		CXlHelper::MakeNetData(pAsioData, m_readBuff + sizeof(CXlProtocol::CmdHeader), cmdHeader.length + sizeof(CXlProtocol::CmdTail));
		pAsioData->ioCall = J_AsioDataBase::j_read_e;

		if (CXlProtocol::xld_start_real_play == cmdHeader.cmd)
		{
			J_OS::LOGDEBUG("read_head cmd = %d len = %d", cmdHeader.cmd, cmdHeader.length);
		}

		m_ioState = CXlProtocol::xl_read_data_state;
	}
	else if (m_ioState == CXlProtocol::xl_read_data_state)
	{
		CXlHostRespData *pRespData = (CXlHostRespData *)m_readBuff;
		if (CXlProtocol::xld_start_vod_download == pRespData->respHeader.cmd)
		{
			J_OS::LOGINFO("read_data cmd = %d flag = %d", pRespData->respHeader.cmd, pRespData->respHeader.flag);
		}
		switch (pRespData->respHeader.cmd)
		{
		case CXlProtocol::xld_message:
			OnMessage(*pRespData);
			break;
		case CXlProtocol::xld_heartbeat:
			OnHeartBeat(*pRespData);
			break;
		case CXlProtocol::xld_server_ready:
			OnPrepare(*pRespData);
			break;
		case CXlProtocol::xld_alarm_info:
			OnAlarmInfo(*pRespData);
			break;
		case CXlProtocol::xld_start_real_play:
			assert(pRespData->respHeader.length + sizeof(CXlProtocol::CmdTail) == pAsioData->ioRead.finishedLen);
			OnRealData(pRespData);
			break;
		case CXlProtocol::xld_start_vod_play:
		case CXlProtocol::xld_start_vod_download:
			OnVodData(pRespData);
			break;
		case CXlProtocol::xld_conrrent_time:
			OnConrrectTime(*pRespData);
			break;
		case CXlProtocol::xld_get_dvr_info:
			OnHostInfo(*pRespData);
			break;
		case CXlProtocol::xld_get_on_off_log:
			OnOnOffInfo(*pRespData);
			break;
		case CXlProtocol::xld_update_vod_info:
			OnUpdateVodInfo(*pRespData);
			break;			
		case CXlProtocol::xld_talk_data_out:
			OnTalkBackData(*pRespData);
			break;
		default:
			OnResponse(*pRespData);
			break;
		}
		CXlHelper::MakeNetData(pAsioData, m_readBuff, sizeof(CXlProtocol::CmdHeader));
		pAsioData->ioCall = J_AsioDataBase::j_read_e;
		m_ioState = CXlProtocol::xl_read_head_state;
	}

	return J_OK;
}

j_result_t CXlHost::OnHandleWrite(J_AsioDataBase *pAsioData)
{
	memset(&m_streamHeader, 0, sizeof(J_StreamHeader));
	m_ringBuffer.PopBuffer(m_writeBuff, m_streamHeader);
	pAsioData->ioWrite.buf = m_writeBuff;
	pAsioData->ioWrite.bufLen = m_streamHeader.dataLen;
	pAsioData->ioWrite.finishedLen = 0;
	pAsioData->ioWrite.whole = true;
	pAsioData->ioWrite.shared = true;

	if (m_streamHeader.dataLen > 0)
	{
		J_OS::LOGINFO("CXlHost::OnHandleWrite send len = %d", m_streamHeader.dataLen);
	}

	return J_OK;
}

j_result_t CXlHost::OnRequest(const CXlClientCmdData &cmdData)
{
	switch (cmdData.cmdHeader.cmd)
	{
	case CXlProtocol::xlc_real_play:
		if (cmdData.cmdHeader.flag == CXlProtocol::xl_ctrl_start)
		{
			StartRealPlay(cmdData);
		}
		else
		{
			StopRealPlay(cmdData);
		}
		break;
	case CXlProtocol::xlc_vod_play:
	case CXlProtocol::xlc_vod_download:
		if (cmdData.cmdHeader.flag == CXlProtocol::xl_ctrl_start)
		{
			StartVod(cmdData);
		}
		else
		{
			StopVod(cmdData);
		}
		break;
	case CXlProtocol::xlc_trans_context:
			break;
	case CXlProtocol::xlc_upload_file:
			break;
	case CXlProtocol::xlc_talk_cmd_out:
	case CXlProtocol::xlc_talk_data_out:
		break;
	}
	return J_OK;
}

j_result_t CXlHost::OnMessage(const CXlHostRespData &cmdData)
{
	CXlClientRespData data = { 0 };
	data.respHeader = cmdData.respHeader;
	memcpy(&data.respHeader, &cmdData.respHeader, sizeof(CXlClientRespData::RespMessage));

	JoDataBus->OnMessage(m_hostId, data);
	return J_OK;
}

j_result_t CXlHost::OnHeartBeat(const CXlHostRespData &cmdData)
{
	m_lastBreatTime = time(0);
	return J_OK;
}

j_result_t CXlHost::OnPrepare(const CXlHostRespData &respData)
{
	CXlHostCmdData hostCmdData = {0};
	time_t logLastTime = JoDataBaseObj->GetDevLogLastTime(m_hostId.c_str());
	if (logLastTime == 0)
	{
		hostCmdData.cmdOnOffInfo.tmStart = -1;
		hostCmdData.cmdOnOffInfo.tmEnd = -1;
	}
	else
	{
		hostCmdData.cmdOnOffInfo.tmStart = logLastTime;
		hostCmdData.cmdOnOffInfo.tmEnd = -1;
	}

	CXlHelper::MakeRequest(CXlProtocol::xld_get_on_off_log,  CXlProtocol::xl_ctrl_start, JoDataBus->GetUniqueSeq(), hostCmdData.pData, sizeof(CXlHostCmdData::CmdOnOffInfo), m_dataBuffer);

	J_StreamHeader streamHeader = {0};
	memset(&streamHeader, 0, sizeof(J_StreamHeader));
	streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + sizeof(CXlHostCmdData::CmdOnOffInfo) + sizeof(CXlProtocol::CmdTail);
	m_ringBuffer.PushBuffer(m_dataBuffer, streamHeader);

	return J_OK;
}

j_result_t CXlHost::OnAlarmInfo(const CXlHostRespData &respData)
{
	if (m_bReady)
	{
		JoDataBaseObj->InsertAlarmInfo(m_hostId.c_str(), respData.respAlarmInfo);

		CXlClientRespData data = { 0 };
		data.respHeader = respData.respHeader;
		strcpy(data.respAlarmInfo.szID, m_hostId.c_str());
		data.respAlarmInfo.tmTimeStamp = respData.respAlarmInfo.tmTimeStamp;
		data.respAlarmInfo.bAlarm = respData.respAlarmInfo.bAlarm;
		data.respAlarmInfo.dLongitude = respData.respAlarmInfo.dLongitude;
		data.respAlarmInfo.dLatitude = respData.respAlarmInfo.dLatitude;
		data.respAlarmInfo.dGPSSpeed = respData.respAlarmInfo.dGPSSpeed;
		JoDataBus->OnMessage(m_hostId, data);
	}

	return J_OK;
}

j_result_t CXlHost::OnRealData(const CXlHostRespData *respData)
{
	TLock(m_channelLocker);
	J_OS::LOGINFO("OnRealData channel = %d %d", respData->respRealData.channel & 0xFF, respData->respHeader.flag);
	ChannelMap::iterator it = m_channelMap.find(respData->respRealData.channel & 0xFF);
	if (it != m_channelMap.end())
	{
		CXlChannel *pXlChannel = dynamic_cast<CXlChannel *>(it->second.pChannel);
		if (pXlChannel != NULL)
		{
			pXlChannel->InputData(CXlProtocol::xlc_real_play, respData);
		}

		if (respData->respHeader.flag == CXlProtocol::xl_ctrl_end || respData->respHeader.flag == CXlProtocol::xl_ctrl_stop)
		{
			if (pXlChannel != NULL)
			{
				CXlClientCmdData *cmdData = (CXlClientCmdData *)respData;
				pXlChannel->CloseStream(*cmdData);
			}

			if (respData->respHeader.flag == CXlProtocol::xl_ctrl_end)
			{
				it->second.nRef--;
				if (it->second.nRef < 0)
				{
					it->second.nRef = 0;
				}
			}

			if (it->second.nRef == 0)
			{
				ReleaseChannel(respData->respRealData.channel);
			}
		}
	}
	TUnlock(m_channelLocker);

	return J_OK;
}

j_result_t CXlHost::OnTalkBack(const CXlHostRespData &respData)
{
	return J_OK;
}

j_result_t CXlHost::OnResponse(const CXlHostRespData &respData)
{
	CXlClientRespData data = { 0 };
	data.respHeader = respData.respHeader;
	JoDataBus->Response(data);
	return J_OK;
}

j_result_t CXlHost::OnVodData(const CXlHostRespData *respData)
{
	TLock(m_channelLocker);
	ChannelMap::iterator it = m_channelMap.find(respData->respVodData.channel & 0xFF);
	if (it != m_channelMap.end())
	{
		CXlChannel *pXlChannel = dynamic_cast<CXlChannel *>(it->second.pChannel);
		if (pXlChannel != NULL)
		{
			pXlChannel->InputData(CXlProtocol::xl_ctrl_stream, respData);
		}

		if (respData->respHeader.flag == CXlProtocol::xl_ctrl_end || respData->respHeader.flag == CXlProtocol::xl_ctrl_stop)
		{
			if (pXlChannel != NULL)
			{
				CXlClientCmdData *cmdData = (CXlClientCmdData *)respData;
				pXlChannel->CloseVod(*cmdData);
			}

			if (respData->respHeader.flag == CXlProtocol::xl_ctrl_end)
			{
				it->second.nRef--;
				if (it->second.nRef < 0)
				{
					it->second.nRef = 0;
				}
			}

			if (it->second.nRef == 0)
			{
				ReleaseChannel(respData->respVodData.channel);
			}
		}
	}
	TUnlock(m_channelLocker);

	return J_OK;
}

j_result_t CXlHost::OnConrrectTime(const CXlHostRespData &respData)
{
	JoDataBus->RegisterDevice(m_hostId, this);
	//JoDataBus->OnMessage(m_hostId, xlc_msg_host, xlc_host_connected);
	CXlHelper::MakeRequest(CXlProtocol::xld_server_ready,  CXlProtocol::xl_ctrl_start, JoDataBus->GetUniqueSeq(), NULL, 0, m_dataBuffer);

	J_StreamHeader streamHeader = {0};
	memset(&streamHeader, 0, sizeof(J_StreamHeader));
	streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + sizeof(CXlProtocol::CmdTail);
	m_ringBuffer.PushBuffer(m_dataBuffer, streamHeader);

	return J_OK;
}

j_result_t CXlHost::OnHostInfo(const CXlHostRespData &respData)
{
	JoDataBaseObj->UpdateDevInfo(respData.respHostInfo, true);

	CXlHostCmdData hostCmdData = { 0 };
	hostCmdData.cmdConrrectTime.systime = time(0);
	CXlHelper::MakeRequest(CXlProtocol::xld_conrrent_time, CXlProtocol::xl_ctrl_start, JoDataBus->GetUniqueSeq(), 
		(char *)hostCmdData.pData, sizeof(CXlHostCmdData::CmdConrrectTime), m_dataBuffer);

	J_StreamHeader streamHeader = { 0 };
	streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + sizeof(CXlHostCmdData::CmdConrrectTime) + sizeof(CXlProtocol::CmdTail);
	m_ringBuffer.PushBuffer(m_dataBuffer, streamHeader);

	return J_OK;
}

j_result_t CXlHost::OnOnOffInfo(const CXlHostRespData &respData)
{
	JoDataBaseObj->InsertLogInfo(m_hostId.c_str(), respData.respOnOffInfo.tmStart, respData.respOnOffInfo.tmEnd);
	return J_OK;
}

j_result_t CXlHost::OnUpdateVodInfo(const CXlHostRespData &respData)
{
	JoDataBaseObj->InsertLogInfo(m_hostId.c_str(), respData.respVodInfo.tmStart, respData.respVodInfo.tmEnd);

	CXlHostCmdData hostCmdData = { 0 };
	CXlHelper::MakeResponse(CXlProtocol::xld_update_vod_info, CXlProtocol::xl_ctrl_end, respData.respHeader.seq,
		(char *)&hostCmdData.cmdUpdateVodAck, sizeof(CXlHostCmdData::CmdUpdateVodAck), m_dataBuffer);

	J_StreamHeader streamHeader = { 0 };
	streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + sizeof(CXlHostCmdData::CmdUpdateVodAck) + sizeof(CXlProtocol::CmdTail);
	m_ringBuffer.PushBuffer(m_dataBuffer, streamHeader);	

	return J_OK;
}

j_result_t CXlHost::StartRealPlay(const CXlClientCmdData &cmdData)
{
	j_result_t nResult = J_UNKNOW;
	GetChannel(cmdData.cmdRealPlay.channel);

	std::vector<int>::iterator itChannel = m_vecChannel.begin();
	for (; itChannel != m_vecChannel.end(); itChannel++)
	{
		TLock(m_channelLocker);
		ChannelMap::iterator it = m_channelMap.find(*itChannel);
		if (it == m_channelMap.end())
		{
			J_Obj *pObj = NULL;
			CreateChannel(*itChannel, pObj);
			if (pObj != NULL)
			{
				J_Channel *pChannel = dynamic_cast<J_Channel *>(pObj);
				if (pChannel != NULL)
				{
					pChannel->OpenStream(cmdData);
				}
			}

			ChannelMap::iterator itReal = m_channelRealMap.find(cmdData.cmdRealPlay.channel);
			if (itReal == m_channelRealMap.end())
			{
				m_channelRealMap[cmdData.cmdRealPlay.channel] = ChannelInfo();

				CXlHostCmdData hostCmdData = { 0 };
				strcpy(hostCmdData.cmdRealPlay.szID, m_hostId.c_str());
				hostCmdData.cmdRealPlay.llChnStatus = cmdData.cmdRealPlay.channel;
				CXlHelper::MakeRequest(CXlProtocol::xld_start_real_play, cmdData.cmdHeader.flag, cmdData.cmdHeader.seq,
					(char *)&hostCmdData.cmdRealPlay, sizeof(CXlHostCmdData::CmdRealPlay), m_dataBuffer);

				J_StreamHeader streamHeader = { 0 };
				streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + sizeof(CXlHostCmdData::CmdRealPlay) + sizeof(CXlProtocol::CmdTail);
				m_ringBuffer.PushBuffer(m_dataBuffer, streamHeader);	
			}
		}
		else
		{
			J_Channel *pChannel = dynamic_cast<J_Channel *>(it->second.pChannel);
			if (pChannel != NULL)
			{
				pChannel->OpenStream(cmdData);
			}

			++(it->second.nRef);
		}
		TUnlock(m_channelLocker);
	}

	return nResult;
}

j_result_t CXlHost::StopRealPlay(const CXlClientCmdData &cmdData)
{
	GetChannel(cmdData.cmdRealPlay.channel);

	std::vector<int>::iterator itChannel = m_vecChannel.begin();
	for (; itChannel != m_vecChannel.end(); itChannel++)
	{
		TLock(m_channelLocker);
		ChannelMap::iterator it = m_channelMap.find(*itChannel);
		if (it != m_channelMap.end())
		{
			--(it->second.nRef);
			if (it->second.nRef == 0)
			{
				ChannelMap::iterator itReal = m_channelRealMap.find(cmdData.cmdRealPlay.channel);
				if (itReal != m_channelRealMap.end())
				{
					m_channelRealMap.erase(itReal);

					CXlHostCmdData hostCmdData = { 0 };
					strcpy(hostCmdData.cmdRealPlay.szID, m_hostId.c_str());
					hostCmdData.cmdRealPlay.llChnStatus = cmdData.cmdRealPlay.channel;
					CXlHelper::MakeRequest(CXlProtocol::xld_stop_real_play, cmdData.cmdHeader.flag, cmdData.cmdHeader.seq, 
						(char *)&hostCmdData.cmdRealPlay, sizeof(CXlHostCmdData::CmdRealPlay), m_dataBuffer);

					J_StreamHeader streamHeader = { 0 };
					streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + sizeof(CXlHostCmdData::CmdRealPlay) + sizeof(CXlProtocol::CmdTail);
					m_ringBuffer.PushBuffer(m_dataBuffer, streamHeader);
				}
			}
			else 
			{
				J_Channel *pChannel = dynamic_cast<J_Channel *>(it->second.pChannel);
				if (pChannel != NULL)
				{
					pChannel->CloseStream(cmdData);
				}

				if (it->second.nRef < 0)
				{
					it->second.nRef = 0;
				}
			}
		}
		TUnlock(m_channelLocker);
	}

	return J_OK;
}

j_result_t CXlHost::StartVod(const CXlClientCmdData &cmdData)
{
	GetChannel(cmdData.cmdStartVod.channel);

	std::vector<int>::iterator itChannel = m_vecChannel.begin();
	for (; itChannel != m_vecChannel.end(); itChannel++)
	{
		TLock(m_channelLocker);
		ChannelMap::iterator it = m_channelMap.find(*itChannel);
		if (it == m_channelMap.end())
		{
			J_Obj *pObj = NULL;
			CreateChannel(*itChannel, pObj);
			if (pObj != NULL)
			{
				J_Vod *pChannel = dynamic_cast<J_Vod *>(pObj);
				if (pChannel != NULL)
				{
					pChannel->OpenVod(cmdData);
				}
			}

			std::map<GUIDKey, int>::iterator itVod = m_channelVodMap.find(cmdData.cmdStartVod.sessionId);
			if (itVod == m_channelVodMap.end())
			{
				m_channelVodMap[cmdData.cmdStartVod.sessionId] = 0;

				CXlHostCmdData hostCmdData = { 0 };
				hostCmdData.cmdStartVod.session = cmdData.cmdStartVod.sessionId;
				strcpy(hostCmdData.cmdStartVod.szID, m_hostId.c_str());
				hostCmdData.cmdStartVod.llChnStatus = cmdData.cmdStartVod.channel;
				hostCmdData.cmdStartVod.tmStartTime = cmdData.cmdStartVod.tmStartTime;
				hostCmdData.cmdStartVod.tmEndTime = cmdData.cmdStartVod.tmEndTime;

				CXlProtocol::CmdType cmd = CXlProtocol::xld_start_vod_play;
				if (cmdData.cmdHeader.cmd == CXlProtocol::xlc_vod_download)
				{
					cmd = CXlProtocol::xld_start_vod_download;
				}
				CXlHelper::MakeRequest(cmd, cmdData.cmdHeader.flag, cmdData.cmdHeader.seq, 
					(char *)&hostCmdData.cmdStartVod, sizeof(CXlHostCmdData::CmdStartVod), m_dataBuffer);
				J_StreamHeader streamHeader = { 0 };
				streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + sizeof(CXlHostCmdData::CmdStartVod) + sizeof(CXlProtocol::CmdTail);
				m_ringBuffer.PushBuffer(m_dataBuffer, streamHeader);
			}
		}
		else
		{
			J_Vod *pChannel = dynamic_cast<J_Vod *>(it->second.pChannel);
			if (pChannel != NULL)
			{
				pChannel->OpenVod(cmdData);
			}

			++(it->second.nRef);
		}
		TUnlock(m_channelLocker);
	}

	return J_OK;
}

j_result_t CXlHost::StopVod(const CXlClientCmdData &cmdData)
{
	GetChannel(cmdData.cmdStartVod.channel);

	std::vector<int>::iterator itChannel = m_vecChannel.begin();
	for (; itChannel != m_vecChannel.end(); itChannel++)
	{
		TLock(m_channelLocker);
		ChannelMap::iterator it = m_channelMap.find(*itChannel);
		if (it != m_channelMap.end())
		{
			--(it->second.nRef);
			if (it->second.nRef == 0)
			{
				std::map<GUIDKey, int>::iterator itVod = m_channelVodMap.find(cmdData.cmdStartVod.sessionId);
				if (itVod == m_channelVodMap.end())
				{
					m_channelVodMap.erase(itVod);

					CXlHostCmdData hostCmdData = { 0 };
					hostCmdData.cmdStopVod.session = cmdData.cmdStopVod.sessionId;
					strcpy(hostCmdData.cmdStopVod.szID, m_hostId.c_str());
					hostCmdData.cmdStopVod.llChnStatus = cmdData.cmdStartVod.channel;
					CXlProtocol::CmdType cmd = CXlProtocol::xld_start_vod_play;
					if (cmdData.cmdHeader.cmd == CXlProtocol::xlc_vod_download)
					{
						cmd = CXlProtocol::xld_start_vod_download;
					}
					CXlHelper::MakeRequest(cmd, cmdData.cmdHeader.flag, cmdData.cmdHeader.seq, 
						(char *)&hostCmdData.cmdStopVod, sizeof(CXlHostCmdData::CmdStopVod), m_dataBuffer);
					J_StreamHeader streamHeader = { 0 };
					streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + sizeof(CXlHostCmdData::CmdStopVod) + sizeof(CXlProtocol::CmdTail);
					m_ringBuffer.PushBuffer(m_dataBuffer, streamHeader);
				}
				else
				{
					J_Vod *pChannel = dynamic_cast<J_Vod *>(it->second.pChannel);
					if (pChannel != NULL)
					{
						pChannel->CloseVod(cmdData);
					}

					if (it->second.nRef < 0)
					{
						it->second.nRef = 0;
					}
				}
			}
		}
		TUnlock(m_channelLocker);
	}

	return J_OK;
}

j_result_t CXlHost::SendContent(const CXlClientCmdData &cmdData)
{
	m_contextVec.clear();
	JoDataBaseObj->GetContextList(m_hostId.c_str(), m_contextVec);

	HostContextVec::iterator it = m_contextVec.begin();
	J_StreamHeader streamHeader = { 0 };
	int nHeaderLen = 0;
	for (; it != m_contextVec.end(); it++)
	{
		/// Send Context Header
		nHeaderLen = sizeof(CXlHostCmdData::CmdContextInfo::ContextHeader) - 1 + it->header.nMessageTitleSize;
		CXlHelper::MakeRequest(CXlProtocol::xld_trans_context, CXlProtocol::xl_ctrl_start, JoDataBus->GetUniqueSeq(), 
			(char *)it->pData, nHeaderLen, m_dataBuffer);

		memset(&streamHeader, 0, sizeof(J_StreamHeader));
		streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + nHeaderLen + sizeof(CXlProtocol::CmdTail);
		m_ringBuffer.PushBuffer(m_dataBuffer, streamHeader);

		/// Send Context Info
		CXlHelper::MakeRequest(CXlProtocol::xld_trans_context, CXlProtocol::xl_ctrl_stream, JoDataBus->GetUniqueSeq(), 
			(char *)it->pContext, it->header.ulMessageSize, m_dataBuffer);

		memset(&streamHeader, 0, sizeof(J_StreamHeader));
		streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + it->header.ulMessageSize + sizeof(CXlProtocol::CmdTail);
		m_ringBuffer.PushBuffer(m_dataBuffer, streamHeader);

		/// Send MD5 Code

		/// Free Memory
		if (it->pData != NULL)
		{
			delete it->pData;
		}

		if (it->pContext != NULL)
		{
			delete it->pContext;
		}
	}

	return J_OK;
}


j_result_t CXlHost::SendFile(const CXlClientCmdData &cmdData)
{
	m_fileInfoVec.clear();
	JoDataBaseObj->GetFileInfoList(m_hostId.c_str(), m_fileInfoVec);

	HostFileInfoVec::iterator it = m_fileInfoVec.begin();
	J_StreamHeader streamHeader = { 0 };
	int nHeaderLen = 0;
	for (; it != m_fileInfoVec.end(); it++)
	{
		/// Send File Info Header
		nHeaderLen = sizeof(CXlHostCmdData::CmdFileInfo::FileInfoHeader) - 1 + it->header.nFileNameSize;
		CXlHelper::MakeRequest(CXlProtocol::xld_upload_file, CXlProtocol::xl_ctrl_start, JoDataBus->GetUniqueSeq(), 
			(char *)it->pData, nHeaderLen, m_dataBuffer);

		memset(&streamHeader, 0, sizeof(J_StreamHeader));
		streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + nHeaderLen + sizeof(CXlProtocol::CmdTail);
		m_ringBuffer.PushBuffer(m_dataBuffer, streamHeader);

		/// Send File
		FILE *fp = fopen(it->header.pData, "rb+");
		if (fp != NULL)
		{
			int nReadLen = 0;
			char fileBuffer[2048] = {0};
			while ((nReadLen = fread(fileBuffer, 1, 2048, fp)) > 0)
			{
				CXlHelper::MakeRequest(CXlProtocol::xld_upload_file, CXlProtocol::xl_ctrl_stream, JoDataBus->GetUniqueSeq(), 
					(char *)fileBuffer, nReadLen, m_dataBuffer);

				memset(&streamHeader, 0, sizeof(J_StreamHeader));
				streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + nReadLen + sizeof(CXlProtocol::CmdTail);
				m_ringBuffer.PushBuffer(m_dataBuffer, streamHeader);
			}

			fclose(fp);
		}

		/// Send MD5 Code

		/// Free Memory
		if (it->pData != NULL)
		{
			delete it->pData;
		}
	}

	return J_OK;
}

j_result_t CXlHost::TalkBackCommand(const CXlClientCmdData &cmdData)
{
	return J_OK;
}

j_result_t CXlHost::TalkBackData(const CXlClientCmdData &cmdData)
{
	return J_OK;
}

void CXlHost::GetChannel(j_uint64_t channel)
{
	m_vecChannel.clear();
	for (int i=0; i<64; i++)
	{
		if (channel & ((j_uint64_t)1 << i))
		{
			m_vecChannel.push_back(i + 1);
		}
	}
}