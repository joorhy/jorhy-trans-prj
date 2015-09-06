#ifndef __XL_CLIENT_RESP_DATA__
#define __XL_CLIENT_RESP_DATA__
#include "XlProtocol.h"

#pragma pack(push)
#pragma pack(1)
struct CXlClientRespData
{
	struct RespMessage
	{
		char szID[32];					///< 车号
		unsigned int unMsg;				///< 消息码
		unsigned long ulDataSize;		///< 消息数据长度
		char data[1];					///< 消息体
	};

	struct RespLogin
	{
		int code;				///< 0-登录成功, 1-登录失败, 2-用户名错误, 3-密码错误, 4-用户服务器用户满, 5-客户端版本过低或过高, 6-用户已在其他位置进行登录
	};

	struct RespLogout
	{
		unsigned char  code;	///< 0-成功, 1-失败
	};

	struct RespErrorCode
	{
		unsigned char  code;	///< 0-成功, 1-失败
	};

	typedef struct RespAlarmInfo
	{
		char szID[32];					///< 车号
		time_t tmTimeStamp;				///< 时间戳
		char bAlarm;					///< 报警类型
		double dLatitude;
		double dLongitude;
		double dGPSSpeed;
	};

	CXlProtocol::CmdHeader respHeader;
	union
	{
		RespMessage respMessage;
		RespLogin respLogin;
		RespLogout respLogout;
		RespErrorCode respErrorCode;
		RespAlarmInfo respAlarmInfo;
		char pData[1];
	};
};
#pragma pack(pop)

#endif //!__XL_CLIENT_RESP_DATA__