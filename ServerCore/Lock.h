#pragma once

// 영역 구분 //
/*-------------------------------------
	|상호배타적/read-write| |Lock 방식|
-------------------------------------*/

#include "Types.h"

/*-----------------------------
	Reader-writer SpinLock
-----------------------------*/

/*-----------------------------------------------------------------------------------------------------
- 사용할 리소스 : 32bit 변수(int32 변수, _lockFlag)
[WWWWWWWW][WWWWWWWW][RRRRRRRR][RRRRRRRR]
* W
  - Writer flag
  - 상위 16bit
  - Exclusive Lock Owner Thread ID(현재 lock을 획득한 쓰레드의 ID)
  - write는 상호배타적이면서 재귀적(recursive)하게 호출 가능
* R
  - Reader flag
  - 읽기 횟수, 하위 16bit
  - Shared Lock Count(공유해서 사용하고 있는 read count, 현재 리소스를 읽고 있는 쓰레드의 개수)
-----------------------------------------------------------------------------------------------------*/

/*-----------------------------
- 정책(정책은 맘에 안들면 내가 바꿔도 되는 부분)
1) write lock을 잡은 상태에서 동일한 쓰레드가 read lock을 잡는 것은 허용
2) read lock을 잡은 상태에서 동일한 쓰레드가 write lock을 잡는 것은 허용하지 않음
-----------------------------*/

class Lock
{
	// 하드 코딩한 값(flag)
	enum : uint32
	{
		// 최대로 기다릴 tick 횟수 //
		ACQUIRE_TIMEOUT_TICK = 10'000,

		// 최대로 spin할 횟수 //
		MAX_SPIN_COUNT = 5'000,

		// 상위 16bit mask //
		WRITE_THREAD_MASK = 0xFFFF'0000,

		// 하위 16bit mask //
		READ_COUNT_MASK = 0x0000'FFFF,

		// EMPTY //
		EMPTY_FLAG = 0x0000'0000
	};

public:
	Lock();
	~Lock();

	void WriteLock(const char* name);
	void WriteUnlock(const char* name);
	void ReadLock(const char* name);
	void ReadUnlock(const char* name);

private:
	Atomic<uint32> _lockFlag;
	uint16 _writeCount;
};



/*----------------------------------
	Lock Guard(Read-write Lock)
----------------------------------*/

class ReadLockGuard
{
public:
	ReadLockGuard(Lock& lock, const char* name);
	~ReadLockGuard();

private:
	Lock& _lock;
	const char* _name;
};

class WriteLockGuard
{
public:
	WriteLockGuard(Lock& lock, const char* name);
	~WriteLockGuard();

private:
	Lock& _lock;
	const char* _name;
};