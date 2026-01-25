#pragma once

/*----------------------
* Memory Pool을 통해 할당된 메모리의 구조
* [MemoryHeader][Data]
* MemoryHeader의 역할
* - 디버깅을 도와주는 역할
* - 객체마다 크기가 모두 다르기 때문에 이를 추적, 관리하기 위해서 header를 붙여주자.
* STL에서도 MemoryHeader과 비슷한 역할을 하는 애가 있다.
* -> 할당된 메모리의 크기, 다음 주소 등등을 저장

* 원리 : 하나의 커다란 메모리 영역을 할당 한 후 같은 크기로 쪼개서 넘겨준다
* -> 내부 단편화가 발생할 수도 있겠네??
* -> 근데 평소와 같이 할당하면 추후에 외부 단편화가 발생할 수 있어.
* -> 그러면 compaction 같은 방식으로 메모리를 최적화해야 하는데, 그러기엔 성능이 저하될거 같아
* -> 왜냐? 레이드 돌때 1분 1초 1프레임 하나하나가 빠른 시간에 동작해야 하는데, compaction을 한 후 할당하면 시간이 너무 오래 걸려
* -> 그래서 내부 단편화가 발생하더라도 메모리 풀링을 이용하는게 더 좋을거 같아
* -> 메모리 풀링을 사용하면 사이드 이펙트로 메모리 관리도 용이해진다는 장점도 따라오니까 괜찮은 방법인거 같아.

* 메모리 풀을 사용하는 이유
* -> 메모리 파편화를 방지하기 위해
* -> 비슷한 크기의 메모리를 재사용하기 위해
* 
* 현재 추세
* -> windows 쪽은 메모리 풀리을 안해도 큰 성능 저하가 없음
* -> linux 쪽은 여전히 느려서 메모리 풀을 사용하는 경우가 있음
----------------------*/

/*----------------------
	 Memory Header
----------------------*/

struct MemoryHeader
{
	MemoryHeader(int32 size) : allocSize(size) {}

	// 메모리를 받아서 헤더를 만들어주는 함수 //
	// 데이터가 들어갈 영역 반환
	static void* AttachHeader(MemoryHeader* header, int32 size)
	{
		// 할당된 메모리에 header를 할당(placement new)
		new(header) MemoryHeader(size);

		// Data 영역의 포인터 반환
		return reinterpret_cast<void*>(++header);
	}

	// 메모리를 받아서 반납하는 함수 //
	// header를 포함한 할당받은 영역을 반환
	static MemoryHeader* DetachHeader(void* ptr)
	{
		MemoryHeader* header = reinterpret_cast<MemoryHeader*>(ptr) - 1;
		return header;
	}


	// 할당된 메모리의 크기
	int32 allocSize;
};


/*----------------------
	 Memory Pool
----------------------*/

// [MemoryHeader][Data] //
class MemoryPool
{
public:
	MemoryPool(int32 allocSize);
	~MemoryPool();

	void				Push(MemoryHeader* ptr);
	MemoryHeader*		Pop();

private:

	// 객체가 담당하고 있는 메모리 풀의 크기
	int32 _allocSize;

	// 메모리를 할당한 횟수
	std::atomic<int32> _allocCount;

	USE_LOCK;
	std::queue<MemoryHeader*> _queue;
};

