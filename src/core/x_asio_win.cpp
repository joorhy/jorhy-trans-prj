#include "x_asio_win.h"
#include "x_errtype.h"
#include "j_module.h"
#include "x_socket.h"

JO_IMPLEMENT_SINGLETON(XAsio)

#define TCP_PACKET_SIZE 1024

J_OS::CTLock CXAsio::m_user_locker;

CXAsio::CXAsio()
{
	m_bStarted = false;
	WSADATA wsaData; 
	WSAStartup(MAKEWORD(2,2), &wsaData);
	Init();
}

CXAsio::~CXAsio()
{
	Deinit();
	WSACleanup();
}

int CXAsio::Init()
{
	if (!m_bStarted)
	{
		m_hCompletionPort = INVALID_HANDLE_VALUE;
		if ((m_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)) == NULL)
		{
			J_OS::LOGINFO( "CRdAsio::Init() CreateIoCompletionPort failed with error: %d\n", GetLastError());
			return J_SOCKET_ERROR;
		} 

		m_bStarted = true;

		j_thread_parm parm = {0};
		parm.entry = CXAsio::WorkThread;
		parm.data = this;
		m_workThread.Create(parm);

		parm.entry = CXAsio::WriteThread;
		parm.data = this;
		m_writeThread.Create(parm);
	}
	return J_OK;
}

void CXAsio::Deinit()
{
	if (m_bStarted)
	{
		m_bStarted = false;
		m_workThread.Release();
		m_writeThread.Release();
		m_listenThread.Release();

		if (m_hCompletionPort != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_hCompletionPort);
			m_hCompletionPort = INVALID_HANDLE_VALUE;
		}
	}
}

int CXAsio::Listen(J_AsioDataBase *pAsioData)
{
	if (pAsioData->ioUser != NULL)
	{
		j_socket_t nSocket;
		nSocket.sock = socket(AF_INET, SOCK_STREAM, 0);

		int retval;
		setsockopt(nSocket.sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&retval, sizeof(int));
		J_OS::CTCPSocket *tcpSocket = new J_OS::CTCPSocket(nSocket);
		if (tcpSocket->Listen(nSocket, pAsioData->ioAccept.peerPort, 1024, false) != J_OK)
			return J_UNKNOW;

		m_listenSocket = nSocket;
		m_listenAsioData = pAsioData;
		
		j_thread_parm parm = {0};
		parm.entry = CXAsio::ListenThread;
		parm.data = this;
		m_listenThread.Create(parm);
	}
	return J_OK;
}

int CXAsio::AddUser(j_socket_t nSocket, J_AsioUser *pUser)
{
	if (CreateIoCompletionPort((HANDLE)nSocket.sock, m_hCompletionPort, (DWORD)0, 1) == NULL)
	{
		J_OS::LOGINFO("CXAsio::AddUser CreateIoCompletionPort failed with error %d\n", WSAGetLastError());
		return J_SOCKET_ERROR;
	} 

	AsioUserMap::iterator it = m_userMap.find(nSocket);
	if (it != m_userMap.end())
	{
		it->second.nRef = 1;
		it->second.pAsioUser = pUser;
	}
	else
	{
		UserInfo info;
		info.pAsioUser = pUser;
		info.nRef = 1;
		m_userMap[nSocket] = info;
	}

	J_OS::LOGINFO("CXAsio::AddUser epoll set insertion sucess fd = %d", nSocket.sock);

	return J_OK;
}

void CXAsio::DelUser(j_socket_t nSocket)
{
	AsioUserMap::iterator it = m_userMap.find(nSocket);
	if (it != m_userMap.end())
	{
		it->second.nRef = 0;
	}
	J_OS::LOGINFO("CXAsio::DelUser epoll set insertion sucess fd = %d", nSocket.sock);

	//CancelIo((HANDLE)nSocket.sock);
	j_close_socket(nSocket.sock);
}

void CXAsio::OnWork()
{
	DWORD dwBytesTransferred;
	LPOVERLAPPED Overlapped;
	DWORD dwFlag = 0;
	J_AsioDataBase *pPerIoData;        
	DWORD SendBytes, RecvBytes;
	DWORD Flags;
	while (m_bStarted)
	{
		//SetLastError(ERROR_SUCCESS);
		if (GetQueuedCompletionStatus(m_hCompletionPort, &dwBytesTransferred, (LPDWORD)&dwFlag, (LPOVERLAPPED *)&pPerIoData, 200) == 0)
		{
			CXAsio::LockSlice();
			if (WSAGetLastError() != WAIT_TIMEOUT && WSAGetLastError() != ERROR_CONNECTION_ABORTED)
			{
				if (pPerIoData != NULL && IsValidUser(pPerIoData->ioHandle))
				{
					DelUserRef(pPerIoData->ioHandle);
					CancelIoEx((HANDLE)pPerIoData->ioHandle, pPerIoData);
					//shutdown(pPerIoData->ioHandle, SD_BOTH);
					J_OS::LOGINFO("CXAsio::OnWork GetQueuedCompletionStatus failed with error %d %d %d", WSAGetLastError(), pPerIoData, pPerIoData->ioHandle);
					ProcessIoEvent(pPerIoData->ioHandle, J_AsioDataBase::j_disconnect_e);
				}
			}
			CXAsio::UnlockSlice();
		}
		else
		{
			CXAsio::LockSlice();
			// 检查数据传送完了吗
			if (dwBytesTransferred == 0)
			{
				DelUserRef(pPerIoData->ioHandle);
				J_OS::LOGINFO("CXAsio::OnWork Broken %d sock=%d", WSAGetLastError(), pPerIoData->ioHandle);
				ProcessIoEvent(pPerIoData->ioHandle, J_AsioDataBase::j_disconnect_e);
			}
			else
			{
				DelUserRef(pPerIoData->ioHandle);
				if (pPerIoData->ioCall == J_AsioDataBase::j_read_e)
				{
					try
					{
						J_AsioUser *pAsioUser = dynamic_cast<J_AsioUser *>((J_Obj *)pPerIoData->ioUser);
						if (pAsioUser != NULL)
						{
							pPerIoData->ioRead.finishedLen += dwBytesTransferred;
							if ((pPerIoData->ioRead.finishedLen < pPerIoData->ioRead.bufLen) && pPerIoData->ioRead.whole)
							{
								Read(pPerIoData->ioHandle, pPerIoData);
							}
							else
							{
								pAsioUser->OnRead(pPerIoData, J_OK);
							}
						}
					}
					catch (...)
					{
						J_OS::LOGINFO("ASIO Read except");
					}
				}
			}
			CXAsio::UnlockSlice();
		}
	}
}

void CXAsio::OnWrite()
{
	timeval timeout;				// 定时变量 
	timeout.tv_sec = 0;
	timeout.tv_usec = 1;

	fd_set rfd;						// 读描述符集 
	SOCKET sWait = socket(AF_INET, SOCK_DGRAM, 0);
	while (m_bStarted)
	{
		//j_sleep(1);
		FD_ZERO(&rfd);				// 在使用之前总是要清空  
		FD_SET(sWait, &rfd);
		select(1, &rfd, NULL, NULL, &timeout);

		CXAsio::LockSlice();
		J_AsioUser *pAsioUser = NULL;
		AsioUserMap::iterator itUser = m_userMap.begin();
		for (; itUser != m_userMap.end();)
		{
			pAsioUser = dynamic_cast<J_AsioUser *>(itUser->second.pAsioUser);
			if (pAsioUser != NULL && itUser->second.nRef > 0)
			{
				J_AsioDataBase asioData;
				asioData.ioHandle = itUser->first.sock;
				pAsioUser->OnWrite(&asioData, J_OK);
				itUser++;
			}
			else
			{
				m_userMap.erase(itUser++);
			}
		}
		CXAsio::UnlockSlice();
	}
	closesocket(sWait);
}

void CXAsio::OnListen()
{
	while (true)
	{
		j_socket_t active_fd;
		struct sockaddr_in sonnAddr;
		socklen_t connLen = sizeof(sockaddr_in);
		j_asio_handle connSocket = j_invalid_socket_val;
		if ((connSocket = WSAAccept(m_listenSocket.sock, (struct sockaddr*)&sonnAddr, &connLen, NULL, NULL)) == j_invalid_socket_val)
		{
			J_OS::LOGERROR("CXAsio::OnListen WSAAccept() failed with error");
			if (GetLastError() == WSAEINTR )
				continue;

			return;
		} 

		CXAsio::LockSlice();
		active_fd.sock = connSocket;
		if (IsValidUser(active_fd))
		{
			J_OS::LOGINFO("CXAsio::OnListen() exception sock = %d Error = %d", connSocket, GetLastError());
			ProcessIoEvent(active_fd, J_AsioDataBase::j_disconnect_e);
		}
		else
		{
			m_listenAsioData->ioAccept.subHandle = connSocket;
			m_listenAsioData->ioAccept.peerIP = sonnAddr.sin_addr.s_addr;
			m_listenAsioData->ioAccept.peerPort = sonnAddr.sin_port;

			unsigned long ul = 1;
			ioctlsocket(active_fd.sock, FIONBIO, (unsigned long*)&ul);
			EnableKeepalive(active_fd);

			ProcessAccept(active_fd, m_listenAsioData);
		}
		CXAsio::UnlockSlice();
	}
}
	
void CXAsio::EnableKeepalive(j_socket_t sock)
{
	//开启tcp探测
	int keepAlive = 1; 		// 开启keepalive属性
	setsockopt(sock.sock, SOL_SOCKET, SO_KEEPALIVE, (const char *)&keepAlive, sizeof(keepAlive));
	tcp_keepalive alive_in               = {0};
	tcp_keepalive alive_out              = {0};
	alive_in.keepalivetime               = 3000;                // 开始首次KeepAlive探测前的TCP空闭时间
	alive_in.keepaliveinterval			 = 1000;				// 两次KeepAlive探测间的时间间隔
	alive_in.onoff                       = TRUE;
	unsigned long ulBytesReturn = 0;
	WSAIoctl(sock.sock, SIO_KEEPALIVE_VALS, &alive_in, sizeof(alive_in), &alive_out, sizeof(alive_out), &ulBytesReturn, NULL, NULL);
}

int CXAsio::Read(j_socket_t nSocket, J_AsioDataBase *pAsioData)
{
	AsioUserMap::iterator it = m_userMap.find(nSocket);
	if (it == m_userMap.end())
	{
		return J_NOT_EXIST;
	}
	
	DWORD Flags = 0;
	WSABUF buf;
	DWORD dwFinishedLen = 0;
	buf.len = pAsioData->ioRead.bufLen;
	pAsioData->ioHandle = nSocket.sock;

	buf.len = pAsioData->ioRead.bufLen - pAsioData->ioRead.finishedLen;
	buf.buf = pAsioData->ioRead.buf + pAsioData->ioRead.finishedLen;
	if (WSARecv(nSocket.sock, &buf, 1, (LPDWORD)&dwFinishedLen, &Flags, pAsioData, NULL) == SOCKET_ERROR
		&& GetLastError() != ERROR_IO_PENDING)
	{
		ProcessIoEvent(nSocket, J_AsioDataBase::j_disconnect_e);
	}
	else
	{
		AddUserRef(nSocket);
	}

	return J_OK;
}

int CXAsio::Write(j_socket_t nSocket, J_AsioDataBase *pAsioData)
{
	AsioUserMap::iterator it = m_userMap.find(nSocket);
	if (it == m_userMap.end())
	{
		return J_NOT_EXIST;
	}

	int bufCount = (pAsioData->ioWrite.bufLen + TCP_PACKET_SIZE - 1) / TCP_PACKET_SIZE;
	if (bufCount == 0)
	{
		bufCount = 1;
	}

	WSABUF *lpBuffer = (WSABUF *)malloc(sizeof(WSABUF) * bufCount);
	for (int i=0; i<bufCount; i++)
	{
		lpBuffer[i].buf = (CHAR *)pAsioData->ioWrite.buf + (i * TCP_PACKET_SIZE);
		if (i == bufCount-1)
		{
			lpBuffer[i].len = pAsioData->ioWrite.bufLen - i * TCP_PACKET_SIZE;
		}
		else
		{
			lpBuffer[i].len = TCP_PACKET_SIZE;
		}
	}

	pAsioData->ioHandle = nSocket.sock;
	pAsioData->ioCall = J_AsioDataBase::j_write_e;
	if (WSASend(nSocket.sock, lpBuffer, bufCount, (LPDWORD)&pAsioData->ioWrite.finishedLen, 0, pAsioData, NULL) == SOCKET_ERROR
		&& GetLastError() != ERROR_IO_PENDING)
	{
		J_OS::LOGINFO("CXAsio::Write WSASend error = %d", GetLastError());
		ProcessIoEvent(nSocket, J_AsioDataBase::j_disconnect_e);
	}
	else
	{
		AddUserRef(nSocket);
	}
	free(lpBuffer);

	return J_OK;
}

int CXAsio::ProcessAccept(j_socket_t nSocket, J_AsioDataBase *asioData)
{
	J_AsioUser *pAsioUser = dynamic_cast<J_AsioUser *>(asioData->ioUser);
	if (pAsioUser != NULL)
	{
		pAsioUser->OnAccept(asioData, J_OK);
	}

	return J_OK;
}

int CXAsio::ProcessIoEvent(j_socket_t nSocket, int nType)
{
	switch (nType)
	{
		case J_AsioDataBase::j_disconnect_e:
		{
			J_AsioUser *pAsioUser = NULL;
			AsioUserMap::iterator itUser = m_userMap.find(nSocket);
			if (itUser != m_userMap.end())
			{
				pAsioUser = dynamic_cast<J_AsioUser *>(itUser->second.pAsioUser);
			}

			J_OS::LOGINFO("CXAsio::ProcessIoEvent user ref = %d", itUser->second.nRef);
			if (pAsioUser != NULL)
			{
				if (itUser->second.nRef <= 1)
				{
					J_AsioDataBase asioData;
					asioData.ioHandle = nSocket.sock;
					pAsioUser->OnBroken(&asioData, J_SOCKET_ERROR);
				}
			}
			break;
		}
	}
		
	return J_OK;
}

bool CXAsio::IsValidUser(j_socket_t nSocket)
{
	bool bIsValidUser = false;
	AsioUserMap::iterator it = m_userMap.find(nSocket);
	if (it != m_userMap.end() && it->second.nRef > 0)
	{
		bIsValidUser = true;
	}

	return bIsValidUser;
}

void CXAsio::AddUserRef(j_socket_t nSocket)
{
	AsioUserMap::iterator it = m_userMap.find(nSocket);
	if (it != m_userMap.end())
	{
		it->second.nRef++;
	}
}

void CXAsio::DelUserRef(j_socket_t nSocket)
{
	AsioUserMap::iterator it = m_userMap.find(nSocket);
	if (it != m_userMap.end())
	{
		it->second.nRef--;
	}
}

