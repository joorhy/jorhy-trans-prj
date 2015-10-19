///////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2009, xx科技有限公司(版权声明) 
/// All rights reserved. 
/// 
/// @file      XlHost.h 
/// @brief     XL_DVR设备模块
///
/// @version   1.0 (版本声明)
/// @author    Jorhy (joorhy@gmail.com) 
/// @date      2013/09/22 17:40 
///
///
/// 修订说明：最初版本
///////////////////////////////////////////////////////////////////////////  
#ifndef __XLHOST_H_
#define __XLHOST_H_
#include "j_includes.h"
#include "x_socket.h"
#include "XlDataBusDef.h"

/// 本类的功能:  XL设备接入类
class CXlHost : public J_Host
{
public:
	CXlHost(j_string_t strHostId, j_socket_t nSock);
	~CXlHost();

public:
	///J_Host
	virtual j_result_t CreateChannel(const j_int32_t nChannelNum, J_Obj *&pObj);
	virtual j_boolean_t IsReady();
	virtual j_result_t OnBroken();
	virtual j_result_t GetHostId(j_string_t &strDevId);
	virtual j_result_t OnHandleRead(J_AsioDataBase *pAsioData);
	virtual j_result_t OnHandleWrite(J_AsioDataBase *pAsioData);
	virtual j_result_t OnRequest(const CXlDataBusInfo &cmdData);

private:
	/// 发送文本消息
	j_result_t OnMessage(const CXlDataBusInfo &respData);
	/// 心跳消息
	j_result_t OnHeartBeat(const CXlDataBusInfo &respData);
	/// 服务端准备就绪
	j_result_t OnPrepare(const CXlDataBusInfo &respData);
	/// 报警消息(包括GPS)
	j_result_t OnAlarmInfo(const CXlDataBusInfo &respData);
	/// 实时视频数据
	j_result_t OnRealData(const CXlDataBusInfo *respData);
	/// 历史视频数据
	j_result_t OnVodData(const CXlDataBusInfo *respData);
	/// 设置时钟的反馈信息
	j_result_t OnConrrectTime(const CXlDataBusInfo &respData);
	/// 获取设备信息的反馈信息
	j_result_t OnHostInfo(const CXlDataBusInfo &respData);
	/// 获取日志的反馈信息
	j_result_t OnOnOffInfo(const CXlDataBusInfo &respData);
	/// 删除录像信息
	j_result_t OnDeleteVodInfo(const CXlDataBusInfo &respData);
	/// 更新录像信息
	j_result_t OnUpdateVodInfo(const CXlDataBusInfo &respData);
	/// 对讲数据
	j_result_t OnTalkBackCommand(const CXlDataBusInfo &respData);
	j_result_t OnTalkBackData(const CXlDataBusInfo &respData);
	/// 请求返回处理
	j_result_t OnResponse(const CXlDataBusInfo &respData);

private:
	/// 客户端发来的请求实时视频
	j_result_t StartRealPlay(const CXlDataBusInfo &cmdData);
	/// 客户端发来的停止实时视频
	j_result_t StopRealPlay(const CXlDataBusInfo &cmdData);
	/// 客户端发来的请求历史视频
	j_result_t StartVod(const CXlDataBusInfo &cmdData);
	/// 客户端发来的停止实时视频
	j_result_t StopVod(const CXlDataBusInfo &cmdData);
	/// 客户端发来的文本信息
	j_result_t SendContent(const CXlDataBusInfo &cmdData);
	/// 客户端发来的文件上传
	j_result_t SendFile(const CXlDataBusInfo &cmdData);
	/// 客户端发来的对讲数据
	j_result_t TalkBackCommand(const CXlDataBusInfo &cmdData);
	j_result_t TalkBackData(const CXlDataBusInfo &cmdData);

private:
	void GetChannel(j_uint64_t channel);

private:
	j_boolean_t m_bReady;
	j_socket_t m_socket;
	j_string_t m_hostId;
	j_int32_t m_lastBreatTime;

	j_char_t *m_readBuff;
	j_char_t *m_writeBuff;
	j_char_t *m_dataBuffer;

	j_int32_t m_ioState;							//设备端状态
	J_StreamHeader m_streamHeader;					//视频队列头信
	CRingBuffer m_ringBuffer;

	J_OS::TLocker_t m_vecClientLocker;
	std::vector<J_Client *> m_vecClient;

	std::vector<int> m_vecChannel;
	ChannelMap m_channelRealMap;

	std::map<int, int> m_channelVodMap;

	HostContextVec m_contextVec;
	HostFileInfoVec m_fileInfoVec;

	int m_nDownloadSize;		// for test
};

#endif