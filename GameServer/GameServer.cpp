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
    Session()
    {
        ::memset(&addr, 0, sizeof(addr));
    }

    SOCKET socket = INVALID_SOCKET;
    SOCKADDR_IN addr = { 0, };
    char recvBuffer[BUFSIZE] = { 0, };
    int32 recvByte = 0;
    int32 sendByte = 0;
};

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

    // session 배열 생성
    std::vector<Session> sessions;
    sessions.reserve(100);

    // socket set 생성 //
    fd_set readSet;
    fd_set writeSet;
    
    while (true)
    {
        // socket set 초기화 //
        FD_ZERO(&readSet);
        FD_ZERO(&writeSet);

        // 소켓을 socket set에 등록 //
        // 서버 소켓은 read set에 등록해야 한다.
        // -> accept할 대상이 있을 때까지 기다리기 때문
        FD_SET(serverSocket, &readSet);

        for (Session& s : sessions)
        {
            if(s.recvByte <= s.sendByte)
                FD_SET(s.socket, &readSet);
            else
                FD_SET(s.socket, &writeSet);
        }

        // select() 함수 호출 //
        // select() 함수의 마지막 인자
        // -> 옵션(넣어도 되고, nullptr로 넣어도 됨)
        // -> timeout(대기 시간)를 설정하는 부분
        // -> nullptr이라면 소켓이 하나라도 준비될 때까지 무한 대기
        // -> timeval 구조체의 포인터를 넣어준다면 대기 시간만큼 대기하고 리턴
        //      -> 대기 시간만큼 기다리는 동안 소켓이 하나라도 준비되었다면 리턴
        //      -> 대기 시간만큼 기다린 후에도 소켓이 하나라도 준비되지 않았다면 리턴
        int32 retVal = ::select(0, &readSet, &writeSet, nullptr, nullptr);
        if (SOCKET_ERROR == retVal)
        {
            ErrorHandling("select()");
            break;
        }

        // 소켓이 socket set에 등록되어 있는 지 확인 //
        // socket set에 등록되어 있지 않으면 0을 리턴
        // socket set에 등록되어 있으면 0이 아닌 값을 리턴
        if (FD_ISSET(serverSocket, &readSet))
        {
            Session client;
            int32 addrLen = sizeof(client.addr);

            client.socket = ::accept(serverSocket, (SOCKADDR*)&(client.addr), &addrLen);
            if (INVALID_SOCKET == client.socket)
            {
                ErrorHandling("accept()");
                break;
            }

            std::cout << "Client Connected!" << std::endl;

            sessions.push_back(client);
        }

        for (Session& s : sessions)
        {
            if (FD_ISSET(s.socket, &readSet))
            {
                int32 recvLen = ::recv(s.socket, s.recvBuffer, sizeof(s.recvBuffer), 0);
                if (0 >= recvLen)
                {
                    // TODO: sessions 제거

                    continue;
                }

                s.recvByte = recvLen;
            }

            if (FD_ISSET(s.socket, &writeSet))
            {
                // send() 함수 동작
                // 블로킹 소켓 : 모든 데이터를 다 보냄
                // 논블로킹 소켓 : 상대방 recv buffer의 상황에 따라서 일부만 보낼 수 있음
                int32 sendLen = ::send(s.socket, &(s.recvBuffer[s.sendByte]), s.recvByte - s.sendByte, 0);
                if (SOCKET_ERROR == sendLen)
                {
                    // TODO: sessions 제거

                    continue;
                }

                s.sendByte += sendLen;
                if (s.sendByte == s.recvByte)
                {
                    s.recvByte = 0;
                    s.sendByte = 0;
                }
            }
        }
    }

    // FD_CLR() 매크로 함수
    // - 소켓을 socket set에서 제거하는 함수
    // - ex) FD_CLR(serverSocket, &readSet);

    ::closesocket(serverSocket);
    ::WSACleanup();

    return 0;
}