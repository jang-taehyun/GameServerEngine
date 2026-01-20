#include "pch.h"
#include "ThreadManager.h"

/*------------------
	ThreadManager
------------------*/

ThreadManager::ThreadManager()
{
	// 메인 쓰레드의 TLS 초기화
	InitTLS();
}

ThreadManager::~ThreadManager()
{
	Join();
}

void ThreadManager::Launch(function<void(void)> callback)
{
	LockGuard guard(_lock);

	_threads.push_back(thread(
		[=]()
		{
			InitTLS();
			callback();
			DestroyTLS();
		}
	));
}

void ThreadManager::Join()
{
	for (thread& t : _threads)
	{
		if (t.joinable())
			t.join();
	}

	_threads.clear();
}

void ThreadManager::InitTLS()
{
	static Atomic<uint32> SThreadID = 1;
	LThreadID = SThreadID.fetch_add(1);
}

void ThreadManager::DestroyTLS()
{
}
