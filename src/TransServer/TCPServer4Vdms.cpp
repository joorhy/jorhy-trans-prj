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
#include "TcpServer4Vdms.h"
#include "DeviceManager.h"
#include "netmediaapi.h"

CTcpServer4Vdms::CTcpServer4Vdms()
	: m_bLogining(false)
	, m_bLoginSuccess(false)
{
	NETCLIENT_Initialize();
	NETMEDIA_Initialize(0); // 0-TCP, 1-UDP
}

CTcpServer4Vdms::~CTcpServer4Vdms()
{
	NETMEDIA_UnInitialize();
	NETCLIENT_UnInitialize();
}

j_result_t CTcpServer4Vdms::StartServer(const j_string_t &strIniPath)
{
	char strAddr[16] = { 0 };
	char strUser[32] = { 0 };
	char strPassWord[32] = { 0 };
	int nPort = GetPrivateProfileInt("platform", "port", 6605, strIniPath.c_str());
	GetPrivateProfileString("platform", "addr", "222.214.218.237", strAddr, sizeof(strAddr), strIniPath.c_str());
	GetPrivateProfileString("platform", "user", "server02", strUser, sizeof(strUser), strIniPath.c_str());
	GetPrivateProfileString("platform", "password", "000000", strPassWord, sizeof(strPassWord), strIniPath.c_str());

	m_bLogining = TRUE;
	NETCLIENT_RegLoginMsg(this, FuncLoginMsg);
	NETCLIENT_LoginDirSvr(strAddr, nPort, "", strUser, strPassWord, 0);

	return J_OK;
}

j_result_t CTcpServer4Vdms::StopServer()
{
	NETCLIENT_RegLoginMsg(NULL, NULL);
	NETCLIENT_LogoutDirSvr();
	if (m_bLoginSuccess)
	{
		StopDeviceLoad();
		m_bLoginSuccess = FALSE;
	}
	m_bLogining = FALSE;

	return J_OK;
}

void CTcpServer4Vdms::StartDeviceLoad(j_boolean_t bReload)
{
	StopDeviceLoad();
	m_bReloadDev = bReload;
	NETCLIENT_DEVOpenDevDownEx(&m_lLoadHandle);
	NETCLIENT_DEVRegDevDownCBEx(m_lLoadHandle, this, FuncDownloadDev);
	NETCLIENT_DEVStartDevDownEx(m_lLoadHandle, 0, 0);
}

void CTcpServer4Vdms::StopDeviceLoad()
{
	NETCLIENT_DEVStopDevDownEx(m_lLoadHandle);
	NETCLIENT_DEVCloseDevDownEx(m_lLoadHandle);
	m_lLoadHandle = NULL;
}

void CTcpServer4Vdms::StartDevAlarm()
{
	int nDeviceCount = m_deviceMap.size();
	GPSDeviceIDNO_S *pGPSDevInfo = (GPSDeviceIDNO_S *)malloc(sizeof(GPSDeviceIDNO_S) * nDeviceCount);
	int iCount = 0;
	for (DeviceMap::iterator it = m_deviceMap.begin(); it != m_deviceMap.end(); it++)
	{
		memset((pGPSDevInfo + iCount)->szDevIDNO, 0, sizeof((pGPSDevInfo + iCount)->szDevIDNO));
		memcpy((pGPSDevInfo + iCount)->szDevIDNO, it->second.szIDNO, sizeof(it->second.szIDNO));
		iCount++;
	}

	if (m_lAlarmHaneld == NULL)
	{
		NETCLIENT_VAOpenVehiAlarm(&m_lAlarmHaneld);
		NETCLIENT_VASetAlarmDevice(m_lAlarmHaneld, pGPSDevInfo, nDeviceCount);
		NETCLIENT_VASetAlarmInfoCB(m_lAlarmHaneld, this, FuncAlarmInfo);
		NETCLIENT_VAStartVAChan(m_lAlarmHaneld);
	}
	free(pGPSDevInfo);
}

void CTcpServer4Vdms::StopDevAlarm()
{
	if (m_lAlarmHaneld != NULL)
	{
		NETCLIENT_VAStopVAChan(m_lAlarmHaneld);
		NETCLIENT_VACloseVehiAlarm(m_lAlarmHaneld);
		m_lAlarmHaneld = NULL;
		m_deviceMap.clear();
	}
}

void CTcpServer4Vdms::DoLoginMsg(int nMsg)
{
	switch (nMsg)
	{
	case GPS_LOGIN_SUC:
		m_bLoginSuccess = true;
		StartDeviceLoad(FALSE);	//开始加载设备信息
		J_OS::LOGINFO("GPS_LOGIN_SUC");
		break;
	case GPS_LOGIN_FAILED:
		J_OS::LOGINFO("GPS_LOGIN_FAILED");
		m_bLoginSuccess = false;
		break;
	case GPS_LOGIN_DISCNT:
		J_OS::LOGINFO("GPS_LOGIN_DISCNT");
		m_bLoginSuccess = false;
		break;
	case GPS_LOGIN_NAME_ERR:
		J_OS::LOGINFO("GPS_LOGIN_NAME_ERR");
		m_bLoginSuccess = false;
		break;
	case GPS_LOGIN_PWD_ERR:
		J_OS::LOGINFO("GPS_LOGIN_PWD_ERR");
		m_bLoginSuccess = false;
		break;
	case GPS_LOGIN_FULL_ERR:
		J_OS::LOGINFO("GPS_LOGIN_FULL_ERR");
		m_bLoginSuccess = false;
		break;
	case GPS_LOGIN_VER_ERR:
		J_OS::LOGINFO("GPS_LOGIN_VER_ERR");
		m_bLoginSuccess = false;
		break;
	case GPS_LOGIN_USR_DEL:
		J_OS::LOGINFO("GPS_LOGIN_USR_DEL");
		m_bLoginSuccess = false;
		break;
	case GPS_LOGIN_EXPIRED:
		J_OS::LOGINFO("GPS_LOGIN_EXPIRED");
		m_bLoginSuccess = false;
		break;
	case GPS_LOGIN_SERVER_EMPTY:
		J_OS::LOGINFO("GPS_LOGIN_SERVER_EMPTY");
		m_bLoginSuccess = false;
		break;
	default:
		J_OS::LOGINFO("GPS_LOGIN_DEFAULT_ERR %d", nMsg);
		m_bLoginSuccess = false;
		break;
	}
}

void CTcpServer4Vdms::DoDownloadDev(int nType, void *pData)
{
	switch (nType)
	{
		case GPS_DEV_DOWN_DEV:
		{
			GPSDevInfo_S* pDevInfo = (GPSDevInfo_S*)pData;
			m_deviceMap[pDevInfo->szIDNO] = *pDevInfo;
			if (pDevInfo->nOnline == 1)
			{
				JoDeviceManager->CreateDevObj(2, pDevInfo->szIDNO);
				J_OS::LOGINFO("Enable ID=%s NAME=%s Paren=%d online=%d", pDevInfo->szIDNO, pDevInfo->szName, pDevInfo->nGroup, pDevInfo->nOnline);
			}
			else
			{
				JoDeviceManager->ReleaseDevObj(pDevInfo->szIDNO);
				J_OS::LOGINFO("Disable ID=%s NAME=%s Paren=%d online=%d", pDevInfo->szIDNO, pDevInfo->szName, pDevInfo->nGroup, pDevInfo->nOnline);
			}
			break;
		}
		case GPS_DEV_DOWN_GROUP:
		{
			GPSDEVGroup_S* pDevGroup = (GPSDEVGroup_S*)pData;
			break;
		}
		case GPS_DEV_DOWN_FAILED:
		{
			J_OS::LOGINFO("GPS_DEV_DOWN_FAILED");
			break;
		}
		case GPS_DEV_DOWN_SUC:
		{
			J_OS::LOGINFO("GPS_DEV_DOWN_SUC");
			StartDevAlarm();
			break;
		}
	}
}

void CTcpServer4Vdms::DoAlarmInfo(const char* pAlarm, int nLength)
{
	GPSVehicleAlarm_S *pAlarmInfo = (GPSVehicleAlarm_S *)pAlarm;
	DeviceMap::iterator it = m_deviceMap.find(pAlarmInfo->szDevIDNO);
	if (it != m_deviceMap.end())
	{
		if (pAlarmInfo->Alarm.AlarmType == GPS_ALARM_TYPE_DEV_ONLINE)
		{
			it->second.nOnline = 1;
			JoDeviceManager->CreateDevObj(2, pAlarmInfo->szDevIDNO);
			J_OS::LOGINFO("Vehicle %s  online = 1", pAlarmInfo->szDevIDNO);
		}
		else if (pAlarmInfo->Alarm.AlarmType == GPS_ALARM_TYPE_DEV_DISONLINE)
		{
			it->second.nOnline = 0;
			JoDeviceManager->ReleaseDevObj(pAlarmInfo->szDevIDNO);
			J_OS::LOGINFO("Vehicle %s online = 0", pAlarmInfo->szDevIDNO);
		}
	}
}