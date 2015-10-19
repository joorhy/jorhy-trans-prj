///////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2009, xx¿Æ¼¼ÓÐÏÞ¹«Ë¾(°æÈ¨ÉùÃ÷) 
/// All rights reserved. 
/// 
/// @file      VnHost.h 
/// @brief     VN_DVRÉè±¸Ä£¿é
///
/// @version   1.0 (°æ±¾ÉùÃ÷)
/// @author    Jorhy (joorhy@gmail.com) 
/// @date      2015/04/22 17:40 
///
///
/// ÐÞ¶©ËµÃ÷£º×î³õ°æ±¾
///////////////////////////////////////////////////////////////////////////  
#ifndef __VNHOST_H_
#define __VNHOST_H_

#include "j_includes.h"
#include "netclientapi.h"

/// ±¾ÀàµÄ¹¦ÄÜ:  ºÏÖÚÖÇ»ÛÆ½Ì¨½ÓÈëÀà
class CVnHost : public J_Host
{
	typedef std::map<j_string_t, GPSDevInfo_S> DeviceMap;

public:
	CVnHost(j_string_t strHostId);
	~CVnHost();

public:
	///J_Host
	virtual j_result_t CreateChannel(const j_int32_t nChannelNum, J_Obj *&pObj);
	virtual j_boolean_t IsReady() { return true; };
	virtual j_result_t OnBroken() { return J_OK; }
	virtual j_result_t GetHostId(j_string_t &strDevId) { strDevId = m_strHostId; return J_OK; }
	virtual j_result_t OnHandleRead(J_AsioDataBase *pAsioData) { return J_OK; }
	virtual j_result_t OnHandleWrite(J_AsioDataBase *pAsioData) { return J_OK; }
	virtual j_result_t OnRequest(const CXlDataBusInfo &cmdData);

private:
	/// ¿Í»§¶Ë·¢À´µÄÇëÇóÊµÊ±ÊÓÆµ
	j_result_t StartRealPlay(const CXlDataBusInfo &cmdData);
	/// ¿Í»§¶Ë·¢À´µÄÍ£Ö¹ÊµÊ±ÊÓÆµ
	j_result_t StopRealPlay(const CXlDataBusInfo &cmdData);

private:
	j_string_t m_strHostId;
	j_boolean_t m_bReuseSource;
};

#endif //__VNHOST_H_