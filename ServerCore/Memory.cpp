#include "pch.h"
#include "Memory.h"
#include "MemoryPool.h"

/*---------------
	 Memory
---------------*/

Memory::Memory()
{
	int32 size = 0;
	int32 tableIndex = 1;

	// pool 생성
	_pools.reserve(MAX_ALLOC_SIZE + 1);
	CreateMemoryPool(0, 1024, 32, tableIndex);
	CreateMemoryPool(1024, 2048, 128, tableIndex);
	CreateMemoryPool(2048, 4096, 256, tableIndex);
	
	// 0번째 pool 제거
	delete _pools[0];
	_pools[0] = nullptr;
}

Memory::~Memory()
{
	for (MemoryPool* pool : _pools)
		if (nullptr != pool)
			delete pool;
	_pools.clear();
}

void* Memory::Allocate(int32 size)
{
	ASSERT_CRASH((0 < size));

	MemoryHeader* header = nullptr;
	const int32 allocSize = size + sizeof(MemoryHeader);

	if (MAX_ALLOC_SIZE < allocSize)
	{
		// 메모리 풀링의 최대 크기를 벗어나면 일반 할당
		header = reinterpret_cast<MemoryHeader*>(::malloc(allocSize));
	}
	else
	{
		// 메모리 풀에서 꺼내 온다.
		header = _poolTable[allocSize]->Pop();
	}

	// 헤더를 붙여서 할당한 메모리 반환
	return MemoryHeader::AttachHeader(header, allocSize);
}

void Memory::Release(void* ptr)
{
	ASSERT_CRASH((nullptr != ptr));

	// header를 포함한 메모리의 포인터 가져오기
	MemoryHeader* header = MemoryHeader::DetachHeader(ptr);

	const int32 allocSize = header->allocSize;
	ASSERT_CRASH((0 < allocSize));

	if (MAX_ALLOC_SIZE > allocSize)
	{
		// 메모리 풀링 최대 크기를 벗어나면 일반 해제
		::free(header);
	}
	else
	{
		// 메모리 풀에 반납
		_poolTable[allocSize]->Push(header);
	}

	header = nullptr;
}

void Memory::CreateMemoryPool(const int32 start, const int32 end, const int32 offset, int32& tableIdx)
{
	for (int32 size = start + offset; size <= end; size += offset)
	{
		MemoryPool* pool = new MemoryPool(size);	// 내부적으로 offset 크기의 chunk를 여러 개 할당
		_pools.push_back(pool);

		while (tableIdx <= size)
		{
			_poolTable[tableIdx] = pool;
			++tableIdx;
		}
	}
}