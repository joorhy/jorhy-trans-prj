///////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2009, xx科技有限公司(版权声明) 
/// All rights reserved. 
/// 
/// @file      TcpServer4Mobile.cpp 
/// @brief     客户端TCP服务器模块实现
///
/// @version   1.0 (版本声明)
/// @author    Jorhy (joorhy@gmail.com) 
/// @date      2015/04/22 09:50 
///
///
/// 修订说明：最初版本
///////////////////////////////////////////////////////////////////////////  
#include "TcpServer4RTSP.h"
#include "ClientManager.h"

CTcpServer4RTSP::CTcpServer4RTSP()
{
	J_OS::LOGINFO("CTcpServer4RTSP::CTcpServer4RTSP() construct");
}

CTcpServer4RTSP::~CTcpServer4RTSP()
{
	J_OS::LOGINFO("CTcpServer4RTSP::~CTcpServer4RTSP() destruct");
}

j_result_t CTcpServer4RTSP::StartServer(const j_string_t &strIniPath)
{
	int nPort = GetPrivateProfileInt("rtsp", "port", 554, strIniPath.c_str());
	memset(&m_asioData, 0, sizeof(m_asioData));
	m_asioData.ioAccept.peerPort = nPort;
	m_asioData.ioUser = this;
	m_asio.Listen(&m_asioData);

	return J_OK;
}

j_result_t CTcpServer4RTSP::StopServer()
{
	return J_OK;
}

j_result_t CTcpServer4RTSP::OnAccept(J_AsioDataBase *pAsioData, int nRet)
{
	//u_long ret = 0;
	//ioctlsocket(pAsioData->ioAccept.subHandle, FIONBIO, &ret);

	j_socket_t nSocket = pAsioData->ioAccept.subHandle;
	m_asio.AddUser(nSocket, this);
	J_Client *pClient = JoClientManager->CreateClientObj(nSocket, RTSP_CLIENT);

	if (pClient != NULL)
	{
		J_AsioDataBase *pAsioReadData = new J_AsioDataBase;
		memset(pAsioReadData, 0, sizeof(J_AsioDataBase));

		J_AsioDataBase *pAsioWriteData = new J_AsioDataBase;
		memset(pAsioWriteData, 0, sizeof(J_AsioDataBase));

		AsioDataInfo dataInfo;
		dataInfo.pAsioReadData = pAsioReadData;
		dataInfo.pAsioWriteData = pAsioWriteData;
		m_asioDataMap[nSocket] = dataInfo;

		pAsioReadData->ioUser = this;
		pClient->OnHandleRead(pAsioReadData);
		m_asio.Read(nSocket, pAsioReadData);
	}

	return J_OK;
}

j_result_t CTcpServer4RTSP::OnRead(J_AsioDataBase *pAsioData, int nRet)
{
	j_socket_t nSocket = pAsioData->ioHandle;
	J_Client *pClient = JoClientManager->GetClientObj(nSocket);
	if (pClient != NULL)
	{
		pAsioData->ioUser = this;
		pClient->OnHandleRead(pAsioData);
		m_asio.Read(nSocket, pAsioData);
	}
	
	return J_OK;
}

j_result_t CTcpServer4RTSP::OnWrite(J_AsioDataBase *pAsioData, int nRet)
{
	j_socket_t nSocket = pAsioData->ioHandle;
	J_Client *pClient = JoClientManager->GetClientObj(nSocket);
	if (pClient != NULL)
	{
		J_AsioDataMap::iterator it = m_asioDataMap.find(nSocket);
		if (it != m_asioDataMap.end())
		{
			it->second.pAsioWriteData->ioUser = this;
			pClient->OnHandleWrite(it->second.pAsioWriteData);
			if (it->second.pAsioWriteData->ioWrite.bufLen > 0)
			{
				m_asio.Write(nSocket, it->second.pAsioWriteData);
			}
		}
	}

	return J_OK;
}

j_result_t CTcpServer4RTSP::OnBroken(J_AsioDataBase *pAsioData, int nRet)
{
	j_socket_t nSocket = pAsioData->ioHandle;
	J_AsioDataMap::iterator it = m_asioDataMap.find(nSocket);
	if (it != m_asioDataMap.end())
	{
		delete it->second.pAsioReadData;
		delete it->second.pAsioWriteData;
		m_asioDataMap.erase(it);
	}
	m_asio.DelUser(nSocket);
	JoClientManager->ReleaseClientObj(nSocket);

	return J_OK;
}