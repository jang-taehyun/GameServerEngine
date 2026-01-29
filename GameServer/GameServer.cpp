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

    // non-blocking 소켓으로 설정 //
    u_long on = 1;
    if (INVALID_SOCKET == ::ioctlsocket(serverSocket, FIONBIO, &on))
    {
        ErrorHandling("ioctlsocket()");
        return 0;
    }

    SOCKADDR_IN serverAddr = { 0, };
    ::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
    serverAddr.sin_port = htons(7777);

    if (SOCKET_ERROR == ::bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)))
    {
        ErrorHandling("bind()");
        return 0;
    }

    // back log queue의 크기를 알아서 지정 //
    if (SOCKET_ERROR == ::listen(serverSocket, SOMAXCONN))
    {
        ErrorHandling("listen()");
        return 0;
    }

    /**
    * 논블로킹 소켓에서 WSAEWOULDBLOCK 코드가 반환되는 경우
    * -> recv(), send()가 완료되지 않았을 때
    * -> accept() 호출 시, 접속한 클라이언트가 없을 때
    */

    std::cout << "Accept" << std::endl;
    while (true)
    {
        SOCKADDR_IN clientAddr = {};
        ::memset(&clientAddr, 0, sizeof(clientAddr));
        int32 clientAddrSize = sizeof(clientAddr);
        SOCKET clientSocket = ::accept(serverSocket, (SOCKADDR*)&clientAddr, &clientAddrSize);

        // non-blocking 소켓인 경우, accept() 호출 시 접속한 클라이언트가 없다면 INVALID_SOCKET 반환 //
        if (INVALID_SOCKET == clientSocket)
        {
            // 접속한 클라이언트가 없는 경우(문제 상황이 아님)
            if (WSAEWOULDBLOCK == ::WSAGetLastError())
                continue;

            // error 발생!
            break;
        }

        std::cout << "Client Connected!" << std::endl;

        // recv
        while (true)
        {
            char recvBuffer[1000] = { NULL, };

            // non-blocking 소켓인 경우, recv(), recvfrom() 호출 시 recv buffer에 데이터가 없다면 SOCKET_ERROR 반환 //
            int32 recvLen = ::recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
            if (SOCKET_ERROR == recvLen)
            {
                // 해당 socket의 recv buffer에 데이터가 없는 경우
                if (WSAEWOULDBLOCK == ::WSAGetLastError())
                    continue;

                // error 발생!
                break;
            }
            // 연결이 끊긴 상황
            else if (0 == recvLen)
            {
                break;
            }

            std::cout << "Recv Data! Len : " << recvLen << std::endl;

            // send
            while (true)
            {
                // non-blocking 소켓인 경우, send(), sendto() 호출 시 send buffer에 데이터가 들어갈 공간이 없다면 SOCKET_ERROR 반환 //
                int32 sendLen = ::send(clientSocket, recvBuffer, recvLen, 0);
                if (SOCKET_ERROR == sendLen)
                {
                    // 해당 socket의 send buffer에 데이터가 없는 경우
                    if (WSAEWOULDBLOCK == ::WSAGetLastError())
                        continue;

                    // error 발생!
                    break;
                }

                std::cout << "Send Data! Len : " << sendLen << std::endl;
            }
        }
    }
    

    

    ::closesocket(serverSocket);
    ::WSACleanup();

    return 0;
}