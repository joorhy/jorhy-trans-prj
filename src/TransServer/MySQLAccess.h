///////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2009, xx科技有限公司(版权声明) 
/// All rights reserved. 
/// 
/// @file      MySQLAccess.h 
/// @brief     MySQL数据库模块
///
/// @version   1.0 (版本声明)
/// @author    Jorhy (joorhy@gmail.com) 
/// @date      2015/04/22 11:08 
///
///
/// 修订说明：最初版本
///////////////////////////////////////////////////////////////////////////  
#ifndef __MYSQLACCESS_H_
#define __MYSQLACCESS_H_
#include "j_includes.h"
#include "mysql.h"
#include "XlHostRespData.h"
#include "XlHostCmdData.h"

/// 本类的功能:  MySQL数据库操作类
/// 数据库的增删改查 
class CMySQLAccess : public J_DbAccess
{
public:
	CMySQLAccess();
	~CMySQLAccess();

public:
	///J_DbAccess
	virtual j_result_t Connect(const j_char_t *pAddr, j_int16_t nPort, const j_char_t *pUa, const j_char_t *pPwd);
	virtual j_result_t Init();
	virtual j_result_t Release();

public:
	/// 用户登录
	j_result_t Login(const char *pUserName, const char *pPasswd, int nForce, int &nRet);
	/// 获取用户名
	j_result_t GetUserNameById(j_int32_t userId, j_string_t &userName);
	/// 用户退出
	j_result_t Logout(const char *pUserName);
	/// 修改文件信息
	j_result_t UpdateFileInfo(j_int32_t nFileId, j_int32_t nState, bool bFlag = true);
	/// 修改设备信息
	j_result_t UpdateDevInfo(const CXlHostRespData::RespHostInfo &hostInfo, bool bOnline);
	/// 获取设备信息
	j_result_t GetDevInfo(CXlHostRespData::RespHostInfo &hostInfo);
	/// 增加报警信息
	j_result_t InsertAlarmInfo(const char *pHostId, const CXlHostRespData::RespAlarmInfo& alarmInfo);
	/// 获得设备日志的最近时间
	time_t GetDevLogLastTime(const char *pHostId);
	/// 增加日志信息
	j_result_t InsertLogInfo(const char *pHostId, time_t tmStart, time_t tmEnd);
	/// 保存文本信息
	j_result_t UpdateContextInfo(long lUserID, const char *pTitle, const char *pContext, std::vector<j_string_t> &vecHost);
	/// 保存文件
	j_result_t UpdateFileInfo(long lUserID, const char *pTitle, const char *pFilePath, std::vector<j_string_t> &vecHost);
	/// 获取文本信息列表
	j_result_t GetContextList(const char *pHostId, HostContextVec &contextVec);
	/// 获取文件列表
	j_result_t GetFileInfoList(const char *pHostId, HostFileInfoVec &contextVec);

private:  
	MYSQL *m_mysql;
	j_boolean_t m_bConnected;
};

JO_DECLARE_SINGLETON(MySQLAccess)

#endif // ~__MYSQLACCESS_H_
