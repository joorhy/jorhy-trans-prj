#ifndef __XL_HOST_CMD_H__
#define __XL_HOST_CMD_H__
#include "XlProtocol.h"

#pragma pack(push)
#pragma pack(1)
struct CXlHostCmdData
{
	struct CmdConrrectTime
	{
		time_t systime;				///< 当前系统时间
	};

	struct CmdConfigDev
	{
		char szID[32];             	//设置ID
		char szVehicleID[32];       //车号
		ULONG ulPhoneNum;           //电话号码
		int nTotalChannels;         //通道个数
		int nTypeSize;				//摄像头类型
		int nNameSize;		  		//通道名称长度，以“#”隔开
	};

	struct CmdRealPlay
	{
		char szID[32];              //车辆ID
		long long llChnStatus; 		//通道开启状态  最大支持64个通道
	};

	struct CmdStartVod
	{
		GUID session;				//回放session
		char szID[32];				//车辆ID 
		long long llChnStatus; 		//通道开启状态  最大支持64个通道
		time_t tmStartTime;			//回放开始时间
		time_t tmEndTime;			//回放结束时间。若回放到现在，设置为-1
	};

	struct CmdStopVod
	{
		GUID session;				//回放session
		char szID[32];				//车辆ID
		long long llChnStatus; 		//通道开启/关闭状态  最大支持64个通道
	};

	struct CmdOnOffInfo
	{
		time_t tmStart;				//开始时间, 若取得所有的日志tmStart为-1
		time_t tmEnd;				//结束时间, 若取得所有的日志tmEnd为-1
	};

	struct CmdUpdateVodAck
	{
		unsigned char  code;		///< 0-成功, 1-失败
	};

	struct CmdContextInfo
	{
		struct ContextHeader
		{
			long lUserID;                             ///< 用户ID 
			int nMessageTitleSize;  				  ///< 消息标题长度
			unsigned long ulMessageSize;			  ///< 消息内容长度
			int nDevCount;							  ///< 设备个数
			char data[1];
		};

		char *pContext;								  ///< 消息内容
		union
		{
			ContextHeader header;
			char pData[1];
		};
	};

	struct CmdFileInfo
	{
		struct FileInfoHeader
		{
			long lUserID;                         ///< 用户ID 
			int nFileNameSize;  				  ///< 文件名称长度
			unsigned long ulFileSize;			  ///< 文件长度
			int nDevCount;						  ///< 设备个数
			char pData[1];
		};

		union
		{
			FileInfoHeader header;
			char pData[1];
		};
	};

	typedef struct CmdTalkCmd
	{
		char account[32];   	///< 账户名
		char equID[32];		///< 设备ID
		int state;

	};

	typedef struct CmdTalkData
	{
		char account[32];	 	///< 账户名
		char equID[32];		///< 设备ID
		unsigned int size;		///< 数据区长度
	};

	CXlProtocol::CmdHeader cmdHeader;
	union
	{
		CmdConrrectTime cmdConrrectTime;
		CmdConfigDev cmdConfigDev;
		CmdRealPlay cmdRealPlay;
		CmdStartVod cmdStartVod;
		CmdStopVod cmdStopVod;
		CmdOnOffInfo cmdOnOffInfo;
		CmdUpdateVodAck cmdUpdateVodAck;
		CmdTalkCmd cmdTalkCmd;
		CmdTalkData cmdTalkData;
		char pData[1];
	};
	CXlProtocol::CmdTail tail;
};

typedef std::vector<CXlHostCmdData::CmdContextInfo> HostContextVec;
typedef std::vector<CXlHostCmdData::CmdFileInfo> HostFileInfoVec;

#pragma pack(pop)
#endif //!__XL_HOST_CMD_H__