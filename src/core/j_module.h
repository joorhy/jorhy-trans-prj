#ifndef __J_VIDEOADAPTER_H_
#define __J_VIDEOADAPTER_H_

#include "j_obj.h"
#include "j_type.h"
#include "x_ringbuffer.h"
#include "x_log.h"
#include "x_errtype.h"
#include "x_lock.h"

struct J_DObj : virtual public J_Obj
{
public:
	virtual ~J_DObj() {}

	/// 格式转换函数
	virtual j_result_t convert() = 0;
	virtual j_result_t reverse() = 0;

	/// 内部检测函数
	virtual j_boolean_t check() { return true; }
};

struct J_AsioUser : virtual public J_Obj
{
	///连接事件完成(用于网络IO)
	///@param[in]	asioData IO数据集
	///@param[in]	错误码,见x_error_type.h	
	virtual j_result_t OnAccept(J_AsioDataBase *pAsioData, j_result_t nRet) = 0;

	///读事件完成
	///@param[in]	asioData IO数据集
	///@param[in]	错误码,见x_error_type.h
	virtual j_result_t OnRead(J_AsioDataBase *pAsioData, j_result_t nRet) = 0;

	///写事件完成
	///@param[in]	asioData IO数据集
	///@param[in]	错误码,见x_error_type.h
	virtual j_result_t OnWrite(J_AsioDataBase *pAsioData, j_result_t nRet) = 0;

	///断线事件完成(用于网络IO)
	///@param[in]	错误码,见x_error_type.h
	virtual j_result_t OnBroken(J_AsioDataBase *pAsioData, j_result_t nRet) = 0;
};
struct AsioDataInfo { J_AsioDataBase *pAsioReadData;  J_AsioDataBase *pAsioWriteData; };
typedef std::map<j_socket_t, AsioDataInfo> J_AsioDataMap;

struct J_DbAccess : virtual public J_Obj
{
	///连接数据库
	///@param[in]		pAddr IP地址
	///@param[in]		nPort 端口号
	///@param[in]		pUa 用户名
	///@param[in]		pPwd 密码
	///@return			参见x_errtype.h
	virtual j_result_t Connect(const j_char_t *pAddr, j_int16_t nPort, const j_char_t *pUa, const j_char_t *pPwd) = 0;
	///初始化数据库
	///@return			参见x_errtype.h
	virtual j_result_t Init() = 0;
	///释放数据库连接
	///@return			参见x_errtype.h
	virtual j_result_t Release() = 0;
};

struct CXlClientCmdData;
struct J_Channel : virtual public J_Obj 
{
	///打开实时视频
	///@param[in]		pRingBuffer 流队列对象
	///@return			参见x_errtype.h
	virtual j_result_t OpenStream(const CXlClientCmdData &cmdData) = 0;
	///关闭实时视频
	///@param[in]		pRingBuffer 流队列对象
	///@return			参见x_errtype.h
	virtual j_result_t CloseStream(const CXlClientCmdData &cmdData) = 0;
};

struct J_Vod : virtual public J_Obj 
{
	///打开历史视频
	///@param[in]		sessionId 回话ID
	///@param[in]		pRingBuffer 流队列对象
	///@return			参见x_errtype.h
	virtual j_result_t OpenVod(const CXlClientCmdData &cmdData) = 0;
	///关闭历史视频
	///@param[in]		sessionId 回话ID
	///@param[in]		pRingBuffer 流队列对象
	///@return			参见x_errtype.h
	virtual j_result_t CloseVod(const CXlClientCmdData &cmdData) = 0;
};


/// 通道信息
struct ChannelInfo
{
	J_Obj *pChannel;
	j_int32_t nRef;
};
typedef std::map<j_int32_t, ChannelInfo> ChannelMap;

struct J_Client;
struct J_Host : virtual public J_Obj 
{
	///获取通道对象
	///@param[in]		nChannelNum 通道号 
	///@param[out]	pObj 通道对象
	///@return			参见x_errtype.h
	virtual j_result_t CreateChannel(const j_int32_t nChannelNum, J_Obj *&pObj) = 0;

	///释放通道对象
	///@param[in]		nChannelNum 通道号 
	///@return			参见x_errtype.h
	virtual j_result_t ReleaseChannel(const j_int32_t nChannelNum)
	{
		ChannelMap::iterator it = m_channelMap.find(nChannelNum);
		if (it != m_channelMap.end())
		{
			delete it->second.pChannel;
			m_channelMap.erase(it);
		}

		return J_OK;
	}
	///判断设备是否就绪
	///@return			true-就绪,false-未就绪
	virtual j_boolean_t IsReady() = 0;
	///获取设备ID
	///@param[out]	strDevId 设备ID
	///@return			参见x_errtype.h
	virtual j_result_t GetHostId(j_string_t &strDevId) = 0;
	///用户请求及回复
	///@param[in][out]	pAsioData异步数据	
	///@return				参见x_errtype.h
	virtual j_result_t OnHandleRead(J_AsioDataBase *pAsioData) = 0;
	///用户请求及回复
	///@param[in][out]	pAsioData异步数据	
	///@return				参见x_errtype.h
	virtual j_result_t OnHandleWrite(J_AsioDataBase *pAsioData) = 0;
	///发送消息
	///@param[in]			pData 数据
	///@param[in]			nLen 数据长度
	///@return				参见x_errtype.h
	virtual j_result_t OnRequest(const CXlClientCmdData &cmdData) = 0;
	/// 设备断线
	///@return			参见x_errtype.h
	virtual j_result_t OnBroken() = 0;

protected:
	ChannelMap m_channelMap;
	J_OS::TLocker_t m_channelLocker;
};

struct CXlClientRespData;
struct J_Client : virtual public J_Obj 
{
	/// 获取状态
	/// @return				客户端状态 
	virtual j_boolean_t IsReady() = 0;
	/// 用户请求及回复
	/// @param[in][out]	pAsioData异步数据	
	/// @return				参见j_errtype.h 
	virtual j_result_t OnHandleRead(J_AsioDataBase *pAsioData) = 0;
	/// 用户请求及回复
	/// @param[in][out]	pAsioData异步数据	
	/// @return				参见j_errtype.h 
	virtual j_result_t OnHandleWrite(J_AsioDataBase *pAsioData) = 0;
	/// 异步调用返回
	/// @param[in]			pData异步调用返回数据	
	/// @return				参见j_errtype.h 
	virtual j_result_t OnResponse(const CXlClientRespData &respData) = 0;
	/// 消息
	/// @param[in]			pData异步调用返回数据	
	/// @return				参见j_errtype.h 
	virtual j_result_t OnMessage(j_string_t strHostId, const CXlClientRespData &respData) = 0;
	/// 用户断线
	/// @return				参见j_errtype.h 
	virtual j_result_t OnBroken() = 0;
};

struct J_Service : virtual public J_Obj
{
	/// 启动服务
	/// @param[in]  strIniPath 服务端口号
	/// @return		  参见j_errtype.h 
	virtual j_result_t StartServer(const j_string_t &strIniPath) = 0;
	/// 停止服务
	/// @return		  参见j_errtype.h  
	virtual j_result_t StopServer() = 0;
};

#endif //~__J_VIDEOADAPTER_H_
