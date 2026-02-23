#pragma once

#include "Job.h"
#include "LockQueue.h"


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

	void ClearJobs() { _jobs.Clear(); }

private:
	void Push(JobRef&& job);
	void Execute();

protected:
	LockQueue<JobRef> _jobs;
	Atomic<int32> _jobCount = 0;
};

