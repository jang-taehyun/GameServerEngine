#pragma once

extern class ThreadManager* GThreadManager;
extern class DeadLockProfiler* GDeadlockProfiler;

class CoreGlobal
{
public:
	CoreGlobal();
	~CoreGlobal();

private:

};

extern CoreGlobal GCoreGlobal;
