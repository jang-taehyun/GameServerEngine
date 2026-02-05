#include "pch.h"
#include "IOCPCore.h"
#include "IOCPEvent.h"

IOCPCore GIOCPCore;


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

bool IOCPCore::Register(IOCPObject* iocpObject)
{
	return ::CreateIoCompletionPort(iocpObject->GetHandle(), _iocpHandle, /*key*/ reinterpret_cast<ULONG_PTR>(iocpObject), 0);
}

bool IOCPCore::Dispatch(uint32 timeoutMs)
{
	DWORD numOfBytes = 0;
	IOCPObject* iocpObject = nullptr;
	IOCPEvent* iocpEvent = nullptr;

	if (::GetQueuedCompletionStatus(_iocpHandle, OUT &numOfBytes, OUT reinterpret_cast<PULONG_PTR>(&iocpObject), OUT reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), timeoutMs))
	{
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
			iocpObject->Dispatch(iocpEvent, numOfBytes);
			break;
		}
	}

	return true;
}
