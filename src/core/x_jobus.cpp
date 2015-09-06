#include "x_jobus.h"

CJoBus::CJoBus()
{
	
}

CJoBus::~CJoBus()
{
	
}

j_reuslt_t CJoBus::RegisterSignal(const j_string_t &strHostId, const j_string_t &strSigName, JoBusCallback callback)
{
	return J_OK;
}

j_reuslt_t CJoBus::UnRegisterSignal(const j_string_t &strHostId, const j_string_t &strSigName)
{
	return J_OK;
}

j_reuslt_t CJoBus::SyncInvoke(const j_string_t &strHostId, const j_string_t &strMethodName, void *pParam, void **pResult)
{
	return J_OK;
}

j_reuslt_t CJoBus::AsyncInvoke(const j_string_t &strHostId, const j_string_t &strMethodName, void *pParam, JoBusCallback callback)
{
	return J_OK;
}