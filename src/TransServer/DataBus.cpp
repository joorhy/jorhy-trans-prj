#include "DataBus.h"

JO_IMPLEMENT_SINGLETON(DataBus)

CDataBus::CDataBus()
{
	m_nUniqueSeq = 10000;
	m_timer.Create(1000, CDataBus::OnTimer, this);
}

CDataBus::~CDataBus()
{
	m_timer.Destroy();
}

j_result_t CDataBus::RegisterDevice(j_string_t strHostId, J_DataBus *pHost)
{
	TLock(m_locker);
	CXlDataBusInfo equipmentState = { 0 };
	equipmentState.header.beginCode = 0xFF;
	equipmentState.header.version = CXlProtocol::xl_frame_message;
	equipmentState.header.seq = GetUniqueSeq();
	equipmentState.header.flag = CXlProtocol::xl_ctrl_data;
	equipmentState.header.cmd = CXlProtocol::xlc_dev_state;
	equipmentState.header.length = sizeof(XlClientResponse::EquipmentState);
	memcpy (equipmentState.clientResponse.equipmentState.szID, strHostId.c_str(), strlen(strHostId.c_str()));
	ObjectMap::iterator it = m_objectMap.find(strHostId);
	if (it != m_objectMap.end())
	{
		if (pHost == NULL)
		{
			equipmentState.clientResponse.equipmentState.state = 0;
			OnMessage("", equipmentState);
			m_objectMap.erase(it);
		}
	}
	else
	{
		if (pHost != NULL)
		{
			m_objectMap[strHostId] = pHost;
			equipmentState.clientResponse.equipmentState.state = 1;
			OnMessage("", equipmentState);
		}
	}
	TUnlock(m_locker);

	return J_OK;
}

j_result_t CDataBus::ClearRequest(J_DataBus *pClient)
{
	TLock(m_locker);
	ResponseMap::iterator it = m_responseMap.begin();
	for (; it != m_responseMap.end();)
	{
		RequestMap::iterator itReqest = m_requestMap.find(it->second);
		if (itReqest != m_requestMap.end())
		{
			m_requestMap.erase(itReqest);
		}

		if (it->second.pClient == pClient)
		{
			m_responseMap.erase(it++);
		}
		else
		{
			it++;
		}
	}
	TUnlock(m_locker);

	return J_OK;
}

j_result_t CDataBus::SubscribeMsg(j_string_t strUserId, J_DataBus *pClient, const CXlDataBusInfo &cmdData)
{
	TLock(m_locker);
	MessageMap::iterator it = m_messageMap.find(strUserId);
	if (it != m_messageMap.end())
	{
		if (pClient == NULL)
		{
			/// 退订消息
			std::vector<CXlProtocol::CmdHeader>::iterator itHeader = it->second.headerVec.begin();
			for (; itHeader != it->second.headerVec.end(); itHeader++)
			{
				if (cmdData.header.cmd == itHeader->cmd)
				{
					it->second.headerVec.erase(itHeader);
					break;
				}
			}

			if (it->second.headerVec.size() == 0)
			{
				m_messageMap.erase(it);
			}
		}
		else
		{
			it->second.headerVec.push_back(cmdData.header);
		}
	}
	else
	{
		/// 初次订阅消息,没有建立与消息的映射
		if (pClient != NULL)
		{
			std::vector<CXlProtocol::CmdHeader> headerVec;
			headerVec.push_back(cmdData.header);
			SubMessageInfo info = {0};
			info.pClient = pClient;
			info.headerVec = headerVec;
			m_messageMap[strUserId] = info;
		}
	}
	TUnlock(m_locker);

	return J_OK;
}

j_result_t CDataBus::ClearMessage(J_DataBus *pClient)
{
	TLock(m_locker);
	MessageMap::iterator it = m_messageMap.begin();
	for (; it != m_messageMap.end();)
	{
		if (it->second.pClient == pClient)
		{
			m_messageMap.erase(it++);
		}
		else
		{
			it++;
		}
	}
	TUnlock(m_locker);

	return J_OK;
}

j_result_t CDataBus::Request(j_string_t strHostId, J_DataBus *pClient, const CXlDataBusInfo cmdData, j_uint32_t nTimeOut)
{
	TLock(m_locker);
	ObjectMap::iterator itHost = m_objectMap.find(strHostId);
	if (itHost == m_objectMap.end())
	{
		TUnlock(m_locker);
		return J_NOT_EXIST;
	}

	RequestKey key(pClient, cmdData.header.seq);
	RequestMap::iterator it = m_requestMap.find(key);
	if (it != m_requestMap.end())
	{
		if (cmdData.header.flag == CXlProtocol::xl_ctrl_end || cmdData.header.flag == CXlProtocol::xl_ctrl_stop)
		{
			const_cast<CXlDataBusInfo &>(cmdData).header.seq = it->second.nSeq;
			itHost->second->OnRequest(cmdData);
		}
	}
	else
	{
		/// 请求数据
		j_uint32_t nSeq = GetUniqueSeq();
		m_responseMap[nSeq] = key;
		
		RequestInfo info = {0};
		info.pClient = pClient;
		info.cmdHeader = cmdData.header;
		info.nTimeOut = nTimeOut;
		info.nSeq = nSeq;
		m_requestMap[key] = info;

		const_cast<CXlDataBusInfo &>(cmdData).header.seq = nSeq;
		itHost->second->OnRequest(cmdData);
	}
	TUnlock(m_locker);
	return J_OK;
}

j_result_t CDataBus::RequestStateless(j_string_t strHostId, J_DataBus *pClient, const CXlDataBusInfo &cmdData)
{
	TLock(m_locker);
	ObjectMap::iterator itHost = m_objectMap.find(strHostId);
	if (itHost == m_objectMap.end())
	{
		TUnlock(m_locker);
		return J_NOT_EXIST;
	}

	itHost->second->OnRequest(cmdData);
	TUnlock(m_locker);
	return J_OK;
}

j_result_t CDataBus::Response(const CXlDataBusInfo &respData)
{
	TLock(m_locker);
	ResponseMap::iterator it = m_responseMap.find(respData.header.seq);
	if (it != m_responseMap.end())
	{
		/// 删除请求记录
		RequestMap::iterator itReqest = m_requestMap.find(it->second);
		if (itReqest != m_requestMap.end())
		{
			if (respData.header.flag == CXlProtocol::xl_ctrl_end || respData.header.flag == CXlProtocol::xl_ctrl_stop)
			{
				m_requestMap.erase(itReqest);
			}
			else if (respData.header.flag == CXlProtocol::xl_ctrl_data)
			{
				itReqest->second.nTimeOut = 3000;
			}
		}

		/// 回复数据
		const_cast<CXlDataBusInfo &>(respData).header.seq = it->second.nSeq;
		it->second.pClient->OnResponse(respData);
		if (respData.header.flag == CXlProtocol::xl_ctrl_end || respData.header.flag == CXlProtocol::xl_ctrl_stop)
		{
			m_responseMap.erase(it);
		}
	}
	TUnlock(m_locker);

	return J_OK;
}

j_result_t CDataBus::OnMessage(j_string_t strHostId, const CXlDataBusInfo &respData)
{
	TLock(m_locker);
	if (strHostId == "")
	{
		MessageMap::iterator it = m_messageMap.begin();
		for (; it != m_messageMap.end(); it++)
		{
			it->second.pClient->OnMessage(strHostId, respData);
		}
	}
	else
	{
		MessageMap::iterator it = m_messageMap.begin();
		for (; it != m_messageMap.end(); it++)
		{
			/// 发送消息
			std::vector<CXlProtocol::CmdHeader>::iterator itHeader = it->second.headerVec.begin();
			for (; itHeader != it->second.headerVec.end(); itHeader++)
			{
				if (respData.header.cmd == itHeader->cmd)
				{
					const_cast<CXlDataBusInfo &>(respData).header.seq = itHeader->seq;
					break;
				}
			}
			it->second.pClient->OnMessage(strHostId, respData);
		}
	}
	TUnlock(m_locker);

	return J_OK;
}

void CDataBus::CheckState()
{
	TLock(m_locker);
	RequestMap::iterator it = m_requestMap.begin();
	for (; it != m_requestMap.end();)
	{
		/// 判断是否超时
		if (it->second.nTimeOut > 0)
		{
			it->second.nTimeOut -= 1000;
			it++;
		}
		else
		{
			/// 发送超时消息
			CXlDataBusInfo respData = { 0 };
			respData.header = it->second.cmdHeader;
			respData.header.length = 0;
			respData.header.flag = CXlProtocol::xl_ctrl_stop;

			it->second.pClient->OnResponse(respData);
			m_responseMap.erase(it->second.nSeq);
			m_requestMap.erase(it++);
		}
	}
	TUnlock(m_locker);
}