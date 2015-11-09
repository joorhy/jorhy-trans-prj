#ifndef __JO_COMMON_H_
#define __JO_COMMON_H_

#include "j_obj.h"
#define JO_DECLARE_SINGLETON(_name) \
	extern JO_API C##_name* Single_##_name; \
	extern JO_API C##_name* X_JO_API Get##_name##Layer();  

#define  JO_IMPLEMENT_SINGLETON(_name) \
	C##_name* Single_##_name = NULL; \
	C##_name* X_JO_API Get##_name##Layer() \
	{\
		if (Single_##_name == NULL) \
			Single_##_name = new C##_name(); \
		return Single_##_name; \
	}

#define JO_INSTANSE(_name) \
	Get##_name##Layer()

///时间 Singleton对象
#define JoTime \
	JO_INSTANSE(Time)
///设备管理Singleton对象
#define JoDeviceManager \
	JO_INSTANSE(DeviceManager)
///客户端管理Singleton对象
#define JoClientManager \
	JO_INSTANSE(ClientManager)
///据库接口对象
#define JoDataBaseObj \
	JO_INSTANSE(MySQLAccess)

///FTP服务器管理对象
#define JoHostManager \
	JO_INSTANSE(HostManager)
///数据总线对象
#define JoDataBus \
	JO_INSTANSE(DataBus)

#include "x_errtype.h"

typedef bool 								j_boolean_t;
typedef long long						    j_time_t;
typedef char								j_char_t;
typedef int									j_result_t;
typedef long								j_long_t;
typedef void								j_void_t;

typedef char 								j_int8_t;
typedef unsigned char					    j_uint8_t;
typedef short 								j_int16_t;
typedef unsigned short 						j_uint16_t;
typedef int 								j_int32_t;
typedef unsigned int 					    j_uint32_t;
typedef long long 						    j_int64_t;
typedef unsigned long long 			        j_uint64_t;
typedef float								j_float_t;

typedef std::string							j_string_t;	
typedef std::vector<j_string_t>		        j_vec_str_t;
typedef std::wstring						j_wstring_t;

typedef unsigned char					    j_uuid_t[16];

#ifdef WIN32
#define j_thread_t						HANDLE
#define j_invalid_thread_val		NULL
#define j_invalid_filemap_val		NULL
#define j_invalid_module_val		NULL
#define j_invalid_socket_val		INVALID_SOCKET
#else
#define ULONG_MAX				0xffffffffUL
#define j_thread_t	 pthread_t
#define j_invalid_thread_val		0
#define j_invalid_filemap_val		-1
#define j_invalid_module_val		NULL
#define j_invalid_socket_val		-1
#endif

#ifdef WIN32
#define j_sleep(x)	Sleep(x)
#define j_close_socket(x)	closesocket(x); 
#else
#define j_sleep(x)	usleep(x*1000)
#define j_close_socket(x)	close(x)
#endif

#ifdef WIN32
#define j_atoll(x) _atoi64(x)
#else
#define j_atoll(x) atoll(x)
#endif

#ifdef WIN32
typedef unsigned (X_JO_API *j_thread_entry_t)(void*);
#else
typedef void *(*j_thread_entry_t)(void *); 
#endif 

typedef struct
{
#ifdef WIN32
	CRITICAL_SECTION mutex;
#else
	pthread_mutex_t mutex;
#endif
}j_mutex_t;

typedef struct
{
#ifdef WIN32
	HANDLE  hFile;
	char *flock;
#else
	int hFile;
	struct flock flock;
#endif
}j_filemap_t;

typedef struct
{
#ifdef WIN32
	HANDLE   handle;
#else
	pthread_cond_t   handle;
	pthread_mutex_t mutex;
#endif
}j_cond_t;

typedef struct  
{
#ifdef WIN32
	HMODULE handle;
#else
	void *handle;
#endif
} j_module_t; 

#ifdef WIN32
typedef SOCKET j_asio_handle;
#else
typedef int j_asio_handle;
#endif

typedef struct j_socket
{
	j_socket()
	{
		sock = j_invalid_socket_val;
	}
	j_socket(j_asio_handle s)
	{
		sock = s;
	}
	j_asio_handle sock;
	j_boolean_t operator < (const j_socket &other) const
	{
		return (sock < other.sock);
	}
} j_socket_t;
#define j_invalid_socket j_socket_t

typedef struct j_guid
{
	j_guid(GUID guid)
	{
		m_guid = guid;
	}
	/*GUID j_guid()
	{
		return m_guid;
	}*/
	bool operator<(const j_guid &other) const
	{
		bool b_ret = false;
		if (m_guid.Data1 != other.m_guid.Data1)
			b_ret = m_guid.Data1 < other.m_guid.Data1;
		else if (m_guid.Data2 != other.m_guid.Data2)
			b_ret = m_guid.Data2 < other.m_guid.Data2;
		else if (m_guid.Data3 != other.m_guid.Data3)
			b_ret = m_guid.Data3 < other.m_guid.Data3;
		else if (m_guid.Data4[0] != other.m_guid.Data4[0])
			b_ret = m_guid.Data4[0] < other.m_guid.Data4[0];
		else if (m_guid.Data4[1] != other.m_guid.Data4[1])
			b_ret = m_guid.Data4[1] < other.m_guid.Data4[1];
		else if (m_guid.Data4[2] != other.m_guid.Data4[2])
			b_ret = m_guid.Data4[2] < other.m_guid.Data4[2];
		else if (m_guid.Data4[3] != other.m_guid.Data4[3])
			b_ret = m_guid.Data4[3] < other.m_guid.Data4[3];

		return b_ret;
	}
	GUID m_guid;
} j_guid_t;

struct J_AsioDataBase
: public OVERLAPPED
{
	/// 执行的异步IO调用
	enum J_IoCall
	{
		j_keep_e = 0,
		j_accept_e = 1,
		j_connect_e,
		j_disconnect_e,
		j_read_e,
		j_write_e,
		j_write_keep_e,
	};

	/// 执行的异步IO调用类型
	enum J_IoType
	{
		j_command_e = 1,
		j_data_e,
	};

	struct J_IoAccept
	{
		j_asio_handle subHandle;		///< Accept调用完成后得到的handle
		j_uint32_t peerIP;				///< 对端IP地址
		j_int32_t peerPort;				///< 对端端口
	};

	struct J_IoRead
	{
		j_char_t *buf;					///< 接收数据缓冲区
		j_int32_t bufLen;				///< 接收数据缓冲区字节数
		j_boolean_t whole;				///< TRUE-等到全部数据接收完才算成功, FALSE-接收到数据立即成功
		j_boolean_t shared;				///< 
		j_char_t until_buf[16];

		j_boolean_t peerClosed;			///< TRUE-对端已经关闭, FALSE-没有
		j_int32_t finishedLen;			///< 已经接收的数据字节数
	};

	struct J_IoWrite
	{
		j_boolean_t whole;				///< TRUE-等到全部数据发送完才算成功, FALSE-发送部分数据立即成功
		j_boolean_t shared;				///< 
		const j_char_t *buf;			///< 发送数据缓冲区
		j_int32_t bufLen; 				///< 发送数据缓冲区字节数

		j_int32_t finishedLen; 			///< 已经发送的数据字节数
	};

	j_asio_handle ioHandle;				///异步Io句柄
	J_Obj *ioUser;						///< 异步Io使用者
	J_IoCall ioCall;					/// 执行的异步Io调用
	J_IoType ioType;					/// 执行的异步Io调用类型
	union
	{
		J_IoAccept ioAccept;
		J_IoRead ioRead;
		J_IoWrite ioWrite;
	};
};

template <class k1, class k2>
class compaird_obj
{

};

#endif //~__JO_COMMON_H_
