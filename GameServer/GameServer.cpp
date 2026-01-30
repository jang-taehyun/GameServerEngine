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

    // session 객체(소켓)과 1:1 매칭할 event 객체 배열 생성 //
    std::vector<WSAEVENT> wsaEvents;
    std::vector<Session> sessions;
    wsaEvents.reserve(100);
    sessions.reserve(100);

    // server socket을 위한 event 객체 생성 //
    WSAEVENT serverEvent = ::WSACreateEvent();

    // server socket과 event 객체를 연동 //
    if (SOCKET_ERROR == ::WSAEventSelect(serverSocket, serverEvent, FD_ACCEPT | FD_CLOSE))
    {
        ErrorHandling("WSAEventSelect()");
        return 0;
    }

    // server socket과 server socket과 연동한 event 객체를 배열에 넣기 //
    Session serverSession;
    serverSession.socket = serverSocket;
    sessions.push_back(serverSession);
    wsaEvents.push_back(serverEvent);
    
    while (true)
    {
        // 여러 개의 event 객체 중 하나라도 singal 상태가 될때까지 대기 //
        DWORD retVal = ::WSAWaitForMultipleEvents(wsaEvents.size(), &wsaEvents[0], FALSE, WSA_INFINITE, FALSE);
        if (WSA_WAIT_FAILED == retVal)
        {
            ErrorHandling("WSAWaitForMultipleEvents()");
            continue;
        }
        int32 idx = retVal - WSA_WAIT_EVENT_0;

        // 어떤 네트워크 이벤트가 발생했는지 체크 //
        // 이때 event 객체는 자동으로 non-signal 상태로 전환된다.
        WSANETWORKEVENTS networkEvents;
        if (SOCKET_ERROR == ::WSAEnumNetworkEvents(sessions[idx].socket, wsaEvents[idx], &networkEvents))
        {
            ErrorHandling("WSAEnumNetworkEvents()");
            continue;
        }

        // accept 체크 //
        if (FD_ACCEPT & networkEvents.lNetworkEvents)
        {
            // error 체크
            if (networkEvents.iErrorCode[FD_ACCEPT_BIT])
            {
                ErrorHandling("error!! : FD_ACCEPT_BIT");
                continue;
            }

            // 정상적으로 accept할 준비가 되었으므로 accept() 호출 //
            Session clientSession;
            int32 addrLen = sizeof(clientSession.addr);
            clientSession.socket = ::accept(serverSocket, (SOCKADDR*)&(clientSession.addr), &addrLen);
            if (INVALID_SOCKET == clientSession.socket)
            {
                ErrorHandling("accept()");
                continue;
            }

            std::cout << "Client Connected!" << std::endl;

            // client socket과 연동할 event 객체를 생성하고 연동 //
            WSAEVENT event = ::WSACreateEvent();
            if (SOCKET_ERROR == ::WSAEventSelect(clientSession.socket, event, FD_READ | FD_WRITE | FD_CLOSE))
            {
                ErrorHandling("WSAEventSelect()");
                continue;
            }

            // session과 event 객체를 배열에 넣기 //
            sessions.push_back(clientSession);
            wsaEvents.push_back(event);
        }

        // client session의 소켓 체크 //
        if ((FD_READ & networkEvents.lNetworkEvents) || (FD_WRITE & networkEvents.lNetworkEvents))
        {
            // error 체크
            if ((FD_READ & networkEvents.lNetworkEvents) && networkEvents.iErrorCode[FD_READ_BIT])
            {
                ErrorHandling("error!! : FD_READ_BIT");
                continue;
            }
            if ((FD_WRITE & networkEvents.lNetworkEvents) && networkEvents.iErrorCode[FD_WRITE_BIT])
            {
                ErrorHandling("error!! : FD_WRITE_BIT");
                continue;
            }

            Session& s = sessions[idx];

            // read //
            if (0 == s.recvByte)
            {
                int32 recvLen = ::recv(s.socket, s.recvBuffer, BUFSIZE, 0);
                
                // error 체크
                if (SOCKET_ERROR == recvLen)
                {
                    if (::WSAGetLastError() != WSAEWOULDBLOCK)
                    {
                        ErrorHandling("recv()");
                        // TODO: Remove Session
                    }

                    continue;
                }

                s.recvByte = recvLen;
                std::cout << "Recv len : " << s.recvByte << std::endl;
            }

            // write //
            if (s.recvByte > s.sendByte)
            {
                int32 sendLen = ::send(s.socket, &(s.recvBuffer[s.sendByte]), s.recvByte - s.sendByte, 0);

                // error 체크
                if (SOCKET_ERROR == sendLen)
                {
                    if (::WSAGetLastError() != WSAEWOULDBLOCK)
                    {
                        ErrorHandling("send()");
                        // TODO: Remove Session
                    }

                    continue;
                }

                s.sendByte += sendLen;
                if (s.recvByte == s.sendByte)
                {
                    s.recvByte = 0;
                    s.sendByte = 0;
                }

                std::cout << "Send len : " << sendLen << std::endl;
            }
        }

        // FD_CLOSE 처리 //
        if (FD_CLOSE & networkEvents.lNetworkEvents)
        {
            // TODO: Remove session
        }
    }

    ::closesocket(serverSocket);
    ::WSACleanup();

    return 0;
}