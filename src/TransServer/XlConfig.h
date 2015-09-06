///////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2009, xx科技有限公司(版权声明) 
/// All rights reserved. 
/// 
/// @file      XlConfig.h 
/// @brief     数据库相关中间接口
///
/// @version   1.0 (版本声明)
/// @author    Jorhy (joorhy@gmail.com) 
/// @date      2013/11/01 09:02 
/// 修订说明：最初版本
///////////////////////////////////////////////////////////////////////////  
#ifndef __XLCONFIG_H_
#define __XLCONFIG_H_
#include "j_includes.h"
class CXlConfig
{
public:
	CXlConfig();
	~CXlConfig();

public:
	void Init(j_char_t *pReadBuff, j_char_t * pWriteBuff)
	{
		m_readBuff = pReadBuff;
		m_writeBuff = pWriteBuff;
	}
	j_result_t ProcessConfigRequest(j_int32_t nType, J_AsioDataBase *pAsioData);
	j_result_t ProcessConfigData(j_int32_t nType, J_AsioDataBase *pAsioData);

private:
	/// 分析数据
	j_result_t ProcessDvrList(J_AsioDataBase *pAsioData);
	j_result_t ProcessUserList(J_AsioDataBase *pAsioData);
	j_result_t ProcessDepartmentList(J_AsioDataBase *pAsioData);
	j_result_t ProcessLog(J_AsioDataBase *pAsioData);
	j_result_t ProcessAlarm(J_AsioDataBase *pAsioData);
	///生成数据接口
	j_result_t MakeLogData(J_AsioDataBase *pAsioData);
	j_result_t MakeAlarmData(J_AsioDataBase *pAsioData);
	j_result_t MakeDvrListData(J_AsioDataBase *pAsioData);
	j_result_t MakeUserListData(J_AsioDataBase *pAsioData);
	j_result_t MakeDepartmentListData(J_AsioDataBase *pAsioData);
	///管理数据接口
	j_result_t OnGetAlarmInfo(J_AsioDataBase *pAsioData);
	j_result_t OnGetTotleDvrInfo(J_AsioDataBase *pAsioData);
	j_result_t OnGetTotleUserInfo(J_AsioDataBase *pAsioData);
	j_result_t OnGetDvrList(J_AsioDataBase *pAsioData);
	j_result_t OnGetUserList(J_AsioDataBase *pAsioData);
	j_result_t OnGetDepartmentList(J_AsioDataBase *pAsioData);
	j_result_t OnGetDvrInfo(J_AsioDataBase *pAsioData);
	j_result_t OnGetUserInfo(J_AsioDataBase *pAsioData);
	j_result_t OnGetDepartmentInfo(J_AsioDataBase *pAsioData);
	j_result_t OnAddDvrInfo(J_AsioDataBase *pAsioData);
	j_result_t OnAddUserInfo(J_AsioDataBase *pAsioData);
	j_result_t OnAddDepartmentInfo(J_AsioDataBase *pAsioData);
	j_result_t OnModDvrInfo(J_AsioDataBase *pAsioData);
	j_result_t OnModUserInfo(J_AsioDataBase *pAsioData);
	j_result_t OnModDepartmentInfo(J_AsioDataBase *pAsioData);
	j_result_t OnDelDvrInfo(J_AsioDataBase *pAsioData);
	j_result_t OnDelUserInfo(J_AsioDataBase *pAsioData);
	j_result_t OnDelDepartmentInfo(J_AsioDataBase *pAsioData);
	///设备信息
	j_result_t OnGetDevInfo(J_AsioDataBase *pAsioData);
	j_result_t OnGetLogInfo(J_AsioDataBase *pAsioData);
private:
	j_int32_t m_ioDvrListState;						//设备信息发送状态
	j_int32_t m_ioUserListState;						//用户信息发送状态
	j_int32_t m_ioDepartmentListState;			//单位信息发送状态
	j_int32_t m_ioAlarmState;							//报警发送状态
	j_int32_t m_ioLogState;								//日志发送状态

	j_char_t *m_readBuff;								//命令请求缓存区
	j_char_t *m_writeBuff;								//命令发送缓存区
	j_char_t *m_dvrBuff;									//设备信息发送缓存区
	j_char_t *m_userBuff;								//用户信息发送缓存区
	j_char_t *m_departmentBuff;						//单位信息发送缓存区
	j_char_t *m_alarmBuff;								//报警发送缓存区
	j_char_t *m_logBuff;									//日志发送缓存区 

	CliHostInfoQueue m_dvrInfoQueue;							//设备信息缓存队列
	CliUserInfoQueue m_userInfoQueue;						//用户信息缓存队列
	CliDepartmentInfoQueue m_departmentInfoQueue;	//单位信息缓存队列
	DevLogInfoQueue m_logInfoQueue;								//历史日志缓存队列
	CliAlarmInfoQueue m_alarmInfoQueue;						//历史报警缓存队列
};
#endif //~__XLCONFIG_H_