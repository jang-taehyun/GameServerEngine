#pragma once

#include "Allocator.h"

/*------------
	 xnew
------------*/

namespace Memory
{
	template<typename Type, typename... Args>
	Type* xnew(Args&&... args)	//-> 보편 참조(univeral reference)로 매개 변수 전달
	{
		// 메모리 할당 정책에 따라 메모리 할당
		// Type* memory = static_cast<Type*>(BaseAllocator::Alloc(sizeof(Type)));
		Type* memory = static_cast<Type*>(Memory::xalloc(sizeof(Type)));

		// 할당된 메모리에 객체 생성(placement new)
		new(memory) Type(std::forward<Args>(args)...);

		return memory;
	}

	template<typename Type>
	void xdelete(Type* ptr)
	{
		ptr->~Type();
		// BaseAllocator::Release(ptr);
		Memory::xrelease(ptr);
	}
}