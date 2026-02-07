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

void Session::Disconnect(const WCHAR* cause)
{
	if (false == _connected.exchange(false))
		return;

	// TODO: TEMP
	std::wcout << "Disconnect : " << cause << std::endl;

	OnDisconnected();	// 컨텐츠 코드에서 오버로딩
	SocketUtils::Close(_socket);
	GetService()->ReleaseSession(GetSessionRef());
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

void Session::RegisterConnect()
{
	
}

void Session::RegisterRecv()
{
	// 연결이 끊겼으면 더이상 등록하지 않는다. //
	if (false == IsConnected())
		return;

	_recvEvent.Init();
	_recvEvent.owner = shared_from_this();		// ADD_REF

	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer);
	wsaBuf.len = len32(_recvBuffer);

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
	_connected.store(true);

	// session 등록 //
	GetService()->AddSession(GetSessionRef());

	// 컨텐츠 코드에서 오버라이딩한 OnConnected() 함수 호출 //
	OnConnected();

	// 수신 등록 //
	RegisterRecv();
}

void Session::ProcessRecv(int32 numOfBytes)
{
	_recvEvent.owner = nullptr;		// RELEASE_REF

	if (0 == numOfBytes)
	{
		Disconnect(L"Recv 0");
		return;
	}

	// 컨텐츠 코드에서 오버라이딩한 OnRecv() 함수 호출 //
	OnRecv(reinterpret_cast<BYTE*>(_recvBuffer), numOfBytes);

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
