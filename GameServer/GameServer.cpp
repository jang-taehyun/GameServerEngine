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

/**
* MS에서 제공하는 SLIST_ENTRY
* -> MS에서 제공하는 Lock-free List
* -> Lock-free stack을 만들때 사용된다.
* 
* MS에서 제공하는 SLIST_ENTRY를 사용하는 2가지 방법
* 1) SLIST_ENTRY를 상속
* 2) SLIST_ENTRY를 멤버 변수의 맨처음 변수로 갖는다.
* 
* SLIST_ENTRY를 사용한다면,
* -> header를 만들고 초기화해줘야 한다.
*   -> PSLIST_HEADER Gheader;
* -> 반드시 16byte로 정렬해야 한다.
*   -> DECLSPEC_ALIGN(16)
*/
DECLSPEC_ALIGN(16)
class Data : public SLIST_ENTRY
{
public:
    int32 rand = ::rand() % 1000;
};

PSLIST_HEADER Gheader = nullptr;

int main()
{
    // PSLIST_HEADER(SLIST_HEADER*) 생성 및 초기화
    Gheader = new SLIST_HEADER;
    ASSERT_CRASH((0 == (uint64)Gheader % 16));
    ::InitializeSListHead(Gheader);

    for (int32 i = 0; i < 3; ++i)
    {
        GThreadManager->Launch(
            []()
            {
                using namespace std::literals::chrono_literals;

                while (true)
                {
                    Data* data = new Data;
                    ASSERT_CRASH((0 == (uint64)data % 16));

                    // SLIST_HEADER에서 데이터를 push
                    ::InterlockedPushEntrySList(Gheader, (PSLIST_ENTRY)data);

                    std::this_thread::sleep_for(10ms);
                }
            }
        );
    }

    for (int32 i = 0; i < 2; ++i)
    {
        GThreadManager->Launch(
            []()
            {
                using namespace std::literals::chrono_literals;

                while (true)
                {
                    // SLIST_HEADER에서 데이터를 pop
                    Data* pop = static_cast<Data*>(::InterlockedPopEntrySList(Gheader));

                    if (pop)
                    {
                        std::cout << (pop->rand) << std::endl;
                        delete pop;
                    }
                    else
                    {
                        std::cout << "NONE" << std::endl;
                    }

                    std::this_thread::sleep_for(10ms);
                }
            }
        );
    }

    GThreadManager->Join();

    return 0;
}