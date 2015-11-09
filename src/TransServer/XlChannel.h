///////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2009, xx科技有限公司(版权声明) 
/// All rights reserved. 
/// 
/// @file      XlChannel.h 
/// @brief     XL_DVR通道模块
///
/// @version   1.0 (版本声明)
/// @author    Jorhy (joorhy@gmail.com) 
/// @date      2013/10/17 20:47 
///
///
/// 修订说明：最初版本
///////////////////////////////////////////////////////////////////////////  
#ifndef __XLCHANNEL_H_
#define __XLCHANNEL_H_
#include "j_includes.h"
#include "XlDataBusDef.h"

/// 本类的功能:  流管理类
class CXlChannel : public J_Channel
				 , public J_Vod
{
public:
	CXlChannel(J_Obj *pOwner, j_int32_t nChannel);
	~CXlChannel();

public:
	///J_Channel
	virtual j_result_t OpenStream(const CXlDataBusInfo &cmdData);
	virtual j_result_t CloseStream(const CXlDataBusInfo &cmdData);
	///J_Vod
	virtual j_result_t OpenVod(const CXlDataBusInfo &cmdData);
	virtual j_result_t CloseVod(const CXlDataBusInfo &cmdData);

public:
	/// 输入数据
	/// @param[in]				nType 数据类型,0-实时视频,1-历史视频
	/// @param[in]				pData 数据区
	/// @param[in]				nLen 数据长度 
	/// @return					参见j_errtype.h 
	j_result_t InputData(const j_int32_t nType, const CXlDataBusInfo *respData);

private:
	int AddCmdHeader(const CXlProtocol::CmdHeader &cmdHeader)
	{
		TLock(m_vecLocker);
		m_vecCmdHeader.push_back(cmdHeader);
		TUnlock(m_vecLocker);

		return J_OK;
	}
	int DelCmdHeader(const CXlProtocol::CmdHeader &cmdHeader)
	{
		TLock(m_vecLocker);
		std::vector<CXlProtocol::CmdHeader>::iterator it = m_vecCmdHeader.begin();
		for (; it != m_vecCmdHeader.end(); it++)
		{
			if (it->seq == cmdHeader.seq)
			{
				m_vecCmdHeader.erase(it);
				TUnlock(m_vecLocker);

				return J_OK;
			}
		}
		TUnlock(m_vecLocker);
		return J_NOT_EXIST;
	}
private:
	j_int32_t m_nChannelNum;
	J_OS::TLocker_t m_vecLocker;
	J_OS::TLocker_t m_mapLocker;
	std::vector<CXlProtocol::CmdHeader> m_vecCmdHeader;

	char *m_pDataBuff;
};

#endif //~__XLCHANNEL_H_
