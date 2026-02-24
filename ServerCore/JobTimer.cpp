#include "pch.h"
#include "JobQueue.h"
#include "JobTimer.h"


/*----------------
     Job Timer
----------------*/

void JobTimer::Reserve(uint64 tickAfter, std::weak_ptr<JobQueue> owner, JobRef job)
{
    const uint64 executeTick = ::GetTickCount64() + tickAfter;
    JobData* jobData = ObjectPool<JobData>::Pop(owner, job);

    WRITE_LOCK;
    _items.push(TimerItem{ executeTick, jobData });
}

void JobTimer::Distribute(uint64 now)
{
    // 한 번에 한 개의 쓰레드만 통과
    if (true == _distributing.exchange(true))
        return;

    // 우선순위 큐에 있는 job들 중 실행할 수 있는 것들을 모두 꺼내온다.
    Vector<TimerItem> items;
    {
        WRITE_LOCK;
        while (false == _items.empty())
        {
            const TimerItem& timerItem = _items.top();
            if (now < timerItem.executeTick)
                break;

            items.push_back(timerItem);
            _items.pop();
        }
    }

    // 꺼내온 job들을 job queue에 넣는다.
    for (TimerItem& item : items)
    {
        JobQueueRef owner = item.jobData->_owner.lock();
        if (owner)
            owner->Push(item.jobData->_job);

        ObjectPool<JobData>::Push(item.jobData);
    }

    // 끝났으니 풀어준다.
    _distributing.store(false);
}

void JobTimer::Clear()
{
    WRITE_LOCK;

    while (false == _items.empty())
    {
        const TimerItem& timerItem = _items.top();
        ObjectPool<JobData>::Push(timerItem.jobData);
        _items.pop();
    }
}