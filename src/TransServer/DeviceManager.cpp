///////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2009, xx科技有限公司(版权声明) 
/// All rights reserved. 
/// 
/// @file      DeviceManager.cpp 
/// @brief     前端设备管理模块实现
///
/// @version   1.0 (版本声明)
/// @author    Jorhy (joorhy@gmail.com) 
/// @date      2013/09/18 17:10 
///
///
/// 修订说明：最初版本
///////////////////////////////////////////////////////////////////////////  
#include "DeviceManager.h"
#include "XlHost.h"
#include "VnHost.h"
#include "DahuaHost.h"

JO_IMPLEMENT_SINGLETON(DeviceManager)

CDeviceManager::CDeviceManager()
{
	m_timer.Create(1000, CDeviceManager::OnTimer, this);
}

CDeviceManager::~CDeviceManager()
{
	m_timer.Destroy();
}

J_Host *CDeviceManager::CreateDevObj(j_int32_t nHostType, j_string_t strHostId, j_int64_t nParam)
{
	J_Host *pHost = GetDeviceObj(strHostId);
	if (pHost == NULL)
	{
		try
		{
			switch (nHostType)
			{
			case 1://XlHost
				pHost = new CXlHost(strHostId, nParam);
				break;
			case 2://VnHost
				pHost = new CVnHost(strHostId);
				break;
			case 3://DahuaHost
				pHost = new CDahuaHost(strHostId, nParam);
				break;
			default:
				break;
			}
		}
		catch (...)
		{
			J_OS::LOGINFO("CDeviceManager::CreateDevObj Error");
		}

		if (pHost != NULL)
		{
			NetHostInfo info = { 0 };
			info.pHost = pHost;
			info.bRegister = false;
			TLock(m_locker);
			m_devMap[strHostId] = info;
			TUnlock(m_locker);
		}
	}

	return pHost;
}

void CDeviceManager::ReleaseDevObj(j_string_t strHostId)
{
	TLock(m_locker);
	DeviceMap::iterator it = m_devMap.find(strHostId);
	if (it != m_devMap.end())
	{
		it->second.pHost->OnBroken();
		delete it->second.pHost;
		m_devMap.erase(it);
	}
	TUnlock(m_locker);
}

void CDeviceManager::CheckDevice()
{
	TLock(m_locker);
	DeviceMap::iterator it = m_devMap.begin();
	for (; it != m_devMap.end(); ++it)
	{
		J_Host *pHost = it->second.pHost;
		if (pHost->IsReady())
		{
			if (!it->second.bRegister)
			{
				it->second.bRegister = true;
			}
		}
		else
		{
			if (it->second.bRegister)
			{
				TUnlock(m_locker);

				j_string_t hostId;
				pHost->GetHostId(hostId);
				ReleaseDevObj(hostId);

				TLock(m_locker);
				break;
			}
		}
	}
	TUnlock(m_locker);
}

J_Host *CDeviceManager::GetDeviceObj(j_string_t strDevId)
{
	TLock(m_locker);
	DeviceMap::iterator it = m_devMap.find(strDevId);
	if (it != m_devMap.end())
	{
		TUnlock(m_locker);
		return it->second.pHost;
	}
	TUnlock(m_locker);

	return NULL;
}