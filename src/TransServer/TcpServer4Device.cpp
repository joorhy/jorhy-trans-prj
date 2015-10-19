/** 
*  Copyright (c) 2013, xx科技有限公司 
*  All rights reserved. 
* 
*  @file        TcpServer.cpp 
*  @author      Jorhy(joorhy@gmail.com) 
*  @date        2013/09/18 11:23 
*/  
#include "TcpServer4Device.h"
#include "DeviceManager.h"
#include "XlHelper.h"
#include "XlProtocol.h"
#include "XlDataBusDef.h"
#include "DataBus.h"

CTcpServer4Device::CTcpServer4Device()
{
	J_OS::LOGINFO("CTcpServer4Device::CTcpServer4Device() construct");
}

CTcpServer4Device::~CTcpServer4Device()
{
	J_OS::LOGINFO("CTcpServer4Device::~CTcpServer4Device() destruct");
}

j_result_t CTcpServer4Device::StartServer(const j_string_t &strIniPath)
{
	int nPort = GetPrivateProfileInt("device", "port", 8503, strIniPath.c_str());
	memset(&m_asioData, 0, sizeof(m_asioData));
	m_asioData.ioAccept.peerPort = nPort;
	m_asioData.ioUser = this;
	m_asio.Listen(&m_asioData);

	return J_OK;
}

j_result_t CTcpServer4Device::StopServer()
{
	return J_OK;
}

j_result_t CTcpServer4Device::OnAccept(J_AsioDataBase *pAsioData, int nRet)
{
	//u_long ret = 0;
	//ioctlsocket(pAsioData->ioAccept.subHandle, FIONBIO, &ret);

	j_socket_t nSocket = pAsioData->ioAccept.subHandle;
	m_asio.AddUser(nSocket, this);

	J_AsioDataBase *pAsioReadData = new J_AsioDataBase;
	memset(pAsioReadData, 0, sizeof(J_AsioDataBase));

	J_AsioDataBase *pAsioWriteData = new J_AsioDataBase;
	memset(pAsioWriteData, 0, sizeof(J_AsioDataBase));

	DeviceInfo dataInfo;
	dataInfo.strHostId = "";
	dataInfo.asioDataInfo.pAsioReadData = pAsioReadData;
	dataInfo.asioDataInfo.pAsioWriteData = pAsioWriteData;
	m_devMap[nSocket] = dataInfo;

	/// 发送注册请求
	j_char_t requestBuff[32] = { 0 };
	CXlHelper::MakeRequest(CXlProtocol::xld_register, CXlProtocol::xl_ctrl_start, JoDataBus->GetUniqueSeq()
		, NULL, 0, (j_char_t *)requestBuff);

	//pAsioWriteData->ioHandle = pAsioData->ioAccept.subHandle;
	pAsioWriteData->ioUser = this;
	pAsioWriteData->ioCall = J_AsioDataBase::j_write_e;

	pAsioWriteData->ioWrite.buf = requestBuff;
	pAsioWriteData->ioWrite.bufLen = sizeof(CXlProtocol::CmdHeader) + sizeof(CXlProtocol::CmdTail);
	pAsioWriteData->ioWrite.finishedLen = 0;
	pAsioWriteData->ioWrite.whole = true;
	pAsioWriteData->ioWrite.shared = true;
	m_asio.Write(nSocket, pAsioWriteData);
	
	/// 接收设备ID
	j_char_t *pReponseBuff = (j_char_t *)malloc(64);
	//pAsioReadData->ioHandle = pAsioData->ioAccept.subHandle;
	pAsioReadData->ioUser = this;
	pAsioReadData->ioCall = J_AsioDataBase::j_read_e;
	CXlHelper::MakeNetData(pAsioReadData, pReponseBuff, sizeof(CXlProtocol::CmdHeader) + sizeof(CXlDataBusInfo::XldRespHostId) + sizeof(CXlProtocol::CmdTail));
	m_asio.Read(nSocket, pAsioReadData);

	return J_OK;
}

j_result_t CTcpServer4Device::OnRead(J_AsioDataBase *pAsioData, int nRet)
{
	j_socket_t nSocket = pAsioData->ioHandle;
	J_Host *pHost = NULL;
	DeviceMap::iterator it = m_devMap.find(nSocket);
	if (it != m_devMap.end())
	{
		if (it->second.strHostId == "")
		{
			CXlDataBusInfo *respData = (CXlDataBusInfo *)pAsioData->ioRead.buf;
			pHost = JoDeviceManager->CreateDevObj(1, respData->xldRespHostId.hostId);
			if (pHost != NULL)
			{
				it->second.strHostId = respData->xldRespHostId.hostId;

				pAsioData->ioUser = this;
				pHost->OnHandleRead(pAsioData);
				m_asio.Read(nSocket, pAsioData);
			}
		}
		else
		{
			pHost = JoDeviceManager->GetDeviceObj(it->second.strHostId);
			if (pHost != NULL)
			{
				pAsioData->ioUser = this;
				pHost->OnHandleRead(pAsioData);
				if (pAsioData->ioRead.bufLen > 1000)
				{
					J_OS::LOGDEBUG("CTcpServer4Device::OnRead %d ", pAsioData->ioRead.bufLen);
				}
				m_asio.Read(nSocket, pAsioData);
			}
		}
	}

	return J_OK;
}

j_result_t CTcpServer4Device::OnWrite(J_AsioDataBase *pAsioData, int nRet)
{
	j_socket_t nSocket = pAsioData->ioHandle;
	DeviceMap::iterator it = m_devMap.find(nSocket);
	if (it != m_devMap.end())
	{
		J_Host *pHost = JoDeviceManager->GetDeviceObj(it->second.strHostId);
		if (pHost != NULL)
		{
			it->second.asioDataInfo.pAsioWriteData->ioUser = this;
			pHost->OnHandleWrite(it->second.asioDataInfo.pAsioWriteData);
			if (it->second.asioDataInfo.pAsioWriteData->ioWrite.bufLen > 0)
			{
				m_asio.Write(nSocket, it->second.asioDataInfo.pAsioWriteData);
			}
		}
	}
	return J_OK;
}

j_result_t CTcpServer4Device::OnBroken(J_AsioDataBase *pAsioData, int nRet)
{
	j_socket_t nSocket = pAsioData->ioHandle;
	DeviceMap::iterator it = m_devMap.find(nSocket);
	if (it != m_devMap.end())
	{
		delete it->second.asioDataInfo.pAsioReadData;
		delete it->second.asioDataInfo.pAsioWriteData;
		JoDeviceManager->ReleaseDevObj(it->second.strHostId);
		m_devMap.erase(it);
	}

	m_asio.DelUser(nSocket);

	return J_OK;
}