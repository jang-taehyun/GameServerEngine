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
    Knight() { std::cout << "Knight()" << std::endl; }
    ~Knight() { std::cout << "~Knight()" << std::endl; }

    // void* operator new(size_t size)
    // {
    //     std::cout << "operator new " << size << std::endl;
    //     void* ptr = ::malloc(size);
    //     return ptr;
    // }
    // 
    // void operator delete(void* ptr)
    // {
    //     std::cout << "operator delete" << std::endl;
    //     ::free(ptr);
    // }

private:
    int32 _hp;
    int32 _tmp;
};

// void* operator new(size_t size)
// {
//     std::cout << "operator new " << size << std::endl;
//     void* ptr = ::malloc(size);
//     return ptr;
// }
// 
// void operator delete(void* ptr)
// {
//     std::cout << "operator delete" << std::endl;
//     ::free(ptr);
// }

int main()
{
    Knight* knight = Memory::xnew<Knight>();
    Memory::xdelete(knight);

    return 0;
}