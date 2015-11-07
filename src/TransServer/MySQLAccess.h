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
#include "XlDataBusDef.h"

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
	j_result_t UpdateDevInfo(const XlHostResponse::HostInfo &hostInfo, bool bOnline);
	/// 获取设备信息
	j_result_t GetDevInfo(XlHostResponse::HostInfo &hostInfo);
	/// 插入车辆状态信息到数据表里面
	j_result_t AddVehicleStatus(const char *pHostId, const XlHostRequest::VehiclleStatus& vehicleStatus);
	/// 插入报警信息到数据表里面
	j_result_t AddAlarmInfo(const char *pHostId, const XlHostRequest::AlarmInfo& alarmInfo);
	/// 获得设备日志的最近时间
	time_t GetDevLogLastTime(const char *pHostId);
	/// 增加日志信息
	j_result_t InsertLogInfo(const char *pHostId, time_t tmStart, time_t tmEnd);
	/// 根据设备ID删除RecordingLog 里面的日志信息
	j_result_t DeleteLogInfo(const char *pHostId);
	/// 同步日志信息
	j_result_t SyncLogInfo(const char *pHostId, time_t tmFirstItem);
	/// 保存文本信息
	j_result_t AddContextInfo(long lUserID, const char *pTitle, const char *pContext, std::vector<j_string_t> &vecHost);
	/// 保存文件
	j_result_t AddFileInfo(long lUserID, const char *pFileName, const char *pFilePath, std::vector<j_string_t> &vecHost);
	/// 更新文本信息
	j_result_t UpdateContextInfo(long lMessageID, int nMessageState, int nDetailState);
	/// 更新文本信息
	j_result_t UpdateContextInfo(long lMessageID, int nDetailState);
	/// 更新文件
	j_result_t UpdateFileInfo(long lFileID, int nFileState, int nDetailState);
	/// 更新文件
	j_result_t UpdateFileInfo(long lFileID, int nDetailState);
	/// 删除文本信息
	j_result_t DelContextInfo(long lMessageID);
	/// 删除文件
	j_result_t DelFileInfo(long lFileID);
	/// 获取文本信息列表
	j_result_t GetContextList(const char *pHostId, HostContextVec &contextVec);
	/// 获取文件列表
	j_result_t GetFileInfoList(const char *pHostId, HostFileInfoVec &fileVec);
	/// 检查某条数据是否存在
	j_boolean_t DataIsExist(const char *pHostId);
	/// 检查符合查询条件的数据条数
	j_int32_t DataCount(const char *query);
	/// 根据指定的查询条件获取用户ID
	j_long_t GetUserID(const char *query);
	/// 根据消息ID获取对应的用户ID
	j_long_t GetUserIDFrmMsg(long lMessageID);
	/// 根据文件ID获取对应的用户ID
	j_long_t GetUserIDFrmFile(long lFileID);
private:
	MYSQL *m_mysql;
	j_boolean_t m_bConnected;
};

JO_DECLARE_SINGLETON(MySQLAccess)

#endif // ~__MYSQLACCESS_H_
