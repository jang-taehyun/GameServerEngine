#include "pch.h"
#include "Lock.h"
#include "CoreTLS.h"
#include "DeadLockProfiler.h"

Lock::Lock() : _lockFlag(EMPTY_FLAG), _writeCount(0)
{
}

Lock::~Lock()
{
}

void Lock::WriteLock(const char* name)
{
#ifdef _DEBUG
	GDeadlockProfiler->PushLock(name);
#endif // _DEBUG


	// 동일한 쓰레드가 소유하고 있다면 무조건 성공
	const uint32 lockThreadID = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (lockThreadID == LThreadID)
	{
		++_writeCount;
		return;
	}

	// 아무도 소유(write, 상호배타적) 및 공유(read)하고 있지 않을 때, 경합해서 소유권을 얻는다.
	// EMPTY_FLAG인 상태를 의미
	const uint32 desired = ((LThreadID << 16) & WRITE_THREAD_MASK);
	const int64 beginTick = ::GetTickCount64();
	while (true)
	{
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; ++spinCount)
		{
			uint32 expected = EMPTY_FLAG;
			if (_lockFlag.compare_exchange_strong(OUT expected, desired))
			{
				++_writeCount;
				return;
			}
		}

		// 너무 오랜 시간이 지나면 crash 발생
		if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK)
			CRASH("LOCK_TIMEOUT");

		// 잠시 놓아주고 다음에 다시 실행해보자
		std::this_thread::yield();
	}
}

void Lock::WriteUnlock(const char* name)
{
#ifdef _DEBUG
	GDeadlockProfiler->PopLock(name);
#endif // _DEBUG

	// Read Lock을 다 풀기 전에는 Write Unlock 불가능
	if (0 != (_lockFlag.load() & READ_COUNT_MASK))
		CRASH("INVALID_UNLOCK_ORDER");

	// Write Unlock
	const int32 lockCount = --_writeCount;
	if (0 == _writeCount)
		_lockFlag.store(EMPTY_FLAG);
}

void Lock::ReadLock(const char* name)
{
#ifdef _DEBUG
	GDeadlockProfiler->PushLock(name);
#endif // _DEBUG

	// 동일한 쓰레드가 소유하고 있다면 무조건 성공
	const uint32 lockThreadID = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (lockThreadID == LThreadID)
	{
		_lockFlag.fetch_add(1);
		return;
	}

	// 아무도 소유하고 있지 않을 때 경합해서 공유 카운트를 올린다.
	const int64 beginTick = ::GetTickCount64();
	while (true)
	{
		for (int32 spinCount = 0; spinCount < MAX_SPIN_COUNT; ++spinCount)
		{
			uint32 expected = (_lockFlag.load() & READ_COUNT_MASK);
			if (_lockFlag.compare_exchange_strong(OUT expected, expected + 1))
			{
				return;
			}
		}

		// 너무 오랜 시간이 지나면 crash 발생
		if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK)
			CRASH("LOCK_TIMEOUT");

		// 잠시 놓아주고 다음에 다시 실행해보자
		std::this_thread::yield();
	}
}

void Lock::ReadUnlock(const char* name)
{
#ifdef _DEBUG
	GDeadlockProfiler->PopLock(name);
#endif // _DEBUG

	if (0 == _lockFlag.fetch_sub(1))
		CRASH("MULTIPLE_READ_UNLOCK");
}


/*----------------------------------
	Lock Guard(Read-write Lock)
----------------------------------*/

ReadLockGuard::ReadLockGuard(Lock& lock, const char* name) : _lock(lock), _name(name)
{
	_lock.ReadLock(name);
}

ReadLockGuard::~ReadLockGuard()
{
	_lock.ReadUnlock(_name);
}

WriteLockGuard::WriteLockGuard(Lock& lock, const char* name) : _lock(lock), _name(name)
{
	_lock.WriteLock(name);
}

WriteLockGuard::~WriteLockGuard()
{
	_lock.WriteUnlock(_name);
}
