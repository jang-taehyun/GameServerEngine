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

const int32 BUFSIZE = 1000;

// 접속한 클라이언트 //
// 접속한 클라이언트만큼 Session 구조체가 생성된다.
struct Session
{
    WSAOVERLAPPED overlapped = {};
    SOCKET socket = INVALID_SOCKET;
    SOCKADDR_IN addr = { 0, };
    char recvBuffer[BUFSIZE] = { 0, };
    int32 recvByte = 0;

    Session()
    {
        ::memset(&addr, 0, sizeof(addr));
    }
};

int main()
{
    std::cout << "I'm server!!" << std::endl;


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

    std::cout << "Accept" << std::endl;

    while (true)
    {
        Session session;
        int32 addrLen = sizeof(session.addr);

        while (true)
        {
            // 비동기 소켓 생성 //
            session.socket = ::accept(serverSocket, (SOCKADDR*)&session.addr, &addrLen);
            if (INVALID_SOCKET != session.socket)
                break;

            if (WSAEWOULDBLOCK == ::WSAGetLastError())
                continue;

            // 버그 발생
            return 0;
        }

        // event 객체 생성 //
        session.overlapped.hEvent = ::WSACreateEvent();

        std::cout << "Client Connected!" << std::endl;

        while (true)
        {
            // WSABUF 생성 //
            // WSABUF는 없애도 되지만, WSABUF에 넣어준 buffer(session.recvBuffer)는 없애면 안된다.
            WSABUF wsaBuf;
            wsaBuf.buf = session.recvBuffer;
            wsaBuf.len = BUFSIZE;

            DWORD recvLen = 0;
            DWORD flags = 0;

            // Overlapped 계열의 함수 호출 //
            if (SOCKET_ERROR == ::WSARecv(session.socket, &wsaBuf, 1, &recvLen, &flags, (LPWSAOVERLAPPED)&session, nullptr))
            {
                if (WSA_IO_PENDING == ::WSAGetLastError())
                {
                    // pending 상태 //

                    // event가 signal 상태가 될 때까지 대기 //
                    ::WSAWaitForMultipleEvents(1, &(session.overlapped.hEvent), TRUE, WSA_INFINITE, FALSE);

                    // 비동기 입출력 결과 확인 및 데이터 처리 //
                    ::WSAGetOverlappedResult(session.socket, (LPWSAOVERLAPPED)&session, &recvLen, FALSE, &flags);

                    std::cout << "Data Overlapped Recv Len = " << recvLen << std::endl;
                }
                else
                {
                    // TODO: 문제 있는 상황
                    break;
                }
            }
            else
                std::cout << "Data Recv Len = " << recvLen << std::endl;
        }

        ::closesocket(session.socket);
        ::WSACloseEvent(session.overlapped.hEvent);
    }

    ::closesocket(serverSocket);
    ::WSACleanup();

    return 0;
}