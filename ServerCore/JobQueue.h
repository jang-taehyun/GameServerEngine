#pragma once

#include "Job.h"
#include "LockQueue.h"
#include "JobTimer.h"


/*-----------------
	 Job Queue
-----------------*/

class JobQueue : public std::enable_shared_from_this<JobQueue>
{
public:
	// 람다를 사용할 때 호출할 함수 //
	void DoAsync(CallbackType&& callback)
	{
		Push(ObjectPool<Job>::MakeShared(std::move(callback)));
	}

	// 그외 //
	template<typename T, typename Ret, typename... Args>
	void DoAsync(Ret(T::*memFunc)(Args...), Args... args)
	{
		std::shared_ptr<T> owner = std::static_pointer_cast<T>(shared_from_this());
		Push(ObjectPool<Job>::MakeShared(owner, memFunc, std::forward<Args>(args)...));
	}

	// 람다를 사용할 때 호출할 함수 //
	void DoTimer(uint64 tickAfter, CallbackType&& callback)
	{
		JobRef job = ObjectPool<Job>::MakeShared(std::move(callback));
		GJobTimer->Reserve(tickAfter, shared_from_this(), job);
	}

	// 그외 //
	template<typename T, typename Ret, typename... Args>
	void DoTimer(uint64 tickAfter, Ret(T::* memFunc)(Args...), Args... args)
	{
		std::shared_ptr<T> owner = std::static_pointer_cast<T>(shared_from_this());
		JobRef job = ObjectPool<Job>::MakeShared(owner, memFunc, std::forward<Args>(args)...);
		GJobTimer->Reserve(tickAfter, shared_from_this(), job);
		// TODO: 이상한데??
	}

	void ClearJobs() { _jobs.Clear(); }

public:
	void Push(JobRef job, bool pushOnly = false);
	void Execute();

protected:
	LockQueue<JobRef> _jobs;
	Atomic<int32> _jobCount = 0;
};

