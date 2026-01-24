#pragma once

/**
* 할당 정책 정의
*/


/*----------------------
	 BaseAllocator
----------------------*/

// 기본적으로 사용할 할당자 //
class BaseAllocator
{
public:
	static void* Alloc(int32 size);
	static void Release(void* ptr);
};


/*----------------------
	 StompAllocator
----------------------*/

class StompAllocator
{
	enum { PAGE_SIZE = 0x1000 };

public:
	static void* Alloc(int32 size);
	static void Release(void* ptr);
};


/*----------------------
	 STL Allocator
----------------------*/

template<typename T>
class STLAllocator
{
public:
	using value_type = T;

	STLAllocator() {}

	template<typename Other>
	STLAllocator(const STLAllocator<Other>& _other) {}

	// 할당하는 함수 //
	// count : 원소의 개수(크기가 아님)
	T* allocate(size_t count)
	{
		// 할당할 메모리의 크기 구하기
		const int32 size = static_cast<int32>(count * sizeof(T));
		return static_cast<T*>(xxalloc(size));
	}

	void deallocate(T* ptr, size_t count)
	{
		xxrelease(ptr);
	}
};