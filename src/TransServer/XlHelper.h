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

class CXlHelper
{
public:
	/// 公用接口
	static j_result_t MakeRequest(j_char_t bCmd, j_char_t bFlag, j_int32_t nSeq, j_char_t *pData, j_int32_t nLen, j_char_t *pBody);
	static j_result_t MakeResponse(j_char_t bCmd, j_char_t bFlag, j_int32_t nSeq, j_char_t *pData, j_int32_t nLen, j_char_t *pBody);
	static j_result_t MakeResponse2(j_char_t bCmd, j_char_t bFlag, j_int32_t nSeq, j_char_t *pData, j_int32_t nLen, j_char_t *pBody);
	static j_uint32_t CheckNum(j_char_t *pData, j_int32_t nLen);
	static j_uint32_t CheckNum2(j_char_t *pData, j_int32_t nLen);
	static j_result_t MakeNetData(J_AsioDataBase *pAsioData, j_char_t *pDataBuff, j_int32_t nLen);
	//static j_result_t BlockSocket();
	static int Unicode2Ansi(LPCWSTR sInput, j_string_t& sOutput);
};
#endif // !__XLHELPER_H_
