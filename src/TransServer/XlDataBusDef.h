#ifndef __XL_DATA_BUS_DEF__
#define __XL_DATA_BUS_DEF__
#include "XlProtocol.h"
#include "x_ringbuffer.h"

#pragma pack(push)
#pragma pack(1)
struct CXlDataBusInfo
{
/***************************************************************************************/
/************************for client request*********************************************/
	struct XlcCmdLogin
	{
		char userName[16];		///< 账户名
		char passWord[16];  	///< 密码
		int  nForced;			///< 1 强制登录；0 非强制登录
		long version;			///< 客户端版本
	};

	struct XlcCmdRealAlarm
	{
		char  hostId[32];		///< 设备ID
	};

	struct XlcCmdRealPlay
	{
		char hostId[32];		///< 设备ID
		long long channel;		///< 通道号
		CRingBuffer *pBuffer;
	};

	struct XlcCmdStartVod
	{
		GUID sessionId;			///< 回话ID
		char hostId[32];		///< 设备ID
		long long channel;		///< 通道号	
		time_t tmStartTime;		///< 开始时间
		time_t tmEndTime;		///< 结束时间
		CRingBuffer *pBuffer;
	};

	struct XlcCmdStopVod
	{
		GUID sessionId;			///< 回话ID
		char hostId[32];		///< 设备ID
		long long channel;		///< 通道号	
		CRingBuffer *pBuffer;
	};

	struct XlcCmdContextInfo
	{
		long lUserID;                             ///< 用户ID 
		int nMessageTitleSize;  				  ///< 消息标题长度
		unsigned long ulMessageSize;			  ///< 消息内容长度
		int nDevCount;							  ///< 设备个数
		char pData[1];
	};

	struct XlcCmdFileInfo
	{
		long lUserID;                         ///< 用户ID 
		int nFileNameSize;  				  ///< 文件名称长度
		unsigned long ulFileSize;			  ///< 文件长度
		int nDevCount;						  ///< 设备个数
		char pData[1];
	};

	struct XlcCmdTalkCmd
	{
		char account[32];   	///< 账户名
		char equID[32];			///< 设备ID
		int state;

	};

	struct XlcCmdTalkData
	{
		char account[32];	 	///< 账户名
		char equID[32];		///< 设备ID
		unsigned int size;		///< 数据区长度
	};

/***************************************************************************************/
/************************for client response********************************************/
	struct XlcRespMessage
	{
		char szID[32];					///< 车号
		unsigned int unMsg;				///< 消息码
		unsigned long ulDataSize;		///< 消息数据长度
		char data[1];					///< 消息体
	};

	struct XlcRespLogin
	{
		int code;				///< 0-登录成功, 1-登录失败, 2-用户名错误, 3-密码错误, 4-用户服务器用户满, 5-客户端版本过低或过高, 6-用户已在其他位置进行登录
	};

	struct XlcRespLogout
	{
		unsigned char  code;	///< 0-成功, 1-失败
	};

	struct XlcRespErrorCode
	{
		unsigned char  code;	///< 0-成功, 1-失败
	};

	struct XlcRespAlarmInfo
	{
		char szID[32];					///< 车号
		time_t tmTimeStamp;				///< 时间戳
		char bAlarm;					///< 报警类型
		double dLatitude;
		double dLongitude;
		double dGPSSpeed;
	};

	struct XlcRespTransmitMessage
	{
		unsigned long ulMessageID;    	/// 消息ID
		int state;  					/// 状态
	};

	struct XlcRespTransmitFile
	{
		unsigned long ulFileID;    		/// 文件ID
		int state;  					/// 状态
	};

	struct XlcRespEquipmentState
	{
		char szID[32];					/// 设备ID
		int  state;						/// 1在线；0离线
	};
	
	struct XlcRespTalkCmd
	{
		char account[32];   	///< 账户名
		char equID[32];		///< 设备ID
		int state;

	};

	struct XlcRespTalkData
	{
		char account[32];	 	///< 账户名
		char equID[32];		///< 设备ID
		unsigned int size;		///< 数据区长度
	};

/***************************************************************************************/
/************************for device request*********************************************/
	struct XldCmdConrrectTime
	{
		time_t systime;				///< ??C?I?I3E??a
	};

	struct XldCmdConfigDev
	{
		char szID[32];             	//EeOAID
		char szVehicleID[32];       //3?oA
		ULONG ulPhoneNum;           //?c>?oAAe
		int nTotalChannels;         //I??A,oEu
		int nTypeSize;				//EaInI.AaDI
		int nNameSize;		  		//I??AAu3A3$?E???O!?#!?,o?a
	};

	struct XldCmdRealPlay
	{
		char szID[32];              //3?A?ID
		long long llChnStatus; 		//I??A?aAo??`?  ?i?oO?3O64,oI??A
	};

	struct XldCmdStartVod
	{
		GUID session;				//>O.Asession
		char szID[32];				//3?A?ID 
		long long llChnStatus; 		//I??A?aAo??`?  ?i?oO?3O64,oI??A
		time_t tmStartTime;			//>O.A?aE?E??a
		time_t tmEndTime;			//>O.A?aEoE??a!?Eo>O.A??IOOU??EeOAIa-1
	};

	struct XldCmdStopVod
	{
		GUID session;				//>O.Asession
		char szID[32];				//3?A?ID
		long long llChnStatus; 		//I??A?aAo/1O?O??`?  ?i?oO?3O64,oI??A
	};

	struct XldCmdOnOffInfo
	{
		time_t tmStart;				//?aE?E??a, EoE!?AEuOD?AEOO?tmStartIa-1
		time_t tmEnd;				//?aEoE??a, EoE!?AEuOD?AEOO?tmEndIa-1
	};

	struct XldCmdUpdateVodAck
	{
		unsigned char  code;		///< 0-3E1|, 1-E??U
	};

	struct XldCmdContextInfo
	{
		struct XldContextHeader
		{
			long lUserID;                             ///< OA>?ID 
			long lMessageID;                          ///< 消息ID 
			int nMessageTitleSize;  				  ///< IuIc?eIa3$?E
			unsigned long ulMessageSize;			  ///< IuIcAUEY3$?E
			char data[1];
		};

		char *pContext;								  ///< IuIcAUEY
		union
		{
			XldContextHeader header;
			char pData[1];
		};
	};

	struct XldCmdFileInfo
	{
		struct XldFileInfoHeader
		{
			long lUserID;                         ///< 用户ID 
			int nFileID;                          ///< 文件ID 
			int nFileNameSize;  			      ///< 文件名称长度
			unsigned long ulFileSize;			  ///< 文件总长
			char pData[1];
		};

		char *pFileName;
		union
		{
			XldFileInfoHeader header;
			char pData[1];
		};
	};

	struct XldCmdTalkCmd
	{
		char account[32];   	///< 账户名
		char equID[32];		///< 设备ID
		int state;

	};

	struct XldCmdTalkData
	{
		char account[32];	 	///< 账户名
		char equID[32];		///< 设备ID
		unsigned int size;		///< 数据区长度
	};
/***************************************************************************************/
/************************for device request*********************************************/
	struct XldRespMessage
	{
		char szID[32];					///< 车号
		unsigned int unMsg;				///< 消息码
		unsigned long ulDataSize;		///< 消息数据长度
		char data[1];					///< 消息体
	};

	struct XldRespHostId
	{
		char hostId[32];				///< 主机ID
	};

	struct XldRespCorrectTime
	{
		unsigned char  code;			///< 0-成功, 1-失败
	};

	struct XldRespHostInfo
	{
		char hostId[32];			///< 设备ID
		char vehicleNum[8];			///< 列车号
		char phoneNum[12];			///< 电话号码
		char totalChannels;			///< 通道数目
		char mediaTypeNum;			///< 媒体类型
		int  chNameSize;			///< 通道名称长度
		char data[1];				///< 通道名称
	};

	typedef struct XldRespAlarmInfo
	{
		time_t tmTimeStamp;				///< 时间戳
		char bAlarm;					///< 报警类型
		double dLatitude;
		double dLongitude;
		double dGPSSpeed;
	};

	struct XldRespRealData
	{
		char hostId[32];				///< 设备ID
		unsigned char channel;			///< 通道号
		char data[1];
	};

	struct XldRespStopReal
	{
		unsigned char  code;			///< 0-成功, 1-失败
	};

	struct XldRespVodData
	{
		GUID sessionId;					///< 回话ID	
		char hostId[32];				///< 设备ID
		unsigned char channel;			///< 通道号
		char data[1];
	};

	struct XldRespStopVod
	{
		unsigned char  code;			///< 0-成功, 1-失败
	};

	struct XldRespConrrectTime
	{
		unsigned char  code;			///< 0-成功, 1-失败
	};

	struct XldRespOnOffInfo
	{
		time_t tmStart;					/// 开始时间
		time_t tmEnd;					/// 结束时间
	};

	struct XldRespVodInfo
	{
		time_t tmStart;					/// 开始时间
		time_t tmEnd;					/// 结束时间
	};
	struct XldRespSyncVodInfo
	{
		time_t tmFristItem;				/// 第一条数据时间
	};

	typedef struct XldRespTalkCmd
	{
		char account[32];   	///< 账户名
		char equID[32];			///< 设备ID
		int state;

	};

	typedef struct XldRespTalkData
	{
		char account[32];	 	///< 账户名
		char equID[32];			///< 设备ID
		unsigned int size;		///< 数据区长度
	};

	CXlProtocol::CmdHeader header;
	union
	{
		/// for client request
		XlcCmdLogin xlcCmdLogin;
		XlcCmdRealAlarm xlcCmdRealAlarm;
		XlcCmdRealPlay xlcCmdRealPlay;
		XlcCmdStartVod xlcCmdStartVod;
		XlcCmdStopVod xlcCmdStopVod;
		XlcCmdContextInfo xlcCmdContextInfo;
		XlcCmdFileInfo xlcCmdFileInfo;
		XlcCmdTalkCmd xlcCmdTalkCmd;
		XlcCmdTalkData xlcCmdTalkData;
		/// for client response
		XlcRespMessage xlcRespMessage;
		XlcRespLogin xlcRespLogin;
		XlcRespLogout xlcRespLogout;
		XlcRespErrorCode xlcRespErrorCode;
		XlcRespAlarmInfo xlcRespAlarmInfo;
		XlcRespTransmitMessage xlcRespTransmitMessage;
		XlcRespTransmitFile xlcRespTransmitFile;
		XlcRespEquipmentState xlcRespEquipmentState;
		XlcRespTalkCmd xlcRespTalkCmd;
		XlcRespTalkData xlcRespTalkData;
		/// for device request
		XldCmdConrrectTime xldCmdConrrectTime;
		XldCmdConfigDev xldCmdConfigDev;
		XldCmdRealPlay xldCmdRealPlay;
		XldCmdStartVod xldCmdStartVod;
		XldCmdStopVod xldCmdStopVod;
		XldCmdOnOffInfo xldCmdOnOffInfo;
		XldCmdUpdateVodAck xldCmdUpdateVodAck;
		XldCmdTalkCmd xldCmdTalkCmd;
		XldCmdTalkData xldCmdTalkData;
		/// for device response
		XldRespMessage xldRespMessage;
		XldRespHostId xldRespHostId;
		XldRespCorrectTime xldRespConrrectTime;
		XldRespHostInfo xldRespHostInfo;
		XldRespAlarmInfo xldRespAlarmInfo;
		XldRespRealData xldRespRealData;
		XldRespStopReal xldRespStopReal;
		XldRespVodData xldRespVodData;
		XldRespStopVod xldRespStopVod;
		XldRespOnOffInfo xldRespOnOffInfo;
		XldRespVodInfo xldRespVodInfo;
		XldRespSyncVodInfo xldRespSyncVodInfo;
		XldRespTalkCmd xldRespTalkCmd;
		XldRespTalkData xldRespTalkData;
		/// for data
		char pData[1];
	};
};
#pragma pack(pop)

typedef std::vector<CXlDataBusInfo::XldCmdContextInfo *> HostContextVec;
typedef std::vector<CXlDataBusInfo::XldCmdFileInfo *> HostFileInfoVec;

#endif //!__XL_DATA_BUS_DEF__