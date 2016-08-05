#include "stdafx.h"
#include "TcpSocket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

TcpSocket::TcpSocket()
  : m_socket(INVALID_SOCKET) 
{
	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2, 2);
	if (::WSAStartup(sockVersion, &wsaData) != 0)
	{
		exit(0);
	}
}

TcpSocket::~TcpSocket() 
{
	::WSACleanup();
}

bool TcpSocket::Connect(LPCTSTR address, int port, int milliseconds/* = 2000*/) 
{
	// 创建TCP协议的流套接字
	m_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_socket == INVALID_SOCKET)
	{
		SetLastError(WSAGetLastError());
		return false;
	}

	// 将套接字设为阻塞模式
	unsigned long ul = 1;
	ioctlsocket(m_socket, FIONBIO, &ul);

	sockaddr_in addr;// 需要连接的目标地址
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = inet_addr(CT2A(address));
	addr.sin_port = htons(port);

	if (connect(m_socket, (const sockaddr *)&addr, sizeof(addr)) != 0) 
	{
		timeval tm;
		tm.tv_sec = 0;
		tm.tv_usec = milliseconds * 1000;
		fd_set set;
		FD_ZERO(&set);
		FD_SET(m_socket, &set);
		bool success = false;
		if (select(m_socket + 1, NULL, &set, NULL, &tm) > 0)
		{
			int error = -1, len = sizeof(int);
			getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (char *)&error, &len);
			success = (error == 0);
		}
		if (!success) 
		{
			Close();
			SetLastError(ERROR_TIMEOUT);
			return false;
		}
	}

	ul = 0;
	ioctlsocket(m_socket, FIONBIO, &ul);

	return true;
}

bool TcpSocket::Listen(LPCTSTR address, int port) 
{
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = inet_addr(CT2A(address));
	addr.sin_port = htons(port);

	return Listen((const sockaddr *)&addr, sizeof(addr));
}

bool TcpSocket::Listen(const sockaddr *addr, int addrLength) 
{
	m_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (m_socket == INVALID_SOCKET) 
	{
		SetLastError(WSAGetLastError());
		return false;
	}

	if (bind(m_socket, addr, addrLength) == SOCKET_ERROR) 
	{
		SetLastError(WSAGetLastError());
		return false;
	}

	if (listen(m_socket, 1) == SOCKET_ERROR) 
	{
		SetLastError(WSAGetLastError());
		return false;
	}

	return true;
}

SOCKET TcpSocket::Accept(int waitMilliseconds) 
{
	// 由于Accept函数均在Listen函数之后执行，而listen函数中已经创建套接字和绑定地址
	timeval tm;
	tm.tv_sec = 0;
	tm.tv_usec = waitMilliseconds * 1000;
	fd_set set;
	FD_ZERO(&set);
	FD_SET(m_socket, &set);
	int ret = select(m_socket + 1, &set, NULL, NULL, &tm);
	if (ret > 0) 
	{
		sockaddr_in addr;
		int addrLen = sizeof(addr);
		SOCKET s = accept(m_socket, (sockaddr *)&addr, &addrLen);
		if (s == INVALID_SOCKET) 
		{
			SetLastError(WSAGetLastError());
			return INVALID_SOCKET;
		}
		return s;
	} 
	else 
	{
		SetLastError(ret == 0 ? WSAETIMEDOUT : WSAGetLastError());
		return INVALID_SOCKET;
	}
}

void TcpSocket::Attach(SOCKET s) 
{
	Close();
	m_socket = s;
}

bool TcpSocket::SetTimeout(int milliseconds) 
{
	DWORD timeout = milliseconds;

	if (setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout)) == SOCKET_ERROR ||
	  setsockopt(m_socket, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout, sizeof(timeout)) == SOCKET_ERROR) 
	{
		return false;
	}
	return true;
}

bool TcpSocket::SetSendBufferSize(int size) 
{
	if (setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, (const char *)&size, sizeof(size)) == SOCKET_ERROR) 
	{
		return false;
	}
	return true;
}

bool TcpSocket::SetRecvBufferSize(int size) 
{
	if (setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (const char *)&size, sizeof(size)) == SOCKET_ERROR) 
	{
		return false;
	}
	return true;
}

int TcpSocket::Receive(void *buffer, int size) 
{  
	return recv(m_socket, (char *)buffer, size, 0);
}

bool TcpSocket::ReceiveFully(void *buffer, int size) 
{
	int received = 0;
	while (received < size) 
	{
		int ret = recv(m_socket, (char *)buffer + received, size - received, 0);
		if (ret == -1) 
		{
			SetLastError(WSAGetLastError());
			return false;
		}
		if (ret == 0) 
		{
			SetLastError(ERROR_NO_MORE_ITEMS);
			return false;
		}
		received += ret;
	}
	return true;
}

bool TcpSocket::Send(const void *data, int size) 
{
	int ret = send(m_socket, (const char *)data, size, 0);

	return ret == size;
}

void TcpSocket::Close() 
{
	if (m_socket != INVALID_SOCKET) 
	{
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}
}

bool TcpSocket::IsOpened() const 
{
	return m_socket != INVALID_SOCKET;
}

bool TcpSocket::GetAddress(sockaddr *addr, int *addrLen) 
{
	if (getsockname(m_socket, addr, addrLen) == SOCKET_ERROR) 
	{
		SetLastError(WSAGetLastError());
		return false;
	}
	return true;
}

bool TcpSocket::ListenClosed()
{
	timeval tm;
	tm.tv_sec = 0;
	tm.tv_usec = 100000; //100ms
	fd_set set;
	FD_ZERO(&set);
	FD_SET(m_socket, &set);
	int ret = select(m_socket + 1, &set, NULL, NULL, &tm);
	if (ret == 1)
	{
		char* buf=new char[20];
		if (Receive(buf, 20) <= 0 /*&& WSAGetLastError() == EINTR*/)
		{
			SetLastError(WSAGetLastError());
			delete buf;
			buf = NULL;
			return true;//已断开连接
		}
	}
	else
		return false;
}