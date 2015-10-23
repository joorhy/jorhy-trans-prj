#ifndef __XL_DATA_BUS_DEF__
#define __XL_DATA_BUS_DEF__
#include "XlProtocol.h"
#include "x_ringbuffer.h"

#pragma pack(push)
#pragma pack(1)

namespace XlClientRequest
{
	struct Login
	{
		char userName[16];		///< 账户名
		char passWord[16];  	///< 密码
		int  nForced;			///< 1 强制登录；0 非强制登录
		long version;			///< 客户端版本
	};

	struct RealAlarm
	{
		char  hostId[32];		///< 设备ID
	};

	struct RealPlay
	{
		char hostId[32];		///< 设备ID
		long long channel;		///< 通道号
		CRingBuffer *pBuffer;
	};

	struct StartVod
	{
		GUID sessionId;			///< 回话ID
		char hostId[32];		///< 设备ID
		long long channel;		///< 通道号	
		time_t tmStartTime;		///< 开始时间
		time_t tmEndTime;		///< 结束时间
		CRingBuffer *pBuffer;
	};

	struct StopVod
	{
		GUID sessionId;			///< 回话ID
		char hostId[32];		///< 设备ID
		long long channel;		///< 通道号	
		CRingBuffer *pBuffer;
	};

	struct ContextInfo
	{
		long lUserID;                             ///< 用户ID 
		int nMessageTitleSize;  				  ///< 消息标题长度
		unsigned long ulMessageSize;			  ///< 消息内容长度
		int nDevCount;							  ///< 设备个数
		char pData[1];
	};

	struct FileInfo
	{
		long lUserID;                         ///< 用户ID 
		int nFileNameSize;  				  ///< 文件名称长度
		unsigned long ulFileSize;			  ///< 文件长度
		int nDevCount;						  ///< 设备个数
		char pData[1];
	};

	struct TalkCmd
	{
		char account[32];   	///< 账户名
		char equID[32];			///< 设备ID
		int state;

	};

	struct TalkData
	{
		char account[32];	 	///< 账户名
		char equID[32];		///< 设备ID
		unsigned int size;		///< 数据区长度
	};
}

namespace XlClientResponse
{
	struct Message
	{
		char szID[32];					///< 车号
		unsigned int unMsg;				///< 消息码
		unsigned long ulDataSize;		///< 消息数据长度
		char data[1];					///< 消息体
	};

	struct Login
	{
		int code;				///< 0-登录成功, 1-登录失败, 2-用户名错误, 3-密码错误, 4-用户服务器用户满, 5-客户端版本过低或过高, 6-用户已在其他位置进行登录
	};

	struct Logout
	{
		unsigned char  code;	///< 0-成功, 1-失败
	};

	struct ErrorCode
	{
		unsigned char  code;	///< 0-成功, 1-失败
	};

	struct AlarmInfo
	{
		char szID[32];					///< 车号
		time_t tmTimeStamp;				///< 时间戳
		char bAlarm;					///< 报警类型
		double dLatitude;
		double dLongitude;
		double dGPSSpeed;
	};

	struct TransmitMessage
	{
		unsigned long ulMessageID;    	/// 消息ID
		int state;  					/// 状态
	};

	struct TransmitFile
	{
		unsigned long ulFileID;    		/// 文件ID
		int state;  					/// 状态
	};

	struct EquipmentState
	{
		char szID[32];					/// 设备ID
		int  state;						/// 1在线；0离线
	};

	struct TalkCmd
	{
		char account[32];   	///< 账户名
		char equID[32];		///< 设备ID
		int state;

	};

	struct TalkData
	{
		char account[32];	 	///< 账户名
		char equID[32];			///< 设备ID
		unsigned int size;		///< 数据区长度
	};
}

namespace XlHostRequest
{
	struct ConrrectTime
	{
		time_t systime;				///< ??C?I?I3E??a
	};

	struct ConfigDev
	{
		char szID[32];             	//EeOAID
		char szVehicleID[32];       //3?oA
		ULONG ulPhoneNum;           //?c>?oAAe
		int nTotalChannels;         //I??A,oEu
		int nTypeSize;				//EaInI.AaDI
		int nNameSize;		  		//I??AAu3A3$?E???O!?#!?,o?a
	};

	struct RealPlay
	{
		char szID[32];              //3?A?ID
		long long llChnStatus; 		//I??A?aAo??`?  ?i?oO?3O64,oI??A
	};

	struct StartVod
	{
		GUID session;				//>O.Asession
		char szID[32];				//3?A?ID 
		long long llChnStatus; 		//I??A?aAo??`?  ?i?oO?3O64,oI??A
		time_t tmStartTime;			//>O.A?aE?E??a
		time_t tmEndTime;			//>O.A?aEoE??a!?Eo>O.A??IOOU??EeOAIa-1
	};

	struct StopVod
	{
		GUID session;				//>O.Asession
		char szID[32];				//3?A?ID
		long long llChnStatus; 		//I??A?aAo/1O?O??`?  ?i?oO?3O64,oI??A
	};

	struct OnOffInfo
	{
		time_t tmStart;				//?aE?E??a, EoE!?AEuOD?AEOO?tmStartIa-1
		time_t tmEnd;				//?aEoE??a, EoE!?AEuOD?AEOO?tmEndIa-1
	};

	struct UpdateVodAck
	{
		unsigned char  code;		///< 0-3E1|, 1-E??U
	};

	struct ContextInfo
	{
		struct ContextHeader
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
			ContextHeader header;
			char pData[1];
		};
	};

	struct FileInfo
	{
		struct FileInfoHeader
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
			FileInfoHeader header;
			char pData[1];
		};
	};

	struct TalkCmd
	{
		char account[32];   	///< 账户名
		char equID[32];		///< 设备ID
		int state;

	};

	struct TalkData
	{
		char account[32];	 	///< 账户名
		char equID[32];		///< 设备ID
		unsigned int size;		///< 数据区长度
	};
}

namespace XlHostResponse
{
	struct Message
	{
		char szID[32];					///< 车号
		unsigned int unMsg;				///< 消息码
		unsigned long ulDataSize;		///< 消息数据长度
		char data[1];					///< 消息体
	};

	struct HostId
	{
		char hostId[32];				///< 主机ID
	};

	struct CorrectTime
	{
		unsigned char  code;			///< 0-成功, 1-失败
	};

	struct HostInfo
	{
		char hostId[32];			///< 设备ID
		char vehicleNum[8];			///< 列车号
		char phoneNum[12];			///< 电话号码
		char totalChannels;			///< 通道数目
		char mediaTypeNum;			///< 媒体类型
		int  chNameSize;			///< 通道名称长度
		char data[1];				///< 通道名称
	};

	struct AlarmInfo
	{
		time_t tmTimeStamp;				///< 时间戳
		char bAlarm;					///< 报警类型
		double dLatitude;
		double dLongitude;
		double dGPSSpeed;
	};

	struct RealData
	{
		char hostId[32];				///< 设备ID
		unsigned char channel;			///< 通道号
		char data[1];
	};

	struct StopReal
	{
		unsigned char  code;			///< 0-成功, 1-失败
	};

	struct VodData
	{
		GUID sessionId;					///< 回话ID	
		char hostId[32];				///< 设备ID
		unsigned char channel;			///< 通道号
		char data[1];
	};

	struct StopVod
	{
		unsigned char  code;			///< 0-成功, 1-失败
	};

	struct ConrrectTime
	{
		unsigned char  code;			///< 0-成功, 1-失败
	};

	struct OnOffInfo
	{
		time_t tmStart;					/// 开始时间
		time_t tmEnd;					/// 结束时间
	};

	struct VodInfo
	{
		time_t tmStart;					/// 开始时间
		time_t tmEnd;					/// 结束时间
	};
	struct SyncVodInfo
	{
		time_t tmFristItem;				/// 第一条数据时间
	};

	struct TalkCmd
	{
		char account[32];   	///< 账户名
		char equID[32];			///< 设备ID
		int state;

	};

	struct TalkData
	{
		char account[32];	 	///< 账户名
		char equID[32];			///< 设备ID
		unsigned int size;		///< 数据区长度
	};
}

struct CXlDataBusInfo
{

	CXlProtocol::CmdHeader header;
	union
	{
		struct {
			XlClientRequest::Login login;
			XlClientRequest::RealAlarm realAlarm;
			XlClientRequest::RealPlay realPlay;
			XlClientRequest::StartVod startVod;
			XlClientRequest::StopVod stopVod;
			XlClientRequest::ContextInfo contextInfo;
			XlClientRequest::FileInfo fileInfo;
			XlClientRequest::TalkCmd talkCmd;
			XlClientRequest::TalkData talkData;
		} clientRequest;

		struct {
			XlClientResponse::Message message;
			XlClientResponse::Login login;
			XlClientResponse::Logout logout;
			XlClientResponse::ErrorCode errorCode;
			XlClientResponse::AlarmInfo alarmInfo;
			XlClientResponse::TransmitMessage transmitMessage;
			XlClientResponse::TransmitFile transmitFile;
			XlClientResponse::EquipmentState equipmentState;
			XlClientResponse::TalkCmd talkCmd;
			XlClientResponse::TalkData talkData;
		} clientResponse;

		struct {
			XlHostRequest::ConrrectTime conrrectTime;
			XlHostRequest::ConfigDev configDev;
			XlHostRequest::RealPlay realPlay;
			XlHostRequest::StartVod startVod;
			XlHostRequest::StopVod stopVod;
			XlHostRequest::OnOffInfo onOffInfo;
			XlHostRequest::UpdateVodAck updateVodAck;
			XlHostRequest::TalkCmd talkCmd;
			XlHostRequest::TalkData talkData;
		} hostRequest;

		struct {
			XlHostResponse::Message message;
			XlHostResponse::HostId hostId;
			XlHostResponse::ConrrectTime conrrectTime;
			XlHostResponse::HostInfo hostInfo;
			XlHostResponse::AlarmInfo alarmInfo;
			XlHostResponse::RealData realData;
			XlHostResponse::StopReal stopReal;
			XlHostResponse::VodData vodData;
			XlHostResponse::StopVod stopVod;
			XlHostResponse::OnOffInfo onOffInfo;
			XlHostResponse::VodInfo vodInfo;
			XlHostResponse::SyncVodInfo syncVodInfo;
			XlHostResponse::TalkCmd talkCmd;
			XlHostResponse::TalkData talkData;
		} hostResponse;
		char pData[1];
	};
};
#pragma pack(pop)

typedef std::vector<XlHostRequest::ContextInfo *> HostContextVec;
typedef std::vector<XlHostRequest::FileInfo *> HostFileInfoVec;

#endif //!__XL_DATA_BUS_DEF__