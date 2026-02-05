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

void CALLBACK RecvCallback(
    IN DWORD dwError,                   // 오류 코드(오류가 발생하면 0이 아닌 값)
    IN DWORD cbTransferred,             // 수신한 byte
    IN LPWSAOVERLAPPED lpOverlapped,    // 비동기 입출력 함수 호출 시 넘겨준 WSAOVERLAPPED 구조체
    IN DWORD dwFlags                    // 0
)
{
    using namespace std;
    cout << "Data Recv Len Callback : " << cbTransferred << endl;

    // session 구조체 복구 //
    Session* s = reinterpret_cast<Session*>(lpOverlapped);
    s->overlapped;

    // TODO
}

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

            // 비동기 입출력 함수 호출 //
            if (SOCKET_ERROR == ::WSARecv(session.socket, &wsaBuf, 1, &recvLen, &flags, (LPWSAOVERLAPPED)&session, RecvCallback))
            {
                if (WSA_IO_PENDING == ::WSAGetLastError())
                {
                    // pending 상태 //

                    // 쓰레드를 alterable wait 상태로 전환 //
                    ::SleepEx(INFINITE, TRUE);
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
    }

    ::closesocket(serverSocket);
    ::WSACleanup();

    return 0;
}

/**
* I/O 멀티플렉싱 모델(성능이 낮은 순에서 높은 순으로 나열)
* 
* 1) Select 모델
*   - 장점)
*       - 윈도우/리눅스 공통(크로스 플랫폼)
*   - 단점)
*       - 성능 최하(매번 소켓을 socket set에 등록하는 비용이 존재)
*       - socket set에 등록할 수 있는 소켓의 최대 개수가 존재(64개)
* 
* 2) WSAAsyncSelect 모델
*   - socket의 이벤트(네트워크 이벤트)를 윈도우 메시지 형태로 처리하는 모델
*   - 일반 윈도우 메시지랑 같이 처리해서 성능이 애매함
* 
* 3) WSAEventSelect 모델
*   - 장점)
*       - 비교적 뛰어난 성능
*   - 단점
*       - OS에 감시하도록 요청할 수 있는 event 객체의 최대 개수가 존재(64개)
* 
* 4) event 기반 Overlapped 모델
*   - 장점)
*       - 성능이 좋다.
*   - 단점)
*       - OS에 감시하도록 요청할 수 있는 event 객체의 최대 개수가 존재(64개)
* 
* 5) completion routine 기반 Overlapped 모델
*   - 장점)
*       - 성능이 좋다.
*   - 단점
*       - 모든 비동기 소켓 함수에서 사용 가능하지 않음(예를 들어서 AcceptEX() 함수에서 사용 못함)
*       - 빈번한 alterable wait 상태로 전환하는 비용으로 인해 성능이 저하될 수 있다.
*       - 다른 쓰레드가 completion routine을 처리할 수 없다.(일감 분배 측면에서 아쉽다)
* 
* 6) IOCP
* -> 매번마다 alterable wait로 전환하는 비용이 없다.
*/

/**
* Reactor Pattern
* - 소켓의 상태를 확인한 후에 뒤늦게 recv, send를 호출하는 패턴
* - ex) WSAEventSelect 모델, Select 모델
* 
* Proactor Pattern
* - 미리 recv, send를 호출한 후, 내부적으로 처리하는 패턴
* - ex) Overlapped I/O 모델
*/