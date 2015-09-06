///////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2009, xx科技有限公司(版权声明) 
/// All rights reserved. 
/// 
/// @file      TcpServer4Dahua.cpp 
/// @brief     客户端TCP服务器模块实现
///
/// @version   1.0 (版本声明)
/// @author    Jorhy (joorhy@gmail.com) 
/// @date      2015/07/15 15:18 
///
///
/// 修订说明：最初版本
///////////////////////////////////////////////////////////////////////////  
#include "TcpServer4Dahua.h"
#include "DeviceManager.h"

CTcpServer4Dahua::CTcpServer4Dahua()
{
	m_handle = 0;
	CLIENT_Init(ffDisConnect, (DWORD)this);
}

CTcpServer4Dahua::~CTcpServer4Dahua()
{
	CLIENT_Cleanup();
}

j_result_t CTcpServer4Dahua::StartServer(const j_string_t &strIniPath)
{
	int nPort = GetPrivateProfileInt("dahua", "port", 8502, strIniPath.c_str());
	m_handle = CLIENT_ListenServer("192.168.1.200", nPort, 0, ffServiceCallBack, (DWORD)this);
	if (m_handle == 0)
	{
		return J_UNKNOW;
	}

	return J_OK;
}

j_result_t CTcpServer4Dahua::StopServer()
{
	CLIENT_StopListenServer(m_handle);
	m_handle = 0;

	return J_OK;
}

void CTcpServer4Dahua::OnServiceCallBack(LLONG lHandle, char *ip, WORD port, LONG lCommand, void *pParam, DWORD dwParamLen)
{
	char *deviceNo = NULL;
	if (lCommand == DH_DVR_SERIAL_RETURN)
	{
		deviceNo = (char*)pParam;
		BOOL ret = CLIENT_ResponseDevReg(deviceNo, ip, port, true);
		if (ret)
		{
			J_OS::LOGINFO("client_responseDevReg successfully");
		}
		else
		{
			J_OS::LOGINFO("client_responseDevReg failed");
		}

		if (JoDeviceManager->GetDeviceObj(deviceNo) == NULL)
		{
			NET_DEVICEINFO lpDeviceInfo = { 0 };
			int error = 0;
			LONG lLoginHandle = CLIENT_LoginEx(ip, 37776, "admin", "admin", 0, deviceNo, &lpDeviceInfo, &error);
			//LONG lLoginHandle = CLIENT_LoginEx(ip, port, "admin", "admin", 2, deviceNo, &lpDeviceInfo, &error);
			if (lLoginHandle)
			{
				JoDeviceManager->CreateDevObj(3, deviceNo, lLoginHandle);
				TLock(m_locker);
				m_devMap[lLoginHandle] = deviceNo;
				TUnlock(m_locker);

				J_OS::LOGINFO("login successfully,device id is %s", deviceNo);
			}
			else
			{
				J_OS::LOGINFO("login failed");
			}
		}
	}
}

void CTcpServer4Dahua::OnDisConnect(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort)
{
	TLock(m_locker);
	DeviceMap::iterator it = m_devMap.find(lLoginID);
	if (it != m_devMap.end())
	{
		JoDeviceManager->ReleaseDevObj(it->second);
		m_devMap.erase(it);
	}
	TUnlock(m_locker);
}