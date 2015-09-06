#ifndef __JO_DBUS_H_
#define __JO_DBUS_H_
#include "j_includes.h"
#include "x_timer.h"
#include "x_lock.h"

#ifdef WIN32
typedef unsigned (X_JO_API *j_data_event_t)(J_DObj *);
#else
typedef void *(*j_data_event_t)(J_DObj *); 
#endif 

class CDBus
{
	typedef std::map<j_string_t, j_data_event_t> DBusEventMap;
	typedef std::map<J_Obj*, DBusEventMap> ClientMethodMap;
	typedef std::map<j_string_t, DBusEventMap> ServerMethodMap;
	typedef ClientMethodMap ClientSignalMap;

	struct InvokeKey
	{
		InvokeKey(j_string_t strServerID, j_string_t strMethodName)
		{
			serverID = strServerID;
			methodName = strMethodName;
		}
		j_string_t serverID;
		j_string_t methodName;

		j_boolean_t operator < (const InvokeKey &other) const
		{
			if (serverID == other.serverID)
			{
				return methodName < other.methodName;
			}

			return serverID < other.serverID;
		}
	};
	typedef std::vector<j_data_event_t> DBusEventVector;
	typedef std::map<InvokeKey, DBusEventVector> InvokeMethodMap;

public:
	CDBus();
	~CDBus();

public:
	/// 客户端调用接口
	///@param[in]	pObj 客户端对象
	///@param[in]	strName 方法名称
	///@param[in]	pCallback 回调函数
	///@return 		无
	void registerMethod(J_Obj *pObj, j_string_t strName, j_data_event_t pCallback);
	///@param[in]	pObj 客户端对象
	///@param[in]	strName 信号名称
	///@param[in]	pCallback 回调函数
	///@return 		无
	void registerSignal(J_Obj *pObj, j_string_t strName, j_data_event_t pCallback);
	///@param[in]	pObj 客户端对象
	///@param[in]	strServerID 服务器对象ID
	///@param[in]	strName 方法名称
	///@param[in]	pParm 参数
	///@return 		无
	void invoke(J_Obj *pObj, j_string_t strServerID, j_string_t strName, J_DObj *pParm) ;

	/// 服务器端调用接口
	///@param[in]	serverID 服务器对象ID
	///@param[in]	strName 方法名称
	///@param[in]	pCallback 回调函数
	///@return 		无
	void registerMethod(j_string_t serverID, j_string_t strName, j_data_event_t pCallback);
	///@param[in]	serverID 服务器对象ID
	///@param[in]	strName 方法名称
	///@param[in]	pParm 参数
	///@return 		无
	void sendResponse(j_string_t serverID, j_string_t strName, J_DObj *pParm);
	///@param[in]	serverID 服务器对象ID
	///@param[in]	strName 信号名称
	///@param[in]	pParm 参数
	///@return 		无
	void boradcastSignal(j_string_t serverID, j_string_t strName, J_DObj *pParm);

private:
	ClientMethodMap m_clientMethodMap;
	ClientSignalMap m_clientSignalMap;
	ServerMethodMap m_serverMethodMap;
	InvokeMethodMap m_invokeMethodMap;
};

JO_DECLARE_SINGLETON(DBus)

#endif //~__JO_DBUS_H_