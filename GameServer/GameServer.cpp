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
        std::vector<BuffData> buffs{
            BuffData{100, 1.5f},
            BuffData{200, 2.3f},
            BuffData{300, 0.7f}
        };
        SendBufferRef sendBuffer = ServerPacketHandler::Make_S_TEST(1001, 100, 10, buffs, L"안녕하세요");

        GSessionManager->BroadCast(sendBuffer);

        using std::chrono::operator""ms;
        std::this_thread::sleep_for(250ms);
    }

    GThreadManager->Join();

    delete GSessionManager;

    return 0;
}
