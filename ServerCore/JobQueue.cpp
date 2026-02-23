#include "pch.h"
#include "JobQueue.h"


/*-----------------
	 Job Queue
-----------------*/

void JobQueue::Push(JobRef&& job)
{
	const int32 prevCount = _jobCount.fetch_add(1);
	_jobs.Push(job);		// WRITE_LOCK

	// 첫번째 job을 넣은 쓰레드가 실행까지 담당한다.
	if (0 == prevCount)
	{
		Execute();
	}
}

void JobQueue::Execute()
{
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
			return;
		}
	}
}