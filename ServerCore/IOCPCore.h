#pragma once

/*-------------------
	 IOCP Object
-------------------*/

// IOCP Object 객체는 session과 비슷한 역할
// IOCP Event 객체는 WSAOVERLAPPED와 비슷한 역할
class IOCPObject
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

	bool Register(class IOCPObject* iocpObject);
	bool Dispatch(uint32 timeoutMs = INFINITE);

private:
	HANDLE _iocpHandle = INVALID_HANDLE_VALUE;
};

// TEMP
extern IOCPCore GIOCPCore;