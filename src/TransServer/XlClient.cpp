#include "XlClient.h"
#include "DeviceManager.h"
#include "ClientManager.h"
#include "XlHelper.h"
#include "DataBus.h"
#include "XlDataBusDef.h"
#include "MySQLAccess.h"
#include <direct.h>

#include <iostream>
#include <strstream>

#define XL_BUFFER_SIZE (1024 * 1024)
#define TIME_OUT_INTERVAL	20

extern const char *g_ini_file;

CXlClient::CXlClient(j_socket_t nSock)
	: m_ringBuffer(50 * 1024 * 1024)
{
	m_pFile = NULL;
	memset(m_userName, 0, sizeof(m_userName));
	strcpy(m_userName, "");
	m_readBuff = new char[XL_BUFFER_SIZE];
	m_writeBuff = new char[XL_BUFFER_SIZE];
	m_dataBuff = new char[XL_BUFFER_SIZE];
	m_ioCmdState = CXlProtocol::xl_init_state;
	m_ioDataState = CXlProtocol::xl_init_state;

	m_nRefCnt = 0;
	m_lastBreatTime = time(0);

	J_OS::LOGINFO("CXlClient::CXlClient() %d", this);
}

CXlClient::~CXlClient()
{
	delete m_readBuff;
	delete m_writeBuff;
	delete m_dataBuff;
	OnBroken();
	JoDataBus->RegisterDevice(m_userName, NULL);

	J_OS::LOGINFO("CXlClient::~CXlClient() %d", this);
}

j_result_t CXlClient::OnHandleRead(J_AsioDataBase *pAsioData)
{
	if (m_ioCmdState == CXlProtocol::xl_init_state)
	{
		pAsioData->ioCall = J_AsioDataBase::j_read_e;
		CXlHelper::MakeNetData(pAsioData, m_readBuff, sizeof(CXlProtocol::CmdHeader));

		m_ioCmdState = CXlProtocol::xl_read_head_state;
	}
	else if (m_ioCmdState == CXlProtocol::xl_read_head_state)
	{
		CXlProtocol::CmdHeader cmdHeader = *((CXlProtocol::CmdHeader *)m_readBuff);
		//J_OS::LOGINFO("%d ", cmdHeader.cmd);
		//if (cmdHeader.length > 1000)
		//{
		//	J_OS::LOGINFO("");
		//}
		CXlHelper::MakeNetData(pAsioData, m_readBuff + sizeof(CXlProtocol::CmdHeader), cmdHeader.length + sizeof(CXlProtocol::CmdTail));
		pAsioData->ioCall = J_AsioDataBase::j_read_e;

		m_ioCmdState = CXlProtocol::xl_read_data_state;
	}
	else if (m_ioCmdState == CXlProtocol::xl_read_data_state)
	{
		CXlDataBusInfo *pCmdData = (CXlDataBusInfo *)m_readBuff;
		//J_OS::LOGINFO("CXlClient read_data cmd = %d flag = %d", pCmdData->header.cmd, pCmdData->header.flag);
		if (pCmdData->header.cmd != CXlProtocol::xlc_heart_beat)
		{
			J_OS::LOGINFO("CXlClient::OnHandleRead cseq = %d cmd = %d", pCmdData->header.seq, pCmdData->header.cmd);
		}
		switch (pCmdData->header.cmd)
		{
		case CXlProtocol::xlc_login:
			OnLogin(*pCmdData);
			break;
		case CXlProtocol::xlc_logout:
			OnLogout(*pCmdData);
			break;
		case CXlProtocol::xlc_heart_beat:
			OnHeartBeat(*pCmdData);
			break;
		case CXlProtocol::xlc_real_play:
			SaveRequest(*pCmdData, pCmdData->header.flag == CXlProtocol::xl_ctrl_start);
			//pCmdData->clientRequest.realPlay.pBuffer = &m_ringBuffer;
			JoDataBus->Request(pCmdData->clientRequest.realPlay.hostId, this, *pCmdData);
			break;
		case CXlProtocol::xlc_vod_play:
		case CXlProtocol::xlc_vod_download:
			m_nFileTotleSize = 0;
			J_OS::LOGINFO("CXlClient::OnHandleRead channel = %d", pCmdData->clientRequest.vodPlay.channel);
			SaveRequest(*pCmdData, pCmdData->header.flag == CXlProtocol::xl_ctrl_start);
			//pCmdData->clientRequest.starvodPlaytVod.pBuffer = &m_ringBuffer;
			JoDataBus->Request(pCmdData->clientRequest.vodPlay.hostId, this, *pCmdData);
			break;
		case CXlProtocol::xlc_vehicle_status:
		case CXlProtocol::xlc_alarm_info:
			EnableVehStatus(*pCmdData, pCmdData->header.flag == CXlProtocol::xl_ctrl_start);
			break;
		case CXlProtocol::xlc_trans_context:
			SaveContext(*pCmdData);
			break;
		case CXlProtocol::xlc_upload_file:
			SaveFiles(*pCmdData);
			break;
		case CXlProtocol::xlc_talk_cmd_out:
			OnTalkBackCommand(*pCmdData);
			break;
		case CXlProtocol::xlc_talk_data_out:
			OnTalkBackData(*pCmdData);
			break;
		default:
			SendRequest(*pCmdData);
			break;
		}

		CXlHelper::MakeNetData(pAsioData, m_readBuff, sizeof(CXlProtocol::CmdHeader));
		pAsioData->ioCall = J_AsioDataBase::j_read_e;
		m_ioCmdState = CXlProtocol::xl_read_head_state;
	}

	return J_OK;
}

j_result_t CXlClient::OnHandleWrite(J_AsioDataBase *pAsioData)
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
		CXlDataBusInfo *pRespData = (CXlDataBusInfo *)m_writeBuff;
		if (pRespData->header.cmd == CXlProtocol::xlc_vod_play ||
			(pRespData->header.cmd == CXlProtocol::xlc_vod_download))
		{
			if (pRespData->header.flag == CXlProtocol::xl_ctrl_end || pRespData->header.flag == CXlProtocol::xl_ctrl_stop)
			{
				J_OS::LOGINFO("Send FIle TotleSize = %d", m_nFileTotleSize);
			}
			else
			{
				m_nFileTotleSize += pRespData->header.length;
			}
		}
		//J_OS::LOGINFO("CXlClient::OnHandleWrite send len = %d flag = %d", m_streamHeader.dataLen, pRespData->header.flag);
		//if (pRespData->respHeader.length + sizeof(CXlProtocol::CmdHeader) + sizeof(CXlProtocol::CmdTail) != m_streamHeader.dataLen)
		//{
		//	assert(false);
		//}
	}

	return J_OK;
}

j_result_t CXlClient::OnRequest(const CXlDataBusInfo &cmdData)
{
	switch (cmdData.header.cmd)
	{
	case CXlProtocol::xlc_talk_cmd_in:
		TalkBackCommand(cmdData);
		break;
	case CXlProtocol::xlc_talk_data_in:
		TalkBackData(cmdData);
		break;
	default:
		MessageBack(cmdData);
		break;
	}
	return J_OK;
}

j_result_t CXlClient::OnResponse(const CXlDataBusInfo &respData)
{
	CXlHelper::MakeResponse(respData.header.cmd, respData.header.flag, respData.header.seq,
		(j_char_t *)respData.pData, respData.header.length, m_dataBuff);
	J_StreamHeader streamHeader = { 0 };
	streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + respData.header.length + sizeof(CXlProtocol::CmdTail);
	m_ringBuffer.PushBuffer(m_dataBuff, streamHeader);

	return J_OK;
}

j_result_t CXlClient::OnMessage(j_string_t strHostId, const CXlDataBusInfo &respData)
{
	switch (respData.header.cmd)
	{
	case CXlProtocol::xlc_vehicle_status:
	{
		VehicleEnableMap::iterator it = m_vehEnableMap.find(strHostId);
		if (it == m_vehEnableMap.end() || it->second.vehStatus == false)
		{
			return J_OK;
		}
		break;
	}
	case CXlProtocol::xlc_alarm_info:
	{
		VehicleEnableMap::iterator it = m_vehEnableMap.find(strHostId);
		if (it == m_vehEnableMap.end() || it->second.alarmInfo == false)
		{
			return J_OK;
		}
		break;
	}
	case CXlProtocol::xlc_dev_state:
		/// TODO:
		Recovery();
		break;
	}

	if (respData.header.cmd == CXlProtocol::xlc_dev_state)
	{
		CXlHelper::MakeMessage(respData.header.cmd, respData.header.flag, respData.header.seq,
			(j_char_t *)respData.pData, respData.header.length, m_dataBuff);
	}
	else
	{
		CXlHelper::MakeResponse(respData.header.cmd, respData.header.flag, respData.header.seq,
			(j_char_t *)respData.pData, respData.header.length, m_dataBuff);
	}

	J_StreamHeader streamHeader = { 0 };
	streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + respData.header.length + sizeof(CXlProtocol::CmdTail);
	m_ringBuffer.PushBuffer(m_dataBuff, streamHeader);

	return J_OK;
}

j_result_t CXlClient::OnBroken()
{
	TLock(m_locker);
	RequestVec::iterator it = m_requestVec.begin();
	for (; it != m_requestVec.end(); it++)
	{
		it->header.flag = CXlProtocol::xl_pack_end;
		SendRequest(*it);
	}
	m_requestVec.clear();
	TUnlock(m_locker);

	if (m_pFile != NULL)
	{
		fclose(m_pFile);
		m_pFile = NULL;
	}

	JoClientManager->Logout(m_userName, this);
	JoDataBus->ClearRequest(this);
	JoDataBus->ClearMessage(this);

	return J_OK;
}

j_boolean_t CXlClient::IsReady()
{
	//if (time(0) - m_lastBreatTime > 30)
	//	return false;

	return true;
}

j_result_t CXlClient::OnLogin(const CXlDataBusInfo &cmdData)
{
	XlClientResponse::Login data = { 0 };
	JoClientManager->Login(cmdData.clientRequest.login.userName, cmdData.clientRequest.login.passWord, cmdData.clientRequest.login.nForced, data.code, this);
	memset(m_userName, 0, sizeof(m_userName));
	strcpy(m_userName, cmdData.clientRequest.login.userName);
	if (data.code == 0)
	{
		JoDataBus->RegisterDevice(m_userName, this);
		JoDataBus->SubscribeMsg(m_userName, this, cmdData);
		m_lastBreatTime = time(0);
	}
	CXlHelper::MakeResponse(CXlProtocol::xlc_login, cmdData.header.flag, cmdData.header.seq,
		(j_char_t *)&data, sizeof(XlClientResponse::Login), m_dataBuff);

	J_StreamHeader streamHeader = { 0 };
	streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + sizeof(XlClientResponse::Login) + sizeof(CXlProtocol::CmdTail);
	m_ringBuffer.PushBuffer(m_dataBuff, streamHeader);

	return J_OK;
}

j_result_t CXlClient::OnLogout(const CXlDataBusInfo &cmdData)
{
	JoClientManager->Logout(m_userName, this);
	XlClientResponse::Logout data = { 0 };
	data.code = 0x00;
	CXlHelper::MakeResponse(CXlProtocol::xlc_logout, cmdData.header.flag, cmdData.header.seq,
		(j_char_t *)&data, sizeof(XlClientResponse::Logout), m_dataBuff);

	J_StreamHeader streamHeader = { 0 };
	streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + sizeof(XlClientResponse::Logout) + sizeof(CXlProtocol::CmdTail);
	m_ringBuffer.PushBuffer(m_dataBuff, streamHeader);

	return J_OK;
}

j_result_t CXlClient::OnHeartBeat(const CXlDataBusInfo &cmdData)
{
	m_lastBreatTime = time(0);
	return J_OK;
}

j_result_t CXlClient::OnTalkBackCommand(const CXlDataBusInfo &cmdData)
{
	JoDataBus->RequestStateless(cmdData.clientRequest.talkCmd.equID, this, cmdData);
	return J_OK;
}

j_result_t CXlClient::OnTalkBackData(const CXlDataBusInfo &cmdData)
{
	JoDataBus->RequestStateless(cmdData.clientRequest.talkData.equID, this, cmdData);
	return J_OK;
}

j_result_t CXlClient::SendRequest(const CXlDataBusInfo &cmdData)
{
	j_result_t nResult = J_OK;
	return nResult;
}

j_result_t CXlClient::SaveRequest(const CXlDataBusInfo &cmdData, j_boolean_t bSave)
{
	TLock(m_locker);
	if (bSave)
	{
		m_requestVec.push_back(cmdData);
	}
	else
	{
		RequestVec::iterator it = m_requestVec.begin();
		for (; it != m_requestVec.end(); it++)
		{
			if (it->header.seq == cmdData.header.seq)
			{
				m_requestVec.erase(it);
				break;
			}
		}
	}
	TUnlock(m_locker);

	return J_OK;
}

j_result_t CXlClient::EnableVehStatus(const CXlDataBusInfo &cmdData, j_boolean_t bEnable)
{
	TLock(m_locker);
	VehicleEnableMap::iterator it = m_vehEnableMap.find(cmdData.clientRequest.realAlarm.hostId);
	if (it != m_vehEnableMap.end())
	{
		switch (cmdData.header.cmd)
		{
		case CXlProtocol::xlc_vehicle_status:
			it->second.vehStatus = bEnable;
			break;
		case CXlProtocol::xlc_alarm_info:
			it->second.alarmInfo = bEnable;
			break;
		default:
			break;
		}

		if (bEnable == true)
		{
			JoDataBus->SubscribeMsg(cmdData.clientRequest.realAlarm.hostId, this, cmdData);
		}
		else
		{
			if (it->second.vehStatus == false && it->second.alarmInfo == false)
			{
				m_vehEnableMap.erase(it);
			}	
			JoDataBus->SubscribeMsg(cmdData.clientRequest.realAlarm.hostId, NULL, cmdData);

			XlClientResponse::ErrorCode data = { 0 };
			data.code = 0x00;
			CXlHelper::MakeResponse(cmdData.header.cmd, cmdData.header.flag, cmdData.header.seq,
				(j_char_t *)&data, sizeof(XlClientResponse::ErrorCode), m_dataBuff);
			J_StreamHeader streamHeader = { 0 };
			streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + sizeof(XlClientResponse::ErrorCode) + sizeof(CXlProtocol::CmdTail);
			m_ringBuffer.PushBuffer(m_dataBuff, streamHeader);
		}
	}
	else
	{
		if (bEnable == true)
		{
			VehicleStatusEnable enableInfo  = {0};
			switch (cmdData.header.cmd)
			{
			case CXlProtocol::xlc_vehicle_status:
				enableInfo.vehStatus = bEnable;
				break;
			case CXlProtocol::xlc_alarm_info:
				enableInfo.alarmInfo = bEnable;
				break;
			default:
				break;
			}
			m_vehEnableMap[cmdData.clientRequest.realAlarm.hostId] = enableInfo;
			JoDataBus->SubscribeMsg(cmdData.clientRequest.realAlarm.hostId, this, cmdData);
		}
	}
	TUnlock(m_locker);

	return J_OK;
}

j_result_t CXlClient::Recovery()
{
	return J_OK;
}

j_result_t CXlClient::SaveContext(const CXlDataBusInfo &cmdData)
{
	if (cmdData.header.flag == CXlProtocol::xl_ctrl_start)
	{
		m_lUserID = 0;
		m_transTargetMap.clear();
		m_strTitle.clear();
		m_strContext.clear();

		m_lUserID = cmdData.clientRequest.contextInfo.lUserID;
		for (int i = 0; i < cmdData.clientRequest.contextInfo.nDevCount; i++)
		{
			char chHostID[33] = { 0 };
			memcpy(chHostID, cmdData.clientRequest.contextInfo.pData + (i * 32), 32);
			m_transTargetMap.push_back(chHostID);
		}
		int nStrLen = cmdData.header.length - (cmdData.clientRequest.contextInfo.nDevCount * 32) - sizeof(XlClientRequest::ContextInfo) + 1;
		WCHAR *wChTitle = new WCHAR[nStrLen / 2 + 1];
		memcpy(wChTitle, cmdData.clientRequest.contextInfo.pData + (cmdData.clientRequest.contextInfo.nDevCount * 32), nStrLen);
		wChTitle[nStrLen / 2] = L'\0';
		CXlHelper::Unicode2Ansi(wChTitle, m_strTitle);
		delete[]wChTitle;
		//m_strTitle = cmdData.cmdContextInfo.pData + (cmdData.cmdContextInfo.nDevCount * 32);
	}
	else if (cmdData.header.flag == CXlProtocol::xl_ctrl_stream)
	{
		int nStrLen = cmdData.header.length;
		WCHAR *wChContent = new WCHAR[nStrLen / 2 + 1];
		memcpy(wChContent, cmdData.pData, nStrLen);
		wChContent[nStrLen / 2] = L'\0';
		CXlHelper::Unicode2Ansi(wChContent, m_strContext);
		delete[]wChContent;
		//m_strContext = cmdData.pData;
	}
	else if (cmdData.header.flag == CXlProtocol::xl_ctrl_end)
	{
		JoDataBaseObj->AddContextInfo(m_lUserID, m_strTitle.c_str(), m_strContext.c_str(), m_transTargetMap);

		std::vector<j_string_t>::iterator it = m_transTargetMap.begin();
		for (; it != m_transTargetMap.end(); it++)
		{
			JoDataBus->Request(*it, this, cmdData);
		}
		XlClientResponse::TransmitMessage data = { 0 };
		data.ulMessageID = 0;
		data.state = 0x00;
		CXlHelper::MakeResponse(CXlProtocol::xlc_trans_context, cmdData.header.flag, cmdData.header.seq,
			(j_char_t *)&data, sizeof(XlClientResponse::TransmitMessage), m_dataBuff);

		J_StreamHeader streamHeader = { 0 };
		streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + sizeof(XlClientResponse::TransmitMessage) + sizeof(CXlProtocol::CmdTail);
		m_ringBuffer.PushBuffer(m_dataBuff, streamHeader);
	}
	else
	{
		m_strContext.clear();
		JoDataBaseObj->UpdateContextInfo(m_lUserID, 2);																// 更新各个车收到消息的状态,同时间更新总信息的状态等待回传的消息状态
	}
	return J_OK;
}


/***********************************************************************************************************
 * 程序创建：刘进朝                     程序修改:赵进军
 * 函数功能：联络文件发送保存，以及数据库操作
 * 参数说明：
 *  cmdData：数据体
 * 注意事项：null
 * 修改日期：2015/10/15 11:33:00
 ***********************************************************************************************************/
j_result_t CXlClient::SaveFiles(const CXlDataBusInfo &cmdData)
{
	if (cmdData.header.flag == CXlProtocol::xl_ctrl_start)
	{
		m_lUserID = 0;
		m_transTargetMap.clear();
		m_strTitle.clear();
		m_strContext.clear();

		m_lUserID = cmdData.clientRequest.fileInfo.lUserID;
		for (int i = 0; i < cmdData.clientRequest.fileInfo.nDevCount; i++)
		{
			char chHostID[33] = { 0 };
			memcpy(chHostID, cmdData.clientRequest.fileInfo.pData + (i * 32), 32);
			m_transTargetMap.push_back(chHostID);
		}
		int nStrLen = cmdData.header.length - (cmdData.clientRequest.fileInfo.nDevCount * 32) - sizeof(XlClientRequest::FileInfo) + 1;
		WCHAR *wChTitle = new WCHAR[nStrLen / 2 + 1];
		memcpy(wChTitle, cmdData.clientRequest.fileInfo.pData + (cmdData.clientRequest.fileInfo.nDevCount * 32), nStrLen);
		wChTitle[nStrLen / 2] = L'\0';
		CXlHelper::Unicode2Ansi(wChTitle, m_strTitle);
		delete[]wChTitle;

		if (m_pFile == NULL)
		{
			char pFilePath[256] = { 0 };
			GetPrivateProfileString("file_info", "path", "E:/FileRecord", pFilePath, sizeof(pFilePath), g_ini_file);
			CreateDirectory(pFilePath, NULL);																		// 创建文件夹
			m_strContext = CXlHelper::RenameFile(m_strTitle);														// 重命名联络文件的名称 防止因联络文件名重复被覆盖
			sprintf(pFilePath, "%s/%s", pFilePath, m_strContext.c_str());
			m_pFile = fopen(pFilePath, "wb+");																		// 打开文件
			if (m_pFile == NULL)
			{
				// 预留错误处理
			}
		}
	}
	else if (cmdData.header.flag == CXlProtocol::xl_ctrl_stream)
	{
		if (NULL != m_pFile)																						// 判断当前要写入的文件是否已经被打开
			fwrite(cmdData.pData, 1, cmdData.header.length, m_pFile);
	}
	else if (cmdData.header.flag == CXlProtocol::xl_ctrl_end)
	{
		if (m_pFile != NULL)
		{
			fclose(m_pFile);
			m_pFile = NULL;
		}

		JoDataBaseObj->AddFileInfo(m_lUserID, m_strTitle.c_str(), m_strContext.c_str(), m_transTargetMap);
		std::vector<j_string_t>::iterator it = m_transTargetMap.begin();
		for (; it != m_transTargetMap.end(); it++)
		{
			JoDataBus->Request(*it, this, cmdData);
		}
		XlClientResponse::TransmitFile data = { 0 };
		data.ulFileID = 0;
		data.state = 0x00;
		CXlHelper::MakeResponse(CXlProtocol::xlc_upload_file, cmdData.header.flag, cmdData.header.seq,
			(j_char_t *)&data, sizeof(XlClientResponse::TransmitFile), m_dataBuff);

		J_StreamHeader streamHeader = { 0 };
		streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + sizeof(XlClientResponse::TransmitFile) + sizeof(CXlProtocol::CmdTail);
		m_ringBuffer.PushBuffer(m_dataBuff, streamHeader);
	}
	else
	{
		if (m_pFile != NULL)
		{
			fclose(m_pFile);
			m_pFile = NULL;
		}
		//JoDataBaseObj->DeleteFileInfo(m_lUserID, m_strTitle.c_str(), "", m_transTargetMap);
		// 取消数据发送
		JoDataBaseObj->UpdateFileInfo(m_lUserID, 2);																// 更新各个车收到文件的状态,同时间更新总文件的状态等待回传的文件状态
	}

	return J_OK;
}

j_result_t CXlClient::TalkBackCommand(const CXlDataBusInfo &cmdData)
{
	CXlHelper::MakeMessage(CXlProtocol::xlc_talk_cmd_in, cmdData.header.flag, cmdData.header.seq,
		(j_char_t *)cmdData.pData, cmdData.header.length, m_dataBuff);

	J_StreamHeader streamHeader = { 0 };
	streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + cmdData.header.length + sizeof(CXlProtocol::CmdTail);
	m_ringBuffer.PushBuffer(m_dataBuff, streamHeader);

	return J_OK;
}

j_result_t CXlClient::TalkBackData(const CXlDataBusInfo &cmdData)
{
	CXlHelper::MakeMessage(CXlProtocol::xlc_talk_data_in, cmdData.header.flag, cmdData.header.seq,
		(j_char_t *)cmdData.pData, cmdData.header.length, m_dataBuff);

	J_StreamHeader streamHeader = { 0 };
	streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + cmdData.header.length + sizeof(CXlProtocol::CmdTail);
	m_ringBuffer.PushBuffer(m_dataBuff, streamHeader);

	return J_OK;
}

j_result_t CXlClient::MessageBack(const CXlDataBusInfo &cmdData)
{
	CXlHelper::MakeMessage(CXlProtocol::xlc_message, cmdData.header.flag, cmdData.header.seq,
	(j_char_t *)cmdData.pData, cmdData.header.length, m_dataBuff);
	J_StreamHeader streamHeader = { 0 };
	streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + cmdData.header.length + sizeof(CXlProtocol::CmdTail);
	m_ringBuffer.PushBuffer(m_dataBuff, streamHeader);
	return J_OK;
}