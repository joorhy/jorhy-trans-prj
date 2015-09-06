#ifndef _NET_DEVICE_DEF_H_
#define _NET_DEVICE_DEF_H_


#include "hi_dataType.h"

#define GPS_FRM_TYPE_HEAD						1
#define GPS_FRM_TYPE_I							0x63643000
#define GPS_FRM_TYPE_P							0x63643100
#define GPS_FRM_TYPE_A							0x63643230
#define GPS_FRM_TYPE_INFO						0x63643939

#define GPS_DOWN_TYPE_OFFSET					0
#define GPS_DOWN_TYPE_TIME						1

#define GPS_FILE_TYPE_NORMAL					0
#define GPS_FILE_TYPE_ALARM						1
#define GPS_FILE_TYPE_ALL						-1

#define GPS_CHANNEL_ALL							99

#define GPS_ALARM_TYPE_ALL						0		//所有报警

#define GPS_ALARM_TYPE_USEDEFINE				1		//自定义报警
#define GPS_ALARM_TYPE_URGENCY_BUTTON			2		//紧急按钮报警
#define GPS_ALARM_TYPE_SHAKE					3		//振动报警
#define GPS_ALARM_TYPE_VIDEO_LOST				4		//摄像头无信号报警
#define GPS_ALARM_TYPE_VIDEO_MASK				5		//摄像头遮挡报警
#define GPS_ALARM_TYPE_DOOR_OPEN_LAWLESS		6		//非法开门报警
#define GPS_ALARM_TYPE_WRONG_PWD				7		//三次密码错误报警
#define GPS_ALARM_TYPE_FIRE_LOWLESS				8		//非法点火报警
#define GPS_ALARM_TYPE_TEMPERATOR				9		//温度报警
#define GPS_ALARM_TYPE_DISK_ERROR				10		//硬盘错误报警
#define GPS_ALARM_TYPE_OVERSPEED				11		//超速报警
#define GPS_ALARM_TYPE_BEYOND_BOUNDS			12		//越界报警
#define GPS_ALARM_TYPE_DOOR_ABNORMAL			13		//异常开关车门报警
#define GPS_ALARM_TYPE_PARK_TOO_LONG			14		//停车过长报警
#define GPS_ALARM_TYPE_MOTION					15		//移动侦测报警
#define GPS_ALARM_TYPE_ACC_ON					16		//ACC开启报警
#define GPS_ALARM_TYPE_DEV_ONLINE				17		//设备在线
#define GPS_ALARM_TYPE_GPS_SIGNAL_LOSS          18      //GPS讯号丢失开始
#define GPS_ALARM_TYPE_IO_1                     19      //IO_1报警
#define GPS_ALARM_TYPE_IO_2                     20      //IO_2报警
#define GPS_ALARM_TYPE_IO_3                     21      //IO_3报警
#define GPS_ALARM_TYPE_IO_4                     22      //IO_4报警
#define GPS_ALARM_TYPE_IO_5                     23      //IO_5报警
#define GPS_ALARM_TYPE_IO_6                     24      //IO_6报警
#define GPS_ALARM_TYPE_IO_7                     25		//IO_7报警
#define GPS_ALARM_TYPE_IO_8                     26      //IO_8报警
#define GPS_ALARM_TYPE_IN_FENCE		            27      //进入区域报警
#define GPS_ALARM_TYPE_OUT_FENCE		        28      //出区域报警
#define GPS_ALARM_TYPE_IN_FENCE_OVER_SPEED      29      //区域内高速报警
#define GPS_ALARM_TYPE_OUT_FENCE_OVER_SPEED     30      //区域外高速报警
#define GPS_ALARM_TYPE_IN_FENCE_LOW_SPEED       31      //区域内低速报警
#define GPS_ALARM_TYPE_OUT_FENCE_LOW_SPEED      32      //区域外低速报警
#define GPS_ALARM_TYPE_IN_FENCE_STOP			33      //区域内停车报警
#define GPS_ALARM_TYPE_OUT_FENCE_STOP			34      //区域外停车报警
#define GPS_ALARM_TYPE_FIRE                     35      //火警
#define GPS_ALARM_TYPE_PANIC                    36      //劫警
#define GPS_ALARM_TYPE_TASK_FINISHED			37		//调度任务完成
#define GPS_ALARM_TYPE_IMAGE_UPLOAD				38		//图片上传完成
#define GPS_ALARM_TYPE_DISK1_NO_EXIST			39		//硬盘1不存在
#define GPS_ALARM_TYPE_DISK2_NO_EXIST			40		//硬盘2不存在
#define GPS_ALARM_TYPE_GPS_UNENABLE				45		//GPS无效
#define GPS_ALARM_TYPE_REFUEL					46		//加油	AlarmInfo为加油的油量(9999=99.99升)，Param[0]为加油前油耗
#define GPS_ALARM_TYPE_STILL_FUEL				47		//偷油	AlarmInfo为偷油的油量(9999=99.99升)，Param[0]为偷油前油耗


#define GPS_ALARM_TYPE_END_USEDEFINE			51		//自定义报警
#define GPS_ALARM_TYPE_END_URGENCY_BUTTON		52		//紧急按钮报警
#define GPS_ALARM_TYPE_END_SHAKE				53		//振动报警
#define GPS_ALARM_TYPE_END_VIDEO_LOST			54		//摄像头无信号报警
#define GPS_ALARM_TYPE_END_VIDEO_MASK			55		//摄像头遮挡报警
#define GPS_ALARM_TYPE_END_DOOR_OPEN_LAWLESS	56		//非法开门报警
#define GPS_ALARM_TYPE_END_WRONG_PWD			57		//三次密码错误报警
#define GPS_ALARM_TYPE_END_FIRE_LOWLESS			58		//非法点火报警
#define GPS_ALARM_TYPE_END_TEMPERATOR			59		//温度报警
#define GPS_ALARM_TYPE_END_DISK_ERROR			60		//硬盘错误报警
#define GPS_ALARM_TYPE_END_OVERSPEED			61		//超速报警
#define GPS_ALARM_TYPE_END_BEYOND_BOUNDS		62		//越界报警
#define GPS_ALARM_TYPE_END_DOOR_ABNORMAL		63		//异常开关车门报警
#define GPS_ALARM_TYPE_END_PARK_TOO_LONG		64		//停车过长报警
#define GPS_ALARM_TYPE_END_MOTION				65		//移动侦测报警
#define GPS_ALARM_TYPE_ACC_OFF					66		//ACC关闭报警
#define GPS_ALARM_TYPE_DEV_DISONLINE			67		//设备断线
#define GPS_ALARM_TYPE_END_GPS_SIGNAL_LOSS      68      //GPS讯号丢失结束
#define GPS_ALARM_TYPE_END_IO_1                 69      //IO_1报警
#define GPS_ALARM_TYPE_END_IO_2                 70      //IO_2报警
#define GPS_ALARM_TYPE_END_IO_3                 71      //IO_3报警
#define GPS_ALARM_TYPE_END_IO_4                 72      //IO_4报警
#define GPS_ALARM_TYPE_END_IO_5                 73      //IO_5报警
#define GPS_ALARM_TYPE_END_IO_6                 74      //IO_6报警
#define GPS_ALARM_TYPE_END_IO_7                 75      //IO_7报警
#define GPS_ALARM_TYPE_END_IO_8                 76      //IO_8报警
#define GPS_ALARM_TYPE_END_IN_FENCE		            77      //进入区域报警
#define GPS_ALARM_TYPE_END_OUT_FENCE		        78      //出区域报警
#define GPS_ALARM_TYPE_END_IN_FENCE_OVER_SPEED      79      //区域内高速报警
#define GPS_ALARM_TYPE_END_OUT_FENCE_OVER_SPEED     80      //区域外高速报警
#define GPS_ALARM_TYPE_END_IN_FENCE_LOW_SPEED       81      //区域内低速报警
#define GPS_ALARM_TYPE_END_OUT_FENCE_LOW_SPEED		82      //区域外低速报警
#define GPS_ALARM_TYPE_END_IN_FENCE_STOP			83      //区域内停车报警
#define GPS_ALARM_TYPE_END_OUT_FENCE_STOP			84      //区域外停车报警
#define GPS_ALARM_TYPE_END_GPS_UNENABLE				85		//GPS无效

//加油和偷油没有结束
#define GPS_ALARM_TYPE_END_REFUEL					86		//加油	AlarmInfo为加油的油量(9999=99.99升)，Param[0]为加油前油耗
#define GPS_ALARM_TYPE_END_STILL_FUEL				87		//偷油	AlarmInfo为偷油的油量(9999=99.99升)，Param[0]为偷油前油耗



//#define GPS_ALARM_TYPE_END_FIRE                 85      //火警	未使用
//#define GPS_ALARM_TYPE_END_PANIC                86      //劫警  未使用
//#define GPS_ALARM_TYPE_END_TASK_FINISHED		87		//调度任务完成	未使用
//#define GPS_ALARM_TYPE_END_IMAGE_UPLOAD			88		//图片上传完成	未使用

#define GPS_EVENT_TYPE_PARK						101		//停车事件			Param[0]为停车秒数，Param[1]为停车前油耗(9999=99.99升)，Param[2]为停车后油耗
#define GPS_EVENT_TYPE_PARK_ACCON				102		//停车未熄火事件	Param[0]为停车秒数，Param[1]为停车前油耗(9999=99.99升)，Param[2]为停车后油耗
														//停车未熄火事件一般处于停车事件里面的一段时间内，11:00 - 11:20处于停车，11:00 - 11:05 处于停车未熄火
#define GPS_EVENT_TYPE_NET_FLOW					103		//流量	Param[0]为当前时间（单位秒，如：7206 = 当天02:06时 ），Param[1]为上行流量，Param[2]为下行流量
#define GPS_EVENT_TYPE_REFUEL					104		//加油	AlarmInfo为加油的油量(9999=99.99升)，Param[0]为加油前油耗
#define GPS_EVENT_TYPE_STILL_FUEL				105		//偷油	AlarmInfo为偷油的油量(9999=99.99升)，Param[0]为偷油前油耗
#define GPS_EVENT_TYPE_OVERSPEED				106		//超速事件	AlarmInfo为速度(999=99.9KM/H)，Param[0]超速的时间，Param[1]为超速类型（超高速或者超低速、暂时无效）
#define GPS_EVENT_TYPE_FENCE_ACCESS				107		//进出区域事件	 Param[0]区域编号，Param[1]出区域经度，Param[2]出区域纬度，Param[3]区域停留时间(秒）
#define GPS_EVENT_TYPE_FENCE_PARK				108		//区域停车事件	 Param[0]区域编号，Param[3]区域停车时间（秒）

#define GPS_ALARM_TYPE_TALK_BACK_REQ			127		//车辆主动对讲请求，以报警方式发送
#define GPS_ALARM_TYPE_DEVICE_INFO_CHANGE		128		//车辆信息发生变化，如用户修改设备信息
#define GPS_ALARM_TYPE_SNAPSHOT_FINISH			129		//存储服务器抓拍完成，以报警方式发送给客户端

#define GPS_CNT_MSG_SUCCESS				0
#define GPS_CNT_MSG_FAILED				-1
#define GPS_CNT_MSG_DISCONNECT			-2
#define GPS_CNT_MSG_FINISHED			-3
#define GPS_CNT_MSG_USR_FULL_ERROR		-4
#define GPS_CNT_MSG_USR_ERROR			-5

#define GPS_NOTIFY_TYPE_AUDIO_COM		1

#define GPS_AUDIO_TYPE_HEAD				1
#define GPS_AUDIO_TYPE_DATA				2

#define GPS_SETUP_MODE_SEND_ONLY		1	//只发送指令，不等待反馈
#define GPS_SETUP_MODE_WAIT_RET			2	//发送指令，并等待反馈
#define GPS_SETUP_MODE_WAIT_ASYN		3	//发送指令，上层检查返回

#define GPS_GPS_VALID					1

#define GPS_DEV_TYPE_MDVR				1 //车载终端
#define GPS_DEV_TYPE_MOBILE				2 //手机终端
#define GPS_DEV_TYPE_DVR				3 //普通DVR

#define GPS_MOBILE_TYPE_ANDROID			1 //手机终端-Android
#define GPS_MOBILE_TYPE_IPHONE			2 //手机终端-Iphone

#define GPS_MDVR_TYPE_HI3512_4			1 //车载终端-3512
#define GPS_MDVR_TYPE_HI3515_4			2 //车载终端-3515-4
#define GPS_MDVR_TYPE_HI3515_8			3 //车载终端-3515-8


#define GPS_MOBILE_USER_POST_TEAM_MEMBER	1	//队员
#define GPS_MOBILE_USER_POST_TEAM_LEADER	2	//队长

#define GPS_MOBILE_USER_SEX_MAN				1	//男
#define GPS_MOBLIE_USER_SEX_WOMAN			2	//女

#define GPS_PTZ_MOVE_LEFT				0
#define GPS_PTZ_MOVE_RIGHT				1
#define GPS_PTZ_MOVE_TOP				2
#define GPS_PTZ_MOVE_BOTTOM				3
#define GPS_PTZ_MOVE_LEFT_TOP			4
#define GPS_PTZ_MOVE_RIGHT_TOP			5
#define GPS_PTZ_MOVE_LEFT_BOTTOM		6
#define GPS_PTZ_MOVE_RIGHT_BOTTOM		7

#define GPS_PTZ_FOCUS_DEL				8
#define GPS_PTZ_FOCUS_ADD				9
#define GPS_PTZ_LIGHT_DEL				10
#define GPS_PTZ_LIGHT_ADD				11
#define GPS_PTZ_ZOOM_DEL				12
#define GPS_PTZ_ZOOM_ADD				13
#define GPS_PTZ_LIGHT_OPEN				14
#define GPS_PTZ_LIGHT_CLOSE				15
#define GPS_PTZ_WIPER_OPEN				16
#define GPS_PTZ_WIPER_CLOSE				17
#define GPS_PTZ_CRUISE					18
#define GPS_PTZ_MOVE_STOP				19

#define GPS_PTZ_PRESET_MOVE				21
#define GPS_PTZ_PRESET_SET				22
#define GPS_PTZ_PRESET_DEL				23

#define GPS_PTZ_SPEED_MIN				0
#define GPS_PTZ_SPEED_MAX				255

#define GPS_GPSINTERVAL_TYPE_DISTANCE	1	//按距离上报
#define GPS_GPSINTERVAL_TYPE_TIME		2	//按时间上报

//客户端查询车辆轨迹的选项
#define GPS_QUERY_TRACK_TYPE_GPS		1  //查询GPS数据
#define GPS_QUERY_TRACK_TYPE_GPSARLMR	2  //查询GPS和报警
#define GPS_QUERY_TRACK_TYPE_ALARM		3  //查询报警数据

//返回
#define GPS_TRACK_DATA_TYPE_GPS			1  //GPS数据
#define GPS_TRACK_DATA_TYPE_ALARM		2  //报警数据

//网络类型
#define GPS_NETWOKR_TYPE_3G				0	//3G类型
#define GPS_NETWOKR_TYPE_WIFI			1	//WIFI类型

//磁盘类型
#define GPS_DISK_TYPE_UNKOWN			0	//SD卡
#define GPS_DISK_TYPE_SD				1	//SD卡
#define GPS_DISK_TYPE_HDD				2	//硬盘
#define GPS_DISK_TYPE_SSD				3	//SSD

//协议类型
#define MDVR_PROTOCOL_TYPE_WKP			1	//WKP协议
#define MDVR_PROTOCOL_TYPE_TTX			2	//通天星协议
#define MDVR_PROTOCOL_TYPE_TQ			3	//天琴协议
#define MDVR_PROTOCOL_TYPE_HANV			4	//HANV部标协议
#define MDVR_PROTOCOL_TYPE_GOOME		5	//谷米协议（兼容泰比特）
#define MDVR_PROTOCOL_TYPE_808			6	//808部标协议
#define MDVR_PROTOCOL_TYPE_RM			7	//RM部标协议
#define MDVR_PROTOCOL_TYPE_YD			8	//YD协议


#pragma pack(4)

typedef struct _tagGPSDeviceInfo
{
	char sVersion[64];///
	char sSerialNumber[32];  		
	char szDevIDNO[32];			///车辆编号  		
	short sDevType;				///设备类型
	char cAlarmInPortNum;		
	char cAlarmOutPortNum;
	char cDiskNum;///硬盘数目	
	char cChanNum;///通道数目
	char cProtocol;				//协议类型
	unsigned char cAudioCodec:5;//音频解码器类型
	unsigned char cDiskType:3;	//0：SD，1：硬盘，2：SSD
	char cPlateNumber[34];		//车牌号
	char cPlateUnicode;			//0表示非unicode，1表示unicode
	char cReserve;
}GPSDeviceInfo_S, *LPGPSDeviceInfo_S;

typedef struct _tagGPSDeviceIDNO
{
	char szDevIDNO[32];
}GPSDeviceIDNO_S, *LPGPSDeviceIDNO_S;

typedef struct _tagGPSSvrAddr
{
	char	IPAddr[80];
	unsigned short usPort;
	unsigned short Reserve;
}GPSSvrAddr_S;

typedef struct _tagGPSRecFile
{
	char	szFile[256]; 	/*带路径的文件名*/
	unsigned int uiBegintime;
	unsigned int uiEndtime;
	int		nChn;
	unsigned int nFileLen;
	int		nRecType;
}GPSRecFile_S, *LPGPSRecFile_S;

//时间
typedef struct _tagGPSTime
{
	unsigned int ucYear:6;		//年(2000+ucYear) 范围(0-64)
	unsigned int ucMonth:4;		//月(1-12)	范围(0-16)
	unsigned int ucDay:5;		//日(1-31)  范围(0-32)
	unsigned int ucHour:5;		//时(0-23)	范围(0-32)
	unsigned int ucMinute:6;	//分(0-59)  范围(0-64)
	unsigned int ucSecond:6;	//秒(0-59)  范围(0-64)
}GPSTime_S;

//车辆状态,每位表示车辆的相关状态
//uiStatus[0]共有32位
//0位表示GPS定位状态		0无效1有效
//1位表示ACC状态		0表示ACC关闭1表示ACC开启
//2位表示左转状态		0无效1左转
//3位表示右转状态		0无效1右转
//4位表示刹车状态		0无效1刹车
//5位表示正转状态		0无效1正转
//6位表示反转状态		0无效1反转
//7位表示GPS天线状态		0不存在1存在
//8,9位为表示硬盘状态		0不存在，1存在，2断电
//10,11,12位表示3G模块状态  0模块不存在，1无信号，2信号差，3信号一般，4信号好，5信号优
//13位表示静止状态		1表示静止
//14位表示超速状态		1表示超速
//15位表示补传状态		1表示GPS补传
//16位未使用
//17位表示本日流量已经受限		1表示受限
//18位表示本月流量已经超过90%警界	1表示报警
//19位表示本月流量已经用完		1表示用完
//关于停车未熄火，如果处理静止状态，并且处于ACC开启状态，则表示停车未熄火
//20位表示IO1状态	1表示报警
//21位表示IO2状态	1表示报警
//22位表示IO3状态	1表示报警
//23位表示IO4状态	1表示报警
//24位表示IO5状态	1表示报警
//25位表示IO6状态	1表示报警
//26位表示IO7状态	1表示报警
//27位表示IO8状态	1表示报警
//28位表示盘符2状态	1表示有效
//29、30位表示，硬盘2的状态		0不存在，1存在，2断电
//31未使用
//uiStatus[1]
//0位表示进区域报警	
//1位表示出区域报警	

//2位表示区域内高速报警	
//3位表示区域内低速报警	

//4位表示区域外高速报警
//5位表示区域外低速报警	

//6位表示区域内停车报警	
//7位表示区域外停车报警	
	
//8位表示日流量预警
//9位表示日流量超过
//10位表示月流量预警
//11位表示月流量超过

//12位--主机掉电由后备电池供电
//13位--车门开
//14位--车辆设防
//15位--电池电压过低
//16位--电池坏
//17位--发动机

typedef struct _tagGPSVehicleGps
{
	GPSTime_S Time;
	unsigned int uiSpeed:14;	//速度值 KM/S	9999 = 999.9 	范围(0-16384)
	unsigned int uiYouLiang:18;	//油量   升	9999 = 99.99 	范围(0-262100)
	unsigned int uiLiCheng;		//里程   公里	9999 = 9.999 	
	unsigned int uiHangXiang:9;	//地面航向，(0-360) 范围(0-512)
	unsigned int uiMapType:3;	//地图类型		1表示GOOGLE，2表示百度，0表示无效
	unsigned int uiReserve:20;  //保留位
	unsigned int uiStatus[4];		//目前只使用，每1个状态	0位表示GPS定位状态（0无效，1有效）
	short sTempSensor[4];	//温度传感器的状态
	int	nJingDu;		//经度	4字节 整数部分	9999999 = 9.999999 
	int nWeiDu;			//纬度	4字节 整数部分	9999999 = 9.999999
	int	nGaoDu;			//高度，暂时未使用
	int	nParkTime;		//停车时长 秒	在静止状态时有效
	int	nMapJingDu;		//通过地图模块获取的经度和纬度
	int	nMapWeiDu;		//通过地图模块获取的经度和纬度
	int	nReserve[2];		//保留参数
}GPSVehicleGps_S;

typedef struct _GPSVehicleState
{
	char	szDevIDNO[32];
	GPSVehicleGps_S	Gps;
}GPSVehicleState_S;

typedef struct _GPSVehicleStatus
{
	char	szDevIDNO[32];
	BOOL	bOnline;
	int		nNetworkType;
	char	szNetworkName[32];
}GPSVehicleStatus_S;

typedef struct _tagGPSAlarmInfo
{
	char	guid[36];			//标识唯一的报警信息
	GPSVehicleGps_S Gps;		//报警时候的GPS信息
	GPSTime_S Time;				//报警时间
	short 	AlarmType;			//报警类型
	short 	AlarmInfo;			//报警信息
	int	Param[4];				//报警参数
	char szDesc[256];			//报警描述
	//说明srcAlarmType和srcTime，当报警类型为报警图片上传完成时有效，表示对应的报警参数信息，图片上传完成报警类型
	//报警类型为图片上传报警时，只使用AlarmType和szImgFile参数，其它参数表示对应报警参数
	short srcAlarmType;			//源报警类型
	short sReserve;				//保留参数
	GPSTime_S srcTime;			//源报警时间
	char szImgFile[256];		//图片文件信息，用;分隔，为http地址路径，可以为多个图片信息
	char szReserve[64];			//保留字段
}GPSAlarmInfo_S, *LPGPSAlarmInfo_S;

typedef struct _GPSVehicleAlarm
{
	char	szDevIDNO[32];
	GPSAlarmInfo_S	Alarm;
}GPSVehicleAlarm_S;

//车辆轨迹(包含GPS和报警)
typedef struct _tagGPSVehicleTrack
{
	int nType;					//标识该数据类型(GPS数据或者报警数据)
	GPSVehicleAlarm_S Alarm;	//报警数据(如果类型为GPS数据 则只有GPS字段数据有效)
}GPSVehicleTrack_S, *LPGPSVehicleTrack_S;

const int GPS_VEHICLE_ALARM_LENGTH = sizeof(GPSVehicleAlarm_S);
const int GPS_ALARM_INFO_LENGTH = sizeof(GPSAlarmInfo_S);
const int GPS_VHIECLE_GPS_LENGTH = sizeof(GPSVehicleGps_S);
const int GPS_VHIECLE_RUN_LENGTH = sizeof(GPSVehicleState_S);
const int GPS_VEHICLE_STATUS_LENGTH = sizeof(GPSVehicleStatus_S);
const int GPS_VEHICLE_TRACK_LENGTH = sizeof(GPSVehicleTrack_S);

#define GPS_WLAN_TYPE_2G					0
#define GPS_WLAN_TYPE_3G_EVDO				1
#define GPS_WLAN_TYPE_3G_WCDMA				2

typedef struct _tagGPSWLanStatus
{
	int nWLanActive;	//
	int	nWLanType;		////0：表示2G;1：表示3G-EVDO; 2：表示3G-WCDMA 
	int nWLanQuantity;	//信号值	不显示
	char szWLanAddr[32];//网络地址
}GPSWLanStatus_S, *LPGPSWLanStatus_S;

typedef struct _tagGPSWifiStatus
{
	int nWifiActive;
	char szWifiAP[32];	//hide
	int nWifiQuantity;	//hide
	char szWifiAddr[32];//
}GPSWifiStatus_S, *LPGPSWifiStatus_S;

typedef struct _tagGPSDiskInfo
{
	int nAllVolume;		//15923	表示159.23	G //总容量
	int nLeftVolume;	//同上  ///剩余容量
}GPSDiskInfo_S, *LPGPSDiskInfo_S;

const int GPS_DISK_INFO_COUNT			= 8;	//最多硬盘数目为8个

typedef struct _tagGPSDeviceStatus
{
	GPSDeviceInfo_S	DevInfo;	//设备信息
	GPSWLanStatus_S	WLanInfo;	//3G状态
	GPSWifiStatus_S	WifiInfo;	//WIFI信息
	GPSDiskInfo_S	DiskInfo[GPS_DISK_INFO_COUNT];	//硬盘信息
	int	nVideoLost;		//视频丢失状态  1表示该视频丢失
	int nRecord;		//录像状态 0：没有录像，1录像
	int nVideoTran;		//视频传输数目
	int nReserve[4];	//保留参数
}GPSDeviceStatus_S, *LPGPSDeviceStatus_S;

const int GPS_MOTION_MASK = 9;

typedef struct _tagGPSMotionParam
{
	int	nMask[GPS_MOTION_MASK];	//低11位表示移动侦测
	BOOL bEnable;	//是否启用
	int	nSensitive;	//灵敏度
}GPSMotionParam_S, *LPGPSMotionParam_S;



//////////////////////////////////////////////////////////////////////////
//modiyf by hw 2012-11-07
//网络流量参数
// typedef struct _tagGPSNetFlowParam
// {
// 	BOOL bMonitor;	//是否启用流量限制
// 	int	nTotalFlow;	//套餐限额	单位MB
// 	int	nMonthDay;	//月结日
// 	int nDayFlow;	//每日限额	单位MB（本日流量已经超限）
// 	BOOL bAlertTotal;	//是否进行月流量提醒
// 	int	nAlertRate;	//月流量提醒(90%)
// 
// 	//hw20121107
// 	BOOL nAlertTotalDay;	//是否进行日流量提醒
// 	int nAlertRateDay;	//日流量提醒（90%）
// 	char szRes[32];	//保留32
// }GPSNetFlowParam_S, *LPGPSNetFlowParam_S;

typedef struct _tagGPSNetFlowParam
{
	int nIsOpenFlowCount;		//流量统计开关 0-关闭，1-开启
	int nIsOpenDayFlowRemind;	//日流量提醒 0-不提醒 1-提醒
	int nIsOpenMonthFlwRemind;	//月流量提醒 0-不提醒 1-提醒
	float fDayLimit;			//日流量限制(MB)
	float fMonthLimit;			//月流量限制(MB)
	int nDayRemind;				//每天流量提醒（百分比），20%即20
	int nMonthRemind;			//每月流量提醒（百分比），20%即20
	int nMonthTotleDay;			//月结日
	char szRes[32];				//保留32
}GPSNetFlowParam_S, *LPGPSNetFlowParam_S;

//网络流量统计
typedef struct _tagGPSNetFlowStatistics
{
	GPSNetFlowParam_S	FlowParam;	//流量参数
	float fFlowUsedToday;			//今日流量
	float fFlowUsedMonth;			//本月已使用流量
	int nStatisticsTime;			//统计时间
	char szRes[32];					//保留32
}GPSNetFlowStatistics_S, *LPGPSNetFlowStatistics_S;	
//end
//////////////////////////////////////////////////////////////////////////

//调度指令
typedef struct _tabGPSDispatchCommand
{
	char szGuid[40];		//指令GUID
	int nMapType;			//目的地，地图类型，1表示GOOGLE，2表示百度
	int nJingDu;			//经度
	int nWeiDu;				//纬度
	char szCommand[256];	//命令字
}GPSDispatchCommand_S, *LPGPSDispatchCommand_S;


typedef union _tagUNCfg
{
	HI_S_Video cfgVideo;				/* 视频参数 */
	HI_S_Audio cfgAudio;				/* 音频参数 */
	HI_E_AudioInput cfgAudioInput;		/* 音频输入参数 */
	HI_S_PTZ cfgPTZ;					/* 云台参数 */
}GPSCfg, *LPGPSCfg;

#pragma pack()

#endif
