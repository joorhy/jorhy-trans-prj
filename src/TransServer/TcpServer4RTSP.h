///////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2009, xx科技有限公司(版权声明) 
/// All rights reserved. 
/// 
/// @file		  TcpServer4RTSP.h 
/// @brief       客户端TCP服务器模块
///
/// @version   1.0 (版本声明)
/// @author    Jorhy (joorhy@gmail.com) 
/// @date       2015/04/22 09:43 
///
///
/// 修订说明：最初版本
///////////////////////////////////////////////////////////////////////////  
#ifndef __TCP_SERVER4MOBILE_H_
#define __TCP_SERVER4MOBILE_H_
#include "j_includes.h"
#include "x_asio_win.h"

/// 本类的功能:  客户端请求
/// 完成客户端相关工作 
class CTcpServer4RTSP : public J_AsioUser
					  , public J_Service
{
public:
	CTcpServer4RTSP();
	~CTcpServer4RTSP();

public:
	///AsioUser
	virtual j_result_t OnAccept(J_AsioDataBase *pAsioData, int nRet);
	virtual j_result_t OnRead(J_AsioDataBase *pAsioData, int nRet);
	virtual j_result_t OnWrite(J_AsioDataBase *pAsioData, int nRet);
	virtual j_result_t OnBroken(J_AsioDataBase *pAsioData, int nRet);

	///J_Service   
	j_result_t StartServer(const j_string_t &strIniPath);
	j_result_t StopServer();

private:
	J_AsioDataBase m_asioData;
	CXAsio m_asio;

	J_OS::CTLock m_locker;
	J_AsioDataMap m_asioDataMap;
};
#endif //~__TCP_SERVER4MOBILE_H_