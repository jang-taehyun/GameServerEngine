#pragma once

#include "Job.h"


/*------------
	 Room
------------*/

class Room
{
public:
	// 멀티쓰레드 환경에서는 일감으로 접근
	void PushJob(JobRef job) { _jobs.Push(job); }
	void FlushJob();

	template<typename T, typename Ret, typename... Args>
	void PushJob(Ret(T::* memFunc)(Args...), Args... args)
	{
		auto job = MakeShared<MemberFunctionJob<T, Ret, Args...>>(static_cast<T*>(this), memFunc, args...);
		_jobs.Push(job);
	}

public:
	// 싱글쓰레드 환경인 마냥 코딩해도 됨.
	void Enter(PlayerRef player);
	void Leave(PlayerRef player);
	void Broadcast(SendBufferRef sendBuffer);

private:
	Map<uint64, PlayerRef> _players;
	JobQueue _jobs;
};