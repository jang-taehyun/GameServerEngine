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
    * 3-2) 콜백 함수(Completion routine) 기반 Overlapped I/O 모델
    * - 비동기 입출력 함수를 호출할 때 콜백 함수(Completion routine)을 넘겨줘서, 비동기 입출력이 완료되면 콜백 함수(completion routine)을 실행하는 방식
    * - completion routine을 넘겨줄 때는 함수 포인터의 형식을 참고해서 인자와 매개 변수에 맞는 함수를 넣어줘야 한다.
    * 
    * 동작 과정
    * 1) 비동기 입출력 소켓 생성
    * 2) Completion routine의 주소(함수 포인터)를 넘겨주면서 비동기 입출력 함수 호출
    * 3) 비동기 입출력이 바로 완료되지 않았다면, WSA_IO_PENDING 오류 코드를 반환
    * 4) 비동기 입출력이 완료되면 비동기 입출력을 호출한 쓰레드를 alertable wait 상태로 전환
    *       -> alterable wait 상태가 되면 OS는 APC Queue에 들어 있는 completion routine들을 실행한다.
    * 
    * 
    * APC(Asynchronous procedure call)
    * - 비동기적으로 실행되는 함수 호출
    * - APC를 호출하기 위해서는 APC Queue가 필요하다.
    * 
    * APC Queue
    * - 비동기 입출력 결과를 저장하기 위해 OS가 각 쓰레드마다 할당하는 메모리 영역
    * - 모든 쓰레드는 자신만의 APC Queue를 가지고 있음(쓰레드마다 독립적으로 가지고 있음)
    * - APC Queue에는 비동기적으로 호출되어야 할 함수들과 매개 변수 정보가 저장됨
    * - 쓰레드는 알림 가능 상태(alterable wait)가 되어야 APC Queue에 있는 함수들을 호출할 수 있다.
    * 
    * Alterable wait
    * - 비동기 입출력 함수를 호출한 쓰레드가 completion routine을 실행할 수 있는 상태
    * - 사용자가 제어 가능한 state
    * - Alterable wait 상태로 진입하는 함수들
    *   -> WaitForSingleObjectEx(), WaitForMultipleObjectEx(), SleepEx(), WSAWaitForMultipleEvents() 등
    * - APC Queue 안에 있는 completion routine들이 모두 실행 완료될 때 쓰레드는 alterable wait 상태를 빠져나온다.
    *   -> 만약 completion routine 안에서 비동기 입출력 함수를 호출하면, 쓰레드는 alterable wait 상태에서 빠져나오지 않는다.
    *   -> APC Queue 안에 completion routine이 5개가 있다고 하면, 5개가 모두 완료 될 때 alterable wait 상태를 빠져나온다.
    * - 스레드의 State(Running, Wait, Ready)와 별개이므로 끼워서 생각하지 말자
    * 
    * 
    * Overlapped I/O의 event 방식과 Overlapped I/O의 completion routine 방식의 차이
    * - event 방식은 소켓 하나 당 event 객체를 하나씩 만들어줘야 했음(소켓과 event 객체를 1:1로 연동해야 했음)
    *   -> completion routine 방식은 소켓과 event 객체를 1:1 대응하지 않아도 됨(completion routine 방식의 장점)
    *   -> completion routine 방식은 한 쓰레드가 비동기 입출력 함수를 여러 번 호출한 후 한 번의 alterable wait 상태로 진입해 completion routine을 실행할 수 있음(completion routine 방식의 장점)
    *
    * - event 방식은 OS에 감시하도록 요청할 수 있는 event 객체의 최대 개수(한도)가 정해져 있음 -> 64개
    *   -> 다수의 유저들을 처리할 때 복잡해짐
    * 
    * - completion routine 방식의 단점
    *   -> 어떤 client socket을 대상으로 completion routine이 실행되었는지 알 수 없다.
    *       -> 때문에 이를 극복하기 위해서, session 구조체에서 WSAOVERLAPPED 구조체를 포함할 때 맨 위에 포함한다.
    *           -> session 구조체의 시작 주소와 WSAOVERLAPPED 구조체의 시작 주소를 일치시키기 위해
    *   -> 다른 쓰레드가 completion routine을 처리할 수 없다.
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