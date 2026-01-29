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

    SOCKET serverSocket = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (INVALID_SOCKET == serverSocket)
    {
        ErrorHandling("socket()");
        return 0;
    }

    SOCKADDR_IN serverAddr;
    ::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
    serverAddr.sin_port = htons(7777);

    if (SOCKET_ERROR == ::bind(serverSocket, reinterpret_cast<SOCKADDR*>(&serverAddr), sizeof(serverAddr)))
    {
        ErrorHandling("bind()");
        return 0;
    }

    /**
    * TCP 소켓과 UDP 소켓의 차이
    * -> UDP의 경우 서버에서는 listen(), accept() 함수를 호출하지 않는다.
    *    -> bind()까지 되었다면, 바로 recvfrom(), sendto() 함수를 호출
    * -> UDP의 경우 클라이언트에서는 connect() 함수를 호출하지 않는다.
    *    -> 서버 정보까지 입력되었다면, 바로 recvfrom(), sendto() 함수를 호출
    * -> UDP의 경우, 수신하는 호스트에는 server socket 하나만 있으면 된다.
    *    -> but TCP는 수신하는 호스트에서 송신하는 호스트의 소켓을 하나 더 만든다.
    *    -> ex) TCP의 경우, 클라이언트가 10개라면 서버에서는 11개가 생성된다.(서버 소켓 1개 + 클라이언트 소켓 10개)
    * 
    * client의 경우, TCP 소켓, UDP 소켓 모두 IP 주소와 port 번호를 연동하지 않는다.
    * -> 데이터를 처음 보낼 때, 자동으로 client의 IP 주소와 port 번호가 연동된다.
    * -> UDP의 경우, sendto() 함수를 호출할 때 자동으로 client의 IP 주소와 port 번호가 연동된다.
    * -> TCP의 경우, connect() 함수를 호출할 때 자동으로 client의 IP 주소와 port 번호가 연동된다.
    */
    
    while (true)
    {
        char recvBuffer[1000] = { NULL, };
        SOCKADDR_IN clientAddr = {};
        ::memset(&clientAddr, 0, sizeof(clientAddr));
        int32 addrLen = sizeof(clientAddr);

        int32 recvLen = ::recvfrom(serverSocket, recvBuffer, sizeof(recvBuffer), 0, reinterpret_cast<SOCKADDR*>(&clientAddr), &addrLen);
        if (recvLen <= 0)
        {
            ErrorHandling("recvfrom()");
            return 0;
        }
        std::cout << "Receive Data! Data : " << recvBuffer << ", Len : " << recvLen << std::endl;

        int32 errorCode = ::sendto(serverSocket, recvBuffer, recvLen, 0, reinterpret_cast<SOCKADDR*>(&clientAddr), sizeof(clientAddr));
        if (SOCKET_ERROR == errorCode)
        {
            ErrorHandling("sendto()");
            return 0;
        }
        std::cout << "Send Data! Len : " << recvLen << std::endl;
    }

    ::closesocket(serverSocket);
    ::WSACleanup();

    return 0;
}