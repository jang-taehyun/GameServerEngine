#pragma once

#include "Job.h"
#include "JobQueue.h"


/*----------------------
	 Job Serializer
----------------------*/

class JobSerializer : public std::enable_shared_from_this<JobSerializer>
{
public:
	// 람다를 사용할 때 호출할 함수 //
	void PushJob(CallbackType&& callback)
	{
		auto job = ObjectPool<Job>::MakeShared(std::move(callback));
		_jobQueue.Push(job);
	}

	// 그외 //
	template<typename T, typename Ret, typename... Args>
	void PushJob(Ret(T::*memFunc)(Args...), Args... args)
	{
		std::shared_ptr<T> owner = std::static_pointer_cast<T>(shared_from_this());
		auto job = ObjectPool<Job>::MakeShared(owner, memFunc, args...);
		_jobQueue.Push(job);
	}

	virtual void FlushJob() = 0;

protected:
	JobQueue _jobQueue;
};

