#include "pch.h"
#include "MemoryPool.h"

/**
* C언어의 free() 함수의 특징
* -> 인자 : void* 형식
* -> 동작 : heap 메모리 영역 헤더 쪽에 할당한 메모리의 크기 정보를 남겨놓고, 그 크기 정보를 이용해 메모리를 해제
*	 -> 때문에 MemoryHeader* ptr을 해제하면 MemoryHeader가 차지하는 공간 뿐만 아니라 뒷 공간도 같이 해제됨.
*/

/*----------------------
	 Memory Pool
----------------------*/

MemoryPool::MemoryPool(int32 allocSize) : _allocSize(allocSize), _useCount(0), _reservedCount(0)
{
	// SLIST_HEADER 초기화
	::InitializeSListHead(&_header);
}

MemoryPool::~MemoryPool()
{
	while (MemoryHeader* memory = static_cast<MemoryHeader*>(::InterlockedPopEntrySList(&_header)))
	{
		::_aligned_free(memory);
	}
}

void MemoryPool::Push(MemoryHeader* ptr)
{
	// 메모리 영역 초기화
	ptr->allocSize = 0;

	// Pool에 메모리 반납
	::InterlockedPushEntrySList(&_header, static_cast<PSLIST_ENTRY>(ptr));

	// 횟수 조정
	_useCount.fetch_sub(1);
	_reservedCount.fetch_add(1);
}

MemoryHeader* MemoryPool::Pop()
{
	MemoryHeader* memory = static_cast<MemoryHeader*>(::InterlockedPopEntrySList(&_header));

	// 없으면 새로 만든다.
	if (nullptr == memory)
	{
		memory = reinterpret_cast<MemoryHeader*>(::_aligned_malloc(_allocSize, SLIST_ALIGNMENT));
	}
	else
	{
		ASSERT_CRASH((0 == memory->allocSize));
		_reservedCount.fetch_sub(1);
	}

	// 횟수 조정
	_useCount.fetch_add(1);

	return memory;
}
