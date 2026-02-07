#include "pch.h"
#include "SocketUtils.h"

/*-------------------
	 SocketUtils
-------------------*/


LPFN_CONNECTEX		SocketUtils::ConnectEx		= nullptr;
LPFN_DISCONNECTEX	SocketUtils::DisconnectEx	= nullptr;
LPFN_ACCEPTEX		SocketUtils::AcceptEx		= nullptr;

void SocketUtils::Init()
{
	WSADATA wsaData;
	ASSERT_CRASH(0 == ::WSAStartup(MAKEWORD(2, 2), OUT &wsaData));

	// 런타임에 extension 함수의 주소를 얻어온다 //
	SOCKET dummySocket = CreateSocket();
	ASSERT_CRASH(BindWindowsFunction(dummySocket, WSAID_CONNECTEX, reinterpret_cast<LPVOID*>(&ConnectEx)));
	ASSERT_CRASH(BindWindowsFunction(dummySocket, WSAID_DISCONNECTEX, reinterpret_cast<LPVOID*>(&DisconnectEx)));
	ASSERT_CRASH(BindWindowsFunction(dummySocket, WSAID_ACCEPTEX, reinterpret_cast<LPVOID*>(&AcceptEx)));

	Close(dummySocket);
}

void SocketUtils::Clear()
{
	::WSACleanup();
}

BOOL SocketUtils::BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* function)
{
	DWORD bytes = 0;

	// ConnectEx, DisconnectEx, AcceptEx의 함수 포인터를 런타임에 불러오기 위한 함수
	return SOCKET_ERROR != ::WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), function, sizeof(*function), OUT &bytes, NULL, NULL);
}

SOCKET SocketUtils::CreateSocket()
{
	return ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
}

BOOL SocketUtils::SetLinger(SOCKET socket, uint16 OnOff, uint16 linger)
{
	LINGER option = { 0, };
	option.l_linger = linger;
	option.l_onoff = OnOff;

	return SetSocketOption(socket, SOL_SOCKET, SO_LINGER, option);
}

BOOL SocketUtils::SetReuseAddress(SOCKET socket, BOOL flag)
{
	return SetSocketOption<BOOL>(socket, SOL_SOCKET, SO_REUSEADDR, flag);
}

BOOL SocketUtils::SetRecvBufferSize(SOCKET socket, int32 size)
{
	return SetSocketOption<int32>(socket, SOL_SOCKET, SO_RCVBUF, size);
}

BOOL SocketUtils::SetSendBufferSize(SOCKET socket, int32 size)
{
	return SetSocketOption<int32>(socket, SOL_SOCKET, SO_SNDBUF, size);
}

BOOL SocketUtils::SetTCPNoDelay(SOCKET socket, BOOL flag)
{
	return SetSocketOption<BOOL>(socket, IPPROTO_TCP, TCP_NODELAY, flag);
}

BOOL SocketUtils::SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket)
{
	// listen socket의 특성을 client socket에 그대로 적용한다는 의미 //
	return SetSocketOption<SOCKET>(socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, listenSocket);
}

BOOL SocketUtils::Bind(SOCKET socket, NetworkAddress netAddr)
{
	return SOCKET_ERROR != ::bind(socket, reinterpret_cast<const sockaddr*>(&netAddr.GetSocketAddress()), sizeof(SOCKADDR_IN));
}

BOOL SocketUtils::BindAnyAddress(SOCKET socket, uint16 port)
{
	SOCKADDR_IN myAddress = {};

	::memset(&myAddress, 0, sizeof(myAddress));

	myAddress.sin_family = AF_INET;
	myAddress.sin_addr.s_addr = ::htonl(INADDR_ANY);
	myAddress.sin_port = ::htons(port);

	return SOCKET_ERROR != ::bind(socket, reinterpret_cast<const sockaddr*>(&myAddress), sizeof(myAddress));
}

BOOL SocketUtils::Listen(SOCKET socket, int32 backlog)
{
	return SOCKET_ERROR != ::listen(socket, backlog);
}

void SocketUtils::Close(SOCKET& socket)
{
	if (INVALID_SOCKET != socket)
		::closesocket(socket);
	socket = INVALID_SOCKET;
}
