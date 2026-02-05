#include "pch.h"

#include <atomic>
#include <mutex>
#include <future>
#include <thread>
#include <chrono>

#include "ThreadManager.h"
#include "Listener.h"

int main()
{
    Listener listener;
    listener.StartAccept(NetworkAddress(L"127.0.0.1", 7777));

    for (int32 i = 0; i < 5; ++i)
    {
        GThreadManager->Launch(
            [=]()
            {
                while (true)
                {
                    GIOCPCore.Dispatch();
                }
            }
        );
    }

    GThreadManager->Join();

    return 0;
}
