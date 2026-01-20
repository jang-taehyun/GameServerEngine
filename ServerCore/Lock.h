#pragma once

/**
* 표준 mutex의 단점(Lock을 직접 구현해서 사용해야 하는 이유)
* - 표준 mutex는 재귀적으로 Lock을 잡을 수 없음
*   -> Lock을 잡은 상태에서 동일한 쓰레드가 다시 lock을 잡을 수 없음
*   -> 대안 : std::recursive_mutex
* - 대부분 read만 할 때 lock을 잡는 경우, lock을 잡는 오버헤드가 발생
*   - lock이 필요할 때는 write이기 때문
*   - 극악의 확률(거의 0.00000001%)로 변경이 일어날 때 lock을 잡는 것이 부담이 될 수 있음
*   -> 대안 : read-write lock
*     -> 원하는 형태로 최적화, 또는 Deadlock 탐지를 하기 위해서는 직접 구현하는 것이 좋음
* -> lock을 직접 구현하는 것은 프로젝트마다 다름(그냥 필요성이 느껴지만 직접 구현)
*/

// 재귀적으로 lock을 잡는다 -> 동일한 쓰레드가 lock을 잡은 상태에서 다시 lock을 잡는다는 것
// -> lock을 잡은 횟수만큼, unlock을 해줘야 한다.

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

	void WriteLock();
	void WriteUnlock();
	void ReadLock();
	void ReadUnlock();

private:
	Atomic<uint32> _lockFlag;
	uint16 _writeCount;			// write lock을 재귀적으로 잡기 위해 write count를 따로 관리
};

