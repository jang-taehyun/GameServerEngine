#pragma once

/*-------------------
	 IOCP Object
-------------------*/

// enable_shared_from_this 객체
// -> 자기 자신의 weak_ptr을 가지고 있는 객체
// -> enable_shared_from_this 객체를 상속받아 클래스를 만들면,
//		반드시 shared_ptr로만 객체를 생성해야 한다.
class IOCPObject : public std::enable_shared_from_this<IOCPObject>
{
public:
	virtual HANDLE GetHandle() = 0;
	virtual void Dispatch(class IOCPEvent* iocpEvent, int32 numOfBytes = 0) = 0;
};


/*-------------------
	 IOCP Core
-------------------*/

class IOCPCore
{
public:
	IOCPCore();
	~IOCPCore();

	HANDLE GetHandle() const { return _iocpHandle; }

	bool Register(IOCPObjectRef iocpObject);
	bool Dispatch(uint32 timeoutMs = INFINITE);

private:
	HANDLE _iocpHandle = INVALID_HANDLE_VALUE;
};