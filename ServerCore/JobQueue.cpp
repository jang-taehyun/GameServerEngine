#include "pch.h"
#include "JobQueue.h"
#include "GlobalQueue.h"


/*-----------------
	 Job Queue
-----------------*/

void JobQueue::Push(JobRef job, bool pushOnly)
{
	const int32 prevCount = _jobCount.fetch_add(1);
	_jobs.Push(job);		// WRITE_LOCK

	// 첫번째 job을 넣은 쓰레드가 실행까지 담당한다.
	if (0 == prevCount)
	{
		// 이미 실행중인 JobQueue가 없으면 실행
		if (nullptr == LCurrentJobQueue && false == pushOnly)
			Execute();
	}
	else
	{
		// 여유 있는 다른 쓰레드가 실행하도록 GlobalQueue에 넘긴다.
		GGlobalQueue->Push(shared_from_this());
	}
}

void JobQueue::Execute()
{
	LCurrentJobQueue = this;

	while (true)
	{
		Vector<JobRef> jobs;
		_jobs.PopAll(OUT jobs);

		const int32 jobCount = static_cast<int32>(jobs.size());
		for (int32 i = 0; i < jobCount; ++i)
			jobs[i]->Execute();

		// 남은 job의 개수가 0개라면 종료
		if (jobCount == _jobCount.fetch_sub(jobCount))
		{
			LCurrentJobQueue = nullptr;
			return;
		}

		// 실행할 수 있는 시간 초과
		const uint64 now = ::GetTickCount64();
		if (now >= LEndTickCount)
		{
			LCurrentJobQueue = nullptr;

			// 여유 있는 다른 쓰레드가 실행하도록 GlobalQueue에 넘긴다.
			GGlobalQueue->Push(shared_from_this());

			return;
		}
	}
}