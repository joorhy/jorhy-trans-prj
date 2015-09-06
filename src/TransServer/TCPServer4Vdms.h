///////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2009, xx科技有限公司(版权声明) 
/// All rights reserved. 
/// 
/// @file		  TcpServer4Vdms.h 
/// @brief       客户端TCP服务器模块
///
/// @version   1.0 (版本声明)
/// @author    Jorhy (joorhy@gmail.com) 
/// @date       2015/07/15 15:11 
///
///
/// 修订说明：最初版本
///////////////////////////////////////////////////////////////////////////  
#ifndef __TCP_SERVER4VDMS_H_
#define __TCP_SERVER4VDMS_H_
#include "j_includes.h"
#include "hi_voice_api.h"
#include "netclientapi.h"

/// 本类的功能:  客户端请求
/// 完成客户端相关工作 
class CTcpServer4Vdms : public J_Service
{
	typedef std::map<j_string_t, GPSDevInfo_S> DeviceMap;
public:
	CTcpServer4Vdms();
	~CTcpServer4Vdms();

public:
	/// J_Service
	j_result_t StartServer(const j_string_t &strIniPath);
	j_result_t StopServer();

private:
	void StartDeviceLoad(j_boolean_t bReload);
	void StopDeviceLoad();

	void StartDevAlarm();
	void StopDevAlarm();

private:
	static void WINAPI FuncLoginMsg(int nMsg, void *pUser)
	{
		(static_cast<CTcpServer4Vdms *>(pUser))->DoLoginMsg(nMsg);
	}
	void DoLoginMsg(int nMsg);

	//车辆列表信息
	static void WINAPI FuncDownloadDev(int nType, void *pData, void *pUser)
	{
		(static_cast<CTcpServer4Vdms *>(pUser))->DoDownloadDev(nType, pData);
	}
	void DoDownloadDev(int nType, void *pData);

	static void WINAPI FuncAlarmInfo(const char* pAlarm, int nLength, void* pUser)
	{
		(static_cast<CTcpServer4Vdms *>(pUser))->DoAlarmInfo(pAlarm, nLength);
	}
	void DoAlarmInfo(const char* pAlarm, int nLength);

private:
	j_boolean_t m_bLogining;
	j_boolean_t m_bLoginSuccess;

	//车辆列表句柄
	long m_lLoadHandle;
	j_boolean_t m_bReloadDev;
	long m_lAlarmHaneld;
	DeviceMap m_deviceMap;
};
#endif //~__TCP_SERVER4VDMS_H_