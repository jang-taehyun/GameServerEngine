#include "pch.h"
#include <iostream>
#include <chrono>

#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>

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

    SOCKET clientSocket = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (INVALID_SOCKET == clientSocket)
    {
        ErrorHandling("socket()");
        return 0;
    }

    SOCKADDR_IN serverAddr;
    ::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    ::inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
    serverAddr.sin_port = htons(7777);
    
    while (true)
    {
        char sendBuffer[100] = "Hello World!";

        // 나의 IP 주소, 포트 번호가 자동으로 설정됨.
        int32 errorCode = ::sendto(clientSocket, sendBuffer, sizeof(sendBuffer), 0, reinterpret_cast<SOCKADDR*>(&serverAddr), sizeof(serverAddr));
        if (SOCKET_ERROR == errorCode)
        {
            ErrorHandling("sendto()");
            return 0;
        }
        std::cout << "Send Data! Len : " << sizeof(sendBuffer) << std::endl;

        char recvBuffer[1000] = { NULL, };
        SOCKADDR_IN recvAddr = {};
        ::memset(&recvAddr, 0, sizeof(recvAddr));
        int32 addrLen = sizeof(recvAddr);
        int32 recvLen = ::recvfrom(clientSocket, recvBuffer, sizeof(recvBuffer), 0, reinterpret_cast<SOCKADDR*>(&recvAddr), &addrLen);
        if (recvLen <= 0)
        {
            ErrorHandling("recvfrom()");
            return 0;
        }
        std::cout << "Receive Data! Data : " << recvBuffer << ", Len : " << recvLen << std::endl;

        using std::chrono::operator""s;
        std::this_thread::sleep_for(1s);
    }

    ::closesocket(clientSocket);
    ::WSACleanup();

    return 0;
}