#pragma once

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
		_refCount.fetch_sub(1);
		if (0 == _refCount.load())
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