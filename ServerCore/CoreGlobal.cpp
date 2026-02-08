#include "pch.h"
#include "CoreGlobal.h"
#include "ThreadManager.h"
#include "DeadLockProfiler.h"
#include "Memory.h"
#include "SocketUtils.h"
#include "SendBuffer.h"

CoreGlobal			GCoreGlobal;
ThreadManager*		GThreadManager = nullptr;
DeadLockProfiler*	GDeadlockProfiler = nullptr;
Memory*				GMemory = nullptr;
SendBufferManager*	GSendBufferManager = nullptr;

CoreGlobal::CoreGlobal()
{
	GThreadManager = new ThreadManager;
	GDeadlockProfiler = new DeadLockProfiler;
	GMemory = new Memory;
	GSendBufferManager = new SendBufferManager;
	SocketUtils::Init();
}

CoreGlobal::~CoreGlobal()
{
	delete GThreadManager;
	delete GDeadlockProfiler;
	delete GMemory;
	delete GSendBufferManager;
	SocketUtils::Clear();
}
