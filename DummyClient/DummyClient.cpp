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

    /**
    * 블로킹(Blocking) 소켓
    * - 완료가 될때까지 대기한다.
    *   -> TCP 소켓 기준, connect(), accept(), send(), sendto(), recv(), recvfrom() 함수 등
    *       -> accept() 함수 : 접속한 클라가 있을 때
    *       -> connect() 함수 : 서버 접속이 성공할 때
    *       -> send(), sendto() 함수 : 요청한 데이터를 send buffer에 일부라도 복사했을 때
    *       -> recv(), recvfrom() 함수 : recv buffer에 도착한 데이터가 있고, 이를 유저레벨 buffer에 복사했을 때
    * - 완료가 될때까지 대기하기 때문에, 동시 접속자 수가 많은 경우 성능 저하가 발생
    *   -> 그렇다고 접속자 수만큼 쓰레드를 생성하게 되면, context switching이 빈번하게 일어나기 때문에 성능 저하가 발생
    * 
    */

    SOCKET clientSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (INVALID_SOCKET == clientSocket)
    {
        ErrorHandling("socket()");
        return 0;
    }

    // non-blocking 소켓으로 설정 //
    u_long on = 1;
    if (INVALID_SOCKET == ::ioctlsocket(clientSocket, FIONBIO, &on))
    {
        ErrorHandling("ioctlsocket()");
        return 0;
    }

    SOCKADDR_IN serverAddr = { 0, };
    ::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    ::inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
    serverAddr.sin_port = htons(7777);

    // Connect
    while (true)
    {
        // non-blocking 소켓인 경우, connect() 호출 시 여러 이유로 SOCKET_ERROR를 반환한다.
        if (SOCKET_ERROR == ::connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)))
        {
            // WSAEWOULDBLOCK : 3-way handshaking을 진행하고 있는 경우
            if (WSAEWOULDBLOCK == ::WSAGetLastError())
                continue;

            // WSAEISCONN : 이미 연결된 상태인 경우
            if (WSAEISCONN == ::WSAGetLastError())
                break;

            // error 발생!
            break;
        }
    }

    std::cout << "Connected to Server!" << std::endl;

    // send
    char sendBuffer[100] = "Hello World!";
    while (true)
    {
        // non-blocking 소켓인 경우, send(), sendto() 호출 시 send buffer에 데이터가 들어갈 공간이 없다면 SOCKET_ERROR 반환 //
        int32 sendLen = ::send(clientSocket, sendBuffer, sizeof(sendBuffer), 0);
        if (SOCKET_ERROR == sendLen)
        {
            // 해당 socket의 send buffer에 데이터가 없는 경우
            if (WSAEWOULDBLOCK == ::WSAGetLastError())
                continue;

            // error 발생!
            break;
        }

        std::cout << "Send Data! Len : " << sendLen << std::endl;

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

            std::cout << "Recv Data len : " << recvLen << std::endl;
            break;
        }

        using std::chrono::operator""s;
        std::this_thread::sleep_for(1s);
    }

    ::closesocket(clientSocket);
    ::WSACleanup();

    return 0;
}