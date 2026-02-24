#pragma once

class CoreGlobal
{
public:
	CoreGlobal();
	~CoreGlobal();
};

extern			CoreGlobal			GCoreGlobal;
extern class	ThreadManager*		GThreadManager;
extern class	DeadLockProfiler*	GDeadlockProfiler;
extern class	Memory*				GMemory;
extern class	SendBufferManager*	GSendBufferManager;
extern class	GlobalQueue*		GGlobalQueue;
extern class	JobTimer*			GJobTimer;