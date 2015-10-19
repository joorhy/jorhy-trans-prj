///////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2009, xx科技有限公司(版权声明) 
/// All rights reserved. 
/// 
/// @file      MySQLAccess.cpp 
/// @brief     MySQL数据库模块实现
///
/// @version   1.0 (版本声明)
/// @author    Jorhy (joorhy@gmail.com) 
/// @date      2015/04/22 11:11 
///
///
/// 修订说明：最初版本
///////////////////////////////////////////////////////////////////////////  
#include "MySQLAccess.h"
#include "XlHelper.h"
#include <time.h>
#include "XlHelper.h"


#include <iostream>

JO_IMPLEMENT_SINGLETON(MySQLAccess)

CMySQLAccess::CMySQLAccess()
{
	m_mysql = new MYSQL();
	m_bConnected = false;
}

CMySQLAccess::~CMySQLAccess()
{
	if (m_mysql != NULL)
	{
		delete m_mysql;
	}
}

j_result_t CMySQLAccess::Connect(const j_char_t *pAddr, j_int16_t nPort, const j_char_t *pUa, const j_char_t *pPwd)
{
	//连接数据库  
	if (!mysql_real_connect(m_mysql, pAddr, pUa, pPwd, "veh_data", 0, NULL, CLIENT_LOCAL_FILES))
	{
		J_OS::LOGINFO("Failed to connect to database: Error: %s", mysql_error(m_mysql));
		return J_DB_ERROR;
	}
	mysql_set_character_set(m_mysql, "gbk");
	//mysql_real_query(m_mysql,"SET NAMES 'UTF8'", strlen("SET NAMES 'UTF8'"));
	//mysql_query(m_mysql,"SET NAMES UTF8");
	J_OS::LOGINFO("character name: %s", mysql_character_set_name(m_mysql));

	m_bConnected = true;

	return J_OK;
}

j_result_t CMySQLAccess::Init()
{
	// 初始化mysql  
	if (!mysql_init(m_mysql))
	{
		J_OS::LOGINFO("Failed to connect to database: Error: %s", mysql_error(m_mysql));
		return J_DB_ERROR;
	}

	return J_OK;
}

j_result_t CMySQLAccess::Release()
{
	return J_OK;
}

j_result_t CMySQLAccess::Login(const char *pUserName, const char *pPasswd, int nForce, int &nRet)
{
	if (!m_bConnected)
	{
		return J_DB_ERROR;
	}

	j_result_t nResult = J_OK;
	try
	{
		int ret;
		char strCmd[128] = { 0 };
		sprintf(strCmd, "SELECT * FROM UserInfo WHERE AccountName='%s';", pUserName);
		ret = mysql_real_query(m_mysql, strCmd, strlen(strCmd));//查询
		if (ret == 0)
		{
			MYSQL_FIELD *fd;
			MYSQL_ROW sql_row;
			MYSQL_RES *result = mysql_store_result(m_mysql);//保存查询到的数据到result
			if (result->row_count != 0)
			{
				sql_row = mysql_fetch_row(result);

				int row = mysql_num_rows(result);
				int column = mysql_num_fields(result);
				for (int i = 0; fd = mysql_fetch_field(result); i++)//获取列名
				{
					if (strcmp("Password", fd->name) == 0)
					{
						if (strcmp(pPasswd, sql_row[i]) != 0)
						{
							nRet = 2;
							nResult = J_DB_ERROR;
							break;
						}
						else if (nForce)
						{
							sprintf(strCmd, "UPDATE UserInfo SET Online=1 WHERE AccountName='%s';", pUserName);
							ret = mysql_real_query(m_mysql, strCmd, strlen(strCmd));
							nRet = 0;
							break;
						}
					}

					if (!nForce && strcmp("Online", fd->name) == 0)
					{
						if (sql_row[i] != NULL && atoi(sql_row[i]) == 1)
						{
							nRet = 6;
							nResult = J_DB_ERROR;
							break;
						}
					}
				}

				memset(strCmd, 0, sizeof(strCmd));
				sprintf(strCmd, "UPDATE UserInfo SET Online=1 WHERE AccountName='%s';", pUserName);
				ret = mysql_real_query(m_mysql, strCmd, strlen(strCmd));
			}
			else
			{
				nRet = 1;
				return J_DB_ERROR;
			}

			if (result != NULL)
			{
				mysql_free_result(result);
			}
		}
	}
	catch (...)
	{
		J_OS::LOGDEBUG("CMySQLAccess::Login Exception");
		return J_DB_ERROR;
	}

	return nResult;
}

j_result_t CMySQLAccess::GetUserNameById(j_int32_t userId, j_string_t &userName)
{
	if (!m_bConnected)
	{
		return J_DB_ERROR;
	}

	try
	{
		char strCmd[128] = { 0 };
		sprintf(strCmd, "SELECT AccountName FROM UserInfo WHERE UserID=%d;", userId);
		int ret = mysql_real_query(m_mysql, strCmd, strlen(strCmd));
		if (ret == 0)
		{
			MYSQL_FIELD *fd;
			MYSQL_ROW sql_row;
			MYSQL_RES *result = mysql_store_result(m_mysql);//保存查询到的数据到result
			if (result)
			{
				for (int i = 0; fd = mysql_fetch_field(result); i++)//获取列名
				{
					if (strcmp("AccountName", fd->name) == 0)
					{
						sql_row = mysql_fetch_row(result);
						userName = sql_row[i];
						break;
					}
				}
			}
		}
		else
		{
			return J_DB_ERROR;
		}
	}
	catch (...)
	{
		J_OS::LOGDEBUG("CMySQLAccess::GetUserNameById Exception");
		return J_DB_ERROR;
	}

	return J_OK;
}

j_result_t CMySQLAccess::Logout(const char *pUserName)
{
	if (!m_bConnected)
	{
		return J_DB_ERROR;
	}

	try
	{
		char strCmd[128] = { 0 };
		sprintf(strCmd, "UPDATE UserInfo SET Online=0 WHERE AccountName='%s';", pUserName);
		int ret = mysql_real_query(m_mysql, strCmd, strlen(strCmd));
	}
	catch (...)
	{
		J_OS::LOGDEBUG("CMySQLAccess::Logout Exception");
		return J_DB_ERROR;
	}

	return J_OK;
}

j_result_t  CMySQLAccess::UpdateFileInfo(j_int32_t nFileId, j_int32_t nState, bool bFlag)
{
	if (!m_bConnected)
	{
		return J_DB_ERROR;
	}

	try
	{
		time_t loacl_tm = time(0);
		tm *today = localtime(&loacl_tm);
		char strCmd[128] = { 0 };
		if (bFlag)
		{
			sprintf(strCmd, "UPDATE FileInfo SET State=%d, CopyTime='%04d-%02d-%02d' WHERE FileID=%d;"
				, nState, today->tm_year + 1900, today->tm_mon, today->tm_mday, nFileId);
		}
		else
		{
			sprintf(strCmd, "UPDATE FileInfo SET State=%d WHERE FileID=%d;"
				, nState, nFileId);
		}
		int ret = mysql_real_query(m_mysql, strCmd, strlen(strCmd));
	}
	catch (...)
	{
		J_OS::LOGDEBUG("CMySQLAccess::UpdateFileInfo Exception");
		return J_DB_ERROR;
	}

	return J_OK;
}

j_result_t CMySQLAccess::UpdateDevInfo(const CXlDataBusInfo::XldRespHostInfo &hostInfo, bool bOnline)
{
	if (!m_bConnected)
	{
		return J_DB_ERROR;
	}

	try
	{
		char strCmd[256] = { 0 };
		sprintf(strCmd, "SELECT * FROM Equipment WHERE EquipmentID='%s';", hostInfo.hostId);
		int ret = mysql_real_query(m_mysql, strCmd, strlen(strCmd));

		MYSQL_RES *results = mysql_store_result(m_mysql);
		if (results->row_count == 0)
		{
			memset(strCmd, 0, sizeof(strCmd));
			sprintf(strCmd, "INSERT INTO Equipment (EquipmentID,VehicleNO,PhoneNum,TotalChannels,Online,State) VALUES ('%s','%s','%s',%d,%d,%d);",
				hostInfo.hostId, hostInfo.vehicleNum, hostInfo.phoneNum, hostInfo.totalChannels, bOnline, 1);
			ret = mysql_real_query(m_mysql, strCmd, strlen(strCmd));
		}

		memset(strCmd, 0, sizeof(strCmd));
		if (bOnline == true)
		{
			sprintf(strCmd, "UPDATE Equipment SET VehicleNO='%s',PhoneNum='%s',TotalChannels=%d,Online=%d WHERE EquipmentID='%s';",
				hostInfo.vehicleNum, hostInfo.phoneNum, hostInfo.totalChannels & 0xFF, bOnline, hostInfo.hostId);
		}
		else
		{
			sprintf(strCmd, "UPDATE Equipment SET Online=%d WHERE EquipmentID='%s';",
				bOnline, hostInfo.hostId);
		}
		ret = mysql_real_query(m_mysql, strCmd, strlen(strCmd));

		char *mediaType = (char *)hostInfo.data;

		char *pChannelData = mediaType + hostInfo.mediaTypeNum;
		char *pChannelEnd = NULL;
		for (int i = 0; i < hostInfo.totalChannels; i++)
		{
			memset(strCmd, 0, sizeof(strCmd));
			sprintf(strCmd, "SELECT * FROM EquipmentDetail WHERE EquipmentID='%s' AND channel=%d;", hostInfo.hostId, i + 1);
			ret = mysql_real_query(m_mysql, strCmd, strlen(strCmd));

			char channelName[64] = { 0 };
			j_string_t strChannelName;
			int nameLen = 0;
			pChannelEnd = strstr(pChannelData, "#");
			if (pChannelEnd != NULL)
			{
				nameLen = pChannelEnd - pChannelData;
				memcpy(channelName, pChannelData, nameLen);
			}
			else
			{
				strcpy(channelName, pChannelData);
			}
			pChannelData += (nameLen + 1);

			memset(strCmd, 0, sizeof(strCmd));

			results = mysql_store_result(m_mysql);
			if (results->row_count == 0)
			{
				sprintf(strCmd, "INSERT INTO EquipmentDetail (EquipmentID,channel,channelType,channelName) VALUES ('%s',%d,%d,'%s');",
					hostInfo.hostId, i + 1, mediaType[i] & 0xFF, channelName);
			}
			else
			{
				sprintf(strCmd, "UPDATE EquipmentDetail SET channelType=%d,channelName='%s' WHERE EquipmentID='%s' AND channel = %d;",
					mediaType[i] & 0xFF, channelName, hostInfo.hostId, i + 1);
			}
			ret = mysql_real_query(m_mysql, strCmd, strlen(strCmd));
		}
	}
	catch (...)
	{
		J_OS::LOGDEBUG("CSqlServerAccess::UpdateDevInfo Exception");
		return J_DB_ERROR;
	}

	return J_OK;
}

j_result_t CMySQLAccess::GetDevInfo(CXlDataBusInfo::XldRespHostInfo &hostInfo)
{
	if (!m_bConnected)
	{
		return J_DB_ERROR;
	}

	try
	{
		char strCmd[128] = { 0 };
		sprintf(strCmd, "SELECT * FROM Equipment WHERE EquipmentID='%s';", hostInfo.hostId);
		int ret = mysql_real_query(m_mysql, strCmd, strlen(strCmd));
		if (ret != 0)
		{
			return J_UNKNOW;
		}

		MYSQL_FIELD *fd;
		MYSQL_ROW sql_row;
		MYSQL_RES *result = mysql_store_result(m_mysql);//保存查询到的数据到result
		if (result)
		{
			for (int i = 0; fd = mysql_fetch_field(result); i++)//获取列名
			{
				if (strcmp("VehicleNO", fd->name) == 0)
				{
					sql_row = mysql_fetch_row(result);
					strcpy(hostInfo.vehicleNum, sql_row[i]);
				}
				if (strcmp("PhoneNum", fd->name) == 0)
				{
					sql_row = mysql_fetch_row(result);
					strcpy(hostInfo.phoneNum, sql_row[i]);
				}
				if (strcmp("TotalChannels", fd->name) == 0)
				{
					sql_row = mysql_fetch_row(result);
					hostInfo.totalChannels = atoi(sql_row[i]);
				}
			}
		}
	}
	catch (...)
	{
		J_OS::LOGDEBUG("CMySQLAccess::GetDevInfo Exception");
		return J_DB_ERROR;
	}

	return J_OK;
}

j_result_t CMySQLAccess::InsertAlarmInfo(const char *pHostId, const CXlDataBusInfo::XldRespAlarmInfo& alarmInfo)
{
	if (!m_bConnected)
	{
		return J_DB_ERROR;
	}

	try
	{
		char strCmd[512] = { 0 };
		//sprintf(strCmd, "INSERT INTO Alarm(EquipmentID,Alarm,GPS_Latitude,GPS_Longitude,GPS_Speed,Speed,TimeStamp)VALUES('%s',%I64d,%f,%f,%f,%f,%d);",
		//	pHostId, alarmInfo.bAlarm & 0xFF, alarmInfo.gps.dLatitude, alarmInfo.gps.dLongitude, alarmInfo.gps.dGPSSpeed, alarmInfo.speed, alarmInfo.tmTimeStamp);
		//m_pConn->Execute((_bstr_t)strCmd, NULL, adCmdText);
	}
	catch (...)
	{
		return J_DB_ERROR;
	}

	return J_OK;
}

time_t CMySQLAccess::GetDevLogLastTime(const char *pHostId)
{
	if (!m_bConnected)
	{
		return time(0) * 1000;
	}

	time_t tmReturn = 0;
	try
	{
		char strCmd[128] = { 0 };
		sprintf(strCmd, "SELECT MAX(TimeStamp) FROM EquipmentLog WHERE EquipmentID='%s';", pHostId);
		int ret = mysql_real_query(m_mysql, strCmd, strlen(strCmd));
		if (ret == 0)
		{
			MYSQL_FIELD *fd;
			MYSQL_ROW sql_row;
			MYSQL_RES *result = mysql_store_result(m_mysql);//保存查询到的数据到
			if (result->row_count != 0)
			{
				sql_row = mysql_fetch_row(result);
				if (sql_row[0] != NULL)
				{
					tmReturn = atol(sql_row[0]) * 1000;
				}
			}
		}
	}
	catch (...)
	{

	}

	return tmReturn;
}


/***********************************************************************************************************
 * 程序创建：赵进军                     程序修改:赵进军
 * 函数功能：根据设备ID删除RecordingLog 里面的日志信息
 * 参数说明：
 *  pHostId：设备ID
 * 注意事项：使用之前需要确所有接入的设备ID具有唯一性,慎用
 * 修改日期：2015/10/09 09:36:00
 ***********************************************************************************************************/
j_result_t CMySQLAccess::DeleteLogInfo(const char *pHostId)
{
	if (NULL == pHostId)
		return J_DB_ERROR;

	if (!m_bConnected)
		return J_DB_ERROR;
	try
	{
		char strCmd[512] = { 0 };
		MYSQL_RES *result;
		int resCount;
		sprintf(strCmd, "DELETE * FROM recordinglog WHERE EquipmentID='%s'",
			pHostId);
		int ret = mysql_real_query(m_mysql, strCmd, strlen(strCmd));
		if (ret == 0)
			return J_OK;
		else
			return J_DB_ERROR;
	}
	catch (...)
	{
		return J_DB_ERROR;
	}
	return J_OK;
}


/***********************************************************************************************************
 * 程序创建：刘劲朝           程序修改：赵进军
 * 函数功能：往数据库recordinglog表里面插入日志信息,不能重复插入数据。
 * 参数说明：
 *  pHostId：设备ID
 *  tmStart：日志开始时间
 *    tmEnd：日志结束时间
 * 注意事项：未对输入的时间进行检测，使用之前请不要输入非法数据。
 * 修改日期：2015/09/16/ 16:02:00
 ***********************************************************************************************************/
j_result_t CMySQLAccess::InsertLogInfo(const char *pHostId, time_t tmStart, time_t tmEnd)
{	
	if (!CXlHelper::ActionTimestampCheck(tmStart, 8) || !CXlHelper::ActionTimestampCheck(tmEnd, 8))						//确保插入到指定数据库的时间戳格式是正确的
		return J_DB_ERROR;

	if (!m_bConnected)
		return J_DB_ERROR;
	try
	{
		char strCmd[512] = { 0 };
		MYSQL_RES *result;
		int resCount;
		sprintf(strCmd, "SELECT LogID FROM recordinglog WHERE StartTime = '%I64u' AND EquipmentID='%s'",
			tmStart, pHostId);
		int ret = mysql_real_query(m_mysql, strCmd, strlen(strCmd));

		if (ret == 0)                                                                                               //标志当前数据查询成功
		{
			result = mysql_store_result(m_mysql);
			resCount = mysql_num_rows(result);
			mysql_free_result(result);
			memset(strCmd, 0, sizeof(strCmd));
			if (resCount > 0)																						//标志当前设备的起始时间在数据库中存在
			{
				sprintf(strCmd, "SELECT MAX(LogID) FROM recordinglog WHERE StartTime = '%I64u' AND EndTime < '%I64u' AND EquipmentID='%s'",
					tmStart, tmEnd, pHostId);
				ret = mysql_real_query(m_mysql, strCmd, strlen(strCmd));
				if (ret == 0)																					    //以结束时间为标志的数据查询成功
				{
					long logId = -1;
					MYSQL_ROW sql_row;
					result = mysql_store_result(m_mysql);
					resCount = mysql_num_rows(result);
					if (resCount == 1)																				//标志当前查询有返回结果
					{
						sql_row = mysql_fetch_row(result);
						if (sql_row[0] != NULL)
						{
							logId = atol(sql_row[0]);
							mysql_free_result(result);
							memset(strCmd, 0, sizeof(strCmd));
							sprintf(strCmd, "UPDATE recordinglog SET EndTime = '%I64u' WHERE LogID = '%d'", tmEnd, logId);
							ret = mysql_real_query(m_mysql, strCmd, strlen(strCmd));
							if (ret == 0)
								return J_OK;																		//数据更新成功
							else
								return J_DB_ERROR;																	//数据更新失败
						}
						else
							return J_DB_ERROR;
					}
					else
					{
						return J_DB_ERROR;																			//无法更新结束时间
					}
				}
				else
				{
					return J_DB_ERROR;																			    //以结束时间和车号为条件的数据查询失败
				}
			}
			else																									//当前数据在数据库中不存在,只需直接插入数据
			{
				sprintf(strCmd, "INSERT INTO recordinglog(EquipmentID,StartTime,EndTime) VALUES('%s','%I64u','%I64u')",
					pHostId, tmStart, tmEnd);
				ret = mysql_real_query(m_mysql, strCmd, strlen(strCmd));
				if (ret == 0)																					    //数据插入成功
					return J_OK;
				else
					return J_DB_ERROR;																				//插入数据失败
			}
		}
		else
		{
			return J_DB_ERROR;																						//以开始时间和车号为条件的数据查询失败
		}
	}
	catch (...)
	{
		return J_DB_ERROR;
	}
	return J_OK;
}


/***********************************************************************************************************
 * 程序创建：刘进朝                     程序修改:赵进军
 * 函数功能：新增or删除DVR 更新recordingfirsttime里面的数据
 * 参数说明：
 *  pHostId：车辆ID
 *  tmFirstItem：时间
 * 注意事项：null
 * 修改日期：2015/10/12 14:38:00
 ***********************************************************************************************************/
j_result_t CMySQLAccess::SyncLogInfo(const char *pHostId, time_t tmFirstItem)
{
	if (NULL == pHostId)																							// 确保设备ID 不为空
		return J_DB_ERROR;

	if (!CXlHelper::ActionTimestampCheck(tmFirstItem, 8))														    // 确保当前的时间戳格式是正确的
		return J_DB_ERROR;

	if (!m_bConnected)
		return J_DB_ERROR;

	try
	{
		return J_OK;
		char strCmd[128] = { 0 };
		sprintf(strCmd, "UPDATE recordingfirsttime set FristTime= '%I64u' WHERE EquipmentID = '%s'", tmFirstItem, pHostId);
		int ret = mysql_real_query(m_mysql, strCmd, strlen(strCmd));
		if (ret == 0)
			return J_OK;
		else
			return J_DB_ERROR;
	}
	catch (...)
	{
		//J_OS::LOGDEBUG("CSqlServerAccess::UpdateDevInfo Exception");
		return J_DB_ERROR;
	}
	return J_OK;
}


/***********************************************************************************************************
 * 程序创建：刘进朝                     程序修改:赵进军
 * 函数功能：往TransMessage数据表里面新增一条数据    还差一个设备是否在线？
 * 参数说明：
 *  lUserID：用户ID
 *   pTitle：消息标题
 * pContext：消息内容
 *  vecHost：设备ID
 * 注意事项：null
 * 修改日期：2015/10/12 14:37:00
 ***********************************************************************************************************/
j_result_t CMySQLAccess::AddContextInfo(long lUserID, const char *pTitle, const char *pContext, std::vector<j_string_t> &vecHost)
{
	if (!m_bConnected)
		return J_DB_ERROR;

	try
	{
		char strCmd[256] = { 0 };
		char timeInterval[arr_size] = { 0 };
		CXlHelper::GetTimestamp(true, timeInterval);																// 获取系统时间戳

		/*int locSize = sizeof(pTitle);
		j_string_t klkl = mysql_character_set_name(m_mysql);

		j_string_t s(pTitle);

		j_string_t loc_strs;
		loc_strs = CXlHelper::ASCII2UTF_8(s);*/
		//CXlHelper::Ansi2UTF8("看", loc_strs);


		sprintf(strCmd, "INSERT INTO TransMessage(UserID,Title,Content,SendTime)VALUES(%d,'%s','%s','%s');",
			lUserID, pTitle, pContext, timeInterval);
		int ret = mysql_real_query(m_mysql, strCmd, strlen(strCmd));
		if (ret == 0)
		{
			int nMessageID = mysql_insert_id(m_mysql);																//获取当前插入数据后的MessageID值
			if (nMessageID >= 0)
			{
				std::vector<j_string_t>::iterator it = vecHost.begin();
				for (; it != vecHost.end(); it++)
				{
					memset(strCmd, 0, sizeof(strCmd));
					sprintf(strCmd,																					// 阅读时间和接收时间默认一样，状态默认车载端已经接收
						"INSERT INTO TransMessageDetail(MessageID,EquipmentID,State,ReceTime,ReadTime)VALUES(%d,'%s','%d','%s','%s');",
						nMessageID, it->c_str(), 1, &timeInterval, &timeInterval);
					ret = mysql_real_query(m_mysql, strCmd, strlen(strCmd));
				}
			}
			else
			{
				return J_DB_ERROR;
			}
#pragma region [   原来的代码逻辑   ]

			//memset(strCmd, 0, sizeof(strCmd));
			//sprintf(strCmd, "SELECT MAX(MessageID) FROM TransMessage;");

			//ret = mysql_real_query(m_mysql, strCmd, strlen(strCmd));
			//if (ret == 0)
			//{
			//	MYSQL_ROW sql_row_message_id;
			//	MYSQL_RES *result_message_id = mysql_store_result(m_mysql);                                         //保存查询到的数据到
			//	int nMessageID = 0;
			//	if (result_message_id->row_count != 0)
			//	{
			//		sql_row_message_id = mysql_fetch_row(result_message_id);
			//		if (sql_row_message_id[0] != NULL)
			//		{
			//			nMessageID = atol(sql_row_message_id[0]);
			//		}
			//	}

			//	std::vector<j_string_t>::iterator it = vecHost.begin();
			//	for (; it != vecHost.end(); it++)
			//	{
			//		memset(strCmd, 0, sizeof(strCmd));
			//		sprintf(strCmd, "INSERT INTO TransMessageDetail(MessageID,EquipmentID)VALUES(%d,'%s');", nMessageID, it->c_str());
			//		ret = mysql_real_query(m_mysql, strCmd, strlen(strCmd));
			//	}
			//}
#pragma endregion
		}

	}
	catch (...)
	{
		return J_DB_ERROR;
	}

	return J_OK;
}


/***********************************************************************************************************
 * 程序创建：刘进朝                     程序修改:赵进军
 * 函数功能：往transfile数据表里面新增一条数据---差一个状态
 * 参数说明：
 *   lUserID：发送联络文件的用户ID
 * pFileName：联络文件的名称
 * pFilePath：联络文件在转发服务端存储的位置
 *   vecHost：接收该文件的车载客户端ID
 * 注意事项：null
 * 修改日期：2015\10\15 14:57:00
 ***********************************************************************************************************/
j_result_t CMySQLAccess::AddFileInfo(long lUserID, const char *pFileName, const char *pFilePath, std::vector<j_string_t> &vecHost)
{
	if (!m_bConnected || lUserID < 0 || NULL == pFileName || NULL == pFilePath)
		return J_DB_ERROR;

	char timeInterval[arr_size] = { 0 };
	CXlHelper::GetTimestamp(true, timeInterval);																	// 获取当前系统的时间戳
	try
	{
		char strCmd[512] = { 0 };
		sprintf(strCmd, "INSERT INTO TransFile(UserID,Name,FilePath,SendTime)VALUES(%d,'%s','%s','%s');",
			lUserID, pFileName, pFilePath, timeInterval);															// 默认发送时间和删除时间一致

		int ret = mysql_real_query(m_mysql, strCmd, strlen(strCmd));
		if (ret == 0)
		{
			int nFileID = mysql_insert_id(m_mysql);																    //获取当前插入数据后的nFileID值
			if (nFileID >= 0)
			{
				std::vector<j_string_t>::iterator it = vecHost.begin();
				for (; it != vecHost.end(); it++)
				{
					memset(strCmd, 0, sizeof(strCmd));
					sprintf(strCmd, "INSERT INTO TransFileDetail(FileID,EquipmentID,ReceTime)VALUES(%d,'%s','%s');", nFileID, it->c_str(), timeInterval);
					ret = mysql_real_query(m_mysql, strCmd, strlen(strCmd));
				}
			}
			else
			{
				return J_DB_ERROR;
			}
		}
	}
	catch (...)
	{
		return J_DB_ERROR;
	}

	return J_OK;

#pragma region [   修改之前的代码   ]
	//if (!m_bConnected)
	//{
	//	return J_DB_ERROR;
	//}

	//try
	//{
	//	char strCmd[128] = { 0 };
	//	sprintf(strCmd, "INSERT INTO TransFile(UserID,Name,FilePath)VALUES(%d,'%s','%s');",
	//		lUserID, pTitle, pFilePath);
	//	int ret = mysql_real_query(m_mysql, strCmd, strlen(strCmd));
	//	if (ret == 0)
	//	{
	//		memset(strCmd, 0, sizeof(strCmd));
	//		sprintf(strCmd, "SELECT MAX(FileID) FROM TransFile;");

	//		ret = mysql_real_query(m_mysql, strCmd, strlen(strCmd));
	//		if (ret == 0)
	//		{
	//			MYSQL_ROW sql_row_file_id;
	//			MYSQL_RES *result_file_id = mysql_store_result(m_mysql);//保存查询到的数据到
	//			int nFileID = 0;
	//			if (result_file_id->row_count != 0)
	//			{
	//				sql_row_file_id = mysql_fetch_row(result_file_id);
	//				if (sql_row_file_id[0] != NULL)
	//				{
	//					nFileID = atol(sql_row_file_id[0]);
	//				}
	//			}

	//			std::vector<j_string_t>::iterator it = vecHost.begin();
	//			for (; it != vecHost.end(); it++)
	//			{
	//				memset(strCmd, 0, sizeof(strCmd));
	//				sprintf(strCmd, "INSERT INTO TransFileDetail(FileID,EquipmentID)VALUES(%d,'%s');", nFileID, it->c_str());
	//				ret = mysql_real_query(m_mysql, strCmd, strlen(strCmd));
	//			}
	//		}
	//	}

	//}
	//catch (...)
	//{
	//	return J_DB_ERROR;
	//}

	//return J_OK;
#pragma endregion
}


/***********************************************************************************************************
 * 程序创建：刘进朝                     程序修改:赵进军
 * 函数功能：更新联络消息日志
 * 参数说明：
 *    lMessageID：联络消息ID
 * nMessageState：消息状态
 *  nDetailState：详细状态
 * 注意事项：null
 * 修改日期：2015/10/12 14:15:00
 ***********************************************************************************************************/
j_result_t CMySQLAccess::UpdateContextInfo(long lMessageID, int nMessageState, int nDetailState)
{
	if (lMessageID < 0)																								// 当前位置的消息ID不能为负数
		return J_DB_ERROR;

	if (!m_bConnected)
		return J_DB_ERROR;

	try
	{
		return J_OK;
		char strCmd[128] = { 0 };																					// 更新transmessage表消息状态
		sprintf(strCmd, "UPDATE transmessage set DealStatus= '%s' WHERE MessageID = '%s'", nMessageState, lMessageID);
		int ret = mysql_real_query(m_mysql, strCmd, strlen(strCmd));
		if (ret == 0)
		{
			memset(strCmd, 0, sizeof(strCmd));																		// 更新transmessagedetail表消息状态
			sprintf(strCmd, "UPDATE transmessagedetail set State= '%s' WHERE MessageID = '%s'", nDetailState, lMessageID);
			ret = mysql_real_query(m_mysql, strCmd, strlen(strCmd));
			return	ret == 0 ? J_OK : J_DB_ERROR;
		}
		else
			return J_DB_ERROR;
	}
	catch (...)
	{
		return J_DB_ERROR;
	}
	return J_OK;
}


/***********************************************************************************************************
 * 程序创建：刘进朝                     程序修改:刘进朝
 * 函数功能：更新联络文件日志
 * 参数说明：
 *      lFileID：联络文件ID
 *   nFileState：文件状态
 * nDetailState：详细状态
 * 注意事项：null
 * 修改日期：2015/10/12 14:17:00
 ***********************************************************************************************************/
j_result_t CMySQLAccess::UpdateFileInfo(long lFileID, int nFileState, int nDetailState)
{
	if (lFileID < 0)																								// 当前位置的文件ID不能为负数
		return J_DB_ERROR;

	if (!m_bConnected)
		return J_DB_ERROR;

	try
	{
		return J_OK;
		char strCmd[128] = { 0 };																					// 更新transfile表消息状态
		sprintf(strCmd, "UPDATE transfile set State= '%s' WHERE MessageID = '%s'", nFileState, lFileID);
		int ret = mysql_real_query(m_mysql, strCmd, strlen(strCmd));
		if (ret == 0)
		{
			memset(strCmd, 0, sizeof(strCmd));																		// 更新transfiledetail表消息状态
			sprintf(strCmd, "UPDATE transfiledetail set State= '%s' WHERE MessageID = '%s'", nDetailState, lFileID);
			ret = mysql_real_query(m_mysql, strCmd, strlen(strCmd));
			return	ret == 0 ? J_OK : J_DB_ERROR;
		}
		else
			return J_DB_ERROR;
	}
	catch (...)
	{
		return J_DB_ERROR;
	}
	return J_OK;
}


/***********************************************************************************************************
 * 程序创建：刘进朝                     程序修改:赵进军
 * 函数功能：删除联络消息日志
 * 参数说明：
 * lMessageID：联络消息ID
 * 注意事项：null
 * 修改日期：2015/10/12 14:18:00
 ***********************************************************************************************************/
j_result_t CMySQLAccess::DelContextInfo(long lMessageID)
{
	if (lMessageID < 0)																								// 当前位置的消息ID不能为负数
		return J_DB_ERROR;

	if (!m_bConnected)
		return J_DB_ERROR;

	try
	{
		/// 怎样删除？
	}
	catch (...)
	{
		return J_DB_ERROR;
	}

	/// TODO by zhao jin jun
	return J_OK;
}


/***********************************************************************************************************
 * 程序创建：刘进朝                     程序修改:赵进军
 * 函数功能：删除联络文件日志
 * 参数说明：
 *  lFileID：联络文件ID
 * 注意事项：null
 * 修改日期：2015/10/12 14:20:00
 ***********************************************************************************************************/
j_result_t CMySQLAccess::DelFileInfo(long lFileID)
{
	if (lFileID < 0)																								// 当前位置的文件ID不能为负数
		return J_DB_ERROR;

	if (!m_bConnected)
		return J_DB_ERROR;

	try
	{
		/// 怎样删除？
	}
	catch (...)
	{
		return J_DB_ERROR;
	}
	return J_OK;
}

j_result_t CMySQLAccess::GetContextList(const char *pHostId, HostContextVec &contextVec)
{
	if (!m_bConnected)
	{
		return J_DB_ERROR;
	}

	try
	{
		mysql_set_character_set(m_mysql, "unicode");
		char strCmd[128] = { 0 };
		sprintf(strCmd, "SELECT MessageID FROM TransMessageDetail WHERE EquipmentID='%s';", pHostId);
		int ret = mysql_real_query(m_mysql, strCmd, strlen(strCmd));
		if (ret == 0)
		{
			MYSQL_ROW sql_row_message_id;
			MYSQL_RES *result_message_id = mysql_store_result(m_mysql);//保存查询到的数据到
			int nMessageID = 0;
			if (result_message_id->row_count != 0)
			{
				for (; sql_row_message_id = mysql_fetch_row(result_message_id);)
				{
					if (sql_row_message_id[0] != NULL)
					{
						nMessageID = atol(sql_row_message_id[0]);
					}

					memset(strCmd, 0, sizeof(strCmd));
					sprintf(strCmd, "SELECT UserID,Title,Content  FROM TransMessage WHERE MessageID=%d;", nMessageID);

					ret = mysql_real_query(m_mysql, strCmd, strlen(strCmd));
					if (ret == 0)
					{
						MYSQL_FIELD *fd_message;
						MYSQL_ROW sql_row_message;
						MYSQL_RES *result_message = mysql_store_result(m_mysql);//保存查询到的数据到
						if (result_message->row_count != 0)
						{
							int nUserID = 0;
							int nTitleLen = 0;
							char *pTitle = NULL;
							int nContentLen = 0;
							char *pContent = NULL;

							sql_row_message = mysql_fetch_row(result_message);
							for (int j = 0; fd_message = mysql_fetch_field(result_message); j++)//获取列名
							{
								if (strcmp("UserID", fd_message->name) == 0)
								{
									nUserID = atoi(sql_row_message[j]);
								}
								if (strcmp("Title", fd_message->name) == 0)
								{
									nTitleLen = strlen(sql_row_message[j]);
									pTitle = new char[nTitleLen + 1];
									memset(pTitle, 0, nTitleLen + 1);
									memcpy(pTitle, sql_row_message[j], nTitleLen);
								}
								if (strcmp("Content", fd_message->name) == 0)
								{
									nContentLen = strlen(sql_row_message[j]);
									pContent = new char[nContentLen + 1];
									memset(pContent, 0, nContentLen + 1);
									memcpy(pContent, sql_row_message[j], nContentLen);
								}
							}

							CXlDataBusInfo::XldCmdContextInfo *pContentInfo = (CXlDataBusInfo::XldCmdContextInfo *)malloc(sizeof(CXlDataBusInfo::XldCmdContextInfo) + nTitleLen);
							pContentInfo->header.lUserID = nUserID;
							pContentInfo->header.lMessageID = nMessageID;
							pContentInfo->header.nMessageTitleSize = nTitleLen;
							pContentInfo->header.ulMessageSize = nContentLen;
							memcpy(pContentInfo->header.data, pTitle, nTitleLen);
							delete[]pTitle;
							pContentInfo->pContext = pContent;
							contextVec.push_back(pContentInfo);
						}
					}
				}
			}
		}

	}
	catch (...)
	{
		return J_DB_ERROR;
	}

	return J_OK;
}

j_result_t CMySQLAccess::GetFileInfoList(const char *pHostId, HostFileInfoVec &fileVec)
{
	if (!m_bConnected)
	{
		return J_DB_ERROR;
	}

	try
	{
		char strCmd[128] = { 0 };
		sprintf(strCmd, "SELECT FileID FROM TransFileDetail WHERE EquipmentID='%s';", pHostId);
		int ret = mysql_real_query(m_mysql, strCmd, strlen(strCmd));
		if (ret == 0)
		{
			MYSQL_ROW sql_row_file_id;
			MYSQL_RES *result_file_id = mysql_store_result(m_mysql);//保存查询到的数据到
			int nFileID = 0;
			if (result_file_id->row_count != 0)
			{
				for (; sql_row_file_id = mysql_fetch_row(result_file_id);)
				{
					if (sql_row_file_id[0] != NULL)
					{
						nFileID = atol(sql_row_file_id[0]);
					}

					memset(strCmd, 0, sizeof(strCmd));
					sprintf(strCmd, "SELECT UserID,Name,FilePath FROM TransFile WHERE FileID=%d;", nFileID);

					ret = mysql_real_query(m_mysql, strCmd, strlen(strCmd));
					if (ret == 0)
					{
						MYSQL_FIELD *fd_file;
						MYSQL_ROW sql_row_file;
						MYSQL_RES *result_file = mysql_store_result(m_mysql);//保存查询到的数据到
						if (result_file->row_count != 0)
						{
							int nUserID = 0;
							int nTitleLen = 0;
							char *pTitle = NULL;
							int nFilePathLen = 0;
							char *pFileName = NULL;

							sql_row_file = mysql_fetch_row(result_file);
							for (int j = 0; fd_file = mysql_fetch_field(result_file); j++)//获取列名
							{
								if (strcmp("UserID", fd_file->name) == 0)
								{
									nUserID = atoi(sql_row_file[j]);
								}
								if (strcmp("Name", fd_file->name) == 0)
								{
									nTitleLen = strlen(sql_row_file[j]);
									pTitle = new char[nTitleLen + 1];
									memset(pTitle, 0, nTitleLen + 1);
									memcpy(pTitle, sql_row_file[j], nTitleLen);
								}
								if (strcmp("FilePath", fd_file->name) == 0)
								{
									nFilePathLen = strlen(sql_row_file[j]);
									pFileName = new char[nFilePathLen + 1];
									memset(pFileName, 0, nFilePathLen + 1);
									memcpy(pFileName, sql_row_file[j], nFilePathLen);
								}
							}

							CXlDataBusInfo::XldCmdFileInfo *pFileInfo = (CXlDataBusInfo::XldCmdFileInfo *)malloc(sizeof(CXlDataBusInfo::XldCmdFileInfo) + nTitleLen);
							pFileInfo->header.lUserID = nUserID;
							pFileInfo->header.nFileID = nFileID;
							pFileInfo->header.nFileNameSize = nTitleLen;
							pFileInfo->header.ulFileSize = 0;
							memcpy(pFileInfo->header.pData, pTitle, nTitleLen);
							delete[]pTitle;
							pFileInfo->pFileName = pFileName;
							fileVec.push_back(pFileInfo);
						}

					}
				}
			}
		}

	}
	catch (...)
	{
		return J_DB_ERROR;
	}

	return J_OK;
}