#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>
#include "CoreMacro.h"
#include "ThreadManager.h"

CoreGlobal Core;

void ThreadMain()
{
    using namespace std;

    while (true)
    {
        cout << "Hello! I'm thread..." << LThreadID << endl;
        this_thread::sleep_for(1s);
    }
}

int main()
{
    // crash가 nullptr이 아니라고 가정(컴파일러가 에러를 잡을 수 있기 때문에 __analysis_assume() 매크로를 사용해 컴파일 단계에서는 넘어가도록 설정
    // CRASH("TEST");

    // uint32 a = 3;
    // ASSERT_CRASH(a != 3);

    for (int32 i = 0; i < 5; ++i)
        GThreadManager->Launch(ThreadMain);
    GThreadManager->Join();

    return 0;
}