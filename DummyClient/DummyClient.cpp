#include "pch.h"
#include <iostream>
#include <chrono>

#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>

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
    SOCKET clientSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (INVALID_SOCKET == clientSocket)
    {
        errorCode = ::WSAGetLastError();
        std::cout << "Socket ErrorCode : " << errorCode << std::endl;
        return 0;
    }

    // 서버 정보 입력 //
    // Little-Endian vs Big-Endian
    // Little-Endian : low [0x78][0x56][0x34][0x12] high
    // -> Little-Endian은 주로 Intel CPU에서 사용
    // Big-Endian    : low [0x12][0x34][0x56][0x78] high
    // -> network 표준은 Big-Endian
    SOCKADDR_IN serverAddr;
    ::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    ::inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);    // ::inet_addr() 함수는 deprecated이기 때문에, 최근에는 ::inet_pton() 함수를 사용한다.
    serverAddr.sin_port = htons(7777);                         // -> host to network short

    // server에 연결 요청 //
    // TCP에서는 3-way handshaking이 실행된다.
    if (SOCKET_ERROR == ::connect(clientSocket, reinterpret_cast<SOCKADDR*>(&serverAddr), sizeof(serverAddr)))
    {
        errorCode = ::WSAGetLastError();
        std::cout << "connect() ErrorCode : " << errorCode << std::endl;
        return 0;
    }

    // -----------------------------------------------
    // 연결 성공! 이제부터 데이터 송수신 가능!
    // -----------------------------------------------

    std::cout << "Connected To Server!" << std::endl;
    
    while (true)
    {
        // TODO

        for (int32 i = 0; i < 10; ++i)
        {
            char sendBuffer[100] = "Hello World!";

            int32 result = ::send(clientSocket, sendBuffer, sizeof(sendBuffer), 0);
            if (SOCKET_ERROR == result)
            {
                errorCode = ::WSAGetLastError();
                std::cout << "send() ErrorCode : " << errorCode << std::endl;
                return 0;
            }

            std::cout << "Send Data! Len : " << result << std::endl;
        }

        using std::chrono::operator""s;
        using std::chrono::operator""ms;
        std::this_thread::sleep_for(1s);
        std::this_thread::sleep_for(10ms);
        
        // char recvBuffer[1000] = { NULL, };
        // result = ::recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
        // if (SOCKET_ERROR == result)
        // {
        //     errorCode = ::WSAGetLastError();
        //     std::cout << "recv() ErrorCode : " << errorCode << std::endl;
        //     return 0;
        // }
        // std::cout << "Receive Data! Data : " << recvBuffer << ", Len : " << result << std::endl;
        // 
        // {
        //     using std::chrono::operator""s;
        //     std::this_thread::sleep_for(1s);
        // }
    }

    // socket 리소스 반환 //
    ::closesocket(clientSocket);

    // 윈속 종료 //
    ::WSACleanup();

    return 0;
}