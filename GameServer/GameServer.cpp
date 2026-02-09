#include "pch.h"

#include <chrono>

#include "ThreadManager.h"
#include "Service.h"
#include "Session.h"
#include "GameSessionManager.h"
#include "GameSession.h"
#include "BufferWriter.h"

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

        BufferWriter bw{ sendBuffer->Buffer(), sendBuffer->AllocSize() };
        PacketHeader* header = bw.Reserve<PacketHeader>();

        // ID(uint64), 체력(uint32), 공격력(uint16)
        bw << (uint64)1001 << (uint32)100 << (uint16)10;
        bw.Write(sendData, sizeof(sendData));

        header->size = bw.WriteSize();
        header->ID = PacketHeader::ProtocolID::HELLO_WORLD;
        sendBuffer->Close(bw.WriteSize());

        GSessionManager->BroadCast(sendBuffer);

        using std::chrono::operator""ms;
        std::this_thread::sleep_for(250ms);
    }

    GThreadManager->Join();

    delete GSessionManager;

    return 0;
}
