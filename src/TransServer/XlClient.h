///////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2009, xx科技有限公司(版权声明) 
/// All rights reserved. 
/// 
/// @file      Client.h 
/// @brief     客户端模块
///
/// @version   1.0 (版本声明)
/// @author    Jorhy (joorhy@gmail.com) 
/// @date      2013/09/24 15:11 
///
///
/// 修订说明：最初版本
///////////////////////////////////////////////////////////////////////////  
#ifndef __XL_CLIENT_H_
#define __XL_CLIENT_H_

#include "j_includes.h"
#include "x_socket.h"
#include "x_timer.h"
#include "XlDataBusDef.h"
/// 本类的功能:  客户端业务处理类
class CXlClient : public J_Client
{
	typedef std::vector<CXlDataBusInfo> RequestVec;
	struct VehicleStatusEnable
	{
		j_boolean_t vehStatus;
		j_boolean_t alarmInfo;
	};
	typedef std::map<j_string_t, VehicleStatusEnable> VehicleEnableMap;
public:
	CXlClient(j_socket_t nSock);
	~CXlClient();

public:
	///J_Client
	j_boolean_t IsReady();
	j_result_t OnHandleRead(J_AsioDataBase *pAsioData);
	j_result_t OnHandleWrite(J_AsioDataBase *pAsioData);
	j_result_t OnRequest(const CXlDataBusInfo &cmdData);
	j_result_t OnResponse(const CXlDataBusInfo &respData);
	j_result_t OnMessage(j_string_t strHostId, const CXlDataBusInfo &respData);
	j_result_t OnBroken();

private:
	/// 请求处理函数
	j_result_t OnLogin(const CXlDataBusInfo &cmdData);
	j_result_t OnLogout(const CXlDataBusInfo &cmdData);
	j_result_t OnHeartBeat(const CXlDataBusInfo &cmdData);
	j_result_t SendRequest(const CXlDataBusInfo &cmdData);
	j_result_t OnTalkBackCommand(const CXlDataBusInfo &cmdData);
	j_result_t OnTalkBackData(const CXlDataBusInfo &cmdData);
	/// 订阅消息函数
	j_result_t SaveRequest(const CXlDataBusInfo &cmdData, j_boolean_t bSave);
	j_result_t EnableVehStatus(const CXlDataBusInfo &cmdData, j_boolean_t bEnable);
	j_result_t Recovery();
	/// 数据传输
	j_result_t SaveContext(const CXlDataBusInfo &cmdData);
	j_result_t SaveFiles(const CXlDataBusInfo &cmdData);
	/// 对讲
	j_result_t TalkBackCommand(const CXlDataBusInfo &cmdData);
	j_result_t TalkBackData(const CXlDataBusInfo &cmdData);
	/// 回执消息
	j_result_t MessageBack(const CXlDataBusInfo &cmdData);

private:
	j_char_t m_userName[32];						//用户名
	j_char_t *m_readBuff;							//命令请求缓存区
	j_char_t *m_writeBuff;							//命令发送缓存区
	j_char_t *m_dataBuff;							//数据缓存区

	j_int32_t m_ioCmdState;							//命令请求状态
	j_int32_t m_ioDataState;						//视频发送状态
	CRingBuffer m_ringBuffer;						//视频流队列
	J_StreamHeader m_streamHeader;					//视频队列头信
	j_long_t m_nRefCnt;								//视频流引用计数

	j_int32_t m_state;								//客户端状态
	j_int32_t m_lastBreatTime;

	J_OS::CTLock m_locker;
	RequestVec m_requestVec;
	VehicleEnableMap m_vehEnableMap;

	/// 发送消息
	long m_lUserID;
	j_string_t m_strTitle;
	j_string_t m_strContext;
	FILE *m_pFile;									// 联络文件写文件对象
	std::vector<j_string_t> m_transTargetMap;

	int m_nFileTotleSize;
};
#endif // ~__XL_CLIENT_H_
