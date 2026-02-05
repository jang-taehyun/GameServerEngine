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
    std::cout << "I'm client!!" << std::endl;
    using std::chrono::operator""s;
    std::this_thread::sleep_for(1s);

    int32 errorCode = 0;
    WSADATA wsaData;

    if (errorCode = ::WSAStartup(MAKEWORD(2, 2), &wsaData))
    {
        return errorCode;
    }

    /**
    * socket 입출력 모델
    * - 논블로킹 소켓을 사용하면서, 준비되었을 때 미리 파악할 수 있으면 어떨까?
    * - socket 입출력 모델을 알아야하는 이유
    *   -> 교양적인 이유도 있고, boost::asio를 더 많이 이해하기 위해서도 필요함
    * 
    * 3) Overlapped I/O 모델 (비동기, 논블로킹)
    * 
    * 동작 과정
    * - Overlapped 계열의 함수를 호출(WSASend(), WSARecv(), AcceptEx(), ConnectEx() 등)
    * - Overlapped 계열의 함수가 성공했는지 확인한다.
    *   - 성공했으면 결과를 얻어서 처리
    *   - 실패했으면 사유를 확인한다.
    *       - pending 상태(준비가 되지 않는 상태)라면 추후에 완료가 되었을 때 통지를 하도록 요청한다
    *         (완료를 통지받는 방법 : event 방식, 콜백 함수 방식)
    * 
    * Overlapped 계열의 함수 특징
    * - buffer를 받을 때, WSABUF 배열의 시작 주소 및 개수(buffer의 개수, 배열의 길이가 아님)를 받는다.
    *   - 여러 개의 buffer를 한번에 받는 이유 : Scatter-Gather
    *       -> 여러 개로 쪼개져 있는 buffer들을 하나로 모아서 send할 수 있거나,
    *           받은 데이터를 여러 개로 쪼개져 있는 buffer들에 쪼개서 저장할 수 있다.(성능 향상의 핵심)
    * - WSAOVERLAPPED 구조체의 포인터를 받는다.
    *   - WSAOVERLAPPED 구조체에서는 event 객체의 핸들값만 지정하고 나머지는 건들지 않는다.
    *       -> 나머지 부분들은 OS 또는 Overlapped 계열 함수 내부에서 다루기 때문
    * - WSASend(), WSARecv() 함수를 호출한 뒤에, 매개 변수로 넣어줬던 WSAOVERLAPPED 구조체와 buffer들은 send, recv가 모두 완료하기 전까지 건들면 안된다.
    *   -> 이유 : WSASend(), WSARecv() 함수를 호출한 시점과, read/write가 실행되는 시점이 다르기 때문
    *   -> 때문에 WSASend(), WSARecv() 함수를 여러 번 호출할 때마다, send, recv가 실행 완료되기 전이라면 다른 WSAOVERLAPPED 구조체를 넣어줘야 한다
    * 
    * 4) IOCP(IO completion port) 모델
    * - completion routine 기반의 Overlapped IO와 비슷
    *   -> APC 대신 Completion port에 일감이 쌓인다.
    * - completion port는 쓰레드마다 있는 것이 아니라, 유일하게 1개만 생성한다.
    *   - 다수의 쓰레드가 completion port에서 일감을 받아서 실행
    *   - completion port는 일감을 모아놓는 역할
    * - GetQueuedCompletionStatus() 함수를 통해 completion port의 결과 처리
    * - 멀티쓰레드에 친화적인 모델
    * 
    * 주요 함수
    * - CreateIoCompletionPort() 함수
    *   - completion port를 생성하거나, 또는 소켓을 completion port에 등록하는 함수
    *   - completion port 생성
    *       -> HANDLE iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    *   - 소켓을 completion port에 등록
    *       -> ::CreateIoCompletionPort((HANDLE)session->socket, iocpHandle, (ULONG_PTR)session, 0);
    * 
    * - GetQueuedCompletionStatus() 함수
    *   - 비동기 입출력 결과를 가져오는 함수
    *   - 멀티쓰레드 환경에서 안전하게 동작
    *   - 비동기 입출력이 완료되지 않았다면, GetQueuedCompletionStatus() 함수가 실행될 때 쓰레드는 blocking 상태였다가, 완료되면 깨어난다.
    *       -> 여러 쓰레드가 GetQueuedCompletionStatus() 함수를 호출했을 때, 비동기 입출력이 완료되지 않았다면 blocking 상태가 되었다가, 완료가 될 때마다 하나씩 깨어난다.(한 번에 쓰레드 하나씩만 깨어난다.)
    *   - BOOL ret = ::GetQueuedCompletionStatus(iocpHandle, &bytesTransferred, (PULONG_PTR)&session, (LPOVERLAPPED*)&overlapped, INFINITE);
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

    char sendBuffer[100] = "Hello World!";

    // WSAOVERLAPPED 객체, event 객체 생성 //
    WSAOVERLAPPED overlapped = {};
    overlapped.hEvent = ::WSACreateEvent();

    while (true)
    {
        // WSABUF 생성 //
        // WSABUF는 없애도 되지만, WSABUF에 넣어준 buffer(session.recvBuffer)는 없애면 안된다.
        WSABUF wsaBuf;
        wsaBuf.buf = sendBuffer;
        wsaBuf.len = 100;

        DWORD sendLen = 0;
        DWORD flags = 0;

        // Overlapped 계열의 함수 호출 //
        if (SOCKET_ERROR == ::WSASend(clientSocket, &wsaBuf, 1, &sendLen, flags, &overlapped, nullptr))
        {
            if (WSA_IO_PENDING == ::WSAGetLastError())
            {
                // pending 상태 //

                // event가 signal 상태가 될 때까지 대기 //
                ::WSAWaitForMultipleEvents(1, &overlapped.hEvent, TRUE, WSA_INFINITE, FALSE);

                // 비동기 입출력 결과 확인 및 데이터 처리 //
                ::WSAGetOverlappedResult(clientSocket, &overlapped, &sendLen, FALSE, &flags);

                std::cout << "Data Overlapped Send Len = " << sendLen << std::endl;

            }
            else
            {
                // TODO: 문제 있는 상황
                break;
            }
        }
        else
            std::cout << "Send Data! Len : " << sendLen << std::endl;

        using std::chrono::operator""s;
        std::this_thread::sleep_for(1s);
    }

    ::closesocket(clientSocket);
    ::WSACleanup();

    return 0;
}