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

MemoryPool::MemoryPool(int32 allocSize) : _allocSize(allocSize), _allocCount(0)
{

}

MemoryPool::~MemoryPool()
{
	while (false == _queue.empty())
	{
		MemoryHeader* header = _queue.front();
		_queue.pop();
		::free(header);
	}
}

void MemoryPool::Push(MemoryHeader* ptr)
{
	WRITE_LOCK;

	// header 초기화
	ptr->allocSize = 0;

	// Pool에 메모리 반납
	_queue.push(ptr);

	// 횟수 감소
	_allocCount.fetch_sub(1);
}

MemoryHeader* MemoryPool::Pop()
{
	MemoryHeader* header = nullptr;

	{
		WRITE_LOCK;

		// Pool에 여분이 있는지?
		if (false == _queue.empty())
		{
			// 있으면 하나 꺼내온다.
			header = _queue.front();
			_queue.pop();
		}
	}

	// 없으면 새로 만든다.
	if (nullptr == header)
	{
		header = reinterpret_cast<MemoryHeader*>(::malloc(_allocSize));
	}
	else
	{
		ASSERT_CRASH((0 == header->allocSize));
	}

	// 횟수 증가
	_allocCount.fetch_add(1);

	return header;
}
