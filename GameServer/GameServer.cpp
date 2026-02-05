#include "pch.h"

#include <atomic>
#include <mutex>
#include <future>
#include <thread>
#include <chrono>

#include "ThreadManager.h"
#include "SocketUtils.h"

int main()
{
    std::cout << "I'm server!!" << std::endl;

    SOCKET socket = SocketUtils::CreateSocket();

    SocketUtils::Listen(socket);

    SOCKET clientSocket = ::accept(socket, nullptr, nullptr);

    std::cout << "Client Connected!" << std::endl;

    while (true);

    GThreadManager->Join();

    return 0;
}
