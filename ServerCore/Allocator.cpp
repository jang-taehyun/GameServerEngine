#include "pch.h"
#include "Allocator.h"

/*----------------------
	 BaseAllocator
----------------------*/

void* Memory::BaseAllocator::Alloc(int32 size)
{
	return ::malloc(size);
}

void Memory::BaseAllocator::Release(void* ptr)
{
	::free(ptr);
}
