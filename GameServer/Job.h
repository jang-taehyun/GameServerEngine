#pragma once

/**
* 1세대 방식
* - Job에 대한 클래스를 하나하나 늘려나가는 방식
* 
* 장점
* - 직관적이다.
* 
* 단점
* - 컨텐츠 만들 때 job에 대한 일감을 만들어야 한다.
*/

/*---------------------
	 Interface Job
---------------------*/

class IJob
{
public:
	virtual void Execute() = 0;
};


// SAMPLE //
class HealJob : public IJob
{
public:
	virtual void Execute() override
	{
		// _target을 찾아서
		// _target->AddHP(_healValue) 호출

		using namespace std;
		cout << _target << "한테 힐" << _healValue << " 만큼 줌" << endl;
	}

public:
	uint64 _target = 0;
	uint32 _healValue = 0;
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