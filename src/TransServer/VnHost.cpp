///////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2009, xx科技有限公司(版权声明) 
/// All rights reserved. 
/// 
/// @file      VnHost.cpp 
/// @brief     XL_DVR设备模块实现
///
/// @version   1.0 (版本声明)
/// @author    Jorhy (joorhy@gmail.com) 
/// @date      2015/04/22 17:41 
///
///
/// 修订说明：最初版本
/////////////////////////////////////////////////////////////////////////// a
#include "VnHost.h"
#include "VnChannel.h"
#include "XlProtocol.h"
#include "XlDataBusDef.h"
#include "netclientapi.h"
#include "netmediaapi.h"

CVnHost::CVnHost(j_string_t strHostId)
	: m_bReuseSource(false)
{
	m_strHostId = strHostId;
}

CVnHost::~CVnHost()
{

}


j_result_t CVnHost::CreateChannel(const j_int32_t nChannelNum, J_Obj *&pObj)
{
	J_Obj *pChannel = NULL;
	ChannelMap::iterator it = m_channelMap.find(nChannelNum);
	if (it != m_channelMap.end())
	{
		pChannel = it->second.pChannel;
	}
	else
	{
		pChannel = new CVnChannel(this, nChannelNum);

		TLock(m_channelLocker);
		ChannelInfo info = { 0 };
		info.pChannel = pChannel;
		info.nRef = 1;
		m_channelMap[nChannelNum] = info;
		TUnlock(m_channelLocker);
	}
	pObj = pChannel;

	return J_OK;
}

j_result_t CVnHost::OnRequest(const CXlDataBusInfo &cmdData)
{
	switch (cmdData.header.cmd)
	{
	case CXlProtocol::xlc_real_play:
		if (cmdData.header.flag == CXlProtocol::xl_ctrl_start)
		{
			StartRealPlay(cmdData);
		}
		else if (cmdData.header.flag == CXlProtocol::xl_ctrl_stop)
		{
			StopRealPlay(cmdData);
		}
		break;
	}

	return J_OK;
}

j_result_t CVnHost::StartRealPlay(const CXlDataBusInfo &cmdData)
{
	j_result_t nResult = J_UNKNOW;
	TLock(m_channelLocker);
	ChannelMap::iterator it = m_channelMap.find(cmdData.xlcCmdRealPlay.channel);
	if (it == m_channelMap.end())
	{
		J_Obj *pObj = NULL;
		CreateChannel(cmdData.xlcCmdRealPlay.channel, pObj);
		if (pObj != NULL)
		{
			J_Channel *pChannel = dynamic_cast<J_Channel *>(pObj);
			if (pChannel != NULL)
			{
				pChannel->OpenStream(cmdData);
			}
		}
	}
	else
	{
		++(it->second.nRef);
	}
	TUnlock(m_channelLocker);

	return nResult;
}

j_result_t CVnHost::StopRealPlay(const CXlDataBusInfo &cmdData)
{
	TLock(m_channelLocker);
	ChannelMap::iterator it = m_channelMap.find(cmdData.xlcCmdRealPlay.channel);
	if (it != m_channelMap.end())
	{
		--(it->second.nRef);
		if (it->second.nRef == 0)
		{
			J_Obj *pObj = NULL;
			CreateChannel(cmdData.xlcCmdRealPlay.channel, pObj);
			if (pObj != NULL)
			{
				J_Channel *pChannel = dynamic_cast<J_Channel *>(pObj);
				if (pChannel != NULL)
				{
					pChannel->CloseStream(cmdData);
				}
			}
			ReleaseChannel(cmdData.xlcCmdRealPlay.channel);
		}
		else
		{
			if (it->second.nRef < 0)
			{
				it->second.nRef = 0;
			}
		}
	}
	TUnlock(m_channelLocker);

	return J_OK;
}