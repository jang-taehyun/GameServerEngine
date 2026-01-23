#pragma once

#include "Types.h"
#include "Allocator.h"

/*------------
	 Lock
------------*/

#define USE_MANY_LOCK(count)	Lock _locks[count]
#define USE_LOCK				USE_MANY_LOCK(1)
#define READ_LOCK_IDX(idx)		ReadLockGuard readLockGuard_##idx(_locks[idx], typeid(this).name())
#define READ_LOCK				READ_LOCK_IDX(0)
#define WRITE_LOCK_IDX(idx)		WriteLockGuard writeLockGuard_##idx(_locks[idx], typeid(this).name())
#define WRITE_LOCK				WRITE_LOCK_IDX(0)


/*------------
	 Memory
------------*/

// allocator(할당기) //
namespace Memory
{
#ifdef _DEBUG
#define xalloc(size)	Memory::BaseAllocator::Alloc(size)
#define xrelease(ptr)	Memory::BaseAllocator::Release(ptr)
#else 
#define xalloc(size)	Memory::BaseAllocator::Alloc(size)
#define xrelease(ptr)	Memory::BaseAllocator::Release(ptr)
#endif
}

/*------------
	Crash
------------*/

// 인위적인 크래시 발생 매크로 //
#define CRASH(cause)						\
{											\
	uint32* crash = nullptr;				\
	__analysis_assume(crash != nullptr);	\
	*crash = (uint32)0xDEADBEEF;			\
}											\

// 조건부 크래시 발생 매크로 //
#define ASSERT_CRASH(expr)					\
{											\
	if (!(expr))							\
	{										\
		CRASH("ASSERT_CRASH");				\
		__analysis_assume(expr);			\
	}										\
}											\