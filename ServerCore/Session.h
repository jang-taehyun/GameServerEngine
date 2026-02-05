#pragma once

#include "IOCPCore.h"
#include "IOCPEvent.h"
#include "NetworkAddress.h"


/*---------------
	 Session
---------------*/

class Session : public IOCPObject
{
public:
	Session();
	virtual ~Session();

public:
	/** 정보 관련 */
	void SetNetworkAddress(NetworkAddress address) { _netAddress = address; }
	NetworkAddress GetAddress() const { return _netAddress; }
	SOCKET GetSocket() { return _socket; }

public:
	/** 인터페이스 구현 */
	virtual HANDLE GetHandle() override;
	virtual void Dispatch(class IOCPEvent* iocpEvent, int32 numOfBytes = 0) override;

public:
	// TODO: TEMP
	char _recvBuffer[1000] = { 0, };

private:
	SOCKET _socket = INVALID_SOCKET;
	NetworkAddress _netAddress = {};
	Atomic<bool> _connected = false;
};

