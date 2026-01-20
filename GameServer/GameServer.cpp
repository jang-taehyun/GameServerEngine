#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>
#include <thread>
#include <chrono>
#include "ThreadManager.h"

class TestLock
{
    USE_LOCK;

public:
    int32 TestRead()
    {
        READ_LOCK;

        if (_queue.empty())
            return -1;

        return _queue.front();
    }

    void TestPush()
    {
        WRITE_LOCK;

        _queue.push(rand() % 100);
    }

    void TestPop()
    {
        WRITE_LOCK;

        if (false == _queue.empty())
            _queue.pop();
    }

private:
    std::queue<int32> _queue;
};

CoreGlobal GCoreGlobal;
TestLock testLock;

void ThreadWrite()
{
    using namespace std;

    while (true)
    {
        testLock.TestPush();
        this_thread::sleep_for(1ms);
        testLock.TestPop();
    }
}

void ThreadRead()
{
    using namespace std;

    while (true)
    {
        int32 value = testLock.TestRead();
        cout << value << endl;
        this_thread::sleep_for(1ms);
    }
}

int main()
{
    for (int32 i = 0; i < 2; ++i)
        GThreadManager->Launch(ThreadWrite);

    for (int32 i = 0; i < 2; ++i)
        GThreadManager->Launch(ThreadRead);

    GThreadManager->Join();

    return 0;
}