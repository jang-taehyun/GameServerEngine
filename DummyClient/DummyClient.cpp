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
    * socket 입출력 모델
    * - 논블로킹 소켓을 사용하면서, 준비되었을 때 미리 파악할 수 있으면 어떨까?
    * - socket 입출력 모델을 알아야하는 이유
    *   -> 교양적인 이유도 있고, boost::asio를 더 많이 이해하기 위해서도 필요함
    * 
    * 1) select 모델
    * - 핵심 : 소켓 함수 호출이 성공할 시점을 미리 알 수 있다!
    *   - read, write를 하기 전에 read, write를 할 수 있는지 먼저 체크한다!
    *   -> 문제 상황
            - recv buffer에 데이터가 없는 상황에서 read를 하거나, send buffer가 가득 찬 상황에서 write를 하는 경우
                -> 블로킹 소켓은 recv buffer에 데이터가 들어오거나, send buffer에 데이터가 들어갈 공간이 생길때까지 블로킹을 함.
                -> 논블로킹 소켓의 경우, 리턴은 하지만 WSAEWOULDBLOCK 에러가 발생하면 다시 시도를 해야 되는 경우가 있음
    * - select() 함수가 핵심이 되는 모델
    * - windows, linux 모두 존재
    * - 블로킹 소켓, 논블로킹 소켓 모두 사용 가능
    *   -> 블로킹 소켓에 적용하는 경우 : 조건이 만족되지 않아서 블로킹되는 상황을 예방할 수 있음
    *   -> 논블로킹 소켓에 적요하는 경우 : 조건이 만족되지 않아서 불필요하게 반복 체크하는 상황을 예방할 수 있음
    * 
    * - 사용법
    *   1) socket set을 만든다.
        2) socket set을 초기화하고 소켓을 socket set에 등록해 관찰 대상으로 만든다.
    *       - 관찰은 읽기[ ], 쓰기[ ], 예외(OOB)[ ]로 나뉜다.
    *           - 읽기 : read의 성공 시점을 체크(관찰)
    *           - 쓰기 : write의 성공 시점을 체크(관찰)
    *           - 예외 : OOB(OutOfBand) 체크
    *               - OOB(OutOfBand) : send() 함수의 마지막 인자에 MSG_OOB로 세팅해 보내는 특별한 데이터
    *                   - 받는 쪽에서도 recv OOB 세팅을 해야 읽을 수 있다.
    *                   - 긴급 상황 또는 특이한 상황을 알리는 용도로 사용
    *       - 관찰은 하나만 적용할 수 있는게 아니라, 여러 개를 적용할 수 있다.
    *           - ex) 한 소켓의 읽기, 쓰기를 모두를 관찰하고 싶다면, 읽기, 쓰기 모두 등록 가능
    *   3) select() 함수를 호출한다.
    *       - select() 함수를 호출할 때, 매개변수로 recv socket set, send socket set, exception socket set을 넣어준다.
    *           - 만약 쓰지 않는 socket set이 있다면, NULL로 넣어주면 된다.
    *       - select() 함수를 호출하면 등록된 소켓을 대상으로 관찰을 시작한다.
    *       - socket set에 등록된 소켓 중 준비가 완료된 소켓이 하나라도 있다면 리턴한다.
    *           -> ex) 한 소켓은 recv socket set에, 다른 하나의 소켓은 send socket set에 등록했다면,
    *                  둘 중 하나라도 준비가 완료되었다면 select() 함수는 리턴한다.
    *           - select() 함수가 리턴되었을 때, 준비가 되지 않는 소켓은 관찰 대상에서 제거된다.
    *           - select() 함수의 리턴 값 : 준비된 소켓의 개수
    *       - select() 함수는 동기 함수
    *   4) 관찰 대상에 남아있는 소켓을 대상으로 read 또는 write를 진행한다.
    *   5) 2~4 과정을 반복한다.
    * 
    * - 장점
    *   - 블로킹 소켓 : read, write 관련 함수가 리턴할 때까지 기다리지 않아도 된다.
    *   - 논블로킹 소켓 : 반복 체크하는 상황을 예방할 수 있다.
    *     -> 논블로킹 소켓의 경우, read/write가 가능한지 먼저 체크하기 때문에 안전하게 read/write를 진행 가능
    * 
    * - 단점
    *   - 한계 : socket set에 등록할 수 있는 소켓의 수가 정해져 있음
    *       -> socket set에 등록할 수 있는 소켓의 수가 생각보다 적음
    *   - 매번마다 socket set을 초기화하고 소켓을 등록해야 한다
    *       -> 전체적인 코드 성능에 영향을 끼친다
    */

    /**
    * 함수의 동기 vs 비동기 구분
    * - 동기
    *   -> 결과물이 나올때까지 대기
    *   -> ex) ::select() 함수의 경우, 원하는 결과가 나올 때까지 대기하다가, 원하는 결과가 하나라도 나오면 리턴
    *   -> 근데 블로킹을 말한거 같은데?? 이상하다
    * - 비동기
    *   -> 결과물이 안나와도 리턴
    *   -> 근데 논블로킹을 말한거 같은데?? 이상하다
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
            ErrorHandling("send()");
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
                ErrorHandling("recv()");
                break;
            }
            // 연결이 끊긴 상황
            else if (0 == recvLen)
            {
                std::cout << "Unconnected to Server!" << std::endl;
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