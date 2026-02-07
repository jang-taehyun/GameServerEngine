#pragma once

#include "IOCPCore.h"
#include "IOCPEvent.h"
#include "NetworkAddress.h"


/*---------------
	 Session
---------------*/

class Session : public IOCPObject
{
	friend class Listener;
	friend class IOCPCore;
	friend class Service;

public:
	Session();
	virtual ~Session();

public:
	/** 인터페이스 */
	void Send(BYTE* buffer, int32 len);
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
	void RegisterSend(SendEvent* sendEvent);

	void ProcessConnect();
	void ProcessDisconnect();
	void ProcessRecv(int32 numOfBytes);
	void ProcessSend(SendEvent* sendEvent, int32 numOfBytes);

	/** 에러 처리 */
	void HandleError(int32 errorCode);

protected:
	/** 컨텐츠 코드에서 오버라이드할 코드 */
	virtual void OnConnected() {}
	virtual int32 OnRecv(BYTE* buffer, int32 len) { return len; }
	virtual void OnSend(int32 len) {}
	virtual void OnDisconnected() {}

public:
	// TODO: TEMP
	char _recvBuffer[1000] = { 0, };

private:
	std::weak_ptr<class Service> _service;
	SOCKET _socket = INVALID_SOCKET;
	NetworkAddress _netAddress = {};
	Atomic<bool> _connected = false;

private:
	USE_LOCK;

	/** TODO: 수신 관련 */
	/** TODO: 송신 관련 */

private:
	/** IOCP event 재사용 */
	ConnectEvent _connectEvent;
	DisconnectEvent _disconnectEvent;
	RecvEvent _recvEvent;
};

