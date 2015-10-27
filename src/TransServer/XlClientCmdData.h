#ifndef __XL_CLIENT_CMD_DATA__
#define __XL_CLIENT_CMD_DATA__
#include "XlProtocol.h"

#pragma pack(push)
#pragma pack(1)
struct CXlClientCmdData
{
	struct CmdLogin
	{
		char userName[16];		///< 账户名
		char passWord[16];  	///< 密码
		int  nForced;			///< 1 强制登录；0 非强制登录
		long version;			///< 客户端版本
	};

	struct CmdRealAlarm
	{
		char  hostId[32];		///< 设备ID
	};

	struct CmdRealPlay
	{
		char hostId[32];		///< 设备ID
		long long channel;		///< 通道号
		CRingBuffer *pBuffer;
	};

	typedef struct CmdStartVod
	{
		GUID sessionId;			///< 回话ID
		char hostId[32];		///< 设备ID
		long long channel;		///< 通道号	
		time_t tmStartTime;		///< 开始时间
		time_t tmEndTime;		///< 结束时间
		CRingBuffer *pBuffer;
	};

	typedef struct CmdStopVod
	{
		GUID sessionId;			///< 回话ID
		char hostId[32];		///< 设备ID
		long long channel;		///< 通道号	
		CRingBuffer *pBuffer;
	};

	typedef struct CmdContextInfo
	{
		long lUserID;                             ///< 用户ID 
		int nMessageTitleSize;  				  ///< 消息标题长度
		unsigned long ulMessageSize;			  ///< 消息内容长度
		int nDevCount;							  ///< 设备个数
		char pData[1];
	};

	typedef struct CmdFileInfo
	{
		long lUserID;                         ///< 用户ID 
		int nFileNameSize;  				  ///< 文件名称长度
		unsigned long ulFileSize;			  ///< 文件长度
		int nDevCount;						  ///< 设备个数
		char pData[1];
	};

	CXlProtocol::CmdHeader cmdHeader;
	union
	{
		CmdLogin cmdLogin;
		CmdRealAlarm cmdRealAlarm;
		CmdRealPlay cmdRealPlay;
		CmdStartVod cmdStartVod;
		CmdStopVod cmdStopVod;
		CmdContextInfo cmdContextInfo;
		CmdFileInfo cmdFileInfo;
		char pData[1];
	};
};
#pragma pack(pop)

#endif //!__XL_CLIENT_CMD_DATA__