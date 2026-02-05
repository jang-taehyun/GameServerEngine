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
    SOCKET socket = INVALID_SOCKET;
    SOCKADDR_IN addr = { 0, };
    char recvBuffer[BUFSIZE] = { 0, };
    int32 recvByte = 0;

    Session()
    {
        ::memset(&addr, 0, sizeof(addr));
    }

    ~Session()
    {
        if (INVALID_SOCKET != socket)
            ::closesocket(socket);
    }
};

enum class IO_TYPE
{
    NONE,
    READ,
    WRITE,
    ACCEPT,
    CONNECT,
};

// session과 Overlapped 구조체를 분리한 이유
// -> session에 있는 소켓이 어떤 사유(recv, send)로 이벤트가 발생했는지 판단하기 위해 분리
struct OverlappedEx
{
    WSAOVERLAPPED overlapped = {};
    IO_TYPE type = IO_TYPE::NONE;
};

void WorkerThreadMain(HANDLE iocpHandle)
{
    using namespace std;

    while (true)
    {
        DWORD bytesTransferred = 0;
        Session* session = nullptr;
        OverlappedEx* overlapped = nullptr;

        // 비동기 입출력 결과 가져오기 //
        BOOL ret = ::GetQueuedCompletionStatus(iocpHandle, &bytesTransferred, (PULONG_PTR)&session, (LPOVERLAPPED*)&overlapped, INFINITE);

        // error 체크
        if (FALSE == ret || 0 == bytesTransferred)
        {
            // TODO: 연결 끊김
            continue;
        }
        ASSERT_CRASH(IO_TYPE::READ == overlapped->type);

        cout << "Recv Data IOCP : " << bytesTransferred << endl;

        {
            // 이어서 recv를 해야 하는 경우, 다시 비동기 recv 함수 호출 //
            WSABUF wsaBuf = {};
            wsaBuf.buf = session->recvBuffer;
            wsaBuf.len = BUFSIZE;
            DWORD recvLen = 0;
            DWORD flags = 0;
            ::WSARecv(session->socket, &wsaBuf, 1, &recvLen, &flags, (LPWSAOVERLAPPED)overlapped, nullptr);
        }
    }
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

    // 블로킹 소켓 생성(IOCP를 사용하는 경우, 블로킹 소켓, 논블로킹 소켓 모두 사용 가능) //
    SOCKET serverSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (INVALID_SOCKET == serverSocket)
    {
        ErrorHandling("socket()");
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

    // session을 관리할 manager 생성
    std::vector<Session*> sessionManager;

    // completion port 생성 //
    HANDLE iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

    // worker 쓰레드 생성 //
    for (int32 i = 0; i < 5; ++i)
        GThreadManager->Launch(
            [=]() -> void
            {
                WorkerThreadMain(iocpHandle);
            }
        );

    while (true)
    {
        Session* session = new Session;
        int32 addrLen = sizeof(session->addr);

        session->socket = ::accept(serverSocket, (SOCKADDR*)&(session->addr), &addrLen);
        if (INVALID_SOCKET == session->socket)
        {
            ErrorHandling("accept()");
            return 0;
        }
        sessionManager.push_back(session);

        std::cout << "Client Connected!" << std::endl;
        
        // 소켓을 completion port에 등록 //
        // -> completion port에 소켓을 등록하면 소켓은 completion port의 관찰 대상이 된다.
        // -> 소켓과 키값(session의 주소값)이 연동된다.
        // -> CreateIoCompletionPort() 매개 변수 설명
        //      - CompletionKey                : GetQueuedCompletionStatus() 함수에서 일감을 받아올 때 사용할 키값(아무거나 넣어도 된다.)
        //          -> GetQueuedCompletionStatus() 함수로 네트워크 이벤트를 받아올 때 소켓의 핸들값이 아니라 키값을 통해서 구분해야 한다.
        //      - NumberOfConcurrentThreads    : completion port가 활용할 최대 쓰레드의 개수(0을 넣으면 최대 코어 개수만큼 할당)
        ::CreateIoCompletionPort((HANDLE)session->socket, iocpHandle, /*Key*/ (ULONG_PTR)session, 0);

        // 비동기 recv 함수 호출 //
        // -> 마치 낚시할 때 낚시대를 걸어넣고 물고기가 낚일때까지 기다리는 것과 동일
        // -> WSARecv() 함수만 해당(WSASend() 함수는 안걸어줘도 된다.)
        // -> main thread는 accept와 비동기 recv 함수만 호출하고 역할이 종료된다.
        // -> 이후 recv 처리는 다른 쓰레드에서 진행
        WSABUF wsaBuf = {};
        wsaBuf.buf = session->recvBuffer;
        wsaBuf.len = BUFSIZE;
        DWORD recvLen = 0;
        DWORD flags = 0;
        OverlappedEx* overlapped = new OverlappedEx;
        overlapped->type = IO_TYPE::READ;
        ::WSARecv(session->socket, &wsaBuf, 1, &recvLen, &flags, (LPWSAOVERLAPPED)overlapped, nullptr);
    }

    GThreadManager->Join();

    ::closesocket(serverSocket);
    ::WSACleanup();

    /**
    * 현재 코드의 문제
    * - 클라이언트에서 접속 해제한다면, session과 overlapped 구조체를 동적 해제해야 하는데,
    * - 소켓을 completion port에 등록하고 한번이라도 비동기 입출력 함수를 호출하게 되면,
    * - 누가, 언제 session과 overlapped 구조체를 해제해야 하는지 불분명해지는 문제가 발생
    * - 때문에 현재 코드에서는 session과 overlapped 구조체를 절대로 해제해서는 안된다.
    * 
    * 이유
    * - 비동기 입출력이 언제 완료될지도 모르고, 현재 실행되고 있는지 아닌지도 모르고, 누가 WSASend(), WSARecv()를 걸었는지 모르기 때문
    * - WSARecv()를 걸어준 상태에서 worker 쓰레드에서는 recv 처리를 한 후 다시 WSARecv() 호출하게 되는데, 클라이언트가 접속해제하여 session과 overlapped 구조체가 동적 해제되면, WSARecv() 함수 안의 session 에는 이상한 값이 채워진다.(오염된 메모리 참조)
    * 
    * 해결 방법
    * - session과 overlapped 구조체를 Reference Counting을 해준다.
    */

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
*       - 한번에 OS에 감시하도록 요청할 수 있는 event 객체의 최대 개수가 존재(64개)
* 
* 5) completion routine 기반 Overlapped 모델
*   - 비동기 입출력 함수가 완료되면, 쓰레드마다 있는 APC Queue에 일감이 쌓임
*   - Alertable Wait 상태로 들어가서 APC Queue 비우기(콜백 함수)
* 
*   - 장점)
*       - 성능이 좋다.
*   - 단점
*       - 모든 비동기 소켓 함수에서 사용 가능하지 않음(예를 들어서 AcceptEX() 함수에서 사용 못함)
*       - 빈번한 alterable wait 상태로 전환하는 비용으로 인해 성능이 저하될 수 있다.
*       - 다른 쓰레드가 completion routine을 처리할 수 없다.(일감 분배 측면에서 아쉽다)
*           -> APC Queue는 쓰레드마다 있기 때문
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