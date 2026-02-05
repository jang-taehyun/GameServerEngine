#include "pch.h"
#include "IOCPEvent.h"


/*-------------------
	 IOCP Event
-------------------*/

IOCPEvent::IOCPEvent(EventType type) : _type(type)
{
	Init();
}

void IOCPEvent::Init()
{
	OVERLAPPED::Internal = 0;
	OVERLAPPED::InternalHigh = 0;
	OVERLAPPED::Pointer = 0;
	OVERLAPPED::hEvent = 0;
}