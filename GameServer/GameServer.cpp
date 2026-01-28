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

int main()
{
    int32 errorCode = 0;
    WSADATA wsaData;

    // 윈속 초기화 (ws2_32 라이브러리 초기화) //
    // 관련 정보가 wsaData에 채워진다.
    if (errorCode = ::WSAStartup(MAKEWORD(2, 2), &wsaData))
    {
        return errorCode;
    }

    // socket 생성 //
    // af : address family (AF_INET : IPv4, AF_INET6 : IPv6)
    // type : TCP(SOCK_STREAM) vs UDP(SOCK_DGRAM)
    // protocol : 0
    // return : descriptor
    SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (INVALID_SOCKET == listenSocket)
    {
        errorCode = ::WSAGetLastError();
        std::cout << "Socket ErrorCode : " << errorCode << std::endl;
        return 0;
    }

    // 내 정보(서버의 정보) 입력 //
    // Little-Endian vs Big-Endian
    // Little-Endian : low [0x78][0x56][0x34][0x12] high
    // -> Little-Endian은 주로 Intel CPU에서 사용
    // Big-Endian    : low [0x12][0x34][0x56][0x78] high
    // -> network 표준은 Big-Endian
    SOCKADDR_IN serverAddr;
    ::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);           // -> 니가 알아서 IP를 골라줘
    serverAddr.sin_port = htons(7777);                          // -> host to network short

    // 내 정보를 socket에 바인딩 //
    if (SOCKET_ERROR == ::bind(listenSocket, reinterpret_cast<SOCKADDR*>(&serverAddr), sizeof(serverAddr)))
    {
        errorCode = ::WSAGetLastError();
        std::cout << "bind() ErrorCode : " << errorCode << std::endl;
        return 0;
    }

    // listen socket을 연결 요청 대기 상태로 전환 //
    // back log queue : 대기열의 최대값
    // -> 연결 요청이 들어오면(외부에서 listen() 함수가 호출되면),
    //    back log queue에 삽입되고,
    // -> accept() 함수가 호출될 때마다 back log queue에서 하나씩 꺼내온다.
    // TCP에서 back log queue에 들어왔다는 뜻은 3-way handshaking이 성공했다는 뜻
    if (SOCKET_ERROR == ::listen(listenSocket, 10))
    {
        errorCode = ::WSAGetLastError();
        std::cout << "listen() ErrorCode : " << errorCode << std::endl;
        return 0;
    }

    // -----------------------------------------------
    
    while (true)
    {
        SOCKADDR_IN clientAddr;
        ::memset(&clientAddr, 0, sizeof(clientAddr));

        int32 clientAddrSize = sizeof(clientAddr);

        // client의 연결 요청을 받아들인다. //
        // -> back log queue에서 하나 꺼내온다.
        SOCKET clientSocket = ::accept(listenSocket, reinterpret_cast<SOCKADDR*>(&clientAddr), &clientAddrSize);
        if (INVALID_SOCKET == clientSocket)
        {
            errorCode = ::WSAGetLastError();
            std::cout << "accept() ErrorCode : " << errorCode << std::endl;
            return 0;
        }

        // 손님 입장!
        char ipAddress[16] = { 0, };
        ::inet_ntop(AF_INET, &clientAddr.sin_addr, ipAddress, sizeof(ipAddress));
        std::cout << "Client Conntected! IP : " << ipAddress << std::endl;

        // TODO
        // socket을 통해 전송을 희망하는 데이터는,
        // 커널 영역에 있는 buffer에 복사된 후 전송된다.
    }

    // -----------------------------------------------

    // socket 리소스 반환 //
    ::closesocket(listenSocket);

    // 윈속 종료 //
    ::WSACleanup();

    return 0;
}