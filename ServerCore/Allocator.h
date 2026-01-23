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