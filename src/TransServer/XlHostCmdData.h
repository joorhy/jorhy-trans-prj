#ifndef __XL_HOST_CMD_H__
#define __XL_HOST_CMD_H__
#include "XlProtocol.h"

#pragma pack(push)
#pragma pack(1)
struct CXlHostCmdData
{
	struct CmdConrrectTime
	{
		time_t systime;				///< ??C?I?I3E??a
	};

	struct CmdConfigDev
	{
		char szID[32];             	//EeOAID
		char szVehicleID[32];       //3?oA
		ULONG ulPhoneNum;           //?c>?oAAe
		int nTotalChannels;         //I??A,oEu
		int nTypeSize;				//EaInI.AaDI
		int nNameSize;		  		//I??AAu3A3$?E???O!?#!?,o?a
	};

	struct CmdRealPlay
	{
		char szID[32];              //3?A?ID
		long long llChnStatus; 		//I??A?aAo??`?  ?i?oO?3O64,oI??A
	};

	struct CmdStartVod
	{
		GUID session;				//>O.Asession
		char szID[32];				//3?A?ID 
		long long llChnStatus; 		//I??A?aAo??`?  ?i?oO?3O64,oI??A
		time_t tmStartTime;			//>O.A?aE?E??a
		time_t tmEndTime;			//>O.A?aEoE??a!?Eo>O.A??IOOU??EeOAIa-1
	};

	struct CmdStopVod
	{
		GUID session;				//>O.Asession
		char szID[32];				//3?A?ID
		long long llChnStatus; 		//I??A?aAo/1O?O??`?  ?i?oO?3O64,oI??A
	};

	struct CmdOnOffInfo
	{
		time_t tmStart;				//?aE?E??a, EoE!?AEuOD?AEOO?tmStartIa-1
		time_t tmEnd;				//?aEoE??a, EoE!?AEuOD?AEOO?tmEndIa-1
	};

	struct CmdUpdateVodAck
	{
		unsigned char  code;		///< 0-3E1|, 1-E??U
	};

	struct CmdContextInfo
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

	struct CmdFileInfo
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
		char pData[1];
	};
	CXlProtocol::CmdTail tail;
};

typedef std::vector<CXlHostCmdData::CmdContextInfo *> HostContextVec;
typedef std::vector<CXlHostCmdData::CmdFileInfo *> HostFileInfoVec;

#pragma pack(pop)
#endif //!__XL_HOST_CMD_H__