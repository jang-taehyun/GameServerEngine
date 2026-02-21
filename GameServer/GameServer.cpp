#include "pch.h"

#include <chrono>
#include <tchar.h>

#include "ThreadManager.h"
#include "Service.h"
#include "Session.h"
#include "GameSessionManager.h"
#include "GameSession.h"
#include "BufferWriter.h"
#include "ClientPacketHandler.h"
#include "Room.h"
#include "Protocol.pb.h"

int main()
{
    GSessionManager = new GameSessionManager;
    GRoom = new Room;

    ClientPacketHandler::Init();

    ServerServiceRef service{ MakeShared<ServerService>(
        NetworkAddress(L"127.0.0.1", 7777),
        MakeShared<IOCPCore>(),
        MakeShared<GameSession>,                // TODO: Session manager 등
        100
    ) };

    ASSERT_CRASH(service->Start());

    for (int32 i = 0; i < 5; ++i)
    {
        GThreadManager->Launch(
            [=]()
            {
                while (true)
                {
                    service->GetIOCPCore()->Dispatch();
                }
            }
        );
    }

    GThreadManager->Join();
    delete GSessionManager;

    return 0;
}

/**
* protoBuf의 장점
* - 안정적이다.(그래서 많이 사용하는듯)
* - 다양한 방법으로 데이터를 채울 수 있다.
* - 직렬화, 역직렬화하는 부분을 쉽게 처리할 수 있다.
* - 다른 엔진에 연동할 때도 비슷하게 작업할 수 있다.
* - 퍼블리셔와 통신하는 코드를 작성할 때 협업하기 편하다.
* 
* protoBuf와 flatBuf의 비교
* - protoBuf의 장점 : 작업하기엔 편하다.
* - protoBuf의 단점 : flatBuf보다 성능이 떨어진다(중간에 객체를 생성해 복사 비용이 있기 때문)
* - flatBuf의 장점 : 데이터를 바로 넣을 수 있다, protoBuf보다 성능이 좋다(복사 비용이 없기 때문)
* - flatBuf의 단점 : 사용할 때 불편하다.
*/

/**
* protoBuf, vcpkg 설치 참고 자료
* 
* https://minttea25.tistory.com/128
* https://velog.io/@pikamon/CC-15
*/