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

int main()
{
    int32 errorCode = 0;
    WSADATA wsaData;

    // 윈속 초기화 (ws2_32 라이브러리 초기화) //
    // 관련 정보가 wsaData에 채워진다.
    if (errorCode = ::WSAStartup(MAKEWORD(2, 2), &wsaData))
    {
        return errorCode;
    }

    // socket 생성 //
    // af : address family (AF_INET : IPv4, AF_INET6 : IPv6)
    // type : TCP(SOCK_STREAM) vs UDP(SOCK_DGRAM)
    // protocol : 0
    // return : descriptor
    SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (INVALID_SOCKET == listenSocket)
    {
        errorCode = ::WSAGetLastError();
        std::cout << "Socket ErrorCode : " << errorCode << std::endl;
        return 0;
    }

    // 내 정보(서버의 정보) 입력 //
    // Little-Endian vs Big-Endian
    // Little-Endian : low [0x78][0x56][0x34][0x12] high
    // -> Little-Endian은 주로 Intel CPU에서 사용
    // Big-Endian    : low [0x12][0x34][0x56][0x78] high
    // -> network 표준은 Big-Endian
    SOCKADDR_IN serverAddr;
    ::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);           // -> 니가 알아서 IP를 골라줘
    serverAddr.sin_port = htons(7777);                          // -> host to network short

    // 내 정보를 socket에 바인딩 //
    if (SOCKET_ERROR == ::bind(listenSocket, reinterpret_cast<SOCKADDR*>(&serverAddr), sizeof(serverAddr)))
    {
        errorCode = ::WSAGetLastError();
        std::cout << "bind() ErrorCode : " << errorCode << std::endl;
        return 0;
    }

    // listen socket을 연결 요청 대기 상태로 전환 //
    // back log queue : 대기열의 최대값
    // -> 연결 요청이 들어오면(외부에서 listen() 함수가 호출되면),
    //    back log queue에 삽입되고,
    // -> accept() 함수가 호출될 때마다 back log queue에서 하나씩 꺼내온다.
    // TCP에서 back log queue에 들어왔다는 뜻은 3-way handshaking이 성공했다는 뜻
    if (SOCKET_ERROR == ::listen(listenSocket, 10))
    {
        errorCode = ::WSAGetLastError();
        std::cout << "listen() ErrorCode : " << errorCode << std::endl;
        return 0;
    }

    // -----------------------------------------------
    
    while (true)
    {
        SOCKADDR_IN clientAddr;
        ::memset(&clientAddr, 0, sizeof(clientAddr));

        int32 clientAddrSize = sizeof(clientAddr);

        // client의 연결 요청을 받아들인다. //
        // -> back log queue에서 하나 꺼내온다.
        SOCKET clientSocket = ::accept(listenSocket, reinterpret_cast<SOCKADDR*>(&clientAddr), &clientAddrSize);
        if (INVALID_SOCKET == clientSocket)
        {
            errorCode = ::WSAGetLastError();
            std::cout << "accept() ErrorCode : " << errorCode << std::endl;
            return 0;
        }

        // 손님 입장!
        char ipAddress[16] = { 0, };
        ::inet_ntop(AF_INET, &clientAddr.sin_addr, ipAddress, sizeof(ipAddress));
        std::cout << "Client Conntected! IP : " << ipAddress << std::endl;

        // TODO
        // socket을 통해 전송을 희망하는 데이터는,
        // 커널 영역에 있는 buffer에 복사된 후 전송된다.

        while (true)
        {
            /**
            * send() 함수
            * - 블로킹 버전(기본 버전)
            *   -> 무조건 전송까지 완료될때까지 블로킹이 되는것은 아니다.
            *   -> 블로킹이 되면, wait 상태(blocked 상태)가 되고, send buffer에 데이터가 들어갈 자리가 생길 때까지 blocked 상태가 된다.
            *   - send buffer에 데이터가 일부라도 복사되면 바로 리턴
            *     -> 이후 상대방이 데이터를 일부라도 수신할 수 있으면, 상대방의 recv buffer(커널 영역)에 데이터를 전송하고 ACK를 수신하면, ACK를 수신한 구간의 데이터는 지운다.
            *     -> 또는, 이후 상대방의 recv buffer가 가득 차서, 수신 윈도우(advertised window)의 크기를 줄이거나 0으로 만들 수 있고,
                     -> 그 동안에는 송신이 제한되어 send buffer에 남아있을 수 있다.
            *     -> send buffer에는 일부 데이터만 복사될 수 있음.(전송을 요청한 길이보다 적은 값을 반환할 수 있음)
            *       -> 때문에 전체 데이터를 보내려면 loop를 돌려야 할 수도 있음.
            *   - 언제 블로킹이 됨?
            *     -> 자신의 send buffer에 데이터가 들어갈 자리가 없다면 블로킹이 된다.
            *   -> 어떻게 알아?
            *     -> 클라이언트 코드에서 send()를 할 때 서버 코드에서 recv()를 하지 않으면, 일시적으로 클라이언트 코드가 멈추지 않는 현상이 발생해.
            *     -> 멈추지 않았다는 건 send() 함수가 리턴을 해서 계속 코드가 실행되고 있다는 건데,
            *     -> 이를 통해서 서버 코드에서 recv()를 호출하지 않아도 일시적으로 클라이언트 코드의 send() 함수가 반환될 수 있다는 걸 알 수 있어.
            * 
            * 
            * recv() 함수
            * - 블로킹 버전(기본 버전)
            *   -> 블로킹이 되면, wait 상태(blocked 상태)가 되고, recv buffer에 읽을 데이터가 생길 때까지 blocked 상태가 된다.
            *   - recv buffer에 데이터를 가져오면 바로 리턴
            *     -> but 요청한 크기보다 적게 줄 수도 있음
            *     -> 0을 리턴했다는 것은 상대가 정상적으로 연결 종료를 했다는 것을 의미
            *   - 언제 블로킹이 됨?
            *     -> 내 recv buffer에 가져올 데이터가 없을 때 블로킹이 된다.
            * 
            * 소켓 입출력 버퍼
            * - 소켓을 생성할 때마다, kernel 영역에 소켓 전용 recv buffer, send buffer가 생성된다.
            *
            *
            * 그래서 블로킹 방식의 send(), recv() 함수는 게임 코드에서는 적합하진 않아.
            * -> 왜냐? 클라이언트가 send()를 했는데, 클라이언트 소켓의 send buffer도 가득 차 있으면 블로킹이 되니까 게임이 멈춰버릴 수 있어.
            * -> 다른 예시로, 서버가 recv()를 했는데 서버의 recv buffer에 아무것도 없다면, 읽을 데이터가 있을 때까지 블로킹이 되니까 이 경우에도 게임이 멈출 수 있어.
            * 
            * TCP의 특징
            * -> TCP는 데이터의 경계가 없다.(데이터의 크기가 정해지지 않았다.)
            *    -> 데이터의 경계가 없다는 것은,
                    -> 서버가 recv() 하기 전에 클라이언트가 100, 200, 50, 300 byte 데이터를 전송했다면,
            *           -> 서버가 recv() 함수를 호출하면 650 byte를 한번에 읽을 수 있고,
            *       -> 서버가 recv() 하기 전에 클라이언트가 100 byte 데이터를 전송했다면,
            *           -> 서버가 recv() 함수를 호출하면 20 byte 데이터를 받을 수 있다.
            *    -> 데이터의 일부분만 수신할 수도 있고, 여러 데이터를 한꺼번에 수신할 수도 있다.
            *       -> 즉, 최대 요청 크기만큼 읽어올 수 있고, 현재 도착해 있는 만큼만 읽을 수 있음.
            *    -> 그래서 클라이언트가 보낸 데이터를 쪼개서 데이터의 경계를 판별할 수 있는 수단이 필요해
            *    -> TCP는 스트림 방식이기 때문에 발생하는 현상
            */

            char recvBuffer[1000] = { NULL, };
            int32 recvResult = ::recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
            if (SOCKET_ERROR == recvResult)
            {
                errorCode = ::WSAGetLastError();
                std::cout << "recv() ErrorCode : " << errorCode << std::endl;
                return 0;
            }

            std::cout << "Receive Data! Len : " << recvResult << std::endl;
            std::cout << "Receive string : ";
            for (int32 i = 0; i < recvResult; ++i)
                std::cout << recvBuffer[i];
            std::cout << std::endl;

            // int32 sendResult = ::send(clientSocket, recvBuffer, recvResult, 0);
            // if (SOCKET_ERROR == sendResult)
            // {
            //     errorCode = ::WSAGetLastError();
            //     std::cout << "send() ErrorCode : " << errorCode << std::endl;
            //     return 0;
            // }
        }
    }

    // -----------------------------------------------

    // socket 리소스 반환 //
    ::closesocket(listenSocket);

    // 윈속 종료 //
    ::WSACleanup();

    return 0;
}