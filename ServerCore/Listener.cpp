#include "pch.h"
#include "Listener.h"
#include "SocketUtils.h"
#include "IOCPEvent.h"
#include "Session.h"


/*----------------
	 Listener
----------------*/

Listener::~Listener()
{
	SocketUtils::Close(_socket);

	for (AcceptEvent* acceptEvent : _acceptEvents)
	{
		// TODO: 

		xdelete(acceptEvent);
	}
}

bool Listener::StartAccept(NetworkAddress netAddress)
{
	_socket = SocketUtils::CreateSocket();
	if (INVALID_SOCKET == _socket)
		return false;

	if (false == GIOCPCore.Register(this))
		return false;

	if (FALSE == SocketUtils::SetReuseAddress(_socket, TRUE))
		return false;

	if (FALSE == SocketUtils::SetLinger(_socket, 0, 0))
		return false;

	if (FALSE == SocketUtils::Bind(_socket, netAddress))
		return false;

	if (FALSE == SocketUtils::Listen(_socket))
		return false;

	const int32 acceptCount = 1;
	for (int32 i = 0; i < acceptCount; ++i)
	{
		AcceptEvent* acceptEvent = xnew<AcceptEvent>();
		_acceptEvents.push_back(acceptEvent);
		RegisterAccept(acceptEvent);
	}

	return true;
}

void Listener::CloseSocket()
{
	SocketUtils::Close(_socket);
}

HANDLE Listener::GetHandle()
{
	return reinterpret_cast<HANDLE>(_socket);
}

void Listener::Dispatch(IOCPEvent* iocpEvent, int32 numOfBytes)
{
	ASSERT_CRASH(EventType::ACCEPT == iocpEvent->GetType());

	AcceptEvent* acceptEvent = static_cast<AcceptEvent*>(iocpEvent);
	ProcessAccept(acceptEvent);
}

void Listener::RegisterAccept(AcceptEvent* acceptEvent)
{
	Session* session = xnew<Session>();

	acceptEvent->Init();
	acceptEvent->SetSession(session);

	DWORD bytesReceived = 0;
	BOOL ret = SocketUtils::AcceptEx(
		_socket, session->GetSocket(), session->_recvBuffer, 0,
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16,
		OUT &bytesReceived, static_cast<LPOVERLAPPED>(acceptEvent));

	if (FALSE == ret)
	{
		const int32 errorCode = ::WSAGetLastError();
		if (WSA_IO_PENDING != errorCode)
		{
			// 오류가 발생했지만, 일단 다시 accept를 걸어준다.
			RegisterAccept(acceptEvent);
		}
	}
}

void Listener::ProcessAccept(AcceptEvent* acceptEvent)
{
	Session* session = acceptEvent->GetSession();

	if (FALSE == SocketUtils::SetUpdateAcceptSocket(session->GetSocket(), _socket))
	{
		RegisterAccept(acceptEvent);
		return;
	}

	SOCKADDR_IN sockAddress = { 0, };
	int32 sizeOfSockAddr = sizeof(sockAddress);
	if (SOCKET_ERROR == getpeername(session->GetSocket(), OUT reinterpret_cast<SOCKADDR*>(&sockAddress), &sizeOfSockAddr))
	{
		RegisterAccept(acceptEvent);
		return;
	}

	session->SetNetworkAddress(NetworkAddress(sockAddress));

	std::cout << "Client Connected!!" << std::endl;

	// TODO: 

	RegisterAccept(acceptEvent);
}
