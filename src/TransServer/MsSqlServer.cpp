///////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2009, xx科技有限公司(版权声明) 
/// All rights reserved. 
/// 
/// @file      SqlServerAccess.cpp 
/// @brief     SqlServer数据库模块实现
///
/// @version   1.0 (版本声明)
/// @author    Jorhy (joorhy@gmail.com) 
/// @date      2013/09/22 11:11 
///
///
/// 修订说明：最初版本
///////////////////////////////////////////////////////////////////////////  
#include "MsSqlServer.h"
#include <iostream>
using namespace std;

JO_IMPLEMENT_SINGLETON(SqlServerAccess)

CSqlServerAccess::CSqlServerAccess()
{
	::CoInitialize(NULL); // 初始化OLE/COM库环境 ，为访问ADO接口做准备
}

CSqlServerAccess::~CSqlServerAccess()
{
	::CoUninitialize();
}

j_result_t CSqlServerAccess::Connect(const j_char_t *pAddr, j_int16_t nPort, const j_char_t *pUa, const j_char_t *pPwd)
{
	try 
	{		
		//HRESULT hr = m_pConn.CreateInstance("ADODB.Connection");
		HRESULT hr = m_pConn.CreateInstance(__uuidof(Connection));
		if (FAILED(hr))
		{
			_com_error e(hr);
			J_OS::LOGINFO("CSqlServerAccess::Connect m_pConn.CreateInstance %s", e.ErrorMessage());
			return J_DB_ERROR;
		}
		//hr = m_pRec.CreateInstance("ADODB.Recordset");
		hr = m_pRec.CreateInstance(__uuidof(Recordset));
		if (FAILED(hr))
		{
			_com_error e(hr);
			J_OS::LOGINFO("CSqlServerAccess::Connect m_pRec.CreateInstance %s", e.ErrorMessage());
			return J_DB_ERROR;
		}
		//hr = m_pCmd.CreateInstance("ADODB.Command");
		hr = m_pCmd.CreateInstance(__uuidof(Command));
		if (FAILED(hr))
		{
			_com_error e(hr);
			J_OS::LOGINFO("CSqlServerAccess::Connect m_pCmd.CreateInstance %s", e.ErrorMessage());
			return J_DB_ERROR;
		}
		j_char_t connString[256] = {0};
		sprintf(connString, "Provider=SQLOLEDB;Server=%s,%d;Database=xl; uid=%s; pwd=%s;", pAddr, nPort, pUa, pPwd);
		m_pConn->ConnectionString = connString;
		m_pConn->ConnectionTimeout = 5;				//连接5秒超时
		if (m_pConn->Open("", "", "", adConnectUnspecified) != S_OK)
		{
			J_OS::LOGINFO("CSqlServerAccess::Connect Open Error");
			return J_DB_ERROR;
		}
	}
	catch (...)
	{
		J_OS::LOGINFO("CSqlServerAccess::Connect Except");
		return J_DB_ERROR;
	}

	return J_OK;
}

j_result_t CSqlServerAccess::Init()
{
	try
	{
		char strCmd[512] = {0};
		sprintf(strCmd, "SELECT * FROM Equipment;");
		m_pRec = m_pConn->Execute((_bstr_t)strCmd, NULL, adCmdText);
		while (!m_pRec->EndOfFile)
		{
			memset (strCmd, 0, sizeof(strCmd));
			sprintf(strCmd, "UPDATE Equipment SET Online=0 WHERE EquipmentID='%s';"
				, (char*)_bstr_t(m_pRec->GetCollect("EquipmentID")));
			m_pConn->Execute((_bstr_t)strCmd, NULL, adCmdText);
			m_pRec->MoveNext();
		}
		memset (strCmd, 0, sizeof(strCmd));
		sprintf(strCmd, "SELECT * FROM UserInfo;");
		m_pRec = m_pConn->Execute((_bstr_t)strCmd, NULL, adCmdText);
		while (!m_pRec->EndOfFile)
		{
			memset (strCmd, 0, sizeof(strCmd));
			sprintf(strCmd, "UPDATE UserInfo SET Online=0 WHERE AccountName='%s';"
				, (char*)_bstr_t(m_pRec->GetCollect("AccountName")));
			m_pConn->Execute((_bstr_t)strCmd, NULL, adCmdText);
			m_pRec->MoveNext();
		}
	}
	catch (...)
	{
		J_OS::LOGDEBUG("CSqlServerAccess::Init Exception");
		return J_DB_ERROR;
	}
	return J_OK;
}

j_result_t CSqlServerAccess::Release()
{
	try
	{
		m_pRec->Close();
		m_pConn->Close();
		m_pCmd.Release();
		m_pRec.Release();
		m_pConn.Release();
	}
	catch (...)
	{
		J_OS::LOGDEBUG("CSqlServerAccess::Release Exception");
		return J_DB_ERROR;
	}

	return J_OK;
}

j_result_t CSqlServerAccess::Login(const char *pUserName, const char *pPasswd, int nForce, int &nRet)
{
	try 
	{
		char strCmd[128] = {0};
		sprintf(strCmd, "SELECT * FROM UserInfo WHERE AccountName='%s';", pUserName);
		m_pRec = m_pConn->Execute((_bstr_t)strCmd, NULL, adCmdText);
		if (m_pRec->EndOfFile)
		{
			nRet = 1;
			return J_DB_ERROR;
		}
		else
		{
			if (strcmp(pPasswd, (char*)_bstr_t(m_pRec->GetCollect("Password"))) != 0)
			{
				nRet = 2;
				return J_DB_ERROR;
			}
			else if (nForce)
			{
				sprintf(strCmd, "UPDATE UserInfo SET Online=1 WHERE AccountName='%s';", pUserName);
				m_pConn->Execute((_bstr_t)strCmd, NULL, adCmdText);
			}
			else if (m_pRec->GetCollect("Online").intVal == 1)
			{
				nRet = 6;
				return J_DB_ERROR;
			}
			else
			{
				nRet = 0;
				memset(strCmd, 0, sizeof(strCmd));
				sprintf(strCmd, "UPDATE UserInfo SET Online=1 WHERE AccountName='%s';", pUserName);
				m_pConn->Execute((_bstr_t)strCmd, NULL, adCmdText);
			}
		}
	}
	catch(...)
	{
		J_OS::LOGDEBUG("CSqlServerAccess::Login Exception");
		return J_DB_ERROR;
	}

	return J_OK;
}

j_result_t CSqlServerAccess::GetUserNameById(j_int32_t userId, j_string_t &userName)
{
	try 
	{
		char strCmd[128] = {0};
		sprintf(strCmd, "SELECT AccountName FROM UserInfo WHERE UserID=%d;", userId);
		m_pRec = m_pConn->Execute((_bstr_t)strCmd, NULL, adCmdText);
		if (m_pRec->EndOfFile)
			return J_DB_ERROR;
		else
		{
			userName = (char*)_bstr_t(m_pRec->GetCollect("AccountName"));
		}
	}
	catch(...)
	{
		J_OS::LOGDEBUG("CSqlServerAccess::GetUserNameById Exception");
		return J_DB_ERROR;
	}

	return J_OK;
}

j_result_t CSqlServerAccess::Logout(const char *pUserName)
{
	try 
	{
		char strCmd[128] = {0};
		sprintf(strCmd, "UPDATE UserInfo SET Online=0 WHERE AccountName='%s';", pUserName);
		m_pConn->Execute((_bstr_t)strCmd, NULL, adCmdText);
	}
	catch(...)
	{
		J_OS::LOGDEBUG("CSqlServerAccess::Logout Exception");
		return J_DB_ERROR;
	}
	return J_OK;
}

j_result_t  CSqlServerAccess::UpdateFileInfo(j_int32_t nFileId, j_int32_t nState, bool bFlag)
{
	try 
	{
		time_t loacl_tm = time(0);
		tm *today = localtime(&loacl_tm);
		char strCmd[128] = {0};
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
		m_pConn->Execute((_bstr_t)strCmd, NULL, adCmdText);
	}
	catch(...)
	{
		J_OS::LOGDEBUG("CSqlServerAccess::UpdateFileInfo Exception");
		return J_DB_ERROR;
	}

	return J_OK;
}

j_result_t CSqlServerAccess::UpdateDevInfo(const CXlDataBusInfo::XldRespHostInfo &hostInfo, bool bOnline)
{
	try 
	{
		char strCmd[256] = {0};
		sprintf(strCmd, "SELECT * FROM Equipment WHERE EquipmentID='%s';", hostInfo.hostId);
		m_pRec = m_pConn->Execute((_bstr_t)strCmd, NULL, adCmdText);
		if (m_pRec->EndOfFile)
		{
			memset (strCmd, 0, sizeof(strCmd));
			sprintf(strCmd, "INSERT INTO Equipment (EquipmentID,VehicleNO,PhoneNum,TotalChannels,Online,State) VALUES ('%s','%s','%s',%d,%d,%d);", 
				hostInfo.hostId, hostInfo.vehicleNum, hostInfo.phoneNum, hostInfo.totalChannels, bOnline, 1);
			m_pRec = m_pConn->Execute((_bstr_t)strCmd, NULL, adCmdText);
		}
		
		memset (strCmd, 0, sizeof(strCmd));
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
		m_pConn->Execute((_bstr_t)strCmd, NULL, adCmdText);

		char *mediaType = (char *)hostInfo.data + sizeof(hostInfo.chNameSize);

		char *pChannelData = mediaType + hostInfo.mediaTypeNum;
		char *pChannelEnd = NULL;
		for (int i = 0; i<hostInfo.totalChannels; i++)
		{
			memset(strCmd, 0, sizeof(strCmd));
			sprintf(strCmd, "SELECT * FROM EquipmentDetail WHERE EquipmentID='%s' AND channel=%d;", hostInfo.hostId, i + 1);
			m_pRec = m_pConn->Execute((_bstr_t)strCmd, NULL, adCmdText);
			
			char channelName[64] = {0};
			int nameLen = 0;
			pChannelEnd = strstr(pChannelData, "#");
			if (pChannelEnd != NULL)
			{
				nameLen = pChannelEnd - pChannelData;
				strncpy(channelName, pChannelData, nameLen);
			}
			else
			{
				strcpy(channelName, pChannelData);
			}
			pChannelData += (nameLen + 1);

			memset(strCmd, 0, sizeof(strCmd));
			if (m_pRec->EndOfFile)
			{
				sprintf(strCmd, "INSERT INTO EquipmentDetail (EquipmentID,channel,channelType,channelName) VALUES ('%s',%d,%d,'%s');", 
					hostInfo.hostId, i + 1, mediaType[i] & 0xFF, channelName);
			}
			else
			{
				sprintf(strCmd, "UPDATE EquipmentDetail SET channelType=%d,channelName='%s' WHERE EquipmentID='%s' AND channel = %d;",
					mediaType[i] & 0xFF, channelName, hostInfo.hostId, i + 1);
			}
			m_pRec = m_pConn->Execute((_bstr_t)strCmd, NULL, adCmdText);
		}
	}
	catch(...)
	{
		J_OS::LOGDEBUG("CSqlServerAccess::UpdateDevInfo Exception");
		return J_DB_ERROR;
	}

	return J_OK;
}

j_result_t CSqlServerAccess::GetDevInfo(CXlDataBusInfo::XldRespHostInfo &hostInfo)
{
	try 
	{
		char strCmd[128] = {0};
		sprintf(strCmd, "SELECT * FROM Equipment WHERE EquipmentID='%s';", hostInfo.hostId);
		m_pRec = m_pConn->Execute((_bstr_t)strCmd, NULL, adCmdText);
		if (m_pRec->EndOfFile)
			return J_UNKNOW;
		strcpy(hostInfo.vehicleNum, (char*)_bstr_t(m_pRec->GetCollect("VehicleNO")));
		strcpy(hostInfo.phoneNum, (char*)_bstr_t(m_pRec->GetCollect("PhoneNum")));
		hostInfo.totalChannels = m_pRec->GetCollect("TotalChannels").intVal;
	}
	catch(...)
	{
		J_OS::LOGDEBUG("CSqlServerAccess::GetDevInfo Exception");
		return J_DB_ERROR;
	}

	return J_OK;
}

j_result_t CSqlServerAccess::InsertAlarmInfo(const char *pHostId, const CXlDataBusInfo::XldRespAlarmInfo& alarmInfo)
{
	try 
	{
		char strCmd[512] = {0};
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

time_t CSqlServerAccess::GetDevLogLastTime(const char *pHostId)
{
	time_t tmReturn = 0;
	try 
	{
		char strCmd[128] = {0};
		sprintf(strCmd, "SELECT MAX(TimeStamp) FROM EquipmentLog WHERE EquipmentID='%s';", pHostId);
		m_pRec = m_pConn->Execute((_bstr_t)strCmd, NULL, adCmdText);
		if (!m_pRec->EndOfFile)
		{
			_variant_t vIndex = (long)0;
			tmReturn = m_pRec->GetCollect(vIndex).llVal * 1000;
		}
	}
	catch (...)
	{

	}

	return tmReturn;
}

j_result_t CSqlServerAccess::InsertLogInfo(const char *pHostId, int nState, time_t tmTimeStamp)
{
	try
	{
		char strCmd[512] = {0};
		sprintf(strCmd, "INSERT INTO EquipmentLog(EquipmentID,State,TimeStamp)VALUES('%s',%d,%d);",
			pHostId, nState, tmTimeStamp);
		m_pConn->Execute((_bstr_t)strCmd, NULL, adCmdText);
	}
	catch (...)
	{
		return J_DB_ERROR;
	}

	return J_OK;
}
