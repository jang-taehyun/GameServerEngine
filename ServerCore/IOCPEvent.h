#pragma once

enum class EventType : uint8
{
	NONE,
	CONNECT,
	ACCEPT,
	// PRERECV,		// -> 0 byte recv
	RECV,
	SEND
};

/*-------------------
	 IOCP Event
-------------------*/

// IOCP Object 객체는 session과 비슷한 역할
// IOCP Event 객체는 WSAOVERLAPPED와 비슷한 역할
// 주의점 : IOCPEvent 객체와 IOCPEvent 객체를 상속받는 모든 객체는 가상 함수를 쓰면 안된다.
// -> 가상 함수, 가상 소멸자를 사용하게 되면, virtual table이 만들어지면서,
//	  객체의 시작 주소가 virtual table의 시작 주소로 바뀌어
//	  객체 포인터를 overlapped 객체 포인터로 캐스팅해 쓸 수 없게 된다!!
class IOCPEvent : public WSAOVERLAPPED
{
public:
	IOCPEvent(EventType type);

	void Init();
	EventType GetType() const { return _type; }

protected:
	EventType _type = EventType::NONE;
};


/*--------------------
	 Connect Event
--------------------*/

class ConnectEvent : public IOCPEvent
{
public:
	ConnectEvent() : IOCPEvent(EventType::CONNECT) {}
};


/*--------------------
	 Accept Event
--------------------*/

class AcceptEvent : public IOCPEvent
{
public:
	AcceptEvent() : IOCPEvent(EventType::ACCEPT) {}

	void SetSession(class Session* session) { _session = session; }
	Session* GetSession() const { return _session; }

private:
	// TODO
	class Session* _session = nullptr;
};


/*--------------------
	 Recv Event
--------------------*/

class RecvEvent : public IOCPEvent
{
public:
	RecvEvent() : IOCPEvent(EventType::RECV) {}
};


/*--------------------
	 Send Event
--------------------*/

class SendEvent : public IOCPEvent
{
public:
	SendEvent() : IOCPEvent(EventType::SEND) {}
};

