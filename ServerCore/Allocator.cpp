#include "pch.h"
#include "Allocator.h"

/*----------------------
	 BaseAllocator
----------------------*/

void* BaseAllocator::Alloc(int32 size)
{
	return ::malloc(size);
}

void BaseAllocator::Release(void* ptr)
{
	::free(ptr);
}


/*----------------------
	 StompAllocator
----------------------*/

void* StompAllocator::Alloc(int32 size)
{
	// 나머지를 올림해서 page의 개수를 구한다.
	const int64 pageCount = (size + (PAGE_SIZE - 1)) / PAGE_SIZE;

	// base 구하기(페이지의 시작 주소)
	void* baseAddress = ::VirtualAlloc(NULL, pageCount * PAGE_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	// offset 구하기
	// 할당한 메모리 영역에서 size를 빼면, 시작 주소가 나온다.
	const int64 offset = (pageCount * PAGE_SIZE) - static_cast<int64>(size);

	return static_cast<void*>(static_cast<int8*>(baseAddress) + offset);
}

void StompAllocator::Release(void* ptr)
{
	// 포인터를 정수로 변환
	const int64 address = reinterpret_cast<int64>(ptr);

	// page의 시작 주소 구하기
	// 주소에서 페이지의 크기를 나눈 나머지를 주소에서 빼면 페이지의 시작 주소가 나온다.
	const int64 baseAddress = address - (address % PAGE_SIZE);

	::VirtualFree(reinterpret_cast<void*>(baseAddress), 0, MEM_RELEASE);
}