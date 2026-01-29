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
    * socket이 생성될 때, kernel 영역에서 send buffer, recv buffer가 만들어진다.
    * -> send buffer, recv buffer의 크기 변경 가능
    * 
    * socket의 옵션을 설정하는 함수
    * -> ::setsockopt()
    * 
    * socket의 옵션을 가져오는 함수
    * -> ::getsockopt()
    * 
    * ::setsockopt() 함수의 매개 변수
    * - int level
    *   -> 옵션을 해석하고 처리할 주체(socket, TCP 프로토콜, IP 프로토콜 등)
    *   -> socket 코드(socket 단계) : SOL_SOCKET
    *   -> IPv4 프로토콜 단계 : IPPROTO_IP
    *   -> TCP 프로토콜 단계 : IPPROTO_TCP
    * 
    * - int optname
    *   -> 옵션 지정
    *   -> SOL_SOCKET 단계의 optname 중 자주 사용되는 옵션
    *       - SO_KEEPALIVE          : 주기적으로 연결 상태 확인 여부를 설정(TCP에서만 동작)
    *           -> 상대방이 소리소문없이 연결을 끊는 경우가 있기 때문에 사용
    *           -> 활성화되면 주기적으로 TCP 프로토콜의 연결 상태를 확인하는 패킷을 보내면서, 끊어진 연결을 감지 가능
    *           {
    *               bool enable = true;
    *               ::setsockopt(serverSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&enable, sizeof(enable));
    *           }
    *       - SO_LINGER             : closesocket() 함수가 동작할 때, kernel 영역에 있는 socket의 send buffer에 들어 있는 바로 제거할 지 아니면 일정 시간 동안 기다린 후에 제거할 지 설정
    *           -> LINGER 구조체를 이용
    *               -> LINGER 구조체의 u_short l_onoff  : 0이면 closesocket() 함수가 바로 리턴, 아니면 linger초 만큼 대기(default는 0)
    *               -> LINGER 구조체의 u_short l_linger : 대기 시간(초 단위, second 단위)
    *           -> 
    *           {
    *               LINGER linger;
    *               linger.l_onoff = 1;
    *               linger.l_linger = 5;
    *               ::setsockopt(serverSocket, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger));
    *           }
    *       - SO_RCVBUF, SO_SNDBUF  : kernel 영역의 send buffer, 또는 kernel 영역의 recv buffer의 크기를 조정
    *           -> 너무 큰 값으로 지정하면 내부적으로 무시할 수 있다.
    *       - SO_REUSEADDR          : IP 주소 및 port를 재사용하는 옵션
    *           -> 개발 단계에서 편하기 때문에 설정하는 것을 추천(서버가 종료되고 다시 켤 때 해당 port가 아직 바인딩이 안풀릴 수도 있기 때문)
    * 
    *   -> IPPROTO_TCP 단계의 optname 중 중요한 옵션
    *       - TCP_NODELAY           : Nagle 알고리즘(네이글 알고리즘) 작동 여부 설정
    *           -> true인 경우, Nagle 알고리즘이 작동하지 않는다.
    *           -> 기본값은 0
    * 
    * 
    * - const char* optval
    *   -> 설정한 옵션이 저장된 변수의 주소
    *   -> char*로 캐스팅해서 넣어줘야 한다.
    * 
    * - int optlen
    *   -> optval에 넣어준 변수의 크기
    */

    /**
    * Half-Close
    * - socket을 반환하기 전에 kernel 영역의 send buffer, kernel 영역의 recv buffer 둘 중 하나를 닫는 방법
    * - ::closesocket() 함수를 호출하기 전에 ::shutdown() 함수를 이용해 buffer 중 하나를 끊는다.(socket은 반환하지 않음)
    *   - SD_RECEIVE : kernel 영역의 recv buffer로부터 데이터를 가져오는 것을 막는다.
    *   - SD_SEND : kernel 영역의 send buffer로 전송하는 것을 막는다.(어플리케이션의 buffer에서 kernel 영역의 send buffer로 복사를 못하도록 함)
    *     - send buffer에 데이터가 남아있다면, 남아있는 데이터는 목적지로 전송된다.
    *   - SD_BOTH : SD_RECEIVE, SD_SEND 둘을 합쳐놓은 것
    * - ::shutdown() 함수를 호출할 때 SHUT_WR 인자를 넣으면, 클라이언트에게는 EOF가 전송된다.
    * - 필요성
    *   - close() 함수 호출은 완전종료를 의미하는데, 완전종료라는 것은 데이터 전송은 물론이고 데이터 수신도 더 이상 불가능하다는 것을 의미함.
    *   - 완전종료를 하게 되면, 다른 호스트가 전송하고 있는 데이터를 수신하지 못하는 경우가 발생할 수 있음.
    *   - 이를 막기 위해 필요함
    * 
    * - 강사왈) MMORPG에서는 별의별 상황이 발생하고, Half-Close를 하지 않더라도 큰 일이 발생하는 경우은 거의 없었음
    */

    /**
    * Nagle 알고리즘(네이글 알고리즘)
    * - 원리 : 데이터가 충분히 크면 보내고, 그렇지 않다면 데이터가 충분히 쌓을 때까지 대기한다.
    *   -> 최대한 데이터를 뭉쳐 보내서, 회선 낭비를 줄이겠다!
    * - 장점 : 작은 패킷이 불필요하게 많이 생성되는 일을 방지
    * - 단점 : 반응 시간을 손해본다.
    * -> 게임에서는 Nagle 알고리즘을 끈다.
    */

    SOCKET clientSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (INVALID_SOCKET == clientSocket)
    {
        ErrorHandling("socket()");
        return 0;
    }

    ::closesocket(clientSocket);
    ::WSACleanup();

    return 0;
}