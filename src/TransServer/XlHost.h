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
#include "XlHostRespData.h"
#include "XlHostCmdData.h"
#include "XlClientCmdData.h"
/// 本类的功能:  XL设备接入类
class CXlHost : public J_Host
{
	struct GUIDKey
	{
		GUIDKey(GUID session)
		{
			memcpy(&_session, &session, sizeof(GUID));
		}
		bool operator < (const GUIDKey &other) const
		{
			if (other._session.Data1 == _session.Data1)
			{
				if (_session.Data2 == other._session.Data2)
				{
					if (_session.Data3 == other._session.Data3)
					{
						return ((((UINT64)_session.Data4) < ((UINT64)other._session.Data4)));
					}
					return _session.Data3 < other._session.Data3;
				}
				return _session.Data2 < other._session.Data2;
			}
			return _session.Data1 < other._session.Data1;
		}
		GUID _session;
	};

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
	virtual j_result_t OnRequest(const CXlClientCmdData &cmdData);

private:
	/// 发送文本消息
	j_result_t OnMessage(const CXlHostRespData &respData);
	/// 心跳消息
	j_result_t OnHeartBeat(const CXlHostRespData &respData);
	/// 服务端准备就绪
	j_result_t OnPrepare(const CXlHostRespData &respData);
	/// 报警消息(包括GPS)
	j_result_t OnAlarmInfo(const CXlHostRespData &respData);
	/// 实时视频数据
	j_result_t OnRealData(const CXlHostRespData *respData);
	/// 历史视频数据
	j_result_t OnVodData(const CXlHostRespData *respData);
	/// 设置时钟的反馈信息
	j_result_t OnConrrectTime(const CXlHostRespData &respData);
	/// 获取设备信息的反馈信息
	j_result_t OnHostInfo(const CXlHostRespData &respData);
	/// 获取日志的反馈信息
	j_result_t OnOnOffInfo(const CXlHostRespData &respData);
	/// 更新录像信息
	j_result_t OnUpdateVodInfo(const CXlHostRespData &respData);
	/// 对讲数据
	j_result_t OnTalkBackCommand(const CXlHostRespData &respData);
	j_result_t OnTalkBackData(const CXlHostRespData &respData);
	/// 请求返回处理
	j_result_t OnResponse(const CXlHostRespData &respData);

private:
	/// 客户端发来的请求实时视频
	j_result_t StartRealPlay(const CXlClientCmdData &cmdData);
	/// 客户端发来的停止实时视频
	j_result_t StopRealPlay(const CXlClientCmdData &cmdData);
	/// 客户端发来的请求历史视频
	j_result_t StartVod(const CXlClientCmdData &cmdData);
	/// 客户端发来的停止实时视频
	j_result_t StopVod(const CXlClientCmdData &cmdData);
	/// 客户端发来的文本信息
	j_result_t SendContent(const CXlClientCmdData &cmdData);
	/// 客户端发来的文件上传
	j_result_t SendFile(const CXlClientCmdData &cmdData);
	/// 客户端发来的对讲数据
	j_result_t TalkBackCommand(const CXlClientCmdData &cmdData);
	j_result_t TalkBackData(const CXlClientCmdData &cmdData);

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

	std::map<GUIDKey, int> m_channelVodMap;

	HostContextVec m_contextVec;
	HostFileInfoVec m_fileInfoVec;
};

#endif