#pragma once

/**
* RefCountable, TSharedPtr 조합의 한계
* 1) 이미 만들어진 클래스 대상으로 사용 불가(외부 라이브러리를 사용하는 경우 사용 불가)
*	 -> RefCountable 클래스를 상속받은 객체만 TSharedPtr 클래스을 사용할 수 있기 때문
* 2) 순환 참조 문제
*	 - 서로 reference count가 순환이 되어서 서로의 reference count가 0이 되지 않아, 양쪽 모두 메모리 해제가 되지 않는 문제(memory leak 발생)
*		-> A가 B를 주시하고 있고, B가 A를 주시하고 있다면, A와 B는 서로 reference count를 올렸기 때문에, 양쪽이 모두 메모리 해제가 되지 않음
*	 -> std::shared_ptr 클래스도 가지고 있는 문제
*/

/*-----------------------------
		RefCountalbe
-----------------------------*/

class RefCountable
{
public:
	RefCountable() : _refCount(1) {}
	virtual ~RefCountable() {}

	int32 GetRefCount() { return _refCount.load(); }

	int32 AddRef()
	{ 
		_refCount.fetch_add(1);
		return _refCount.load();
	}

	int32 ReleaseRef()
	{
		// 0이 되었다는 것은 아무도 객체를 기억하고 있지 않기 때문에 멀티 쓰레드 환경에서 삭제 가능
		if (0 == --_refCount)
		{
			delete this;
		}

		return _refCount.load();
	}

private:
	std::atomic<int32> _refCount;
};


/*-------------------------
		TSharedPtr
-------------------------*/

template<typename T>
class TSharedPtr
{
public:
	TSharedPtr() : _ptr(nullptr) {}
	TSharedPtr(T* ptr) { Set(ptr); }
	TSharedPtr(const TSharedPtr& rhs) { Set(rhs._ptr); }
	TSharedPtr(TSharedPtr&& rhs)
	{
		_ptr = rhs._ptr;
		rhs._ptr = nullptr;
	}

	// 상속 관계 복사
	template<typename U>
	TSharedPtr(const TSharedPtr<U>& rhs) { Set(static_cast<T>(rhs._ptr)); }

	// 상속 관계 이동
	template<typename U>
	TSharedPtr(TSharedPtr<U>&& rhs) noexcept
	{
		_ptr = rhs._ptr;
		rhs._ptr = nullptr;
	}

	~TSharedPtr() { Release(); }


	// 인터페이스 //
public:
	bool IsNull() const { return (nullptr == _ptr); }


	// 연산자 //
public:
	TSharedPtr& operator=(const TSharedPtr& rhs)
	{
		if (_ptr != rhs._ptr)
		{
			Release();
			Set(rhs._ptr);
		}

		return *this;
	}

	TSharedPtr& operator=(TSharedPtr&& rhs) noexcept
	{
		if (_ptr != rhs._ptr)
		{
			Release();
			_ptr = rhs._ptr;
			rhs._ptr = nullptr;
		}

		return *this;
	}

	bool		operator==		(const TSharedPtr& rhs) const	{ return (_ptr == rhs._ptr); }
	bool		operator==		(TSharedPtr&& rhs)		const	{ return (_ptr == rhs._ptr); }
	bool		operator==		(T* ptr)				const	{ return (_ptr == ptr); }
	bool		operator!=		(const TSharedPtr& rhs) const	{ return (_ptr != rhs._ptr); }
	bool		operator!=		(TSharedPtr&& rhs)		const	{ return (_ptr != rhs._ptr); }
	bool		operator!=		(T* ptr)				const	{ return (_ptr != ptr); }
	bool		operator<		(const TSharedPtr& rhs) const	{ return (_ptr < rhs._ptr); }
	bool		operator<		(TSharedPtr&& rhs)		const	{ return (_ptr < rhs._ptr); }
	bool		operator<		(T* ptr)				const	{ return (_ptr < ptr); }
	bool		operator>		(const TSharedPtr& rhs) const	{ return (_ptr > rhs._ptr); }
	bool		operator>		(TSharedPtr&& rhs)		const	{ return (_ptr > rhs._ptr); }
	bool		operator>		(T* ptr)				const	{ return (_ptr > ptr); }
	bool		operator<=		(const TSharedPtr& rhs) const	{ return (_ptr <= rhs._ptr); }
	bool		operator<=		(TSharedPtr&& rhs)		const	{ return (_ptr <= rhs._ptr); }
	bool		operator<=		(T* ptr)				const	{ return (_ptr <= ptr); }
	bool		operator>=		(const TSharedPtr& rhs) const	{ return (_ptr >= rhs._ptr); }
	bool		operator>=		(TSharedPtr&& rhs)		const	{ return (_ptr >= rhs._ptr); }
	bool		operator>=		(T* ptr)				const	{ return (_ptr >= ptr); }
	T&			operator*		()								{ return *_ptr; }
	const T&	operator*		()						const	{ return *_ptr; }

	T*			operator->		()								{ return _ptr; }
	const T*	operator->		()						const	{ return _ptr; }

private:
	inline void Set(T* ptr)
	{
		_ptr = ptr;
		if(ptr)
			ptr->AddRef();
	}

	inline void Release()
	{
		if (nullptr != _ptr)
		{
			_ptr->ReleaseRef();
			_ptr = nullptr;
		}
	}

private:
	T* _ptr;
};