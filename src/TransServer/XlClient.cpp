#include "XlClient.h"
#include "DeviceManager.h"
#include "ClientManager.h"
#include "XlHelper.h"
#include "DataBus.h"
#include "XlClientRespData.h"
#include "MySQLAccess.h"

#include <iostream>
#include <strstream>

#define XL_BUFFER_SIZE (1024 * 1024)

extern const char *g_ini_file;

CXlClient::CXlClient(j_socket_t nSock)
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
		CXlClientCmdData *pCmdData = (CXlClientCmdData *)m_readBuff;
		switch (pCmdData->cmdHeader.cmd)
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
			SaveRequest(*pCmdData, pCmdData->cmdHeader.flag == CXlProtocol::xl_ctrl_start);
			pCmdData->cmdRealPlay.pBuffer = &m_ringBuffer;
			JoDataBus->Request(pCmdData->cmdRealPlay.hostId, this, *pCmdData);
			break;
		case CXlProtocol::xlc_vod_play:
		case CXlProtocol::xlc_vod_download:
			SaveRequest(*pCmdData, pCmdData->cmdHeader.flag == CXlProtocol::xl_ctrl_start);
			pCmdData->cmdStartVod.pBuffer = &m_ringBuffer;
			JoDataBus->Request(pCmdData->cmdStartVod.hostId, this, *pCmdData);
			break;
		case CXlProtocol::xlc_real_alarm:
			EnableAlarm(*pCmdData, pCmdData->cmdHeader.flag == CXlProtocol::xl_ctrl_start);
			break;
		case CXlProtocol::xlc_trans_context:
			break;
		case CXlProtocol::xlc_upload_file:
			break;
		case CXlProtocol::xlc_talk_cmd_out:
			TalkBackCommand(*pCmdData);
			break;
		case CXlProtocol::xlc_talk_data_out:
			TalkBackData(*pCmdData);
			break;
		default:
			//assert(false);
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
		CXlClientRespData *pRespData = (CXlClientRespData *)m_writeBuff;
		//J_OS::LOGINFO("CXlClient::OnHandleWrite send len = %d flag = %d", m_streamHeader.dataLen, pRespData->respHeader.flag);
		if (pRespData->respHeader.length + sizeof(CXlProtocol::CmdHeader) + sizeof(CXlProtocol::CmdTail) != m_streamHeader.dataLen)
		{
			assert(false);
		}
	}

	return J_OK;
}

j_result_t CXlClient::OnResponse(const CXlClientRespData &respData)
{
	CXlHelper::MakeResponse(respData.respHeader.cmd, respData.respHeader.flag, respData.respHeader.seq,
		(j_char_t *)respData.pData, respData.respHeader.length, m_dataBuff);
	J_StreamHeader streamHeader = { 0 };
	streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + respData.respHeader.length + sizeof(CXlProtocol::CmdTail);
	m_ringBuffer.PushBuffer(m_dataBuff, streamHeader);

	return J_OK;
}

j_result_t CXlClient::OnMessage(j_string_t strHostId, const CXlClientRespData &respData)
{
	switch (respData.respHeader.cmd)
	{
	case CXlProtocol::xlc_real_alarm:
	{
		AlarmEnableMap::iterator it = m_alarmEnableMap.find(strHostId);
		if (it != m_alarmEnableMap.end() && it->second == false)
		{
			return J_OK;
		}
		break;
	}
	case CXlProtocol::xlc_dev_connected:
		Recovery();
		break;
	}

	CXlHelper::MakeResponse(respData.respHeader.cmd, respData.respHeader.flag, respData.respHeader.seq,
		(j_char_t *)respData.pData, sizeof(CXlClientRespData::RespErrorCode), m_dataBuff);
	J_StreamHeader streamHeader = { 0 };
	streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + sizeof(CXlClientRespData::RespLogin) + sizeof(CXlProtocol::CmdTail);
	m_ringBuffer.PushBuffer(m_dataBuff, streamHeader);


	return J_OK;
}

j_result_t CXlClient::OnBroken()
{
	TLock(m_locker);
	RequestVec::iterator it = m_requestVec.begin();
	for (; it != m_requestVec.end(); it++)
	{
		it->cmdHeader.flag = CXlProtocol::xl_pack_end;
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
	JoDataBus->SubscribeMsg(m_userName, NULL);

	return J_OK;
}

j_boolean_t CXlClient::IsReady()
{
	//if (time(0) - m_lastBreatTime > 30)
	//	return false;

	return true;
}

j_result_t CXlClient::OnLogin(const CXlClientCmdData &cmdData)
{
	CXlClientRespData::RespLogin data = { 0 };
	JoClientManager->Login(cmdData.cmdLogin.userName, cmdData.cmdLogin.passWord, cmdData.cmdLogin.nForced, data.code, this);
	memset(m_userName, 0, sizeof(m_userName));
	strcpy(m_userName, cmdData.cmdLogin.userName);
	if (data.code == 0)
	{
		JoDataBus->SubscribeMsg(m_userName, this);
		m_lastBreatTime = time(0);
	}
	CXlHelper::MakeResponse(CXlProtocol::xlc_login, cmdData.cmdHeader.flag, cmdData.cmdHeader.seq, 
		(j_char_t *)&data, sizeof(CXlClientRespData::RespLogin), m_dataBuff);

	J_StreamHeader streamHeader = { 0 };
	streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + sizeof(CXlClientRespData::RespLogin) + sizeof(CXlProtocol::CmdTail);
	m_ringBuffer.PushBuffer(m_dataBuff, streamHeader);

	return J_OK;
}

j_result_t CXlClient::OnLogout(const CXlClientCmdData &cmdData)
{
	JoClientManager->Logout(m_userName, this);
	CXlClientRespData::RespLogout data = { 0 };
	data.code = 0x00;
	CXlHelper::MakeResponse(CXlProtocol::xlc_logout, cmdData.cmdHeader.flag, cmdData.cmdHeader.seq,
		(j_char_t *)&data, sizeof(CXlClientRespData::RespLogout), m_dataBuff);

	J_StreamHeader streamHeader = { 0 };
	streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + sizeof(CXlClientRespData::RespLogout) + sizeof(CXlProtocol::CmdTail);
	m_ringBuffer.PushBuffer(m_dataBuff, streamHeader);

	return J_OK;
}

j_result_t CXlClient::OnHeartBeat(const CXlClientCmdData &cmdData)
{
	m_lastBreatTime = time(0);
	return J_OK;
}

j_result_t CXlClient::SendRequest(const CXlClientCmdData &cmdData)
{
	j_result_t nResult = J_OK;
	return nResult;
}

j_result_t CXlClient::SaveRequest(const CXlClientCmdData &cmdData, j_boolean_t bSave)
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
			if (it->cmdHeader.seq == cmdData.cmdHeader.seq)
			{
				m_requestVec.erase(it);
				break;
			}
		}
	}
	TUnlock(m_locker);

	return J_OK;
}

j_result_t CXlClient::EnableAlarm(const CXlClientCmdData &cmdData, j_boolean_t bEnable)
{
	TLock(m_locker);
	AlarmEnableMap::iterator it = m_alarmEnableMap.find(cmdData.cmdRealAlarm.hostId);
	if (it != m_alarmEnableMap.end())
	{
		if (bEnable == false)
		{
			m_alarmEnableMap.erase(it);

			CXlClientRespData::RespErrorCode data = { 0 };
			data.code = 0x00;
			CXlHelper::MakeResponse(CXlProtocol::xlc_real_alarm, cmdData.cmdHeader.flag, cmdData.cmdHeader.seq,
				(j_char_t *)&data, sizeof(CXlClientRespData::RespLogout), m_dataBuff);
			J_StreamHeader streamHeader = { 0 };
			streamHeader.dataLen = sizeof(CXlProtocol::CmdHeader) + sizeof(CXlClientRespData::RespErrorCode) + sizeof(CXlProtocol::CmdTail);
			m_ringBuffer.PushBuffer(m_dataBuff, streamHeader);
		}
	}
	else
	{
		if (bEnable == true)
		{
			m_alarmEnableMap[cmdData.cmdRealAlarm.hostId] = true;
		}
	}
	TUnlock(m_locker);

	return J_OK;
}

j_result_t CXlClient::Recovery()
{
	return J_OK;
}

j_result_t CXlClient::SaveContext(const CXlClientCmdData &cmdData, j_boolean_t bSave)
{
	if (cmdData.cmdHeader.flag == CXlProtocol::xl_ctrl_start)
	{
		m_lUserID = 0;
		m_transTargetMap.clear();
		m_strTitle.clear();
		m_strContext.clear();

		m_lUserID = cmdData.cmdContextInfo.lUserID;
		for (int i=0; i<cmdData.cmdContextInfo.nDevCount; i++)
		{
			char chHostID[33] = {0};
			memcpy(chHostID, cmdData.cmdContextInfo.pData + (i * 32), 32);
			m_transTargetMap.push_back(chHostID);
		}
		m_strTitle = cmdData.cmdContextInfo.pData + (cmdData.cmdContextInfo.nDevCount * 32);
	}
	else if (cmdData.cmdHeader.flag == CXlProtocol::xl_ctrl_stream)
	{
		m_strContext = cmdData.pData;
	}
	else if (cmdData.cmdHeader.flag == CXlProtocol::xl_ctrl_end)
	{
		JoDataBaseObj->UpdateContextInfo(m_lUserID, m_strTitle.c_str(), m_strContext.c_str(), m_transTargetMap);

		std::vector<j_string_t>::iterator it = m_transTargetMap.begin();
		for (; it!=m_transTargetMap.end(); it++)
		{
			JoDataBus->Request(*it, this, cmdData);
		}
	}
	else
	{
		m_strContext.clear();
		JoDataBaseObj->UpdateContextInfo(m_lUserID, m_strTitle.c_str(), "", m_transTargetMap);
	}

	return J_OK;
}


j_result_t CXlClient::SaveFiles(const CXlClientCmdData &cmdData, j_boolean_t bSave)
{
	if (cmdData.cmdHeader.flag == CXlProtocol::xl_ctrl_start)
	{
		m_lUserID = 0;
		m_transTargetMap.clear();
		m_strTitle.clear();
		m_strContext.clear();

		m_lUserID = cmdData.cmdFileInfo.lUserID;
		for (int i=0; i<cmdData.cmdFileInfo.nDevCount; i++)
		{
			char chHostID[33] = {0};
			memcpy(chHostID, cmdData.cmdFileInfo.pData + (i * 32), 32);
			m_transTargetMap.push_back(chHostID);
		}
		m_strTitle = cmdData.cmdFileInfo.pData + (cmdData.cmdFileInfo.nDevCount * 32);

		if (m_pFile == NULL)
		{
			char pFilePath[256] = {0}; 
			GetPrivateProfileString("file_info", "path", "C:\\FileRecord", pFilePath, sizeof(pFilePath), g_ini_file);
			sprintf(pFilePath, "%s\\%s", pFilePath, m_strTitle.c_str());
			m_strContext = pFilePath;

			m_pFile = fopen(pFilePath, "wb+");
		}
	}
	else if (cmdData.cmdHeader.flag == CXlProtocol::xl_ctrl_stream)
	{
		fwrite(cmdData.pData, 1, cmdData.cmdHeader.length, m_pFile);
	}
	else if (cmdData.cmdHeader.flag == CXlProtocol::xl_ctrl_end)
	{
		JoDataBaseObj->UpdateFileInfo(m_lUserID, m_strTitle.c_str(), m_strContext.c_str(), m_transTargetMap);

		std::vector<j_string_t>::iterator it = m_transTargetMap.begin();
		for (; it!=m_transTargetMap.end(); it++)
		{
			JoDataBus->Request(*it, this, cmdData);
		}
	}
	else
	{
		fclose(m_pFile);
		m_pFile = NULL;
		JoDataBaseObj->UpdateFileInfo(m_lUserID, m_strTitle.c_str(), "", m_transTargetMap);
	}

	return J_OK;
}

j_result_t CXlClient::TalkBackCommand(const CXlClientCmdData &cmdData)
{
	return J_OK;
}

j_result_t CXlClient::TalkBackData(const CXlClientCmdData &cmdData)
{
	return J_OK;
}