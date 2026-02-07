#include "pch.h"
#include "Listener.h"
#include "SocketUtils.h"
#include "IOCPEvent.h"
#include "Session.h"
#include "Service.h"


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

bool Listener::StartAccept(ServerServiceRef service)
{
	_service = service;
	if (nullptr == _service)
		return false;

	_socket = SocketUtils::CreateSocket();
	if (INVALID_SOCKET == _socket)
		return false;

	if (false == _service->GetIOCPCore()->Register(shared_from_this()))
		return false;

	if (FALSE == SocketUtils::SetReuseAddress(_socket, TRUE))
		return false;

	if (FALSE == SocketUtils::SetLinger(_socket, 0, 0))
		return false;

	if (FALSE == SocketUtils::Bind(_socket, _service->GetNetAddress()))
		return false;

	if (FALSE == SocketUtils::Listen(_socket))
		return false;

	const int32 acceptCount = _service->GetMaxSessionCount();
	for (int32 i = 0; i < acceptCount; ++i)
	{
		AcceptEvent* acceptEvent = xnew<AcceptEvent>();

		// 매우 위험한 코드
		// -> Listener 객체(자기 자신)을 관리하는 새로운 shared_ptr을 생성하기 때문에,
		//		만약 Listener 객체(자기 자신)을 shared_ptr로 관리하고 있는 다른 객체가 있다고 하면,
		//		Listener 객체(자기 자신)을 관리하는 control block이 2개가 생성된다.
		// -> 때문에 어느 한 곳에서 shared_ptr의 refCount가 0이되어 Listener 객체(자기 자신)가 소멸한다면,
		//		다른 한 곳에서는 오염된 메모리를 참조하게 되는 문제가 발생한다!!
		// acceptEvent->owner = std::shared_ptr<IOCPObject>(this);

		// 어케 해결?
		// -> IOCP object가 enable_shared_from_this를 상속받게 하고,
		//		enable_shared_from_this 객체가 가지고 있는 shared_from_this() 멤버 함수를 통해,
		//		enable_shared_from_this 객체가 가지고 있는 weak_ptr(_Wptr)을 shared_ptr로 캐스팅하여 넘겨준다.
		// -> 이러면 Control block이 2개 생기는 문제를 방지할 수 있음
		acceptEvent->owner = shared_from_this();

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
	ASSERT_CRASH(EventType::ACCEPT == iocpEvent->eventType);

	AcceptEvent* acceptEvent = static_cast<AcceptEvent*>(iocpEvent);
	ProcessAccept(acceptEvent);
}

void Listener::RegisterAccept(AcceptEvent* acceptEvent)
{
	SessionRef session = _service->CreateSession();		// Register IOCP

	acceptEvent->Init();
	acceptEvent->_session = session;

	DWORD bytesReceived = 0;
	BOOL ret = SocketUtils::AcceptEx(
		_socket, session->GetSocket(), session->_recvBuffer.WritePos(), 0,
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
	SessionRef session = acceptEvent->_session;

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

	std::cout << "Client Connected!!" << std::endl;

	session->SetNetworkAddress(NetworkAddress(sockAddress));
	session->ProcessConnect();
	RegisterAccept(acceptEvent);
}
