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
#include "Job.h"
#include "Protocol.pb.h"

int main()
{
    GSessionManager = new GameSessionManager;
    GRoom = std::make_shared<Room>();

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

    while (true)
    {
        GRoom->FlushJob();

        using std::chrono::operator""ms;
        std::this_thread::sleep_for(1ms);
    }

    GThreadManager->Join();
    delete GSessionManager;

    return 0;
}


/**
* Command 패턴
* - 어떤 요청을 캡슐화해서 클래스, 함수 객체 등으로 만드는 패턴(주문서를 만들어서 직원에게 전달한다.)
*   - 어떤 요청을 다른 객체로 담고 있다가 누군가에게 건네준다.
*   - 요청을 처리하는 쓰레드는 요청만 처리하고,
*       job를 만드는 쓰레드는 job를 만들고 기다렸다가 요청을 처리하는 쓰레드에게 건내준다.
* - 장점
*   - 쓰레드마다 영역이 분리되어서, 각 쓰레드들은 각자 할일들에 집중할 수 있음
*   - 요청하는 시점, 실행하는 시점을 분리할 수 있음
*   - 요청을 수정, 취소 할수있다.
*/