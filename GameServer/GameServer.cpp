#include "pch.h"

#include <chrono>

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
        MakeShared<GameSession>,                // TODO: Session manager µî
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

    char sendData[1000] = "Hello World";
    while (true)
    {
        std::vector<BuffData> buffs{
            BuffData{100, 1.5f},
            BuffData{200, 2.3f},
            BuffData{300, 0.7f}
        };
        SendBufferRef sendBuffer = ServerPacketHandler::Make_S_TEST(1001, 100, 10, buffs);

        GSessionManager->BroadCast(sendBuffer);

        using std::chrono::operator""ms;
        std::this_thread::sleep_for(250ms);
    }

    GThreadManager->Join();

    delete GSessionManager;

    return 0;
}
