#include "pch.h"

#include <chrono>
#include <tchar.h>

#include "ThreadManager.h"
#include "Service.h"
#include "Session.h"
#include "GameSessionManager.h"
#include "GameSession.h"
#include "BufferWriter.h"
#include "ServerPacketHandler.h"

int main()
{
    GSessionManager = new GameSessionManager;

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

    char sendData1[1000] = "가";         // CP949 인코딩 사용
    char sendData2[1000] = u8"가";       // UTF-8 인코딩 사용(u8을 앞에 붙이면 UTF-8 인코딩을 사용한다.)
    WCHAR sendData3[1000] = L"가";       // UTF-16 인코딩 사용(L을 앞에 붙이면 UTF-16 인코딩을 사용한다, C#과 궁합이 잘 맞아서 많이 사용한다)
    TCHAR sendData4[1000] = _T("가");    // 환경에 따라 달라짐(유니코드를 사용하면 UTF-8를, 멀티바이트를 사용하면 CP949를 사용)

    while (true)
    {
        // 현 시점 패킷 구조 : [ PKT_S_TEST ]
        PKT_S_TEST_WRITE pktWriter{ 1001,100,10 };

        // 현 시점 패킷 구조 : [ PKT_S_TEST ][ BuffsListItem BuffsListItem BuffsListItem ... ]
        PKT_S_TEST_WRITE::BuffsList buffList{ pktWriter.ReserveBuffsList(3) };
        buffList[0] = { 100, 1.5f };
        buffList[1] = { 200, 2.3f };
        buffList[2] = { 300, 0.7f };

        // 현 시점 패킷 구조 : [ PKT_S_TEST ][ BuffsListItem BuffsListItem BuffsListItem ... ][ victim victim ... ][ victim victim ... ]...[ victim victim ... ]
        PKT_S_TEST_WRITE::BuffsVictimsList vic0 = pktWriter.ReserveBuffsVictimsList(&buffList[0], 3);
        {
            vic0[0] = 1000;
            vic0[1] = 2000;
            vic0[2] = 3000;
        }
        PKT_S_TEST_WRITE::BuffsVictimsList vic1 = pktWriter.ReserveBuffsVictimsList(&buffList[1], 1);
        {
            vic1[0] = 1000;
        }
        PKT_S_TEST_WRITE::BuffsVictimsList vic2 = pktWriter.ReserveBuffsVictimsList(&buffList[2], 2);
        {
            vic2[0] = 3000;
            vic2[1] = 5000;
        }

        /**
        * 현재 방법의 장단점
        * - 장점 : 데이터를 바로 넣을 수 있다.
        * - 단점 : 사용하기 불편하다.
        */

        SendBufferRef sendBuffer = pktWriter.CloseAndReturn();
        GSessionManager->BroadCast(sendBuffer);

        using std::chrono::operator""ms;
        std::this_thread::sleep_for(250ms);
    }

    GThreadManager->Join();

    delete GSessionManager;

    return 0;
}

/**
* 직렬화(Serialization)
* - 데이터들을 모아서 바이트 배열로 만드는 작업
* 역직렬화
* - 바이트 배열을 데이터로 복구하는 작업
*/