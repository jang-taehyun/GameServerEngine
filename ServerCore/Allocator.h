#pragma once

/**
* 할당 정책 정의
*/


/*----------------------
	 BaseAllocator
----------------------*/

// 기본적으로 사용할 할당자 //
namespace Memory
{
	class BaseAllocator
	{
	public:
		static void* Alloc(int32 size);
		static void Release(void* ptr);
	};
}