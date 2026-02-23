#include "pch.h"
#include "CoreGlobal.h"
#include "ThreadManager.h"
#include "DeadLockProfiler.h"
#include "Memory.h"
#include "SocketUtils.h"
#include "SendBuffer.h"
#include "GlobalQueue.h"

CoreGlobal			GCoreGlobal;
ThreadManager*		GThreadManager = nullptr;
DeadLockProfiler*	GDeadlockProfiler = nullptr;
Memory*				GMemory = nullptr;
SendBufferManager*	GSendBufferManager = nullptr;
GlobalQueue*		GGlobalQueue = nullptr;

CoreGlobal::CoreGlobal()
{
	GThreadManager = new ThreadManager;
	GDeadlockProfiler = new DeadLockProfiler;
	GMemory = new Memory;
	GSendBufferManager = new SendBufferManager;
	GGlobalQueue = new GlobalQueue;
	SocketUtils::Init();
}

CoreGlobal::~CoreGlobal()
{
	delete GThreadManager;
	delete GDeadlockProfiler;
	delete GMemory;
	delete GSendBufferManager;
	delete GGlobalQueue;
	SocketUtils::Clear();
}
