#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>
#include <thread>
#include <chrono>

#include "Memory.h"

class Knight
{
public:
    Knight() : _hp(0), _tmp(1) { std::cout << "Knight()" << std::endl; }
    ~Knight() { std::cout << "~Knight()" << std::endl; }

    int32 _hp;
    int32 _tmp;
};

int main()
{
    Knight* knight = xnew<Knight>();
    xdelete(knight);

    knight->_hp = 200;

    return 0;
}