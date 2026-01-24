#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>
#include <thread>
#include <chrono>

#include "Allocator.h"

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
    {
        Vector<Knight> v(5);
        Map<int32, Knight> m;
        m[100] = Knight();
    }
    
    int a = 1;

    return 0;
}