#pragma once

class CoreGlobal
{
public:
	CoreGlobal();
	~CoreGlobal();

private:

};

extern			CoreGlobal			GCoreGlobal;
extern class	ThreadManager*		GThreadManager;
extern class	DeadLockProfiler*	GDeadlockProfiler;
extern class	Memory*				GMemory;
