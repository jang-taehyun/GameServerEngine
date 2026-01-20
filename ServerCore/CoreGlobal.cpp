#include "pch.h"
#include "CoreGlobal.h"
#include "ThreadManager.h"

ThreadManager* GThreadManager = nullptr;

class CoreGlobal
{
public:
	CoreGlobal();
	~CoreGlobal();

private:

} GCoreGlobal;

CoreGlobal::CoreGlobal()
{
	GThreadManager = new ThreadManager;
}

CoreGlobal::~CoreGlobal()
{
	delete GThreadManager;
}
