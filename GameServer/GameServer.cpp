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
#include "Protocol.pb.h"

int main()
{
    GSessionManager = new GameSessionManager;

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

    WCHAR sendData3[1000] = L"가";       // UTF-16 인코딩 사용(L을 앞에 붙이면 UTF-16 인코딩을 사용한다, C#과 궁합이 잘 맞아서 많이 사용한다)

    while (true)
    {
        Protocol::S_TEST pkt;

        pkt.set_id(1000);
        pkt.set_hp(100);
        pkt.set_id(10);
        {
            Protocol::BuffData* data = pkt.add_buffs();
            data->set_buffid(100);
            data->set_remaintime(1.2f);
            data->add_victims(4000);
        }
        {
            Protocol::BuffData* data = pkt.add_buffs();
            data->set_buffid(200);
            data->set_remaintime(2.5f);
            data->add_victims(1000);
            data->add_victims(2000);
        }

        SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
        GSessionManager->BroadCast(sendBuffer);

        using std::chrono::operator""ms;
        std::this_thread::sleep_for(250ms);
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