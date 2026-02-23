#pragma once

/**
* 2세대 방식
* - 공용 클래스를 만들어서 job을 실행하는 방식
* 
* 장점
* - 매번마다 클래스를 만들지 않아도, 함수 포인터와 인자만 넘겨주면 된다.
* 
* 단점
* - 
*/


/*---------------------
	 Interface Job
---------------------*/

class IJob
{
public:
	virtual void Execute() = 0;
};


/*---------------------------------------------------
	 C++ 11 이전에 존재했던 apply() 함수 구현 방법
---------------------------------------------------*/

// std::tuple에 접근할 index를 만드는 template struct //

template<int... Remains>
struct seq {};

template<int N, int... Remains>
struct gen_seq : gen_seq<N - 1, N - 1, Remains...> {};

template<int... Remains>
struct gen_seq<0, Remains...> : seq<Remains...> {};


// 전역 함수 //

template<typename Ret, typename... Args>
void xapply(Ret(*func)(Args...), std::tuple<Args...>& tup)
{
	// sizeof...(Args) -> Args로 넣어준 파라미터의 개수를 반환
	return xapply_helper(func, gen_seq<sizeof...(Args)>(), tup);
}

template<typename F, typename... Args, int... ls>
void xapply_helper(F func, seq<ls...>, std::tuple<Args...>& tup)
{
	(func)(std::get<ls>(tup)...);
}


// 클래스 멤버 함수 //

template<typename T, typename Ret, typename... Args>
void xapply(T* obj, Ret(T::* func)(Args...), std::tuple<Args...>& tup)
{
	// sizeof...(Args) -> Args로 넣어준 파라미터의 개수를 반환
	xapply_helper(obj, func, gen_seq<sizeof...(Args)>(), tup);
}

template<typename T, typename F, typename... Args, int... ls>
void xapply_helper(T* obj, F func, seq<ls...>, std::tuple<Args...>& tup)
{
	(obj->*func)(std::get<ls>(tup)...);
}


/*---------------------------------------------------*/


/*---------------------
	 Function Job
---------------------*/

template<typename Ret, typename... Args>
class FuncJob : public IJob
{
	using FuncType = Ret(*)(Args...);

public:
	FuncJob(FuncType func, Args... args) : _func(func), _tuple(args...)
	{

	}

	Ret operator()()
	{
		// C++ 17 이후인 경우, std::apply() 함수를 이용해 함수 포인터와 tuple를 연결해 호출할 수 있음
		//std::apply(_func, _tuple);

		// C++ 11 이전의 경우, 직접 만든 xapply() 템플릿 함수를 사용
		xapply(_func, _tuple);
	}

	virtual void Execute() override
	{
		// C++ 17 이후인 경우, std::apply() 함수를 이용해 함수 포인터와 tuple를 연결해 호출할 수 있음
		//std::apply(_func, _tuple);

		// C++ 11 이전의 경우, 직접 만든 xapply() 템플릿 함수를 사용
		xapply(_func, _tuple);
	}

private:
	FuncType _func;
	std::tuple<Args...>	_tuple;
};


/*----------------------------------
	 Class Memeber Function Job
----------------------------------*/

template<typename T, typename Ret, typename... Args>
class MemberFunctionJob : public IJob
{
	using FuncType = Ret(T::*)(Args...);

public:
	MemberFunctionJob(T* obj, FuncType func, Args... args) : _obj(obj), _func(func), _tuple(args...)
	{

	}

	Ret operator()()
	{
		// C++ 17 이후인 경우, std::apply() 함수를 이용해 함수 포인터와 tuple를 연결해 호출할 수 있음
		//std::apply(_obj, _func, _tuple);

		// C++ 11 이전의 경우, 직접 만든 xapply() 템플릿 함수를 사용
		xapply(_obj, _func, _tuple);
	}

	virtual void Execute() override
	{
		// C++ 17 이후인 경우, std::apply() 함수를 이용해 함수 포인터와 tuple를 연결해 호출할 수 있음
		//std::apply(_obj, _func, _tuple);

		// C++ 11 이전의 경우, 직접 만든 xapply() 템플릿 함수를 사용
		xapply(_obj, _func, _tuple);
	}

private:
	T* _obj;
	FuncType _func;
	std::tuple<Args...>	_tuple;
};


/*------------------
	 Job Queue
------------------*/

using JobRef = std::shared_ptr<IJob>;
class JobQueue
{
public:
	void Push(JobRef job)
	{
		WRITE_LOCK;
		_jobs.push(job);
	}

	JobRef Pop()
	{
		WRITE_LOCK;
		if (_jobs.empty())
			return nullptr;

		JobRef ret = _jobs.front();
		_jobs.pop();

		return ret;
	}

private:
	USE_LOCK;
	Queue<JobRef> _jobs;
};