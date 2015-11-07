///////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2009, xx科技有限公司(版权声明) 
/// All rights reserved. 
/// 
/// @file      XlHelper.cpp 
/// @brief     辅助接口实现
///
/// @version   1.0 (版本声明)
/// @author    Jorhy (joorhy@gmail.com) 
/// @date      2013/11/01 09:03 
/// 修订说明：最初版本
///////////////////////////////////////////////////////////////////////////  
#include "XlHelper.h"
#include "XlProtocol.h"
#include <iostream>
#include <sstream>
using namespace std;

const DWORD SIZE_BUFF = 1024;

j_result_t CXlHelper::MakeRequest(j_char_t bCmd, j_char_t bFlag, j_int32_t nSeq, j_char_t *pData, j_int32_t nLen, j_char_t *pBody)
{
	//填充头信息
	CXlProtocol::CmdHeader *pHeader = (CXlProtocol::CmdHeader *)pBody;
	pHeader->beginCode = 0xFF;
	pHeader->version = CXlProtocol::xl_frame_request;
	pHeader->seq = nSeq;
	pHeader->flag = bFlag;
	pHeader->cmd = bCmd;
	pHeader->length = nLen;
	//填充数据区
	if (nLen > 0)
		memcpy(pBody + sizeof(CXlProtocol::CmdHeader), pData, nLen);
	//填充尾信息
	CXlProtocol::CmdTail *pTail = (CXlProtocol::CmdTail *)(pBody + sizeof(CXlProtocol::CmdHeader) + nLen);
	pTail->verify = CheckNum(pBody, sizeof(CXlProtocol::CmdHeader) + nLen);
	pTail->endCode = 0xFE;

	return J_OK;
}

j_result_t CXlHelper::MakeResponse(j_char_t bCmd, j_char_t bFlag, j_int32_t nSeq, j_char_t *pData, j_int32_t nLen, j_char_t *pBody)
{
	//填充头信息
	CXlProtocol::CmdHeader *pHeader = (CXlProtocol::CmdHeader *)pBody;
	pHeader->beginCode = 0xFF;
	pHeader->version = CXlProtocol::xl_framer_response;
	pHeader->seq = nSeq;
	pHeader->flag = bFlag;
	pHeader->cmd = bCmd;
	pHeader->length = nLen;
	//填充数据区
	if (nLen > 0)
		memcpy(pBody + sizeof(CXlProtocol::CmdHeader), pData, nLen);
	//填充尾信息
	CXlProtocol::CmdTail *pTail = (CXlProtocol::CmdTail *)(pBody + sizeof(CXlProtocol::CmdHeader) + nLen);
	pTail->verify = CheckNum(pBody, sizeof(CXlProtocol::CmdHeader) + nLen);
	pTail->endCode = 0xFE;

	return J_OK;
}

j_result_t CXlHelper::MakeMessage(j_char_t bCmd, j_char_t bFlag, j_int32_t nSeq, j_char_t *pData, j_int32_t nLen, j_char_t *pBody)
{
	//填充头信息
	CXlProtocol::CmdHeader *pHeader = (CXlProtocol::CmdHeader *)pBody;
	pHeader->beginCode = 0xFF;
	pHeader->version = CXlProtocol::xl_frame_message;
	pHeader->seq = nSeq;
	pHeader->flag = bFlag;
	pHeader->cmd = bCmd;
	pHeader->length = nLen;
	//填充数据区
	if (nLen > 0)
		memcpy(pBody + sizeof(CXlProtocol::CmdHeader), pData, nLen);
	//填充尾信息
	CXlProtocol::CmdTail *pTail = (CXlProtocol::CmdTail *)(pBody + sizeof(CXlProtocol::CmdHeader) + nLen);
	pTail->verify = CheckNum(pBody, sizeof(CXlProtocol::CmdHeader) + nLen);
	pTail->endCode = 0xFE;

	return J_OK;
}

j_uint32_t CXlHelper::CheckNum(j_char_t *pData, j_int32_t nLen)
{
	j_uint32_t nCheckNum = 0xFE;
	for (int i = 0; i < nLen; ++i)
	{
		nCheckNum += pData[i];
		//nCheckNum %= 256;
	}

	return (nCheckNum % 256);
}

j_uint32_t CXlHelper::CheckNum2(j_char_t *pData, j_int32_t nLen)
{
	__int64 nCheckNum = 0xFE;
	for (int i = 0; i < nLen; ++i)
	{
		nCheckNum += pData[i];
		//nCheckNum %= 256;
	}

	return (nCheckNum % 256);
}

j_result_t CXlHelper::MakeNetData(J_AsioDataBase *pAsioData, j_char_t *pDataBuff, j_int32_t nLen)
{
	if (pAsioData->ioCall == J_AsioDataBase::j_read_e)
	{
		pAsioData->ioRead.buf = pDataBuff;
		pAsioData->ioRead.bufLen = nLen;
		pAsioData->ioRead.finishedLen = 0;
		pAsioData->ioRead.whole = true;
		pAsioData->ioRead.shared = true;
	}
	else if (pAsioData->ioCall == J_AsioDataBase::j_write_e)
	{
		pAsioData->ioWrite.buf = pDataBuff;
		pAsioData->ioWrite.bufLen = nLen;
		pAsioData->ioWrite.finishedLen = 0;
		pAsioData->ioWrite.whole = true;
		pAsioData->ioWrite.shared = true;
	}
	return J_OK;
}

int	CXlHelper::Unicode2Ansi(LPCWSTR sInput, j_string_t& sOutput)
{
	if (!sInput)	return 0;
	int nnn = wcslen(sInput);
	int len = WideCharToMultiByte(CP_ACP, 0, sInput, wcslen(sInput), NULL, 0, NULL, NULL);

	char *ansiBuffer = new char[len + 1];
	if (len == WideCharToMultiByte(CP_ACP, 0, sInput, wcslen(sInput), ansiBuffer, len, NULL, NULL))
	{
		ansiBuffer[len] = '\0';
		sOutput = ansiBuffer;
		delete[] ansiBuffer;

		return len;
	}
	delete[] ansiBuffer;

	return 0;
}
int CXlHelper::Ansi2Unicode(LPCSTR input, j_wstring_t& output)
{
	if (!input) return 0;

	int wideLen = MultiByteToWideChar(CP_ACP, 0, input, strlen(input), NULL, 0);

	WCHAR *unicodeBuffer = new WCHAR[wideLen + 1];
	if (wideLen == MultiByteToWideChar(CP_ACP, 0, input, strlen(input), unicodeBuffer, wideLen))
	{
		unicodeBuffer[wideLen] = L'\0';
		output = unicodeBuffer;
		delete[] unicodeBuffer;

		return wideLen;
	}
	delete[] unicodeBuffer;

	return 0;
}


/***********************************************************************************************************
 * 程序创建：赵进军                     程序修改:赵进军
 * 函数功能：获取当前时间的时间戳
 * 参数说明：
 *   isMillisecond:true 表示13位的毫秒级别数据，False 表示10位的秒级数据
 * 注意事项：null
 * 修改日期：2015/10/15 15:53:00
 ***********************************************************************************************************/
char* CXlHelper::GetTimestamp(j_boolean_t isMillisecond)
{
	time_t t = time(NULL);																						//获取系统时间
	char timeInterval[32] = { 0 };
	if (isMillisecond)
		sprintf_s(timeInterval, sizeof(timeInterval), "%ld000", t);
	else
		sprintf_s(timeInterval, sizeof(timeInterval), "%ld", t);
	return timeInterval;
}


/***********************************************************************************************************
* 程序创建：赵进军                     程序修改:赵进军
* 函数功能：获取当前时间的时间戳
* 参数说明：
*   isMillisecond:true 表示13位的毫秒级别数据，False 表示10位的秒级数据
* outTimeInterval：用户取得当前计算的时间戳结果，该数组长度必须是32
* 注意事项：outTimeInterval 该数组长度必须是32
* 修改日期：2015/10/15 16:53:00
***********************************************************************************************************/
void CXlHelper::GetTimestamp(j_boolean_t isMillisecond, char outTimeInterval[])
{
	time_t t = time(NULL);																						//获取系统时间
	char timeInterval[arr_size] = { 0 };
	if (isMillisecond)
		sprintf_s(timeInterval, sizeof(timeInterval), "%ld000", t);
	else
		sprintf_s(timeInterval, sizeof(timeInterval), "%ld", t);
	strncpy(outTimeInterval, timeInterval, sizeof(timeInterval) - 1);
}


/***********************************************************************************************************
 * 程序创建：赵进军                     程序修改:赵进军
 * 函数功能：重命名联络文件的文件名
 * 参数说明：
 * oldFileName：原文件名
 * 注意事项：null
 * 修改日期：2015\10\16 15:13:00
 ***********************************************************************************************************/
j_string_t CXlHelper::RenameFile(const j_string_t oldFileName)
{
	const char* newGuid = CreateNewGuid();																			// 创建一个新的Guide,用于更新文件名
	char fileName[128] = { 0 };
	strcpy(fileName, oldFileName.c_str());
	char *ext = strrchr(fileName, '.');

	if (ext)
	{
		*ext = '\0';
		ext++;
	}

	j_string_t loc_str;																								// 重新组装新的文件名
	loc_str = fileName;
	j_string_t newFileName;

	newFileName.append(loc_str);																					// 文件名
	loc_str = newGuid;																								// Guid
	newFileName.append(loc_str);
	newFileName.append(".");																						// .
	loc_str = ext;																									// 扩展名
	newFileName.append(loc_str);
	return newFileName;
}


/***********************************************************************************************************
* 程序作者：赵进军
* 函数功能：检查当年是闰年还是平年
* 参数说明：
*     year：需要检查的年份
* 注意事项：null
* 修改日期：2015/10/08 17:58:00
***********************************************************************************************************/
int CXlHelper::IsLeap(unsigned short year)
{
	return ((year % 4 == 0) && (year % 100 != 0) || (year % 400 == 0));
}

/***********************************************************************************************************
* 程序作者：赵进军
* 函数功能：检查时间戳转换成时间格式是否正常
* 参数说明：
*     time：时间戳
* timezone：默认为8，暂时不用修改
* 注意事项：null
* 修改日期：2015/10/08 18:01:00
***********************************************************************************************************/
bool CXlHelper::ActionTimestampCheck(long long time, long timezone)
{
	ostringstream os;																								// 将long 类型的时间戳转换成字符串类型的时间戳
	os << time;
	string result;
	istringstream is(os.str());
	is >> result;

	bool isMilsecond = false;																						// 标记当前输入的时间戳是毫秒级
	int timeLen = result.size();
	switch (timeLen)
	{
	case 13:
		isMilsecond = true;
		break;
	case 10:
		isMilsecond = false;
		break;
	default:
		return false;
		break;
	}

	const int monthLengths[2][13] =																					// 定义每个月天数累计
	{
		{ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
		{ 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
	};
	const int yearLengths[2] = { 365, 366 };																		// 定义平年、闰年的天数
	int year(0), month(0), minMonth(0), maxMonth(0), days(0), clock(0), isLeap(0), day(0), hour(0), minute(0), second(0);
	if (isMilsecond)
		time /= 1000;																							    // 把毫秒转换成秒
	time += timezone * 60 * 60;
	days = time / 86400;																							// 天数
	clock = time % 86400;																							// 小时数

	if (clock < 0)
	{
		clock += 86400;
		days -= 1;
	}

	if (days >= 0)
	{
		year = days / 366;
		days -= year * 365 + (year + 1) / 4 - (year + 69) / 100 + (year + 369) / 400;
		for (year = year + 1970;; year++)
		{
			isLeap = IsLeap(year);
			if (days < yearLengths[isLeap])
			{
				break;
			}
			days -= yearLengths[isLeap];
		}
	}
	else
	{
		year = days / 366;
		days -= year * 365 + (year - 2) / 4 - (year - 30) / 100 + (year - 30) / 400;
		for (year = year + 1970 - 1;; year--)
		{
			isLeap = false;
			days += yearLengths[isLeap];
			if (days >= 0)
			{
				break;
			}
		}
	}

	minMonth = 0;
	maxMonth = 12;
	for (month = 5; month < 12 && month>0; month = (minMonth + maxMonth) / 2)
	{
		if (days < monthLengths[isLeap][month])
		{
			maxMonth = month;
		}
		else if (days >= monthLengths[isLeap][month + 1])
		{
			minMonth = month;
		}
		else
		{
			break;
		}
	}
	days -= monthLengths[isLeap][month];
	month++;
	day = days + 1;

	hour = clock / 3600;																							// 获取当前时间戳的小时部分
	clock = clock % 3600;
	minute = clock / 60;																							// 获取当前时间戳的分钟部分
	second = clock % 60;																							// 获取当前时间戳的秒钟部分

	if (year < 1970 && year>2099)																					// 检查当前时间戳的年部分是否合法
		return false;

	if (month < 1 && month>12)																						// 检查当前时间戳的月部分是否合法
		return false;

	if (day < 1 && day>31)																							// 检查当前时间戳的天部分是否合法
		return false;

	if (hour < 1 && hour>24)																						// 检查当前时间戳的小时部分是否合法
		return false;

	if (minute < 1 && minute>60)																					// 检查当前时间戳的分钟部分是否合法
		return false;

	if (second < 1 && second>60)																					// 检查当前时间戳的秒钟部分是否合法
		return false;
	return true;
}


/***********************************************************************************************************
 * 程序创建：赵进军                     程序修改:赵进军
 * 函数功能：创建一个新唯一的Guid
 * 参数说明：null
 * 注意事项：
 * 要想修改生成的Guid的格式请在"_%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X"该字符串开始处进行修改
 * 修改日期：2015/10/27 15:08:00
 ***********************************************************************************************************/
const char* CXlHelper::CreateNewGuid()
{
	static char buf[64] = { 0 };
	GUID guid;
	if (S_OK == ::CoCreateGuid(&guid))
	{
		_snprintf(buf, sizeof(buf)
			, "_%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X"
			, guid.Data1
			, guid.Data2
			, guid.Data3
			, guid.Data4[0], guid.Data4[1]
			, guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5]
			, guid.Data4[6], guid.Data4[7]
			);
	}
	return (const char*)buf;
}
