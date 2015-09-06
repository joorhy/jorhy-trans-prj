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
	mysql_set_character_set(m_mysql, "utf8");
	//mysql_real_query(m_mysql,"SET NAMES UTF8", strlen("SET NAMES UTF8"));
	//mysql_query(m_mysql,"SET NAMES UTF8");

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

j_result_t CMySQLAccess::UpdateDevInfo(const CXlHostRespData::RespHostInfo &hostInfo, bool bOnline)
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
		for (int i = 0; i<hostInfo.totalChannels; i++)
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

j_result_t CMySQLAccess::GetDevInfo(CXlHostRespData::RespHostInfo &hostInfo)
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

j_result_t CMySQLAccess::InsertAlarmInfo(const char *pHostId, const CXlHostRespData::RespAlarmInfo& alarmInfo)
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

j_result_t CMySQLAccess::InsertLogInfo(const char *pHostId, time_t tmStart, time_t tmEnd)
{
	if (!m_bConnected)
	{
		return J_DB_ERROR;
	}

	try
	{
		char strCmd[512] = { 0 };
		// 将时间格式修改为13位的longlong类型 [8/25/2015 gaol]
		//time_t startTime = tmStart / 1000;
		//time_t endTime = tmEnd / 1000;
		//tm *pStartTime = localtime(&startTime);
		//char strStartTime[32] = {0};
		//sprintf(strStartTime, "%d-%02d-%02d %02d:%02d:%02d", pStartTime->tm_year + 1900, pStartTime->tm_mon + 1, pStartTime->tm_mday, 
		//	pStartTime->tm_hour, pStartTime->tm_min, pStartTime->tm_sec);

		//tm *pEndTime = localtime(&endTime);
		//char strEndTime[32] = {0};
		//sprintf(strEndTime, "%d-%02d-%02d %02d:%02d:%02d", pEndTime->tm_year + 1900, pEndTime->tm_mon + 1, pEndTime->tm_mday, 
		//	pEndTime->tm_hour, pEndTime->tm_min, pEndTime->tm_sec);

		//sprintf(strCmd, "INSERT INTO RecordingLog(EquipmentID,StartTime,EndTime)VALUES('%s','%s','%s')",
		//	pHostId, strStartTime, strEndTime);

		sprintf(strCmd, "INSERT INTO RecordingLog(EquipmentID,StartTime,EndTime)VALUES('%s','%I64u','%I64u')",
			pHostId, tmStart, tmEnd);
		int ret = mysql_real_query(m_mysql, strCmd, strlen(strCmd));
		if (ret == 0)
		{
		}
	}
	catch (...)
	{
		return J_DB_ERROR;
	}

	return J_OK;
}

j_result_t CMySQLAccess::UpdateContextInfo(long lUserID, const char *pTitle, const char *pContext, std::vector<j_string_t> &vecHost)
{
	return J_OK;
}

j_result_t CMySQLAccess::UpdateFileInfo(long lUserID, const char *pTitle, const char *pFilePath, std::vector<j_string_t> &vecHost)
{
	return J_OK;
}

j_result_t CMySQLAccess::GetContextList(const char *pHostId, HostContextVec &contextVec)
{
	return J_OK;
}

j_result_t CMySQLAccess::GetFileInfoList(const char *pHostId, HostFileInfoVec &contextVec)
{
	return J_OK;
}