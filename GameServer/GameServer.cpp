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

    GThreadManager->Join();

    delete GSessionManager;

    return 0;
}
