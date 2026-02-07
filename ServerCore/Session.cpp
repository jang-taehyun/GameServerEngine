#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"
#include "Service.h"

/*---------------
	 Session
---------------*/

Session::Session()
{
	_socket = SocketUtils::CreateSocket();
}

Session::~Session()
{
	SocketUtils::Close(_socket);
}

void Session::Send(BYTE* buffer, int32 len)
{
	// 생각할 문제
	// 1) buffer 관리
	// 2) sendEvent 관리 -> 단일 또는 여러개?, WSASend() 함수가 중첩 호출이 되는지?

	// TODO: TEMP
	SendEvent* sendEvent = xnew<SendEvent>();
	sendEvent->owner = shared_from_this();
	sendEvent->buffer.resize(len);
	::memcpy(sendEvent->buffer.data(), buffer, len);

	WRITE_LOCK;
	RegisterSend(sendEvent);
}

bool Session::Connect()
{
	return RegisterConnect();
}

void Session::Disconnect(const WCHAR* cause)
{
	if (false == _connected.exchange(false))
		return;

	// TODO: TEMP
	std::wcout << "Disconnect : " << cause << std::endl;

	OnDisconnected();	// 컨텐츠 코드에서 오버로딩
	GetService()->ReleaseSession(GetSessionRef());

	RegisterDisconnect();
}

HANDLE Session::GetHandle()
{
	return reinterpret_cast<HANDLE>(_socket);
}

void Session::Dispatch(IOCPEvent* iocpEvent, int32 numOfBytes)
{
	switch (iocpEvent->eventType)
	{
	case EventType::CONNECT:
		ProcessConnect();
		break;

	case EventType::DISCONNECT:
		ProcessDisconnect();
		break;

	case EventType::RECV:
		ProcessRecv(numOfBytes);
		break;

	case EventType::SEND:
		ProcessSend(static_cast<SendEvent*>(iocpEvent), numOfBytes);
		break;

	default:
		break;
	}
}

bool Session::RegisterConnect()
{
	if (IsConnected())
		return false;

	if (ServiceType::CLIENT != GetService()->GetServiceType())
		return false;

	if (false == SocketUtils::SetReuseAddress(_socket, TRUE))
		return false;

	// ConnectEx() 함수를 호출할 때는 포트 번호를 0으로 만들어줘야 한다.
	// -> 0의 의미 : 남는 포트 중 아무거나 할당해달라는 의미
	if (false == SocketUtils::BindAnyAddress(_socket, /*남는 포트 아무거나*/ 0))
		return false;

	_connectEvent.Init();
	_connectEvent.owner = shared_from_this();	// ADD_REF

	DWORD numOfBytes = 0;
	SOCKADDR_IN sockAddr = GetService()->GetNetAddress().GetSocketAddress();

	if (FALSE == SocketUtils::ConnectEx(_socket, reinterpret_cast<const SOCKADDR*>(&sockAddr), sizeof(sockAddr), nullptr, 0, &numOfBytes, &_connectEvent))
	{
		int32 errorCode = ::WSAGetLastError();
		if (WSA_IO_PENDING != errorCode)
		{
			_connectEvent.owner = nullptr;	// RELEASE_REF
			return false;
		}
	}

	return true;
}

bool Session::RegisterDisconnect()
{
	_disconnectEvent.Init();
	_disconnectEvent.owner = shared_from_this();		// ADD_REF

	if (FALSE == SocketUtils::DisconnectEx(_socket, &_disconnectEvent, TF_REUSE_SOCKET, 0))
	{
		int32 errorCode = ::WSAGetLastError();
		if (WSA_IO_PENDING != errorCode)
		{
			_disconnectEvent.owner = nullptr;			// RELEASE_REF
			return false;
		}
	}

	return true;
}

void Session::RegisterRecv()
{
	// 연결이 끊겼으면 더이상 등록하지 않는다. //
	if (false == IsConnected())
		return;

	_recvEvent.Init();
	_recvEvent.owner = shared_from_this();		// ADD_REF

	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer.WritePos());
	wsaBuf.len = _recvBuffer.FreeSize();

	DWORD numOfBytes = 0;
	DWORD flags = 0;

	if (SOCKET_ERROR == ::WSARecv(_socket, &wsaBuf, 1, OUT &numOfBytes, OUT &flags, &_recvEvent, nullptr))
	{
		int32 errorCode = ::WSAGetLastError();
		if (WSA_IO_PENDING != errorCode)
		{
			HandleError(errorCode);
			_recvEvent.owner = nullptr;		// RELEASE_REF
		}
	}
}

void Session::RegisterSend(SendEvent* sendEvent)
{
	// 연결이 끊겼으면 더이상 등록하지 않는다. //
	if (false == IsConnected())
		return;

	WSABUF wsaBuf;
	wsaBuf.buf = (char*)sendEvent->buffer.data();
	wsaBuf.len = (ULONG)sendEvent->buffer.size();

	DWORD numOfBytes = 0;
	if (SOCKET_ERROR == ::WSASend(_socket, &wsaBuf, 1, OUT & numOfBytes, 0, sendEvent, nullptr))
	{
		int32 errorCode = ::WSAGetLastError();
		if (WSA_IO_PENDING != errorCode)
		{
			HandleError(errorCode);
			sendEvent->owner = nullptr;		// RELEASE_REF
			xdelete(sendEvent);
		}
	}
}

void Session::ProcessConnect()
{
	_connectEvent.owner = nullptr;	// RELEASE_REF
	_connected.store(true);

	// session 등록 //
	GetService()->AddSession(GetSessionRef());

	// 컨텐츠 코드에서 오버라이딩한 OnConnected() 함수 호출 //
	OnConnected();

	// 수신 등록 //
	RegisterRecv();
}

void Session::ProcessDisconnect()
{
	_disconnectEvent.owner = nullptr;		// RELEASE_REF
}

void Session::ProcessRecv(int32 numOfBytes)
{
	_recvEvent.owner = nullptr;		// RELEASE_REF

	if (0 == numOfBytes)
	{
		Disconnect(L"Recv 0");
		return;
	}

	if (false == _recvBuffer.OnWrite(numOfBytes))
	{
		Disconnect(L"OnWrite() overflow!!");
		return;
	}

	// 컨텐츠 코드에서 오버라이딩한 OnRecv() 함수 호출 //
	int32 dataSize = _recvBuffer.DataSize();
	int32 processLen = OnRecv(_recvBuffer.ReadPos(), dataSize);
	if (0 > processLen || dataSize < processLen || false == _recvBuffer.OnRead(processLen))
	{
		Disconnect(L"OnRead() overflow!!");
		return;
	}

	// recv buffer의 커서(read, write 커서) 정리 //
	_recvBuffer.Clean();

	RegisterRecv();
}

void Session::ProcessSend(SendEvent* sendEvent, int32 numOfBytes)
{
	sendEvent->owner = nullptr;		// RELEASE_REF
	xdelete(sendEvent);

	if (0 == numOfBytes)
	{
		Disconnect(L"Send 0");
		return;
	}

	// 컨텐츠 코드에서 오버라이딩한 OnSend() 함수 호출 //
	OnSend(numOfBytes);
}

void Session::HandleError(int32 errorCode)
{
	switch (errorCode)
	{
	case WSAECONNRESET:
	case WSAECONNABORTED:
		Disconnect(L"HandleError");
		break;

	default:
		// TODO: Log
		std::cout << "Handle Error : " << errorCode << std::endl;
		break;
	}
}
