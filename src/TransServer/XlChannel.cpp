#include "XlChannel.h"
#include "DataBus.h"

#define RECV_SIZE (2 * 1024 * 1024)

CXlChannel::CXlChannel(J_Obj *pOwner, j_int32_t nChannel)
{
	m_pDataBuff = NULL;
	m_nChannelNum = nChannel;
	m_parser.Init();
	if (NULL == m_pDataBuff)
	{
		m_pDataBuff = new char[RECV_SIZE];
	}
}

CXlChannel::~CXlChannel()
{
	m_parser.Deinit();
	if (m_pDataBuff != NULL)
	{
		delete m_pDataBuff;
		m_pDataBuff = NULL;
	}
}

j_result_t CXlChannel::OpenStream(const CXlDataBusInfo &cmdData)
{
	//AddRingBuffer(cmdData.cmdRealPlay.pBuffer);
	AddCmdHeader(cmdData.header);
	return J_OK;
}

j_result_t CXlChannel::CloseStream(const CXlDataBusInfo &cmdData)
{
	//DelRingBuffer(cmdData.cmdRealPlay.pBuffer);
	DelCmdHeader(cmdData.header);
	return J_OK;
}

j_result_t CXlChannel::OpenVod(const CXlDataBusInfo &cmdData)
{
	//TLock(m_mapLocker);
	//std::map<j_guid_t, CRingBuffer *>::iterator it = m_mapRingBuffer.find(cmdData.cmdStartVod.sessionId);
	//if (it == m_mapRingBuffer.end())
	//{
	//	m_mapRingBuffer[cmdData.cmdStartVod.sessionId] = cmdData.cmdStartVod.pBuffer;
	//}
	//TUnlock(m_mapLocker);
	AddCmdHeader(cmdData.header);

	return J_OK;
}

j_result_t CXlChannel::CloseVod(const CXlDataBusInfo &cmdData)
{
	//TLock(m_mapLocker);
	//std::map<j_guid_t, CRingBuffer *>::iterator it = m_mapRingBuffer.find(cmdData.cmdStartVod.sessionId);
	//if (it != m_mapRingBuffer.end())
	//{
	//	m_mapRingBuffer.erase(it);
	//}
	//TUnlock(m_mapLocker);
	DelCmdHeader(cmdData.header);

	return J_OK;
}

j_result_t CXlChannel::InputData(const j_int32_t nType, const CXlDataBusInfo *respData)
{
	J_StreamHeader streamHeader = {0};
	streamHeader.dataLen = respData->header.length + sizeof(CXlProtocol::CmdHeader) + sizeof(CXlProtocol::CmdTail);

	if (nType == CXlProtocol::xlc_real_play)
	{
		//J_StreamHeader streamHeader;
		//m_parser.InputData((const char *)respData.pData + 79, respData.respHeader.length - 79);
		//j_result_t nResult = m_parser.GetOnePacket(m_pDataBuff, streamHeader);
		//while (nResult == J_OK)
		//{
		//	streamHeader.dataLen -= 4;
		//	TLock(m_vecLocker);
		//	std::vector<CRingBuffer *>::iterator it = m_vecRingBuffer.begin();
		//	for (; it != m_vecRingBuffer.end(); it++)
		//	{
		//		(*it)->PushBuffer((char *)m_pDataBuff + 4, streamHeader);
		//	}
		//	TUnlock(m_vecLocker);
		//	nResult = m_parser.GetOnePacket(m_pDataBuff, streamHeader);
		//}

		//J_OS::LOGINFO("CXlChannel::InputData send len = %d", streamHeader.dataLen);

		TLock(m_vecLocker);
		std::vector<CXlProtocol::CmdHeader>::iterator it = m_vecCmdHeader.begin();
		for (; it != m_vecCmdHeader.end(); it++)
		{
			CXlDataBusInfo *response = (CXlDataBusInfo *)respData;
			response->header.seq = it->seq;
			response->header.cmd = CXlProtocol::xld_start_real_play;
			JoDataBus->Response(*response);
		}
		TUnlock(m_vecLocker);
	}
	else
	{
		//TLock(m_mapLocker);
		//std::map<j_guid_t, CRingBuffer *>::iterator it = m_mapRingBuffer.find(respData.respVodData.sessionId);
		//if (it != m_mapRingBuffer.end())
		//{
		//	it->second->PushBuffer((char *)&respData, streamHeader);
		//}
		//TUnlock(m_mapLocker);

		//J_OS::LOGINFO("CXlChannel::InputData send len = %d", streamHeader.dataLen);

		TLock(m_vecLocker);
		std::vector<CXlProtocol::CmdHeader>::iterator it = m_vecCmdHeader.begin();
		for (; it != m_vecCmdHeader.end(); it++)
		{
			CXlDataBusInfo *response = (CXlDataBusInfo *)respData;
			response->header.seq = it->seq;
			response->header.cmd = (response->header.cmd == CXlProtocol::xld_start_vod_play) ? CXlProtocol::xlc_vod_play : CXlProtocol::xlc_vod_download;
			JoDataBus->Response(*response);
		}
		TUnlock(m_vecLocker);
	}

	return J_OK;
}


