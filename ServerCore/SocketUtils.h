#pragma once

#include "NetworkAddress.h"

/*-------------------
	 SocketUtils
-------------------*/

class SocketUtils
{
public:
	static LPFN_CONNECTEX		ConnectEx;
	static LPFN_DISCONNECTEX	DisconnectEx;
	static LPFN_ACCEPTEX		AcceptEx;

public:
	static void Init();
	static void Clear();

	static BOOL BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* function);
	static SOCKET CreateSocket();

	static BOOL SetLinger(SOCKET socket, uint16 OnOff, uint16 linger);
	static BOOL SetReuseAddress(SOCKET socket, BOOL flag);
	static BOOL SetRecvBufferSize(SOCKET socket, int32 size);
	static BOOL SetSendBufferSize(SOCKET socket, int32 size);
	static BOOL SetTCPNoDelay(SOCKET socket, BOOL flag);

	static BOOL SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket);

	static BOOL Bind(SOCKET socket, NetworkAddress netAddr);
	static BOOL BindAnyAddress(SOCKET socket, uint16 port);
	static BOOL Listen(SOCKET socket, int32 backlog = SOMAXCONN);
	static void Close(SOCKET& socket);
};

template<typename T>
static inline BOOL SetSocketOption(SOCKET socket, int32 level, int32 optionName, T optionValue)
{
	return SOCKET_ERROR != ::setsockopt(socket, level, optionName, reinterpret_cast<const char*>(&optionValue), sizeof(T));
}