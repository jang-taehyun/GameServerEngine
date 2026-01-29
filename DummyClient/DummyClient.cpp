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

    // Connected UDP //
    // - socket에 보낼 대상의 정보(SOCKADDR_IN 구조체의 내용)을 등록하는 방식
    // - connect() 함수를 호출해 socket에 SOCKADDR_IN 구조체의 내용을 등록한다.
    // -> 이후 데이터를 보낼때는 send() 함수, 데이터를 수신할 때는 recv() 함수를 이용한다.
    if (SOCKET_ERROR == ::connect(clientSocket, reinterpret_cast<SOCKADDR*>(&serverAddr), sizeof(serverAddr)))
    {
        ErrorHandling("connect()");
        return 0;
    }

    // Unconnected UDP(기본적인 UDP), Connected UDP의 차이 //
    // -> Unconnected UDP(기본적인 UDP)  : 보낼 주소의 정보(SOCKADDR_IN 구조체의 내용)을 전송할 떄마다 입력한다.
    // -> Connected UDP                 : 보낼 주소의 정보(SOCKADDR_IN 구조체의 내용)을 전송할 떄마다 입력하지 않는다.

    // UDP의 보안 측면 위험성 //
    // - 서버에서 UDP 소켓을 열게 되면, 어느 누구라도 서버의 UDP 소켓에 데이터를 전송할 수 있기 때문에
    //   보안적인 측면에서는 위험하다.
    // -> TCP의 경우, 소켓끼리 1:1 연결이기 때문에 UDP보다 안전하기 때문에 가급적이면 TCP를 사용한다.

    
    while (true)
    {
        char sendBuffer[100] = "Hello World!";

        // 나의 IP 주소, 포트 번호가 자동으로 설정됨.
        int32 errorCode = ::send(clientSocket, sendBuffer, sizeof(sendBuffer), 0);
        if (SOCKET_ERROR == errorCode)
        {
            ErrorHandling("sendto()");
            return 0;
        }
        std::cout << "Send Data! Len : " << sizeof(sendBuffer) << std::endl;

        char recvBuffer[1000] = { NULL, };
        int32 recvLen = ::recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
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