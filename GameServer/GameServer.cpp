#include "pch.h"
#include <iostream>

#include <atomic>
#include <mutex>
#include <future>
#include <thread>
#include <chrono>

#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>

#include "ThreadManager.h"

void ErrorHandling(const char* cause)
{
    int32 errorCode = ::WSAGetLastError();
    std::cout << cause << " ErrorCode : " << errorCode << std::endl;
}

int main()
{
    int32 errorCode = 0;
    WSADATA wsaData;

    if (errorCode = ::WSAStartup(MAKEWORD(2, 2), &wsaData))
    {
        return errorCode;
    }

    SOCKET serverSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (INVALID_SOCKET == serverSocket)
    {
        ErrorHandling("socket()");
        return 0;
    }

    LINGER linger;
    linger.l_onoff = 1;
    linger.l_linger = 5;
    ::setsockopt(serverSocket, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger));

    ::closesocket(serverSocket);
    ::WSACleanup();

    return 0;
}