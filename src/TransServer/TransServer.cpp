///////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2009, xx科技有限公司(版权声明) 
/// All rights reserved. 
/// 
/// @file      nvr.cpp 
/// @brief     NVR应用程序模块
///
/// @version   1.0 (版本声明)
/// @author    Jorhy (joorhy@gmail.com) 
/// @date      2013/09/18 17:11 
///
///
/// 修订说明：最初版本
///////////////////////////////////////////////////////////////////////////  
#include "TcpServer4Client.h"
#include "TcpServer4Device.h"
#include "MySQLAccess.h"

#ifdef _DEBUG
#pragma comment (lib, "Debug\\core.lib")
#else
#pragma comment (lib, "Release\\core.lib")
#endif

CTcpServer4Device g_deviceServer;
CTcpServer4Client g_clientServer;

const char *g_ini_file = ".//TranServer.ini";

int main(int argc, char **argv)
{
	///初始化数据库
	if (GetPrivateProfileInt("data_base", "enable", 0, g_ini_file) == 1)
	{
		char strDbAddr[16] = { 0 };
		char strDbUser[32] = { 0 };
		char strDbPassWord[32] = { 0 };
		int nDbPort = GetPrivateProfileInt("data_base", "port", 3306, g_ini_file);
		GetPrivateProfileString("data_base", "addr", "10.0.0.123", strDbAddr, sizeof(strDbAddr), g_ini_file);
		GetPrivateProfileString("data_base", "user", "root", strDbUser, sizeof(strDbUser), g_ini_file);
		GetPrivateProfileString("data_base", "password", "123456", strDbPassWord, sizeof(strDbPassWord), g_ini_file);
		JoDataBaseObj->Init();
		if (JoDataBaseObj->Connect(strDbAddr, nDbPort, strDbUser, strDbPassWord) != J_OK)
		{
			J_OS::LOGINFO("数据库连接失败, IP地址:%s,端口:%d,用户名:%s,密码:%s", strDbAddr, nDbPort, strDbUser, strDbPassWord);
			//return -1;
		}
	}

	///启动设备监听服务
	if (GetPrivateProfileInt("device", "enable", 0, g_ini_file) == 1)
	{
		if (g_deviceServer.StartServer(g_ini_file) != J_OK)
		{
			J_OS::LOGINFO("启动设备监听服务失败");
			//return -1;
		}
	}

	///启动客户监听服务
	if (GetPrivateProfileInt("client", "enable", 0, g_ini_file) == 1)
	{
		if (g_clientServer.StartServer(g_ini_file) != J_OK)
		{
			J_OS::LOGINFO("启动客户监听服务失败");
			//return -1;
		}
	}

	while (true)
	{
		Sleep(100);
	}

	///停止客户监听服务
	g_clientServer.StopServer();
	///停止设备监听服务
	g_deviceServer.StopServer();
	///停止平台服务
	JoDataBaseObj->Release();

	return 0;
}