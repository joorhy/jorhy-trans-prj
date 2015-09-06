#ifndef __X_JOBUS_H_
#define __X_JOBUS_H_

typedef j_reuslt_t (* JoBusCallback)(const j_string_t &strSigName, void *pBuffer);

class CJoBus
{
public:
	CJoBus();
	~CJoBus();

public:
	/// 注册消息事件
	/// @param[in]	strHostId 设备ID
	/// @param[in]	strSigName 消息名称
	/// @param[in]	callback 消息回调函数
	/// @return		参见j_errtype.h
	j_reuslt_t RegisterSignal(const j_string_t &strHostId, const j_string_t &strSigName, JoBusCallback callback);
	/// 删除消息事件
	/// @param[in]	strHostId 设备ID
	/// @param[in]	strSigName 消息名称
	/// @return		参见j_errtype.h
	j_reuslt_t UnRegisterSignal(const j_string_t &strHostId, const j_string_t &strSigName);
	/// 同步调用
	/// @param[in]	strHostId 设备ID
	/// @param[in]	strMethodName 方法名称
	/// @param[in]	pParam 方法参数
	/// @param[out]	pResult 方法返回数据
	/// @return		参见j_errtype.h
	j_reuslt_t SyncInvoke(const j_string_t &strHostId, const j_string_t &strMethodName, void *pParam, void **pResult);
	/// 异步调用
	/// @param[in]	strHostId 设备ID
	/// @param[in]	strMethodName 方法名称
	/// @param[in]	pParam 方法参数
	/// @param[in]	callback 方法返回回调函数
	/// @return		参见j_errtype.h
	j_reuslt_t AsyncInvoke(const j_string_t &strHostId, const j_string_t &strMethodName, void *pParam, JoBusCallback callback);
};

#endif __X_JOBUS_H_