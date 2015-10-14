#ifndef __XL_HOST_RESP_DATA_H__
#define __XL_HOST_RESP_DATA_H__
#include "XlProtocol.h"

#pragma pack(push)
#pragma pack(1)
struct CXlHostRespData
{
	struct RespMessage
	{
		char szID[32];					///< 车号
		unsigned int unMsg;				///< 消息码
		unsigned long ulDataSize;		///< 消息数据长度
		char data[1];					///< 消息体
	};

	struct RespHostId
	{
		char hostId[32];				///< 主机ID
	};

	struct RespCorrectTime
	{
		unsigned char  code;			///< 0-成功, 1-失败
	};

	struct RespHostInfo
	{
		char hostId[32];			///< 设备ID
		char vehicleNum[8];			///< 列车号
		char phoneNum[12];			///< 电话号码
		char totalChannels;			///< 通道数目
		char mediaTypeNum;			///< 媒体类型
		int  chNameSize;			///< 通道名称长度
		char data[1];				///< 通道名称
	};

	typedef struct RespAlarmInfo
	{
		time_t tmTimeStamp;				///< 时间戳
		char bAlarm;					///< 报警类型
		double dLatitude;
		double dLongitude;
		double dGPSSpeed;
	};

	struct RespRealData
	{
		char hostId[32];				///< 设备ID
		unsigned char channel;			///< 通道号
		char data[1];
	};

	struct RespStopReal
	{
		unsigned char  code;			///< 0-成功, 1-失败
	};

	struct RespVodData
	{
		GUID sessionId;					///< 回话ID	
		char hostId[32];				///< 设备ID
		unsigned char channel;			///< 通道号
		char data[1];
	};

	struct RespStopVod
	{
		unsigned char  code;			///< 0-成功, 1-失败
	};

	struct RespConrrectTime
	{
		unsigned char  code;			///< 0-成功, 1-失败
	};

	struct RespOnOffInfo
	{
		time_t tmStart;					/// 开始时间
		time_t tmEnd;					/// 结束时间
	};

	struct RespVodInfo
	{
		time_t tmStart;					/// 开始时间
		time_t tmEnd;					/// 结束时间
	};

	typedef struct RespTalkCmd
	{
		char account[32];   	///< 账户名
		char equID[32];		///< 设备ID
		int state;

	};

	typedef struct RespTalkData
	{
		char account[32];	 	///< 账户名
		char equID[32];		///< 设备ID
		unsigned int size;		///< 数据区长度
	};

	CXlProtocol::CmdHeader respHeader;
	union
	{
		RespMessage respMessage;
		RespHostId respHostId;
		RespCorrectTime respConrrectTime;
		RespHostInfo respHostInfo;
		RespAlarmInfo respAlarmInfo;
		RespRealData respRealData;
		RespStopReal respStopReal;
		RespVodData respVodData;
		RespStopVod respStopVod;
		RespOnOffInfo respOnOffInfo;
		RespVodInfo respVodInfo;
		RespTalkCmd respTalkCmd;
		RespTalkData respTalkData;
		char pData[1];
	};
};
#pragma pack(pop)

#endif //!__XL_HOST_RESP_DATA_H__