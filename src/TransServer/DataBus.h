#ifndef __DATA_BUS_H_
#define __DATA_BUS_H_
#include "j_includes.h"
#include "x_timer.h"
#include "x_lock.h"
#include "XlDataBusDef.h"

class CDataBus
{
	/// 设备注册
	typedef std::map<j_string_t, J_DataBus *> ObjectMap;
	/// 消息订阅
	typedef std::map<j_string_t, J_DataBus *> MessageMap;
	/// 消息请求及回复
	struct RequestKey
	{
		RequestKey(){}
		RequestKey(J_DataBus *pClient, j_uint32_t nSeq)
		{
			this->pClient = pClient;
			this->nSeq = nSeq;
		}
		bool operator < (const RequestKey &other) const
		{
			if (other.pClient == pClient)
			{
				return nSeq < other.nSeq;
			}
			return pClient < other.pClient;
		}
		J_DataBus *pClient;
		j_uint32_t nSeq;
	};
	struct RequestInfo
	{
		J_DataBus *pClient;
		CXlProtocol::CmdHeader cmdHeader;
		j_int32_t nTimeOut;
		j_uint32_t nSeq;
	};
	typedef std::map<RequestKey, RequestInfo> RequestMap;
	typedef std::map<j_uint32_t, RequestKey> ResponseMap;
	/// 消息超时
	typedef std::map<RequestKey, J_DataBus *> RequestTimeOutMap;
public:
	CDataBus();
	~CDataBus();

public:
	/// 注册设备对象
	/// @param[in]		strHostId 设备ID
	/// @param[in]		pHost 设备对象 
	/// @return			参见j_errtype.h 
	j_result_t RegisterDevice(j_string_t strHostId, J_DataBus *pHost);
	/// 清除所有请求
	/// @param[in]		pClient 请求对象
	/// @return			参见j_errtype.h 
	j_result_t ClearRequest(J_DataBus *pClient);
	/// 订阅设备消息
	/// @param[in]		pClient 订阅者对象
	/// @param[in]		strUserId 用户ID
	/// @return			参见j_errtype.h 
	j_result_t SubscribeMsg(j_string_t strUserId, J_DataBus *pClient);
	/// 发送请求消息
	/// @param[in]		cmdData 请求消息内容
	/// @param[in]		pClient 请求者
	/// @param[in]		nTimeOut 请求超时时间,单位ms 
	/// @return			参见j_errtype.h 
	j_result_t Request(j_string_t strHostId, J_DataBus *pClient, const CXlDataBusInfo cmdData, j_uint32_t nTimeOut = 3000);
	/// 发送请求消息
	/// @param[in]		cmdData 请求消息内容
	/// @param[in]		pClient 请求者
	/// @param[in]		nTimeOut 请求超时时间,单位ms 
	/// @return			参见j_errtype.h 
	j_result_t RequestMessage(j_string_t strHostId, J_DataBus *pClient, const CXlDataBusInfo &cmdData);
	/// 请求应答消息
	/// @param[in]		respData 应答消息内容
	/// @return			参见j_errtype.h 
	j_result_t Response(const CXlDataBusInfo &respData);
	/// 事件消息
	/// @param[in]		strHostId 设备ID
	/// @param[in]		respData 事件消息内容
	/// @return			参见j_errtype.h 
	j_result_t OnMessage(j_string_t strHostId, const CXlDataBusInfo &respData);
	
	/// 获取全局唯一的序列号
	/// @return			序列号
	j_uint32_t GetUniqueSeq() { return m_nUniqueSeq++; }

private:
	static void OnTimer(void *pUser)
	{
		(static_cast<CDataBus *>(pUser))->CheckState();
	}
	void CheckState();

private:
	J_OS::CTimer m_timer;
	J_OS::CTLock m_locker;

	volatile j_uint32_t m_nUniqueSeq;

	ObjectMap m_objectMap;
	MessageMap m_messageMap;
	RequestMap m_requestMap;
	ResponseMap m_responseMap;
	RequestTimeOutMap m_requestTimeOutMap;
};

JO_DECLARE_SINGLETON(DataBus)

#endif //~__DATA_BUS_H_