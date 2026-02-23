#include "pch.h"
#include "GlobalQueue.h"


/*--------------------
	 Global Queue
--------------------*/

GlobalQueue::GlobalQueue()
{
}

GlobalQueue::~GlobalQueue()
{
}

void GlobalQueue::Push(JobQueueRef jobQueue)
{
	_jobQueues.Push(jobQueue);
}

JobQueueRef GlobalQueue::Pop()
{
	return _jobQueues.Pop();
}