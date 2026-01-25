#pragma once

#include "Allocator.h"

/*---------------
	 Memory
---------------*/

// memory pool를 총괄하는 객체(memory manager 역할) //
class MemoryPool;
class Memory
{
	enum
	{
		// 할당 기준 //
		// 1 ~ 1024		byte 구간은 32 byte 단위
		// 1024 ~ 2048	byte 구간은 128 byte 단위
		// 2048 ~ 4096	byte 구간은 256 byte 단위
		// 이후는 할당기를 통해서 할당
		POOL_COUNT = (1024 / 32) + (1024 / 128) + (2048 / 256),
		MAX_ALLOC_SIZE = 4096
	};

public:
	Memory();
	~Memory();

	void* Allocate(int32 size);
	void Release(void* ptr);

private:
	void CreateMemoryPool(const int32 start, const int32 end, const int32 offset, int32& tableIdx);

private:

	// 메모리 풀 //
	std::vector<MemoryPool*> _pools;

	// 메모리 풀을 상수 시간(O(1)) 안에 찾기 위한 helper 테이블 //
	// 메모리 크기 <-> 메모리 풀
	MemoryPool* _poolTable[MAX_ALLOC_SIZE + 1];
};


/*------------
	 xnew
------------*/

template<typename Type, typename... Args>
Type* xnew(Args&&... args)	//-> 보편 참조(univeral reference)로 매개 변수 전달
{
	// 메모리 할당 정책에 따라 메모리 할당
	// Type* memory = static_cast<Type*>(BaseAllocator::Alloc(sizeof(Type)));
	Type* memory = static_cast<Type*>(xxalloc(sizeof(Type)));

	// 할당된 메모리에 객체 생성(placement new)
	new(memory) Type(std::forward<Args>(args)...);

	return memory;
}

template<typename Type>
void xdelete(Type* ptr)
{
	ptr->~Type();
	// BaseAllocator::Release(ptr);
	xxrelease(ptr);
}