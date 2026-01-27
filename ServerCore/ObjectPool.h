#pragma once

/**
* object pool
* -> 동일한 메모리 영역을 가지면서, 동일한 객체를 모아놓은 것
*	-> 같은 객체들끼리 모을 때 유용
* -> memory pool이 object pool을 포함하는 개념이다.
*	-> memory pool과 비슷
* 
* 단점
* -> stomp allocator랑 같이 사용하는 것은 불가능하다.
*/

#include "Types.h"
#include "MemoryPool.h"

template<typename Type>
class ObjectPool
{
public:
	template<typename... Args>
	static std::shared_ptr<Type> MakeShared(Args&& ...args)
	{
		std::shared_ptr<Type> ptr(Pop(std::forward<Args>(args)...), Push);
		return ptr;
	}

public:
	template<typename... Args>
	static Type* Pop(Args&& ...args)
	{
		Type* memory = nullptr;

#ifdef _STOMP
		MemoryHeader* ptr = reinterpret_cast<MemoryHeader*>(StompAllocator::Alloc(s_allocSize));
		memory = static_cast<Type*>(MemoryHeader::AttachHeader(ptr, s_allocSize));
#else
		memory = static_cast<Type*>(MemoryHeader::AttachHeader(s_pool.Pop(), s_allocSize));
#endif

		new(memory) Type(std::forward<Args>(args)...);
		return memory;
	}

	static void Push(Type* obj)
	{
		obj->~Type();

#ifdef _STOMP
		StompAllocator::Release(MemoryHeader::DetachHeader(obj));
#else
		s_pool.Push(MemoryHeader::DetachHeader(obj));
#endif
	}

private:
	static int32		s_allocSize;
	static MemoryPool	s_pool;
};

template<typename Type>
int32 ObjectPool<Type>::s_allocSize = sizeof(Type) + sizeof(MemoryHeader);

template<typename Type>
MemoryPool ObjectPool<Type>::s_pool(s_allocSize);