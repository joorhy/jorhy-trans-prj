///////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2009, xx科技有限公司(版权声明) 
/// All rights reserved. 
/// 
/// @file      XlHelper.h 
/// @brief     辅助接口
///
/// @version   1.0 (版本声明)
/// @author    Jorhy (joorhy@gmail.com) 
/// @date      2013/11/01 09:02 
/// 修订说明：最初版本
///////////////////////////////////////////////////////////////////////////  
#ifndef __XLHELPER_H_
#define __XLHELPER_H_
#include "j_includes.h"

#define arr_size 32

class CXlHelper
{
public:
	/// 公用接口
	static j_result_t MakeRequest(j_char_t bCmd, j_char_t bFlag, j_int32_t nSeq, j_char_t *pData, j_int32_t nLen, j_char_t *pBody);
	static j_result_t MakeResponse(j_char_t bCmd, j_char_t bFlag, j_int32_t nSeq, j_char_t *pData, j_int32_t nLen, j_char_t *pBody);
	static j_result_t MakeMessage(j_char_t bCmd, j_char_t bFlag, j_int32_t nSeq, j_char_t *pData, j_int32_t nLen, j_char_t *pBody);
	static j_uint32_t CheckNum(j_char_t *pData, j_int32_t nLen);
	static j_uint32_t CheckNum2(j_char_t *pData, j_int32_t nLen);
	static j_result_t MakeNetData(J_AsioDataBase *pAsioData, j_char_t *pDataBuff, j_int32_t nLen);
	//static j_result_t BlockSocket();
	static int Unicode2Ansi(LPCWSTR sInput, j_string_t& sOutput);
	static int Ansi2Unicode(LPCSTR input, j_wstring_t& output);

	// 获取时间戳
	static char* GetTimestamp(j_boolean_t isMillisecond);

	// 获取系统时间当前的时间戳
	static void GetTimestamp(j_boolean_t isMillisecond, char outTimeInterval[]);

	// 验证时间戳是否正确
	static bool ActionTimestampCheck(long long time, long timezone);

	// 重命名联络文件的文件名
	static j_string_t RenameFile(const j_string_t oldFileName);

	// 生成GUID
	static const char* CreateNewGuid();

private:
	// 闰年/平年判断
	static	int IsLeap(unsigned short year);
};
#endif // !__XLHELPER_H_
