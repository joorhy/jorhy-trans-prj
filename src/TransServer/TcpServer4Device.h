///////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2009, xx科技有限公司(版权声明) 
/// All rights reserved. 
/// 
/// @file			TcpServer4Dev.h 
/// @brief			设备端TCP服务模块
///
/// @version    1.0 (版本声明)
/// @author     Jorhy (joorhy@gmail.com) 
/// @date			2013/09/18 17:12 
///
///
/// 修订说明：最初版本
///////////////////////////////////////////////////////////////////////////  

#ifndef __TCP_SERVER4DEV_H_
#define __TCP_SERVER4DEV_H_
#include "j_includes.h"
#include "x_asio_win.h"
/// 本类的功能:  前端设备TCP服务器
/// 本类接受前端设备的请求，\n
/// 完成DVR注册、校时操作、快关机日志上传、心跳维持、实时报警、获取设备信息等功能
class CTcpServer4Device : public J_AsioUser
						, public J_Service
{
	struct DeviceInfo
	{
		j_string_t strHostId;
		AsioDataInfo asioDataInfo;
	};
	typedef std::map<j_socket_t, DeviceInfo> DeviceMap;
public:
	CTcpServer4Device();
	~CTcpServer4Device();

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
	DeviceMap m_devMap;
};

#endif //~__TCP_SERVER4DEV_H_