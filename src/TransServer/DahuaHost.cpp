#include "DahuaHost.h"
#include "DahuaChannel.h"
#include "XlProtocol.h"
#include "XlDataBusDef.h"
#include "dhnetsdk.h"

CDahuaHost::CDahuaHost(j_string_t strHostId, j_int64_t llHandle)
{
	m_llHandle = llHandle;
	m_hostId = strHostId;
}

CDahuaHost::~CDahuaHost()
{

}

j_result_t CDahuaHost::CreateChannel(const j_int32_t nChannelNum, J_Obj *&pObj)
{
	J_Obj *pChannel = NULL;
	ChannelMap::iterator it = m_channelMap.find(nChannelNum);
	if (it != m_channelMap.end())
	{
		pChannel = it->second.pChannel;
	}
	else
	{
		pChannel = new CDahuaChannel(this, nChannelNum);

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

j_result_t CDahuaHost::OnBroken()
{
	if (m_llHandle > 0)
	{
		CLIENT_Logout(m_llHandle);
		m_llHandle = 0;
	}

	return J_OK;
}

j_result_t CDahuaHost::OnRequest(const CXlDataBusInfo &cmdData)
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

j_result_t CDahuaHost::StartRealPlay(const CXlDataBusInfo &cmdData)
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

j_result_t CDahuaHost::StopRealPlay(const CXlDataBusInfo &cmdData)
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