#include "pch.h"

#include <chrono>

#include "ThreadManager.h"
#include "Service.h"
#include "Session.h"
#include "GameSessionManager.h"
#include "GameSession.h"

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

    char sendData[1000] = "Hello World";
    while (true)
    {
        SendBufferRef sendBuffer = GSendBufferManager->Open(4096);

        BYTE* buffer = sendBuffer->Buffer();

        // packet에 header 집어넣기 //
        reinterpret_cast<PacketHeader*>(buffer)->size = sizeof(PacketHeader) + sizeof(sendData);

        ::memcpy((buffer + sizeof(PacketHeader)), sendData, sizeof(sendData));
        sendBuffer->Close(sizeof(PacketHeader) + sizeof(sendData));

        GSessionManager->BroadCast(sendBuffer);

        using std::chrono::operator""ms;
        std::this_thread::sleep_for(250ms);
    }

    GThreadManager->Join();

    delete GSessionManager;

    return 0;
}
