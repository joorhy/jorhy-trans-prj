#ifndef _NET_CLIENT_API_H_
#define _NET_CLIENT_API_H_ 

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the LIBNETCLIENT_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// NETCLIENT_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef LIBNETCLIENT_EXPORTS
#define NETCLIENT_API __declspec(dllexport)
#else
#define NETCLIENT_API __declspec(dllimport)
#endif

#define  API_CALL	WINAPI

#include "GPSDeviceDef.h"
#include "GPSDataDef.h"
#include "GPSErrorDef.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(4)

typedef struct _tagGPSTimeEx
{
	short Year;
	char  Month;
	char  Day;
	char  Hour;
	char  Minute;
	char  Second;
	char  Reserve;
}GPSTimeEx, *LPGPSTimeEx;		//hw

typedef struct _tagGPSRECFile
{
	GPSTimeEx	DateBeg;
	GPSTimeEx	DateEnd;
	char		szFile[MAX_PATH];
}GPSRECFile_S, *LPGPSRECFile_S;

typedef struct _tagGPSFile
{
	char	szFile[256]; 	/*带路径的文件名*/
	int		nYear;
	int		nMonth;
	int		nDay;
	unsigned int uiBegintime;
	unsigned int uiEndtime;
	char	szDevIDNO[32];			//设备ID
	int		nChn;
	unsigned int nFileLen;
	int		nFileType;
	int		nLocation;		//位置，设备上的录像文件，还是存储服务上的录像文件
	int		nSvrID;			//存储服务器ID，在为存储服务器上的文件时有效
}GPSFile_S, *LPIGPSFile_S;

typedef struct _tagGPSMCMsg
{
	int nMsgType;
	int nResult;
	void* pParam[6];
	void* pMngCmdClass;
}GPSMCMsg_S, *LPGPSMCMsg_S;

typedef void (CALLBACK * FUNDownDataCB)(int nType, void* pData, void * pUsr);

#pragma pack()

NETCLIENT_API int	API_CALL	NETCLIENT_Initialize();
NETCLIENT_API void	API_CALL	NETCLIENT_UnInitialize();
//配置重连次数，范围1-10，如果下层与服务器通信中断后，会进行重连
//如日志查询，轨迹搜索等
NETCLIENT_API void	API_CALL	NETCLIENT_SetRecntCount(int nCount);

//hw 2012/1/19
//连接服务器的网络类型 0-使用局域网IP(GPS_NET_TYPE_LAN)， 1-使用广域网IP(GPS_NET_TYPE_WAN)
NETCLIENT_API int	API_CALL	NETCLIENT_GetNetType();

//=============================客户端登录====================================//
NETCLIENT_API int  API_CALL		NETCLIENT_RegLoginMsg(void* pUsr, void (CALLBACK * FUNLoginMsgCB)(int nMsg, void * pUsr));
NETCLIENT_API int	API_CALL	NETCLIENT_LoginDirSvr(const char* pSvrIP, unsigned short usPort, const char* szCompany, const char* szUsr, const char* szPwd, int nType);
NETCLIENT_API int	API_CALL	NETCLIENT_LogoutDirSvr();

//=============================获取服务器的客户端和WEB前台端口信息====================================//
NETCLIENT_API int	API_CALL	NETCLIENT_DOWNOpenAppInfo(long* lpHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_DOWNRegAppInfoCB(long lHandle, void* pUsr, void (CALLBACK * FUNDownAppInfoCB)(int nType, void* pData, void * pUsr));
NETCLIENT_API int	API_CALL	NETCLIENT_DOWNStartAppInfo(long lHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_DOWNStopAppInfo(long lHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_DOWNCloseAppInfo(long lHandle);

//=============================获取web前台端口====================================//
NETCLIENT_API int	API_CALL	NETCLIENT_DEVOpenWebClientInfoDown(long* lpHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_DEVRegWebClientInfoDownCB(long lHandle, void* pUsr, void (CALLBACK * FUNDownWebClientInfoCB)(int nType, void* pData, void * pUsr));
NETCLIENT_API int	API_CALL	NETCLIENT_DEVStartWebClientInfoDown(long lHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_DEVStopWebClientInfoDown(long lHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_DEVCloseWebClientInfoDown(long lHandle);

//=============================设备管理====================================//
//NETCLIENT_API int	API_CALL	NETCLIENT_DEVDownDevList(int nMgrType, int nDevType, void* pUsr, void (CALLBACK * FUNDownDevListCB)(int nType, void* pData, void * pUsr), int nCompany = -1);
NETCLIENT_API int	API_CALL	NETCLIENT_DEVDeviceChangeCB(void* pUsr, void (CALLBACK * FUNDeviceChangeCB)(int nReserve, void* pUsr));

//=============================设备信息下载，使用线程方式====================================//
NETCLIENT_API int	API_CALL	NETCLIENT_DEVOpenDevDown(long* lpHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_DEVRegDevDownCB(long lHandle, void* pUsr, void (CALLBACK * FUNDownDevCB)(int nType, void* pData, void * pUsr));
NETCLIENT_API int	API_CALL	NETCLIENT_DEVStartDevDown(long lHandle, int nMgrType, int nDevType);
NETCLIENT_API int	API_CALL	NETCLIENT_DEVStopDevDown(long lHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_DEVCloseDevDown(long lHandle);

//增加人员终端，保留之前的接口
NETCLIENT_API int	API_CALL	NETCLIENT_DEVOpenDevDownEx(long* lpHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_DEVRegDevDownCBEx(long lHandle, void* pUsr, void (CALLBACK * FUNDownDevCBEx)(int nType, void* pData, void * pUsr));
NETCLIENT_API int	API_CALL	NETCLIENT_DEVStartDevDownEx(long lHandle, int nMgrType, int nDevType);
NETCLIENT_API int	API_CALL	NETCLIENT_DEVStopDevDownEx(long lHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_DEVCloseDevDownEx(long lHandle);

//=============================报警屏蔽管理, 使用线程方式====================================//
NETCLIENT_API int	API_CALL	NETCLIENT_DEVOpenArmShieldDown(long* lpHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_DEVRegArmShieldDownCB(long lHandle, void* pUsr, void (CALLBACK * FUNDownArmShieldCB)(int nType, void* pData, void * pUsr));
NETCLIENT_API int	API_CALL	NETCLIENT_DEVStartArmShieldDown(long lHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_DEVStopArmShieldDown(long lHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_DEVCloseArmShieldDown(long lHandle);
//NETCLIENT_API int	API_CALL	NETCLIENT_AlarmShieldChangCB(void* pUsr,  void(CALLBACK* FUNAlarmShieldChangeCB)(int nReserve, void* pUsr));

//=============================地图标签管理, 使用线程方式====================================//
NETCLIENT_API int	API_CALL	NETCLIENT_DEVOpenMapTabDown(long* lpHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_DEVRegMapTabDownCB(long lHandle, void* pUsr, void (CALLBACK * FUNDownMapTabCB)(int nType, void* pData, void * pUsr));
NETCLIENT_API int	API_CALL	NETCLIENT_DEVStartMapTabDown(long lHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_DEVStopMapTabDown(long lHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_DEVCloseMapTabDown(long lHandle);
//NETCLIENT_API int	API_CALL	NETCLIENT_MapTabChangCB(void* pUsr,  void(CALLBACK* FUNMapTabChangeCB)(int nReserve, void* pUsr));

//=============================用户地图配置, 使用线程方式====================================//
NETCLIENT_API int	API_CALL	NETCLIENT_DEVOpenUserMapInfoDown(long* lpHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_DEVRegUserMapInfoDownCB(long lHandle, void* pUsr, void (CALLBACK * FUNDownUserMapInfoCB)(int nType, void* pData, void * pUsr));
NETCLIENT_API int	API_CALL	NETCLIENT_DEVStartUserMapInfoDown(long lHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_DEVStopUserMapInfoDown(long lHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_DEVCloseUserMapInfoDown(long lHandle);

//=============================用户权限, 使用线程方式====================================//
NETCLIENT_API int	API_CALL	NETCLIENT_DEVOpenUserPrivilegeDown(long* lpHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_DEVRegUserPrivilegeDownCB(long lHandle, void* pUsr, void (CALLBACK * FUNDownUserPrivilegeCB)(int nType, void* pData, void * pUsr));
NETCLIENT_API int	API_CALL	NETCLIENT_DEVStartUserPrivilegeDown(long lHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_DEVStopUserPrivilegeDown(long lHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_DEVCloseUserPrivilegeDown(long lHandle);

//=============================服务器配置，使用线程方式====================================//
NETCLIENT_API int	API_CALL	NETCLIENT_DEVOpenSvrConfigDown(long* lpHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_DEVRegSvrConfigDownCB(long lHandle, void* pUsr, void (CALLBACK * FUNDownSvrConfigCB)(int nType, void* pData, void * pUsr));
NETCLIENT_API int	API_CALL	NETCLIENT_DEVStartSvrConfigDown(long lHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_DEVStopSvrConfigDown(long lHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_DEVCloseSvrConfigDown(long lHandle);

//=============================用户信息，使用线程方式====================================//
NETCLIENT_API int	API_CALL	NETCLIENT_DEVOpenUserInfoDown(long* lpHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_DEVRegUserInfoDownCB(long lHandle, void* pUsr, void (CALLBACK * FUNDownUserInfoCB)(int nType, int nSubType, void* pData, void * pUsr));
NETCLIENT_API int	API_CALL	NETCLIENT_DEVSetUserInfoConfigDown(long lHandle, unsigned int nDownConfig);
NETCLIENT_API int	API_CALL	NETCLIENT_DEVStartUserInfoDown(long lHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_DEVStopUserInfoDown(long lHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_DEVCloseUserInfoDown(long lHandle);

//=============================数据透传到服务器====================================//
NETCLIENT_API int	API_CALL	NETCLIENT_OpenUserTransparent(long* lpHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_RegUserTransparentCB(long lHandle, void* pUsr, void (CALLBACK * FUNUserTransparentCB)(int nType, const char* pDataBuf, int nDataLen, void * pUsr));
NETCLIENT_API int	API_CALL	NETCLIENT_SetUserTransData(long lHandle, const char* pDataBuf, int nDataLen);
NETCLIENT_API int	API_CALL	NETCLIENT_StartUserTransparent(long lHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_StopUserTransparent(long lHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_CloseUserTransparent(long lHandle);

//=============================获取设备参数====================================//
NETCLIENT_API int	API_CALL	NETCLIENT_DLOpenGetDevCfg(long* lpGetDevCfg, const char* szDevIDNO);
NETCLIENT_API int	API_CALL	NETCLIENT_DLSetGetDCfgMsgCB(long lGetDevCfg, void* pUsr
														 , void (CALLBACK *FUNMsgCB)(int nMsg, int nParam, void* pUsr));
NETCLIENT_API int	API_CALL	NETCLIENT_DLSetGetDCfgDataCB(long lGetDevCfg, void* pUsr
														  , void (CALLBACK *FUNDataCB)(const char* pBuf, int nLen, long nPos, void* pUsr));
NETCLIENT_API int	API_CALL	NETCLIENT_DLStartGetDevCfg(long lGetDevCfg);
NETCLIENT_API int	API_CALL	NETCLIENT_DLStopGetDevCfg(long lGetDevCfg);
NETCLIENT_API int	API_CALL	NETCLIENT_DLCloseGetDevCfg(long lGetDevCfg);	

//=============================先上传设备参数到服务器====================================//
NETCLIENT_API int	API_CALL	NETCLIENT_UDPOpenUploadDevParams(long* lHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_UDPSetUploadMsgCB(long lUploadHandle, void* pUsr, void (CALLBACK *FUNMsgCB)(int nMsg, int nParam, void* pUsr));
NETCLIENT_API int	API_CALL	NETCLIENT_UDPStartUpload(long lUploadHandle, const char* szSrcFile);
NETCLIENT_API int	API_CALL	NETCLIENT_UDPStopUpload(long lUploadHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_UDPGetDestFileName(long lUploadHandle, int* lpSvrID, char* pDestFile);
NETCLIENT_API int	API_CALL	NETCLIENT_UDPCloseUpload(long lUploadHandle);

//=============================从服务器将配置文件上传到设备====================================//
NETCLIENT_API int	API_CALL	NETCLIENT_ULOpenSetDevCfg(long* lpSetDevCfg, const char* szDevIDNO, int nSvrID, const char* szFile);
NETCLIENT_API int	API_CALL	NETCLIENT_ULSetSetDCfgMsgCB(long lSetDevCfg, void* pUsr
														 , void (CALLBACK *FUNMsgCB)(int nMsg, int nParam, void* pUsr));
NETCLIENT_API int	API_CALL	NETCLIENT_ULStartSetDevCfg(long lSetDevCfg);
NETCLIENT_API int	API_CALL	NETCLIENT_ULStopSetDevCfg(long lSetDevCfg);
NETCLIENT_API int	API_CALL	NETCLIENT_ULCloseSetDevCfg(long lSetDevCfg);

//=============================文件搜索====================================//
NETCLIENT_API int	API_CALL	NETCLIENT_DLOpenSearchFile(long* lpSearchHandle, const char* szDevIDNO, int nLocation);
NETCLIENT_API int	API_CALL	NETCLIENT_DLOpenSrchFileEx(long* lpSearchHandle, const char* szDevIDNO, int nLocation, int nAttributenFile);
NETCLIENT_API int	API_CALL	NETCLIENT_DLSetSearchMsgCB(long lSearchHandle, void* pUsr
														  , void (CALLBACK *FUNMsgCB)(int nMsg, int nParam, void* pUsr));
NETCLIENT_API int	API_CALL	NETCLIENT_DLSetSearchFileCB(long lSearchHandle, void* pUsr
														   , void (CALLBACK *FUNFileCB)(GPSFile_S* pFile, void* pUsr));
NETCLIENT_API int	API_CALL	NETCLIENT_DLStartSearchFile(long lSearchHandle, int nYear, int nMonth, int nDay
														  , int nRecType, int nChn, int nBegTime, int nEndTime);
NETCLIENT_API int	API_CALL	NETCLIENT_DLStopSearchFile(long lSearchHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_DLCloseSearchFile(long lSearchHandle);	

//=============================文件下载====================================//
NETCLIENT_API int	API_CALL	NETCLIENT_DLOpenDownFile(long* lpDownHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_DLOpenDownFileEx(long* lpDownHandle, int nAttribute);
NETCLIENT_API int	API_CALL	NETCLIENT_DLSetDFileMsgCB(long lDownHandle, void* pUsr
														 , void (CALLBACK *FUNMsgCB)(int nMsg, int nParam, void* pUsr));
//取文件下载数据，单位KByte
NETCLIENT_API int	API_CALL	NETCLIENT_DLGetFlowRate(long lDownHandle, int* lpFlowRate);
//此下载方式，是追加下载，具备断点继传功能
NETCLIENT_API int	API_CALL	NETCLIENT_DLStartDownFile(long lDownHandle, GPSFile_S* pFile, char* szDownFile);
NETCLIENT_API int	API_CALL	NETCLIENT_DLStopDownFile(long lDownHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_DLCloseDownFile(long lDownHandle);	

//=============================报警日志查询====================================//
// NETCLIENT_API int	API_CALL	NETCLIENT_OpenAlarmQuery(long* lpAlarmQueryHandle, int nPage, int nItemCount);
// NETCLIENT_API int	API_CALL	NETCLIENT_SetAlarmQueryMsgCB(long lAlarmQueryHandle, void* pUsr, void (CALLBACK *FUNMsgCB)(int nMsg, int nParam, void* pUsr));
// NETCLIENT_API int	API_CALL	NETCLIENT_SetAlarmQueryDataCB(long lAlarmQueryHandle, void* pUsr, void (CALLBACK *FUNVehiArmCB)(const char* pAlarm, int nLength, void* pUsr));
// NETCLIENT_API int	API_CALL	NETCLIENT_StartAlarmQuery(long lAlarmQueryHandle, GPSDeviceIDNO_S* pDevice, int nDevCount, int* pArmType, int nTyCount, SYSTEMTIME* tmBeg, SYSTEMTIME* tmEnd);
// NETCLIENT_API int	API_CALL	NETCLIENT_StopAlarmQuery(long lAlarmQueryHandle);
// NETCLIENT_API int	API_CALL	NETCLIENT_CloseAlarmQuery(long lAlarmQueryHandle);

//=============================获取设备状态信息====================================//
//下层每5分钟从服务器更新一次设备状态信息
NETCLIENT_API int	API_CALL	NETCLIENT_VSOpenVehiStatus(long* lpVSHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_VSSetStatusCB(long lVSHandle, void* pUsr, void (CALLBACK *FUNVehiStatusCB)(GPSVehicleState_S* pVehiRun, char* pOnline, int nCount, void* pUsr));
NETCLIENT_API int	API_CALL	NETCLIENT_VSSetStatusDevice(long lVSHandle, GPSDeviceIDNO_S* pDevice, int nCount);
NETCLIENT_API int	API_CALL	NETCLIENT_VSStartVSChan(long lVSHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_VSStopVSChan(long lVSHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_VSCloseVehiStatus(long lVSHandle);

//=============================设备运行GPS====================================//
NETCLIENT_API int	API_CALL	NETCLIENT_VROpenVehiRun(long* lpVRHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_VRSetRunningCB(long lVRHandle, void* pUsr, void (CALLBACK *FUNVehiRunningCB)(GPSVehicleState_S* pVehiRun, void* pUsr));
NETCLIENT_API int	API_CALL	NETCLIENT_VRStartVRChan(long lVRHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_VRStopVRChan(long lVRHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_VRAddDevice(long lVRHandle, int nDevID, const char* szDevIDNO);
NETCLIENT_API int	API_CALL	NETCLIENT_VRDelDevice(long lVRHandle, int nDevID);
NETCLIENT_API int	API_CALL	NETCLIENT_VRCloseVehiRun(long lVRHandle);

//=============================设备报警信息====================================//
NETCLIENT_API int	API_CALL	NETCLIENT_VAOpenVehiAlarm(long* lpVAHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_VASetAlarmDevice(long lVAHandle, GPSDeviceIDNO_S* pDevice, int nCount);
NETCLIENT_API int	API_CALL	NETCLIENT_VASetAlarmInfoCB(long lVAHandle, void* pUsr, void (CALLBACK *FUNAlarmInfoCB)(const char* pAlarm, int nLength, void* pUsr));
NETCLIENT_API int	API_CALL	NETCLIENT_VAStartVAChan(long lVAHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_VAStopVAChan(long lVAHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_VACloseVehiAlarm(long lVAHandle);

//=============================轨迹查询====================================//
NETCLIENT_API int	API_CALL	NETCLIENT_VTOpenVehiTrack(long* lpVTrackHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_VTSetVTrackMsgCB(long lVTrackHandle, void* pUsr, void (CALLBACK *FUNVehiTrackMsgCB)(int nMsg, int nParam, void* pUsr));
NETCLIENT_API int	API_CALL	NETCLIENT_VTSetVTrackDataCB(long lVTrackHandle, void* pUsr, void (CALLBACK *FUNVehiTrackDataCB)(GPSVehicleTrack_S* pVehiTrack, void* pUsr));
NETCLIENT_API int	API_CALL	NETCLIENT_VTStartVehiTrack(long lVTrackHandle, char* szDevIDNO, const SYSTEMTIME* pBegTime, const SYSTEMTIME* pEndTime, int nType, int nDistance, int nParkTime);
NETCLIENT_API int	API_CALL	NETCLIENT_VTStopVehiTrack(long lVTrackHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_VTCloseVehiTrack(long lVTrackHandle);

//=============================上传文件====================================//
NETCLIENT_API int	API_CALL	NETCLIENT_GetDevUpgradeFileInfo(const char* szFile, GPSDEVUpgradeFile_S* pFileInfo);

NETCLIENT_API int	API_CALL	NETCLIENT_ULOpenDevUpgradeFile(long* lHandle, GPSDEVUpgradeFile_S* pFile, const char* szSrcFile);
NETCLIENT_API int	API_CALL	NETCLIENT_ULOpenDevParamFile(long* lHandle, const char* szSrcFile);
NETCLIENT_API int	API_CALL	NETCLIENT_ULSetUploadMsgCB(long lUploadHandle, void* pUsr, void (CALLBACK *FUNMsgCB)(int nMsg, int nParam, void* pUsr));
NETCLIENT_API int	API_CALL	NETCLIENT_ULStartUpload(long lUploadHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_ULStopUpload(long lUploadHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_ULGetDestFileName(long lUploadHandle, int* lpSvrID, char* pDestFile);
NETCLIENT_API int	API_CALL	NETCLIENT_ULCloseUpload(long lUploadHandle);

//=============================远程升级====================================//
NETCLIENT_API int	API_CALL	NETCLIENT_DUOpenDevUpgrade(long* lHandle, const char* szDevIDNO, GPSDEVUpgradeFile_S* pFile, int nSvrID, const char* szUpFile);
NETCLIENT_API int	API_CALL	NETCLIENT_DUSetDevUpMsgCB(long lUploadHandle, void* pUsr, void (CALLBACK *FUNMsgCB)(int nMsg, int nParam, void* pUsr));
NETCLIENT_API int	API_CALL	NETCLIENT_DUStartDevUpgrade(long lUploadHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_DUStopDevUgprade(long lUploadHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_DUCloseDevUpgrade(long lUploadHandle);

//=============================向服务器发送命令====================================//
NETCLIENT_API int	API_CALL	NETCLIENT_MCStartWork();
NETCLIENT_API int	API_CALL	NETCLIENT_MCStopWork();
NETCLIENT_API int	API_CALL	NETCLIENT_MCRegMsgCB(void* pUsr, void (CALLBACK * FUNMCMsgCB)(GPSMCMsg_S* pMsg, void* pUsr));
NETCLIENT_API int	API_CALL	NETCLIENT_MCReleaseMsg(GPSMCMsg_S* pMsg);
NETCLIENT_API int	API_CALL	NETCLIENT_MCReleaseHandle(long lHandle);

//发送短信息，如短信通知设备重启、配置中心地址
NETCLIENT_API int	API_CALL	NETCLIENT_MCSendSMS(const char* szDevIDNO, int nSMSType, const char* szPhoneNO, const char* szText); //发送短信
//发送TTS
NETCLIENT_API int	API_CALL	NETCLIENT_MCSendText(const char* szDevIDNO, const char* szText, void* pUsr, void (CALLBACK * FUNMCMsgCB)(GPSMCMsg_S* pMsg, void* pUsr));
//发送重启，断油断电
NETCLIENT_API int	API_CALL	NETCLIENT_MCSendCtrl(const char* szDevIDNO, int nCtrlType, const char* szUsr, const char* szPwd);
//云台控制
NETCLIENT_API int	API_CALL	NETCLIENT_MCSendPtz(const char* szDevIDNO, int nChn, int nCommand, int nSpeed, int nParam);
//获取车辆状态
NETCLIENT_API int	API_CALL	NETCLIENT_MCReadDevStatus(const char* szDevIDNO, void* pUsr, void (CALLBACK * FUNMCMsgCB)(GPSMCMsg_S* pMsg, void* pUsr));
//修改用户密码
NETCLIENT_API int	API_CALL	NETCLIENT_MCChangeUsrPwd(const char* szOld, const char* szNew, void* pUsr, void (CALLBACK * FUNMCMsgCB)(GPSMCMsg_S* pMsg, void* pUsr), long* lpHandle);
//设置GPS时间间隔
NETCLIENT_API int	API_CALL	NETCLIENT_MCSetGpsInterval(const char* szDevIDNO, BOOL bStart, int nDistance, int nTime, int nType);
//发送控制指令时校验平台用户
NETCLIENT_API int	API_CALL	NETCLIENT_MCCheckCtrlUsr(const char* szUsr, const char* szPwd);
//获取移动侦测参数
NETCLIENT_API int	API_CALL	NETCLIENT_MCReadMotionParam(const char* szDevIDNO, int nChannel
														  , void* pUsr, void (CALLBACK * FUNMCMsgCB)(GPSMCMsg_S* pMsg, void* pUsr)
														  , long* lpHandle);
//配置移动侦测参数
NETCLIENT_API int	API_CALL	NETCLIENT_MCSetMotionParam(const char* szDevIDNO, int nChannel, GPSMotionParam_S* pMotion
														 , void* pUsr, void (CALLBACK * FUNMCMsgCB)(GPSMCMsg_S* pMsg, void* pUsr)
														 , long* lpHandle);
//获取网络流量参数信息
NETCLIENT_API int	API_CALL	NETCLIENT_MCReadNetFlowStatistics(const char* szDevIDNO
														  , void* pUsr, void (CALLBACK * FUNMCMsgCB)(GPSMCMsg_S* pMsg, void* pUsr)
														  , long* lpHandle);
//配置网络流量统计参数
NETCLIENT_API int	API_CALL	NETCLIENT_MCSetNetFlowParam(const char* szDevIDNO, GPSNetFlowStatistics_S* pNetParam
														 , void* pUsr, void (CALLBACK * FUNMCMsgCB)(GPSMCMsg_S* pMsg, void* pUsr)
														 , long* lpHandle);
//清空网络流量统计
NETCLIENT_API int	API_CALL	NETCLIENT_MCClearNetFlowStatistics(const char* szDevIDNO
														  , void* pUsr, void (CALLBACK * FUNMCMsgCB)(GPSMCMsg_S* pMsg, void* pUsr)
														  , long* lpHandle);
//调整网络流量参数
NETCLIENT_API int	API_CALL	NETCLIENT_MCAdjustNetFlowStatistics(const char* szDevIDNO, int nMBFlow
														 , void* pUsr, void (CALLBACK * FUNMCMsgCB)(GPSMCMsg_S* pMsg, void* pUsr)
														 , long* lpHandle);
//设置用户报警屏蔽 
NETCLIENT_API int	API_CALL	NETCLIENT_MCSetUsrAlarmShield(const GPSAlarmShield_S* pAlarm, int nCount
														 , void* pUsr, void (CALLBACK * FUNMCMsgCB)(GPSMCMsg_S* pMsg, void* pUsr)
														 , long* lpHandle);
//添加地图标记信息
NETCLIENT_API int	API_CALL	NETCLIENT_MCAddMapMarker(const GPSMapMarker_S* pMarker
														  , void* pUsr, void (CALLBACK * FUNMCMsgCB)(GPSMCMsg_S* pMsg, void* pUsr)
														  , long* lpHandle);
//添加地图标记属性
NETCLIENT_API int	API_CALL	NETCLIENT_MCAddMapMarkerInfo(const GPSMarkerInfo_S* pMarkerInfo
														 , void* pUsr, void (CALLBACK * FUNMCMsgCB)(GPSMCMsg_S* pMsg, void* pUsr)
														  , long* lpHandle);
//删除地图标记信息
NETCLIENT_API int	API_CALL	NETCLIENT_MCDelMapMarker(int nMarkerID
														 , void* pUsr, void (CALLBACK * FUNMCMsgCB)(GPSMCMsg_S* pMsg, void* pUsr)
														 , long* lpHandle);
//发送调度指令信息
NETCLIENT_API int	API_CALL	NETCLIENT_MCSendDispatchCommand(const char* szDevIDNO, const GPSDispatchCommand_S* lpDispatch
														 , void* pUsr, void (CALLBACK * FUNMCMsgCB)(GPSMCMsg_S* pMsg, void* pUsr)
														 , long* lpHandle);
//设置用户地图配置
NETCLIENT_API int	API_CALL	NETCLIENT_MCSetUsrMapConfig(const UserMapConfig_S* pUserMapConfig, void* pUsr, 
															void (CALLBACK * FUNMCMsgCB)(GPSMCMsg_S* pMsg, void* pUsr)
															  , long* lpHandle);

//参数配置
NETCLIENT_API int	API_CALL	NETCLIENT_MCSendCfg(const char* szDevIDNO, const char* pCfg, int nLen, int nCMDCfg
													   , void* pUsr, void (CALLBACK * FUNMCMsgCB)(GPSMCMsg_S* pMsg, void* pUsr)
													   , long* lpHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_MCGetCfg(const char* szDevIDNO, int nCMDCfg
													  , void* pUsr, void (CALLBACK * FUNMCMsgCB)(GPSMCMsg_S* pMsg, void* pUsr)
													  , long* lpHandle);

//=============================与服务器进行指令交互（使用TCP方式直接服务器）====================================//
NETCLIENT_API int	API_CALL	NETCLIENT_CMDAddMapLine(const GPSMapLine_S* pLine, void* pUsr, FUNDownDataCB pfnDownData
														 , long* lpHandle);
NETCLIENT_API int	API_CALL	NETCLIENT_CMDReleaseHandle(long lHandle);


#ifdef __cplusplus
}
#endif

#endif

