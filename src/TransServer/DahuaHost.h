///////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2009, xx科技有限公司(版权声明) 
/// All rights reserved. 
/// 
/// @file      DahuaHost.h 
/// @brief     Dahua_DVR设备模块
///
/// @version   1.0 (版本声明)
/// @author    Jorhy (joorhy@gmail.com) 
/// @date      2015/07/15 17:40 
///
///
/// 修订说明：最初版本
///////////////////////////////////////////////////////////////////////////  
#ifndef __DAHUAHOST_H_
#define __DAHUAHOST_H_

#include "j_includes.h"
#include "dhnetsdk.h"

/// 本类的功能:  大华车载设备接入类
class CDahuaHost : public J_Host
{
public:
	CDahuaHost(j_string_t strHostId, j_int64_t llHandle);
	~CDahuaHost();

public:
	virtual j_result_t CreateChannel(const j_int32_t nChannelNum, J_Obj *&pObj);
	virtual j_boolean_t IsReady() { return true; }
	virtual j_result_t OnBroken();
	virtual j_result_t GetHostId(j_string_t &strDevId) { strDevId = m_hostId; return J_OK; }
	virtual j_result_t OnHandleRead(J_AsioDataBase *pAsioData) { return 0; }
	virtual j_result_t OnHandleWrite(J_AsioDataBase *pAsioData) { return 0; }
	virtual j_result_t OnRequest(const CXlDataBusInfo &cmdData);

public:
	j_int64_t GetLoginHandle() const { return m_llHandle;  }
private:
	/// 客户端发来的请求实时视频
	j_result_t StartRealPlay(const CXlDataBusInfo &cmdData);
	/// 客户端发来的停止实时视频
	j_result_t StopRealPlay(const CXlDataBusInfo &cmdData);

private:
	j_int64_t m_llHandle;
	j_string_t m_hostId;
};

#endif //__DAHUAHOST_H_