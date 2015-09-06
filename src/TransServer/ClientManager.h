///////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2009, xx科技有限公司(版权声明) 
/// All rights reserved. 
/// 
/// @file         ClientManager.h 
/// @brief       客户端管理模块
///
/// @version   1.0 (版本声明)
/// @author    Jorhy (joorhy@gmail.com) 
/// @date       2013/09/18 16:59 
///
///
/// 修订说明：最初版本
///////////////////////////////////////////////////////////////////////////  
#ifndef __CLIENT_MANAGER_H_
#define __CLIENT_MANAGER_H_
#include "j_includes.h"
#include "x_lock.h"
#include "x_timer.h"

/// 本类的功能:  管理客户端信息
/// 本类是个管理模块
/// 管理客户端的登录、退出、请求数据等操作
enum ClientType
{
	RTSP_CLIENT = 1,
	XL_CLIENT = 2,
};

class CClientManager
{
	typedef std::map<j_socket_t, J_Client *> ClientMap;
	typedef std::map<j_string_t, J_Client *> UserMap;
public:
	CClientManager();
	~CClientManager();

public:
	/// 用户登录
	/// @param[in]		pUserName 用户名 
	/// @param[in]		pPasswd 密码
	/// @param[in]		nForce 强制登录标志
	/// @param[in]		nRett 返回值
	/// @param[in]		pClient 客户端对象
	/// @return			参见j_errtype.h 
	j_result_t Login(const char *pUserName, const char *pPasswd, int nForce, int &nRet, J_Client *pClient);
	/// 用户注销
	/// @param[in]		pUserName 用户名 
	/// @param[in]		pClient 客户端对象
	/// @return			参见j_errtype.h 
	j_result_t Logout(const char *pUserName, J_Client *pClient);
	/// 创建客户端对象
	/// @param[in]		nSock 设备连接 
	/// @param[out]   NULL-失败,否则为Client对象
	/// @return			参见j_errtype.h 
	J_Client *CreateClientObj(j_socket_t nSock, j_int32_t clientType = XL_CLIENT);
	/// 根据连接获取客户端对象
	/// @param[in]		nSock 设备连接 
	/// @return			Client对象,NULL-未找到   
	J_Client *GetClientObj(j_socket_t nSock);
	/// 根据用户名获取客户端对象 
	/// @param[in]		userName 用户名 
	/// @return			Client对象,NULL-未找到   
	J_Client *GetClientObj(j_string_t userName);
	///删除客户端对象
	/// @param[in]		nSock 设备连接 
	/// @return			
	void ReleaseClientObj(j_socket_t nSock);

private:
	static void OnTimer(void *pUser)
	{
		(static_cast<CClientManager *>(pUser))->CheckClient();
	}
	void CheckClient();

private:
	ClientMap m_clientMap;
	J_OS::CTLock m_lockerUser;
	UserMap m_userMap;
	J_OS::CTimer m_timer;
};

JO_DECLARE_SINGLETON(ClientManager)

#endif //~__CLIENT_MANAGER_H_