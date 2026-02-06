#include "pch.h"
#include "IOCPCore.h"
#include "IOCPEvent.h"


/*-------------------
	 IOCP Core
-------------------*/

IOCPCore::IOCPCore()
{
	_iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, NULL , 0);
	ASSERT_CRASH(INVALID_HANDLE_VALUE != _iocpHandle);
}

IOCPCore::~IOCPCore()
{
	::CloseHandle(_iocpHandle);
}

bool IOCPCore::Register(IOCPObjectRef iocpObject)
{
	return ::CreateIoCompletionPort(iocpObject->GetHandle(), _iocpHandle, /*key*/ 0, 0);
}

bool IOCPCore::Dispatch(uint32 timeoutMs)
{
	DWORD numOfBytes = 0;
	ULONG_PTR key = 0;
	IOCPEvent* iocpEvent = nullptr;

	if (::GetQueuedCompletionStatus(_iocpHandle, OUT &numOfBytes, OUT &key, OUT reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), timeoutMs))
	{
		IOCPObjectRef iocpObject = iocpEvent->owner;
		iocpObject->Dispatch(iocpEvent, numOfBytes);
	}
	else
	{
		int32 errorCode = ::WSAGetLastError();

		switch (errorCode)
		{
		case WAIT_TIMEOUT:
			return false;
		default:
			// TODO: ·Î±× Âï±â
			IOCPObjectRef iocpObject = iocpEvent->owner;
			iocpObject->Dispatch(iocpEvent, numOfBytes);
			break;
		}
	}

	return true;
}
