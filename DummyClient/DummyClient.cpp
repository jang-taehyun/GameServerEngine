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
    * 2) WSAEventSelect 모델
    * - windows에만 있는 기능
    * - WSAEventSelect() 함수가 핵심이 되는 모델
    * - 소켓과 관련된 네트워크 이벤트를 [이벤트 객체]를 통해 감지해 전달받는다.
    * - select 모델과 비슷하나, select 모델과 다르게 비동기로 동작한다.
    *   - select 모델과 다르게, 소켓을 대상으로 read/write를 하기 전에 event 객체를 통해서 read/write가 가능한지 통지 받는다.
    *   - select 모델과 다르게, 전체적으로 리셋하고 다시 등록하는 과정이 없다.
    * - event 객체를 만들어서 소켓과 연동시켜줘야 한다.
    * - 소켓과 event 객체가 1:1로 매핑된다.
    *   - 소켓 갯수 만큼 event 객체를 만들어줘야 한다.
    * - 왠만하면 논블로킹 소켓을 이용해 WSAEventSelect 모델을 사용한다.
    * 
    * 이벤트 객체를 다루는 함수들
    * - 생성
    *   - WSACreateEvent() 함수 : 수동 리셋 방식, non-signal 상태로 시작
    * 
    * - 삭제
    *   - WSACloseEvent() 함수
    * 
    * - 시그널 상태 감지
    *   - WSAWaitForMultipleEvents() 함수 : event를 통지받는 함수
    *       - 이벤트 객체의 배열 중 event가 발생한 event 객체의 맨 처음 인덱스를 반환
    *       - 매개 변수 설명
                - count, event  : 이벤트 객체의 배열
                - waitAll       : 모두 기다릴지, 아니면 하나만 완료되어도 리턴할지 설정
                - timeout       : 기다릴 시간
                - fAlertable    : false(WSAEventSelect 모델에서는 사용 안함, 나중에 사용)

    * - 구체적인 네트워크 이벤트를 알아는 방법
    *   - WSAEnumNetworkEvents() 함수
    *       - 매개 변수
    *           - 소켓
    *           - 소켓과 관련된 이벤트 객체
    *           - networkEvents
    *       - 소켓과 관련된 이벤트 객체를 넘겨주면, 이벤트 객체는 자동으로 non-signal 상태가 된다.
    *       - networkEvents 객체에 네트워크 이벤트 또는 오류 정보가 저장된다
    * 
    * - 소켓과 event 객체를 연동하는 함수
    *   - WSAEventSelect(소켓, event 객체, networkEvents)
    *       - networkEvents : 어떤 event를 감지하고 싶은지 넣어주는 부분
    *       - 관찰할 네트워크 이벤트(networkEvents)
    *           - FD_ACCEPT     : 접속한 클라이언트가 있음(accept)
    *           - FD_READ       : 데이터 수신 가능(recv, recvfrom)
    *           - FD_WRITE      : 데이터 송신 가능(send, sendto)
    *           - FD_CLOSE      : 상대가 접속 종료
    *           - FD_CONNECT    : 통신을 위한 연결 절차 완료
    *           - FD_OOB
    * 
    * 주의 사항
    * - WSAEventSelect() 함수를 호출하면, 해당 소켓은 자동으로 논블로킹 소켓으로 전환된다.
    * - accept() 함수가 리턴한 소켓은 serverSocket과 동일한 속성(networkEvents)을 갖는다.
    *   - serverSocket은 FD_ACCEPT 속성을 가지고 있는데, accept() 함수가 리턴하는 소켓도 동일하게 FD_ACCEPT 속성을 갖는다.
    *   - 때문에 accept() 함수가 리턴한 소켓은 FD_READ, FD_WRITE 속성을 따로 등록해야 한다.
    * - 드물게 WSAEWOULDBLOCK 오류가 발생할 수 있어 예외 처리가 필요하다.
    * 
    * 중요한 부분
    * - 이벤트 발생 시, 적절한 소켓 함수를 호출해야 한다.
    *   - 그렇지 않으면, 다음 번에 동일한 네트워크 이벤트가 발생하지 않는다.(꺼내 쓸때까지는 다시 통지를 하지 않는다.)
    *   - FD_READ 이벤트가 떴으면, recv(), recvfrom() 함수를 호출해야 하고, 호출하지 않으면 FD_READ 이벤트가 다시 통지되지 않는다.
    * 
    * 장점
    * - select 모델과 다르게, 전체적으로 리셋하고 다시 등록하는 과정이 없다.
    * - select 모델과 다르게, loop를 돌지 않아도 event를 한번에 받을 수 있다.
    *   -> 하지만 등록할 수 있는 event 객체의 최대 개수가 존재한다.
    * 
    * 용도
    * - select 모델, WSAEventSelect 모델은 클라이언트를 서버에 붙일 때 사용(클라이언트에서 사용)
    *   -> 서버에서는 사용하지 않음
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