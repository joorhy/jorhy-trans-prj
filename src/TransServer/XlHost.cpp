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
#include "XlDataBusDef.h"
#include "x_md5.c"

#include <iostream>
#include <strstream> 

extern const char *g_ini_file;

#define TIME_OUT_INTREVAL	120

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

	XlHostResponse::HostInfo hostInfo = { 0 };
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
	//if (time(0) - m_lastBreatTime > TIME_OUT_INTREVAL)
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

		if (CXlProtocol::xld_real_play == cmdHeader.cmd)
		{
			J_OS::LOGDEBUG("read_head cmd = %d len = %d", cmdHeader.cmd, cmdHeader.length);
		}

		m_ioState = CXlProtocol::xl_read_data_state;
	}
	else if (m_ioState == CXlProtocol::xl_read_data_state)
	{
		CXlDataBusInfo *pRespData = (CXlDataBusInfo *)m_readBuff;
		//J_OS::LOGINFO("CXlHost read_data cmd = %d flag = %d", pRespData->header.cmd, pRespData->header.flag);
		if (CXlProtocol::xld_start_vod_download == pRespData->header.cmd || CXlProtocol::xld_vod_play == pRespData->header.cmd
			|| CXlProtocol::xld_stop_vod_download == pRespData->header.cmd)
		{
			if (pRespData->header.flag != 2)
				J_OS::LOGINFO("read_data cmd = %d flag = %d", pRespData->header.cmd, pRespData->header.flag);
		}
		switch (pRespData->header.cmd)
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
		case CXlProtocol::xld_real_play:
			assert(pRespData->header.length + sizeof(CXlProtocol::CmdTail) == pAsioData->ioRead.finishedLen);
			OnRealData(pRespData);
			break;
		case CXlProtocol::xld_vod_play:
		case CXlProtocol::xld_start_vod_download:
			OnVodData(pRespData);
			break;
		case CXlProtocol::xld_stop_vod_download:
			OnVodStop(pRespData);
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
		case CXlProtocol::xld_first_vod_timestamp:
			OnDeleteVodInfo(*pRespData);
			break;
		case CXlProtocol::xld_update_vod_info:
			OnUpdateVodInfo(*pRespData);
			break;	
		case CXlProtocol::xld_talk_cmd_out:
			OnTalkBackCommand(*pRespData);
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

j_result_t CXlHost::OnRequest(const CXlDataBusInfo &cmdData)
{
	switch (cmdData.header.cmd)
	{
	case CXlProtocol::xlc_real_play:
		if (cmdData.header.flag == CXlProtocol::xl_ctrl_start)
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
		if (cmdData.header.flag == CXlProtocol::xl_ctrl_start)
		{
			StartVod(cmdData);
		}
		else
		{
			StopVod(cmdData);
		}
		break;
	case CXlProtocol::xlc_trans_context:
		SendContent(cmdData);
		break;
	case CXlProtocol::xlc_upload_file:
		SendFile(cmdData);
		break;
	case CXlProtocol::xlc_talk_cmd_out:
		TalkBackCommand(cmdData);
		break;
	case CXlProtocol::xlc_talk_data_out:
		TalkBackData(cmdData);
		break;
	}
	return J_OK;
}

j_result_t CXlHost::OnMessage(const CXlDataBusInfo &cmdData)
{
	JoDataBus->OnMessage(m_hostId, cmdData);
	return J_OK;
}

j_result_t CXlHost::OnHeartBeat(const CXlDataBusInfo &cmdData)
{
	m_lastBreatTime = time(0);
	return J_OK;
}

j_result_t CXlHost::OnPrepare(const CXlDataBusInfo &respData)
{
	CXlDataBusInfo hostCmdData = {0};
	time_t logLastTime = JoDataBaseObj->GetDevLogLastTime(m_hostId.c_str());
	if (logLastTime == 0)
	{
		hostCmdData.hostRequest.onOffInfo.tmStart = -1;
		hostCmdData.hostRequest.onOffInfo.tmEnd = -1;
	}
	else
	{
		hostCmdData.hostRequest.onOffInfo.tmStart = logLastTime;
		hostCmdData.hostRequest.onOffInfo.tmEnd = -1;
	}

	CXlHelper::MakeRequest(CXlProtocol::xld_get_on_off_log,  CXlProtocol::xl_ctrl_start, JoDataBus->GetUniqueSeq(), hostCmdData.pData, sizeof(XlHostRequest::OnOffInfo), m_dataBuffer);

	J_StreamHeader streamHeader = {0};
	memset(&streamHeader, 0, sizeof(J_StreamHeader));
	streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + sizeof(XlHostRequest::OnOffInfo) + sizeof(CXlProtocol::CmdTail);
	m_ringBuffer.PushBuffer(m_dataBuffer, streamHeader);

	return J_OK;
}

j_result_t CXlHost::OnAlarmInfo(const CXlDataBusInfo &respData)
{
	if (m_bReady)
	{
		JoDataBaseObj->InsertAlarmInfo(m_hostId.c_str(), respData.hostResponse.alarmInfo);

		CXlDataBusInfo data = { 0 };
		data.header = respData.header;
		strcpy(data.clientResponse.alarmInfo.szID, m_hostId.c_str());
		data.clientResponse.alarmInfo.tmTimeStamp = respData.hostResponse.alarmInfo.tmTimeStamp;
		data.clientResponse.alarmInfo.bAlarm = respData.hostResponse.alarmInfo.bAlarm;
		data.clientResponse.alarmInfo.dLongitude = respData.hostResponse.alarmInfo.dLongitude;
		data.clientResponse.alarmInfo.dLatitude = respData.hostResponse.alarmInfo.dLatitude;
		data.clientResponse.alarmInfo.dGPSSpeed = respData.hostResponse.alarmInfo.dGPSSpeed;
		JoDataBus->OnMessage(m_hostId, data);
	}

	return J_OK;
}

j_result_t CXlHost::OnRealData(const CXlDataBusInfo *respData)
{
	TLock(m_channelLocker);
	//J_OS::LOGINFO("OnRealData channel = %d %d", respData->xldRespRealData.channel & 0xFF, respData->header.flag);
	ChannelMap::iterator it = m_channelMap.find(respData->hostResponse.realData.channel & 0xFF);
	if (it != m_channelMap.end())
	{
		CXlChannel *pXlChannel = dynamic_cast<CXlChannel *>(it->second.pChannel);
		if (pXlChannel != NULL)
		{
			pXlChannel->InputData(CXlProtocol::xlc_real_play, respData);
		}

		if (respData->header.flag == CXlProtocol::xl_ctrl_end || respData->header.flag == CXlProtocol::xl_ctrl_stop)
		{
			if (pXlChannel != NULL)
			{
				CXlDataBusInfo *cmdData = (CXlDataBusInfo *)respData;
				pXlChannel->CloseStream(*cmdData);
			}

			if (respData->header.flag == CXlProtocol::xl_ctrl_end)
			{
				it->second.nRef--;
				if (it->second.nRef < 0)
				{
					it->second.nRef = 0;
				}
			}

			if (it->second.nRef == 0)
			{
				ReleaseChannel(respData->hostResponse.realData.channel);
			}
		}
	}
	TUnlock(m_channelLocker);

	return J_OK;
}

j_result_t CXlHost::OnTalkBackCommand(const CXlDataBusInfo &respData)
{
	const_cast<CXlDataBusInfo &>(respData).header.cmd = CXlProtocol::xlc_talk_cmd_in;
	JoDataBus->RequestMessage(respData.hostResponse.talkCmd.account, this, respData);
	return J_OK;
}

j_result_t CXlHost::OnTalkBackData(const CXlDataBusInfo &respData)
{
	const_cast<CXlDataBusInfo &>(respData).header.cmd = CXlProtocol::xlc_talk_data_in;
	JoDataBus->RequestMessage(respData.hostResponse.talkData.account, this, respData);
	return J_OK;
}

j_result_t CXlHost::OnResponse(const CXlDataBusInfo &respData)
{
	JoDataBus->Response(respData);
	return J_OK;
}

j_result_t CXlHost::OnVodData(const CXlDataBusInfo *respData)
{
	TLock(m_channelLocker);

	ChannelMap::iterator it = m_channelMap.find(respData->hostResponse.vodData.channel & 0xFF);
	//J_OS::LOGINFO("CXlHost::OnVodData channel = %d", respData->respVodData.channel & 0xFF);
	if (it != m_channelMap.end())
	{
		CXlChannel *pXlChannel = dynamic_cast<CXlChannel *>(it->second.pChannel);
		if (pXlChannel != NULL)
		{
			pXlChannel->InputData(CXlProtocol::xl_ctrl_stream, respData);
		}

		if (respData->header.flag == CXlProtocol::xl_ctrl_end || respData->header.flag == CXlProtocol::xl_ctrl_stop)
		{
			J_OS::LOGINFO("CXlHost::OnVodData channel = %d", respData->hostResponse.vodData.channel & 0xFF);
			J_OS::LOGINFO("File TotleSize = %d", m_nDownloadSize);
			if (pXlChannel != NULL)
			{
				CXlDataBusInfo *cmdData = (CXlDataBusInfo *)respData;
				pXlChannel->CloseVod(*cmdData);
			}

			if (respData->header.flag == CXlProtocol::xl_ctrl_end)
			{
				it->second.nRef--;
				if (it->second.nRef < 0)
				{
					it->second.nRef = 0;
				}
			}

			if (it->second.nRef == 0)
			{
				ReleaseChannel(respData->hostResponse.vodData.channel);
			}
		}
		else
		{
			m_nDownloadSize += respData->header.length;
		}
	}
	else
	{
		J_OS::LOGINFO("CXlHost::OnVodData Error");
		J_OS::LOGINFO("CXlHost::OnVodData channel = %d", respData->hostResponse.vodData.channel & 0xFF);
	}
	TUnlock(m_channelLocker);

	return J_OK;
}

j_result_t CXlHost::OnVodStop(const CXlDataBusInfo *respData)
{
	const_cast<CXlDataBusInfo *>(respData)->header.cmd = CXlProtocol::xld_stop_vod_download;
	JoDataBus->Response(*respData);
	return J_OK;
}

j_result_t CXlHost::OnConrrectTime(const CXlDataBusInfo &respData)
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

j_result_t CXlHost::OnHostInfo(const CXlDataBusInfo &respData)
{
	JoDataBaseObj->UpdateDevInfo(respData.hostResponse.hostInfo, true);

	CXlDataBusInfo hostCmdData = { 0 };
	hostCmdData.hostRequest.conrrectTime.systime = time(0);
	CXlHelper::MakeRequest(CXlProtocol::xld_conrrent_time, CXlProtocol::xl_ctrl_start, JoDataBus->GetUniqueSeq(), 
		(char *)hostCmdData.pData, sizeof(XlHostRequest::ConrrectTime), m_dataBuffer);

	J_StreamHeader streamHeader = { 0 };
	streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + sizeof(XlHostRequest::ConrrectTime) + sizeof(CXlProtocol::CmdTail);
	m_ringBuffer.PushBuffer(m_dataBuffer, streamHeader);

	return J_OK;
}

j_result_t CXlHost::OnOnOffInfo(const CXlDataBusInfo &respData)
{
	JoDataBaseObj->InsertLogInfo(m_hostId.c_str(), respData.hostResponse.onOffInfo.tmStart, respData.hostResponse.onOffInfo.tmEnd);
	return J_OK;
}
j_result_t CXlHost::OnDeleteVodInfo(const CXlDataBusInfo &respData)
{
	JoDataBaseObj->SyncLogInfo(m_hostId.c_str(), respData.hostResponse.syncVodInfo.tmFristItem);
	return J_OK;
}
j_result_t CXlHost::OnUpdateVodInfo(const CXlDataBusInfo &respData)
{
	JoDataBaseObj->InsertLogInfo(m_hostId.c_str(), respData.hostResponse.vodInfo.tmStart, respData.hostResponse.vodInfo.tmEnd);

	CXlDataBusInfo hostCmdData = { 0 };
	CXlHelper::MakeResponse(CXlProtocol::xld_update_vod_info, CXlProtocol::xl_ctrl_end, respData.header.seq,
		(char *)&hostCmdData.hostRequest.updateVodAck, sizeof(XlHostRequest::UpdateVodAck), m_dataBuffer);

	J_StreamHeader streamHeader = { 0 };
	streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + sizeof(XlHostRequest::UpdateVodAck) + sizeof(CXlProtocol::CmdTail);
	m_ringBuffer.PushBuffer(m_dataBuffer, streamHeader);	

	return J_OK;
}

j_result_t CXlHost::StartRealPlay(const CXlDataBusInfo &cmdData)
{
	j_result_t nResult = J_UNKNOW;
	GetChannel(cmdData.clientRequest.realPlay.channel);

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

			ChannelMap::iterator itReal = m_channelRealMap.find(cmdData.clientRequest.realPlay.channel);
			if (itReal == m_channelRealMap.end())
			{
				m_channelRealMap[cmdData.clientRequest.realPlay.channel] = ChannelInfo();

				CXlDataBusInfo hostCmdData = { 0 };
				strcpy(hostCmdData.hostRequest.realPlay.szID, m_hostId.c_str());
				hostCmdData.hostRequest.realPlay.llChnStatus = cmdData.clientRequest.realPlay.channel;
				CXlHelper::MakeRequest(CXlProtocol::xld_real_play, cmdData.header.flag, cmdData.header.seq,
					(char *)&hostCmdData.hostRequest.realPlay, sizeof(XlHostRequest::RealPlay), m_dataBuffer);

				J_StreamHeader streamHeader = { 0 };
				streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + sizeof(XlHostRequest::RealPlay) + sizeof(CXlProtocol::CmdTail);
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

j_result_t CXlHost::StopRealPlay(const CXlDataBusInfo &cmdData)
{
	GetChannel(cmdData.clientRequest.realPlay.channel);

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
				ChannelMap::iterator itReal = m_channelRealMap.find(cmdData.clientRequest.realPlay.channel);
				if (itReal != m_channelRealMap.end())
				{
					m_channelRealMap.erase(itReal);

					CXlDataBusInfo hostCmdData = { 0 };
					strcpy(hostCmdData.hostRequest.realPlay.szID, m_hostId.c_str());
					hostCmdData.hostRequest.realPlay.llChnStatus = cmdData.clientRequest.realPlay.channel;
					CXlHelper::MakeRequest(CXlProtocol::xld_real_play, cmdData.header.flag, cmdData.header.seq, 
						(char *)&hostCmdData.hostRequest.realPlay, sizeof(XlHostRequest::RealPlay), m_dataBuffer);

					J_StreamHeader streamHeader = { 0 };
					streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + sizeof(XlHostRequest::RealPlay) + sizeof(CXlProtocol::CmdTail);
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

j_result_t CXlHost::StartVod(const CXlDataBusInfo &cmdData)
{
	GetChannel(cmdData.clientRequest.startVod.channel);

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

			std::map<int, int>::iterator itVod = m_channelVodMap.find(cmdData.header.seq);
			if (itVod == m_channelVodMap.end())
			{
				m_nDownloadSize = 0;
				m_channelVodMap[cmdData.header.seq] = 0;

				CXlDataBusInfo hostCmdData = { 0 };
				hostCmdData.hostRequest.startVod.session = cmdData.clientRequest.startVod.sessionId;
				strcpy(hostCmdData.hostRequest.startVod.szID, m_hostId.c_str());
				hostCmdData.hostRequest.startVod.llChnStatus = cmdData.clientRequest.startVod.channel;
				hostCmdData.hostRequest.startVod.tmStartTime = cmdData.clientRequest.startVod.tmStartTime;
				hostCmdData.hostRequest.startVod.tmEndTime = cmdData.clientRequest.startVod.tmEndTime;

				CXlProtocol::CmdType cmd = CXlProtocol::xld_vod_play;
				if (cmdData.header.cmd == CXlProtocol::xlc_vod_download)
				{
					cmd = CXlProtocol::xld_start_vod_download;
				}
				CXlHelper::MakeRequest(cmd, cmdData.header.flag, cmdData.header.seq, 
					(char *)&hostCmdData.hostRequest.startVod, sizeof(XlHostRequest::StartVod), m_dataBuffer);
				J_StreamHeader streamHeader = { 0 };
				streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + sizeof(XlHostRequest::StartVod) + sizeof(CXlProtocol::CmdTail);
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

///远程回放、视频文件下载手动停止
j_result_t CXlHost::StopVod(const CXlDataBusInfo &cmdData)
{
	GetChannel(cmdData.clientRequest.startVod.channel);

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
				std::map<int, int>::iterator itVod = m_channelVodMap.find(cmdData.header.seq);
				if (itVod != m_channelVodMap.end())
				{
					m_channelVodMap.erase(itVod);

					CXlDataBusInfo hostCmdData = { 0 };
					hostCmdData.hostRequest.stopVod.session = cmdData.clientRequest.startVod.sessionId;
					strcpy(hostCmdData.hostRequest.stopVod.szID, m_hostId.c_str());
					hostCmdData.hostRequest.stopVod.llChnStatus = cmdData.clientRequest.startVod.channel;
					CXlProtocol::CmdType cmd = CXlProtocol::xld_vod_play;
					if (cmdData.header.cmd == CXlProtocol::xlc_vod_download)
					{
						if (cmdData.header.flag == CXlProtocol::xl_ctrl_start)
						{
							cmd = CXlProtocol::xld_start_vod_download;
						}
						else if (cmdData.header.flag == CXlProtocol::xl_ctrl_stop)
						{
							cmd = CXlProtocol::xld_stop_vod_download;
						}
					}
					CXlHelper::MakeRequest(cmd, cmdData.header.flag, cmdData.header.seq, 
						(char *)&hostCmdData.hostRequest.stopVod, sizeof(XlHostRequest::StopVod), m_dataBuffer);
					J_StreamHeader streamHeader = { 0 };
					streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + sizeof(XlHostRequest::StopVod) + sizeof(CXlProtocol::CmdTail);
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

j_result_t CXlHost::SendContent(const CXlDataBusInfo &cmdData)
{
	m_contextVec.clear();
	JoDataBaseObj->GetContextList(m_hostId.c_str(), m_contextVec);

	HostContextVec::iterator it = m_contextVec.begin();
	J_StreamHeader streamHeader = { 0 };
	int nHeaderLen = 0;
	for (; it != m_contextVec.end(); it++)
	{
		/// MD5 initialize
		MD5_CTX md5Ctx = { 0 };
		MD5Init(&md5Ctx);
		/// Send Context Header
		nHeaderLen = sizeof(XlHostRequest::ContextInfo::ContextHeader) - 1 + (*it)->header.nMessageTitleSize;
		CXlHelper::MakeRequest(CXlProtocol::xld_trans_context, CXlProtocol::xl_ctrl_start, JoDataBus->GetUniqueSeq(), 
			(char *)((*it)->pData), nHeaderLen, m_dataBuffer);

		memset(&streamHeader, 0, sizeof(J_StreamHeader));
		streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + nHeaderLen + sizeof(CXlProtocol::CmdTail);
		m_ringBuffer.PushBuffer(m_dataBuffer, streamHeader);

		/// Send Context Info
		MD5Update(&md5Ctx, (unsigned char *)((*it)->pContext), (*it)->header.ulMessageSize);
		CXlHelper::MakeRequest(CXlProtocol::xld_trans_context, CXlProtocol::xl_ctrl_stream, JoDataBus->GetUniqueSeq(), 
			(char *)((*it)->pContext), (*it)->header.ulMessageSize, m_dataBuffer);

		memset(&streamHeader, 0, sizeof(J_StreamHeader));
		streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + (*it)->header.ulMessageSize + sizeof(CXlProtocol::CmdTail);
		m_ringBuffer.PushBuffer(m_dataBuffer, streamHeader);

		/// Send MD5 Code
		unsigned char md5Buffer[16] = { 0 };
		MD5Final(md5Buffer, &md5Ctx);

		CXlHelper::MakeRequest(CXlProtocol::xld_trans_context, CXlProtocol::xl_ctrl_end, JoDataBus->GetUniqueSeq(),
			(char *)md5Buffer, 16, m_dataBuffer);

		memset(&streamHeader, 0, sizeof(J_StreamHeader));
		streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + 16 + sizeof(CXlProtocol::CmdTail);
		m_ringBuffer.PushBuffer(m_dataBuffer, streamHeader);

		/// Free Memory
		if ((*it)->pContext != NULL)
		{
			delete [](*it)->pContext;
		}

		if ((*it)->pData != NULL)
		{
			free(*it);
		}
	}
	return J_OK;
}


j_result_t CXlHost::SendFile(const CXlDataBusInfo &cmdData)
{
	m_fileInfoVec.clear();
	JoDataBaseObj->GetFileInfoList(m_hostId.c_str(), m_fileInfoVec);

	HostFileInfoVec::iterator it = m_fileInfoVec.begin();
	J_StreamHeader streamHeader = { 0 };
	int nHeaderLen = 0;
	for (; it != m_fileInfoVec.end(); it++)
	{
		char pFilePath[256] = { 0 };
		GetPrivateProfileString("file_info", "path", "E:/FileRecord", pFilePath, sizeof(pFilePath), g_ini_file);
		sprintf(pFilePath, "%s/%s", pFilePath, (*it)->pFileName);

		FILE *fp = fopen(pFilePath, "rb+");
		if (fp == NULL)
		{
			continue;
		}

		_fseeki64(fp, 0, SEEK_END);
		(*it)->header.ulFileSize = _ftelli64(fp);
		_fseeki64(fp, 0, SEEK_SET);

		/// MD5 initialize
		MD5_CTX md5Ctx = { 0 };
		MD5Init(&md5Ctx);
		/// Send File Info Header
		nHeaderLen = sizeof(XlHostRequest::FileInfo::FileInfoHeader) - 1 + (*it)->header.nFileNameSize;
		CXlHelper::MakeRequest(CXlProtocol::xld_upload_file, CXlProtocol::xl_ctrl_start, JoDataBus->GetUniqueSeq(), 
			(char *)((*it)->pData), nHeaderLen, m_dataBuffer);

		memset(&streamHeader, 0, sizeof(J_StreamHeader));
		streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + nHeaderLen + sizeof(CXlProtocol::CmdTail);
		m_ringBuffer.PushBuffer(m_dataBuffer, streamHeader);

		/// Send File
		if (fp != NULL)
		{
			int nReadLen = 0;
			char fileBuffer[2048] = {0};
			while ((nReadLen = fread(fileBuffer, 1, 2048, fp)) > 0)
			{
				MD5Update(&md5Ctx, (unsigned char *)fileBuffer, nReadLen);
				CXlHelper::MakeRequest(CXlProtocol::xld_upload_file, CXlProtocol::xl_ctrl_stream, JoDataBus->GetUniqueSeq(), 
					(char *)fileBuffer, nReadLen, m_dataBuffer);

				memset(&streamHeader, 0, sizeof(J_StreamHeader));
				streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + nReadLen + sizeof(CXlProtocol::CmdTail);
				m_ringBuffer.PushBuffer(m_dataBuffer, streamHeader);
			}

			fclose(fp);
		}

		/// Send MD5 Code
		unsigned char md5Buffer[16] = { 0 };
		MD5Final(md5Buffer, &md5Ctx);

		CXlHelper::MakeRequest(CXlProtocol::xld_upload_file, CXlProtocol::xl_ctrl_end, JoDataBus->GetUniqueSeq(),
			(char *)md5Buffer, 16, m_dataBuffer);

		memset(&streamHeader, 0, sizeof(J_StreamHeader));
		streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + 16 + sizeof(CXlProtocol::CmdTail);
		m_ringBuffer.PushBuffer(m_dataBuffer, streamHeader);

		/// Free Memory
		if ((*it)->pFileName != NULL)
		{
			delete[](*it)->pFileName;
		}

		if ((*it)->pData != NULL)
		{
			free(*it);
		}
	}

	return J_OK;
}

j_result_t CXlHost::TalkBackCommand(const CXlDataBusInfo &cmdData)
{
	CXlHelper::MakeMessage(CXlProtocol::xld_talk_cmd_in, cmdData.header.flag, cmdData.header.seq, 
		(char *)&cmdData.clientRequest.talkCmd, sizeof(XlClientRequest::TalkCmd), m_dataBuffer);

	J_StreamHeader streamHeader = { 0 };
	streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + sizeof(XlHostRequest::TalkCmd) + sizeof(CXlProtocol::CmdTail);
	m_ringBuffer.PushBuffer(m_dataBuffer, streamHeader);

	return J_OK;
}

j_result_t CXlHost::TalkBackData(const CXlDataBusInfo &cmdData)
{
	CXlHelper::MakeRequest(CXlProtocol::xld_talk_data_in, cmdData.header.flag, cmdData.header.seq, 
		(char *)&cmdData.clientRequest.talkData, sizeof(XlClientRequest::TalkData), m_dataBuffer);

	J_StreamHeader streamHeader = { 0 };
	streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + sizeof(XlHostRequest::TalkData) + sizeof(CXlProtocol::CmdTail);
	m_ringBuffer.PushBuffer(m_dataBuffer, streamHeader);

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