#include "x_dbus.h"

JO_IMPLEMENT_SINGLETON(DBus)

CDBus::CDBus()
{

}

CDBus::~CDBus()
{

}

// for client
void CDBus::registerMethod(J_Obj *pObj, j_string_t strName, j_data_event_t pCallback)
{
	ClientMethodMap::iterator itMethod = m_clientMethodMap.find(pObj);
	if (itMethod != m_clientMethodMap.end())
	{
		DBusEventMap::iterator itEvent = itMethod->second.find(strName);
		if (itEvent == itMethod->second.end())
		{
			itMethod->second[strName] = pCallback;
		}
	}
	else
	{
		DBusEventMap eventMap;
		eventMap[strName] = pCallback;
		m_clientMethodMap[pObj] = eventMap;
	}
}

void CDBus::registerSignal(J_Obj *pObj, j_string_t strName, j_data_event_t pCallback)
{
	ClientSignalMap::iterator itMethod = m_clientSignalMap.find(pObj);
	if (itMethod != m_clientSignalMap.end())
	{
		DBusEventMap::iterator itEvent = itMethod->second.find(strName);
		if (itEvent == itMethod->second.end())
		{
			itMethod->second[strName] = pCallback;
		}
	}
	else
	{
		DBusEventMap eventMap;
		eventMap[strName] = pCallback;
		m_clientSignalMap[pObj] = eventMap;
	}
}

void CDBus::invoke(J_Obj *pObj, j_string_t strServerID, j_string_t strName, J_DObj *pParm)
{
	InvokeKey key(strServerID, strName);
	InvokeMethodMap::iterator itInvoke = m_invokeMethodMap.find(key);
	if (itInvoke == m_invokeMethodMap.end())
	{
		ClientMethodMap::iterator itClientMethod = m_clientMethodMap.find(pObj);
		if (itClientMethod != m_clientMethodMap.end())
		{
			DBusEventMap itClientEvent = itClientMethod->second.find(strName);
			if (itClientEvent != itClientMethod->second.end())
			{
			}
		}

		ServerMethodMap::iterator itServerMethod = m_serverMethodMap.find(strServerID);
		if (itServerMethod != m_serverMethodMap.end())
		{
			DBusEventMap::iterator itEvent = itServerMethod->second.find(strName);
			if (itEvent != itServerMethod->second.end())
			{
				itEvent->second(pParm);
			}
		}
	}
}

// for server
void CDBus::registerMethod(j_string_t serverID, j_string_t strName, j_data_event_t pCallback)
{
	ServerMethodMap::iterator itMethod = m_serverMethodMap.find(serverID);
	if (itMethod != m_serverMethodMap.end())
	{
		DBusEventMap::iterator itEvent = itMethod->second.find(strName);
		if (itEvent == itMethod->second.end())
		{
			itMethod->second[strName] = pCallback;
		}
	}
	else
	{
		DBusEventMap eventMap;
		eventMap[strName] = pCallback;
		m_serverMethodMap[serverID] = eventMap;
	}
}

void CDBus::sendResponse(j_string_t serverID, j_string_t strMethodName, J_DObj *pParm)
{
}

void CDBus::boradcastSignal(j_string_t serverID, j_string_t strSignalName, J_DObj *pParm)
{
}