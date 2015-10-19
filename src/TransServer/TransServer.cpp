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
#include "TcpServer4RTSP.h"
#include "TCPServer4Vdms.h"
#include "TCPServer4Dahua.h"

#include "MsSqlServer.h"
#include "MySQLAccess.h"

#ifdef _DEBUG
#pragma comment (lib, "Debug\\core.lib")
#else
#pragma comment (lib, "Release\\core.lib")
#endif

CTcpServer4Device g_deviceServer;
CTcpServer4Client g_clientServer;
CTcpServer4Vdms g_vdmsServer;
CTcpServer4RTSP g_rtspServer;
CTcpServer4Dahua g_dahuaServer;

const char *g_ini_file = ".//TranServer.ini";

int main(int argc, char **argv)
{
	///初始化平台
	UINT loc_Result = GetPrivateProfileInt("platform", "enable", 0, g_ini_file);

	if (GetPrivateProfileInt("platform", "enable", 0, g_ini_file) == 1)
	{
		if (g_vdmsServer.StartServer(g_ini_file) != J_OK)
		{
			J_OS::LOGINFO("初始化平台失败");
			//return -1;
		}
	}

	///启动RTSP监听服务
	if (GetPrivateProfileInt("rtsp", "enable", 0, g_ini_file) == 1)
	{
		if (g_rtspServer.StartServer(g_ini_file) != J_OK)
		{
			J_OS::LOGINFO("启动RTSP听服务失败");
			//return -1;
		}
	}

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

	///启动大华监听服务
	if (GetPrivateProfileInt("dahua", "enable", 0, g_ini_file) == 1)
	{
		if (g_dahuaServer.StartServer(g_ini_file) != J_OK)
		{
			J_OS::LOGINFO("启动大华监听服务失败");
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
	g_vdmsServer.StopServer();
	///停止RTSP服务
	g_rtspServer.StopServer();
	///停止大华监听服务
	g_dahuaServer.StopServer();
	///注销数据库连接
	JoDataBaseObj->Release();

	return 0;
}