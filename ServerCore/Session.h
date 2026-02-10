#pragma once

#include "IOCPCore.h"
#include "IOCPEvent.h"
#include "NetworkAddress.h"
#include "RecvBuffer.h"


/*---------------
	 Session
---------------*/

class Session : public IOCPObject
{
	friend class Listener;
	friend class IOCPCore;
	friend class Service;

	enum
	{
		BUFFER_SIZE = 0x10000,		// 64KB
	};

public:
	Session();
	virtual ~Session();

public:
	/** 인터페이스 */
	void Send(SendBufferRef sendBuffer);
	bool Connect();
	void Disconnect(const WCHAR* cause);

	std::shared_ptr<Service> GetService() { return _service.lock(); }
	void SetService(std::shared_ptr<Service> service) { _service = service; }

public:
	/** 정보 관련 */
	void SetNetworkAddress(NetworkAddress address) { _netAddress = address; }
	NetworkAddress GetAddress() const { return _netAddress; }
	SOCKET GetSocket() { return _socket; }

	bool IsConnected() { return _connected; }
	SessionRef GetSessionRef() { return std::static_pointer_cast<Session>(shared_from_this()); }

private:
	/** 상속 받은 인터페이스 구현 */
	virtual HANDLE GetHandle() override;
	virtual void Dispatch(class IOCPEvent* iocpEvent, int32 numOfBytes = 0) override;

private:
	/** 전송 관련 */
	bool RegisterConnect();
	bool RegisterDisconnect();
	void RegisterRecv();
	void RegisterSend();

	void ProcessConnect();
	void ProcessDisconnect();
	void ProcessRecv(int32 numOfBytes);
	void ProcessSend(int32 numOfBytes);

	/** 에러 처리 */
	void HandleError(int32 errorCode);

protected:
	/** 컨텐츠 코드에서 오버라이드할 코드 */
	virtual void OnConnected() {}
	virtual int32 OnRecv(BYTE* buffer, int32 len) { return len; }
	virtual void OnSend(int32 len) {}
	virtual void OnDisconnected() {}

private:
	std::weak_ptr<class Service> _service;
	SOCKET _socket = INVALID_SOCKET;
	NetworkAddress _netAddress = {};
	Atomic<bool> _connected = false;

private:
	USE_LOCK;

	/** TODO: 수신 관련 */
	RecvBuffer _recvBuffer{ BUFFER_SIZE };

	/** TODO: 송신 관련 */
	Queue<SendBufferRef> _sendQueue;
	Atomic<bool> _sendRegistered = false;

private:
	/** IOCP event 재사용 */
	ConnectEvent _connectEvent;
	DisconnectEvent _disconnectEvent;
	RecvEvent _recvEvent;
	SendEvent _sendEvent;
};


/*---------------------
	 Packet Session
---------------------*/

/*
* 우리 만의 프로토콜 정의
* - 데이터 앞에 header를 붙인다.
* - header의 구조
*	[size(2byte)][ID(2byte)][data....]
* - size는 header를 포함한 전체 packet의 길이를 의미
*/

struct PacketHeader
{
	//enum class ProtocolID : uint16
	//{
	//	NONE = 0,
	//	HELLO_WORLD = 1,
	//};

	uint16 size = 0;
	// ProtocolID ID = ProtocolID::NONE;
	uint16 ID = 0;
};

class PacketSession : public Session
{
public:
	PacketSession();
	virtual ~PacketSession();

	PacketSessionRef GetPacketSessionRef() { return std::static_pointer_cast<PacketSession>(shared_from_this()); }

protected:
	virtual int32 OnRecv(BYTE* buffer, int32 len) final;
	virtual void OnRecvPacket(BYTE* buffer, int32 len) = 0;
};