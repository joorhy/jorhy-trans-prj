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
	AddCmdHeader(cmdData.header);
	return J_OK;
}

j_result_t CXlChannel::CloseStream(const CXlDataBusInfo &cmdData)
{
	DelCmdHeader(cmdData.header);
	return J_OK;
}

j_result_t CXlChannel::OpenVod(const CXlDataBusInfo &cmdData)
{
	AddCmdHeader(cmdData.header);

	return J_OK;
}

j_result_t CXlChannel::CloseVod(const CXlDataBusInfo &cmdData)
{
	DelCmdHeader(cmdData.header);

	return J_OK;
}

j_result_t CXlChannel::InputData(const j_int32_t nType, const CXlDataBusInfo *respData)
{
	J_StreamHeader streamHeader = {0};
	streamHeader.dataLen = respData->header.length + sizeof(CXlProtocol::CmdHeader) + sizeof(CXlProtocol::CmdTail);

	if (nType == CXlProtocol::xlc_real_play)
	{
		TLock(m_vecLocker);
		std::vector<CXlProtocol::CmdHeader>::iterator it = m_vecCmdHeader.begin();
		for (; it != m_vecCmdHeader.end(); it++)
		{
			CXlDataBusInfo *response = (CXlDataBusInfo *)respData;
			response->header.seq = it->seq;
			response->header.cmd = CXlProtocol::xld_real_play;
			JoDataBus->Response(*response);
		}
		TUnlock(m_vecLocker);
	}
	else
	{
		TLock(m_vecLocker);
		std::vector<CXlProtocol::CmdHeader>::iterator it = m_vecCmdHeader.begin();
		for (; it != m_vecCmdHeader.end(); it++)
		{
			CXlDataBusInfo *response = (CXlDataBusInfo *)respData;
			response->header.seq = it->seq;
			response->header.cmd = (response->header.cmd == CXlProtocol::xld_vod_play) ? CXlProtocol::xlc_vod_play : CXlProtocol::xlc_vod_download;
			JoDataBus->Response(*response);
		}
		TUnlock(m_vecLocker);
	}

	return J_OK;
}


