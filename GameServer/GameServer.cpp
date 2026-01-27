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
#include "Memory.h"

/**
* MS에서 제공하는 SLIST_ENTRY
* -> MS에서 제공하는 Lock-free List
* -> Lock-free stack을 만들때 사용된다.
* 
* MS에서 제공하는 SLIST_ENTRY를 사용하는 2가지 방법
* 1) SLIST_ENTRY를 상속
*   -> 포인터의 casting이 가능해짐
*   -> 상속을 받는다면, SLIST_ENTRY 안에 있는 멤버 변수들을 맨처음으로 받을 수 있음
* 2) SLIST_ENTRY를 멤버 변수의 맨처음 변수로 갖는다.
* 
* SLIST_ENTRY를 사용한다면,
* -> header를 만들고 초기화해줘야 한다.
*   -> PSLIST_HEADER Gheader;
* -> 반드시 데이터(객체)는 16byte로 정렬해야 한다.
*   -> DECLSPEC_ALIGN(16)
*/

class Knight
{
public:
    int32 _hp = rand() % 1000;
};

int main()
{
    for (int32 i = 0; i < 5; ++i)
    {
        GThreadManager->Launch(
            []()
            {
                using namespace std::chrono;

                while (true)
                {
                    Knight* knight = xnew<Knight>();

                    std::cout << (knight->_hp) << std::endl;

                    std::this_thread::sleep_for(10ms);

                    xdelete(knight);
                }
            }
        );
    }

    GThreadManager->Join();

    return 0;
}