#include "pch.h"
#include "ThreadManager.h"
#include "DeadLockProfiler.h"
#include "Memory.h"
#include "SocketUtils.h"
#include "SendBuffer.h"
#include "GlobalQueue.h"
#include "JobTimer.h"
#include "DBConnectionPool.h"
#include "CoreGlobal.h"

CoreGlobal			GCoreGlobal;
ThreadManager*		GThreadManager = nullptr;
DeadLockProfiler*	GDeadlockProfiler = nullptr;
Memory*				GMemory = nullptr;
SendBufferManager*	GSendBufferManager = nullptr;
GlobalQueue*		GGlobalQueue = nullptr;
JobTimer*			GJobTimer = nullptr;
DBConnectionPool*	GDBConnectionPool = nullptr;

CoreGlobal::CoreGlobal()
{
	GThreadManager = new ThreadManager;
	GDeadlockProfiler = new DeadLockProfiler;
	GMemory = new Memory;
	GSendBufferManager = new SendBufferManager;
	GGlobalQueue = new GlobalQueue;
	GJobTimer = new JobTimer;
	GDBConnectionPool = new DBConnectionPool;
	SocketUtils::Init();
}

CoreGlobal::~CoreGlobal()
{
	delete GThreadManager;
	delete GDeadlockProfiler;
	delete GMemory;
	delete GSendBufferManager;
	delete GGlobalQueue;
	delete GJobTimer;
	delete GDBConnectionPool;
	SocketUtils::Clear();
}
