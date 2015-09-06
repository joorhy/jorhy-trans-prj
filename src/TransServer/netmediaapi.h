#ifndef _NET_MEDIA_API_H_
#define _NET_MEDIA_API_H_

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the IREALPLAY_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// NETMEDIA_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef LIBNETMEDIA_EXPORTS
#define NETMEDIA_API __declspec(dllexport)
#else
#define NETMEDIA_API __declspec(dllimport)
#endif

#ifndef API_CALL
#define  API_CALL	WINAPI
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "GPSDataDef.h"


#define NETMEDIA_OK			0
#define NETMEDIA_FALSE		1

#define NETMEDIA_TYPE_VIEWER			0
#define NETMEDIA_TYPE_REC_SVR		1

#define NETMEDIA_REAL_MSG_SUCCESS			0
#define NETMEDIA_REAL_MSG_CNT_DS_FD			-1
#define NETMEDIA_REAL_MSG_USR_NORIGHT		-2
#define NETMEDIA_REAL_MSG_MS_EMPTY			-3
#define NETMEDIA_REAL_MSG_CNT_MS_FD			-4
#define NETMEDIA_REAL_MSG_CNT_MS_SUC			-5
#define NETMEDIA_REAL_MSG_CNT_DEV_FD			-6
#define NETMEDIA_REAL_MSG_CNT_DEV_SUC		-7
#define NETMEDIA_REAL_MSG_MS_DISCONNECT		-8
#define NETMEDIA_REAL_MSG_USR_FULL_ERROR		-9
#define NETMEDIA_REAL_MSG_USR_ERROR			-10
#define NETMEDIA_REAL_MSG_UNKNOW				-11
#define NETMEDIA_REAL_MSG_FINISHED			-12
#define NETMEDIA_REAL_MSG_SESSION_END		-13
#define NETMEDIA_REAL_MSG_DEV_USED			-14
#define NETMEDIA_REAL_MSG_DEV_OFFLINE		-15	

#define NETMEDIA_REAL_TYPE_MAIN				0
#define NETMEDIA_REAL_TYPE_SUB				1

#define NETMEDIA_CNT_TYPE_TCP				0
#define NETMEDIA_CNT_TYPE_UDP				1

#define NETMEDIA_REAL_DATA_TYPE_HEAD			1
#define NETMEDIA_REAL_DATA_TYPE_I_FRAME		0x63643030
#define NETMEDIA_REAL_DATA_TYPE_P_FRAME		0x63643130
#define NETMEDIA_REAL_DATA_TYPE_A_FRAME		0x63643230

#define NETMEDIA_AUDIO_MIC_CHANNEL			98

NETMEDIA_API int	API_CALL	NETMEDIA_Initialize(int nType);
NETMEDIA_API int	API_CALL	NETMEDIA_UnInitialize();

NETMEDIA_API HANDLE API_CALL	NETMEDIA_GetDecHandle(int nType = 0);

NETMEDIA_API int	API_CALL	NETMEDIA_SetSession(const char* szGUID);
NETMEDIA_API int	API_CALL	NETMEDIA_SetDirSvr(const char* szIP, unsigned short usPort);
//hw 2013/1/19 设置服务器的网络类型 
NETMEDIA_API int	API_CALL	NETMEDIA_SetSvrNetType(int nNetType);

//实时预览
NETMEDIA_API int	API_CALL	NETMEDIA_OpenRealPlay(const char* szDevIDNO, int nChn, int nMode, int nCntMode, BOOL bTransmit, long* lpRealHandle);	
//控件部分需要调用此接口，配置控件对应的服务器地址
NETMEDIA_API int	API_CALL	NETMEDIA_SetRealServer(long lRealHandle, const char* szIP, unsigned short usPort, const char* szSession);	
NETMEDIA_API int	API_CALL	NETMEDIA_SetRealMsgCallBack(long lRealHandle, void* pUsr, void (CALLBACK *FUNRealMsgCB)(int nMsg, void* pUsr));
NETMEDIA_API int	API_CALL	NETMEDIA_SetRealDataCallBack(long lRealHandle, void* pUsr, void (CALLBACK *FUNRealDataCB)(const char* pFrameBuf
									, int nFrameLen, int nFrameType, unsigned __int64 llFrameStamp, void* pUsr));
NETMEDIA_API int	API_CALL	NETMEDIA_SetRealDecCallBack(long lRealHandle, void* pUsr
									, void (CALLBACK* FUNRealDecCB)(unsigned char* pY, int nYStride, unsigned char* pU, unsigned char* pV, int nUVStride, 
									int nWidth, int nHeight, unsigned __int64 nStamp, int nType, void* pUsr));
NETMEDIA_API int	API_CALL	NETMEDIA_SetRealDisplayCallBack(long lRealHandle, void* pUsr
									, void (CALLBACK* FUNRealDisplayCB)(HDC hDc 
									, int nWidth, int nHeight, unsigned __int64 nStamp
									, int nWndWidth, int nWndHeight, void* pUsr));
NETMEDIA_API int	API_CALL	NETMEDIA_SetRealDecUsrDataCallBack(long lRealHandle, void* pUsr
									, void (CALLBACK *FUNDecUsrDataCB)(const char* pUsrData
									, int nDataLen, int nDataType, void* pUsr));
NETMEDIA_API int	API_CALL	NETMEDIA_StartRealPlay(long lRealHandle, HWND hWnd);
NETMEDIA_API int	API_CALL	NETMEDIA_StopRealPlay(long lRealHandle);
NETMEDIA_API int	API_CALL	NETMEDIA_StartView(long lRealHandle, HWND hWnd);
NETMEDIA_API int	API_CALL	NETMEDIA_StopView(long lRealHandle);
NETMEDIA_API int	API_CALL	NETMEDIA_SetStreamMode(long lRealHandle, int nMode);
NETMEDIA_API int	API_CALL	NETMEDIA_SetDelayTime(long lRealHandle, DWORD dwMinMinsecond, DWORD dwMaxMinsecond);
NETMEDIA_API int	API_CALL	NETMEDIA_AdjustedWndRect(long lRealHandle);
NETMEDIA_API int	API_CALL	NETMEDIA_PlaySound(long lRealHandle, long lPlay);
NETMEDIA_API int	API_CALL	NETMEDIA_SetVolume(long lRealHandle, WORD wVolume);
NETMEDIA_API int	API_CALL	NETMEDIA_CaptureBMP(long lRealHandle, const char* szBmpFile);
NETMEDIA_API int	API_CALL	NETMEDIA_CaptureJPEG(long lRealHandle, const char* szJPEGFile);
NETMEDIA_API int	API_CALL	NETMEDIA_SetDivHwnd(long lRealHandle, int nIndex, HWND hWnd);
NETMEDIA_API int	API_CALL	NETMEDIA_SetDivRect(long lRealHandle, int nIndex, RECT* pRect);
NETMEDIA_API int	API_CALL	NETMEDIA_GetDivRect(long lRealHandle, int nIndex, RECT* pRect);
NETMEDIA_API int	API_CALL	NETMEDIA_SetDivPlay(long lRealHandle, int nIndex, int nPlay);
NETMEDIA_API int	API_CALL	NETMEDIA_GetDivPlay(long lRealHandle, int nIndex, int* nPlay);
NETMEDIA_API int	API_CALL	NETMEDIA_GetFlowRate(long lRealHandle, int* lpFlowRate);
NETMEDIA_API int	API_CALL	NETMEDIA_PtzCtrl(long lRealHandle, int nCommand, int nSpeed, int nParam);
NETMEDIA_API int	API_CALL	NETMEDIA_CloseRealPlay(long lRealHandle);

//语音监听
NETMEDIA_API int	API_CALL	NETMEDIA_RAOpenRealAudio(const char* szDevIDNO, int nChn, int nMode, int nCntMode, BOOL bTransmit, long* lpRealHandle);	
NETMEDIA_API int	API_CALL	NETMEDIA_RASetRealServer(long lRealHandle, const char* szIP, unsigned short usPort, const char* szSession);	
NETMEDIA_API int	API_CALL	NETMEDIA_RASetRAudioMsgCallBack(long lRealHandle, void* pUsr, void (CALLBACK *FUNRealMsgCB)(int nMsg, void* pUsr));
NETMEDIA_API int	API_CALL	NETMEDIA_RAStartRealAudio(long lRealHandle);
NETMEDIA_API int	API_CALL	NETMEDIA_RAStopRealAudio(long lRealHandle);
NETMEDIA_API int	API_CALL	NETMEDIA_RAPlaySound(long lRealHandle, long lPlay);
NETMEDIA_API int	API_CALL	NETMEDIA_RASetVolume(long lRealHandle, WORD wVolume);
NETMEDIA_API int	API_CALL	NETMEDIA_RACloseRealAudio(long lRealHandle);

//前端抓拍
NETMEDIA_API int	API_CALL	NETMEDIA_SNOpenSnapshot(long* lpSnapshot, const char* szDevIDNO, int nChn);
NETMEDIA_API int	API_CALL	NETMEDIA_SNSetRealServer(long lSnapshotHandle, const char* szIP, unsigned short usPort, const char* szSession);	
NETMEDIA_API int	API_CALL	NETMEDIA_SNSetSnapMsgCB(long lSnapshotHandle, void* pUsr
													   , void (CALLBACK *FUNMsgCB)(int nMsg, void* pUsr));
NETMEDIA_API int	API_CALL	NETMEDIA_SNSetSnapDataCB(long lSnapshotHandle, void* pUsr
														, void (CALLBACK *FUNDataCB)(const char* pBuf, int nLen, long nPos, void* pUsr));
NETMEDIA_API int	API_CALL	NETMEDIA_SNStartSnapshot(long lSnapshotHandle);
NETMEDIA_API int	API_CALL	NETMEDIA_SNStopSnapshot(long lSnapshotHandle);
NETMEDIA_API int	API_CALL	NETMEDIA_SNCloseSnapshot(long lSnapshotHandle);	

//双向对讲
NETMEDIA_API int	API_CALL	NETMEDIA_TBOpenTalkback(const char* szDevIDNO, int nChn, int nCntMode, long* lpTalkbackHandle);	
NETMEDIA_API int	API_CALL	NETMEDIA_TBSetRealServer(long lTalkbackHandle, const char* szIP, unsigned short usPort, const char* szSession);	
NETMEDIA_API int	API_CALL	NETMEDIA_TBSetTalkbackMsgCallBack(long lTalkbackHandle, void* pUsr, void (CALLBACK *FUNRealMsgCB)(int nMsg, void* pUsr));
NETMEDIA_API int	API_CALL	NETMEDIA_TBStartTalkback(long lTalkbackHandle);
NETMEDIA_API int	API_CALL	NETMEDIA_TBStopTalkback(long lTalkbackHandle);
NETMEDIA_API int	API_CALL	NETMEDIA_TBPlaySound(long lTalkbackHandle, long lPlay);
NETMEDIA_API int	API_CALL	NETMEDIA_TBSetVolume(long lTalkbackHandle, WORD wVolume);
NETMEDIA_API int	API_CALL	NETMEDIA_TBCloseTalkback(long lTalkbackHandle);

//远程回放接口 PB = PlayBack
NETMEDIA_API int	API_CALL	NETMEDIA_PBOpenTalkback(const char* szDevIDNO, const , long* lpTalkbackHandle);	
NETMEDIA_API int	API_CALL	NETMEDIA_DLSetDFileMsgCB(long lDownHandle, void* pUsr
														  , void (CALLBACK *FUNMsgCB)(int nMsg, int nParam, void* pUsr));
//取文件下载数据，单位KByte
NETMEDIA_API int	API_CALL	NETMEDIA_DLGetFlowRate(long lDownHandle, int* lpFlowRate);
//此下载方式，是追加下载，具备断点继传功能
//NETMEDIA_API int	API_CALL	NETMEDIA_DLStartDownFile(long lDownHandle, GPSFile_S* pFile, char* szDownFile);
NETMEDIA_API int	API_CALL	NETMEDIA_DLStopDownFile(long lDownHandle);

#ifdef __cplusplus
}
#endif

#endif
