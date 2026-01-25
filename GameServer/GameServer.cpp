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

class Knight
{
public:
    Knight() : _hp(100), _mp(10) { std::cout << "Knight()" << std::endl; }
    ~Knight() { std::cout << "~Knight()" << std::endl; }

    int32 _hp;
    int32 _mp;
};

int main()
{
    for (int32 i = 0; i < 5; ++i)
    {
        GThreadManager->Launch(
            []()
            {
                using std::literals::chrono_literals::operator""ms;

                while (true)
                {
                    Vector<Knight> v(10);
                    Map<int32, Knight> m;
                    m[100] = Knight();

                    std::this_thread::sleep_for(10ms);
                }
            }
        );
    }

    GThreadManager->Join();

    return 0;
}