///////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2009, xx科技有限公司(版权声明) 
/// All rights reserved. 
/// 
/// @file		  TcpServer4Dahua.h 
/// @brief       客户端TCP服务器模块
///
/// @version   1.0 (版本声明)
/// @author    Jorhy (joorhy@gmail.com) 
/// @date       2015/07/15 15:11 
///
///
/// 修订说明：最初版本
///////////////////////////////////////////////////////////////////////////  
#ifndef __TCP_SERVER4DAHUA_H_
#define __TCP_SERVER4DAHUA_H_
#include "j_includes.h"
#include "x_asio_win.h"
#include "dhnetsdk.h"

/// 本类的功能:  客户端请求
/// 完成客户端相关工作 
class CTcpServer4Dahua : public J_Service
{
	typedef std::map<LONG, j_string_t> DeviceMap;
public:
	CTcpServer4Dahua();
	~CTcpServer4Dahua();

public:
	/// J_Service
	j_result_t StartServer(const j_string_t &strIniPath); 
	j_result_t StopServer();

private:
	static int CALLBACK ffServiceCallBack(LLONG lHandle, char *ip, WORD port, LONG lCommand, void *pParam, DWORD dwParamLen, LDWORD dwUserData)
	{
		CTcpServer4Dahua *pThis = reinterpret_cast<CTcpServer4Dahua *>(dwUserData);
		pThis->OnServiceCallBack(lHandle, ip, port, lCommand, pParam, dwParamLen);
		return 1;
	}
	void OnServiceCallBack(LLONG lHandle, char *ip, WORD port, LONG lCommand, void *pParam, DWORD dwParamLen);
	static void CALLBACK ffDisConnect(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
	{
		CTcpServer4Dahua *pThis = reinterpret_cast<CTcpServer4Dahua *>(dwUser);
		pThis->OnDisConnect(lLoginID, pchDVRIP, nDVRPort);
	}
	void OnDisConnect(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort);

private:
	LLONG m_handle;
	J_OS::CTLock m_locker;
	DeviceMap m_devMap;
};
#endif //~__TCP_SERVER4DAHUA_H_