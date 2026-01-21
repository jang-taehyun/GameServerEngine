#include "pch.h"
#include "CoreGlobal.h"
#include "ThreadManager.h"
#include "DeadLockProfiler.h"

CoreGlobal GCoreGlobal;
ThreadManager* GThreadManager = nullptr;
DeadLockProfiler* GDeadlockProfiler = nullptr;

CoreGlobal::CoreGlobal()
{
	GThreadManager = new ThreadManager;
	GDeadlockProfiler = new DeadLockProfiler;
}

CoreGlobal::~CoreGlobal()
{
	delete GThreadManager;
	delete GDeadlockProfiler;
}
