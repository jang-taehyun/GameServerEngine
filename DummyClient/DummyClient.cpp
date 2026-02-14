#include "pch.h"

#include <chrono>

#include "ThreadManager.h"
#include "Service.h"
#include "Session.h"

#include "ServerPacketHandler.h"

char sendData[12] = "Hello World";

class ServerSession : public PacketSession
{
public:
    virtual ~ServerSession()
    {
        using namespace std;
        cout << "~ServerSession()" << endl;
    }

    virtual void OnConnected() override
    {
        // using namespace std;
        // cout << "Connected To Server!!" << endl;
    }

    virtual void OnRecvPacket(BYTE* buffer, int32 len) override
    {
        PacketSessionRef session = GetPacketSessionRef();
        PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

        // TODO: packetID 대역 체크
        ServerPacketHandler::HandlePacket(session, buffer, len);
    }

    virtual void OnSend(int32 len) override
    {
        // Echo
        // using namespace std;
        // cout << "OnSend len : " << len << endl;
    }

    virtual void OnDisconnected() override
    {
        // Echo
        // using namespace std;
        // cout << "Disconnected!" << endl;
    }
};

int main()
{
    using std::chrono::operator""s;
    std::this_thread::sleep_for(1s);

    ServerPacketHandler::Init();

    ClientServiceRef service{ MakeShared<ClientService>(
        NetworkAddress(L"127.0.0.1", 7777),
        MakeShared<IOCPCore>(),
        MakeShared<ServerSession>,                // TODO: Session manager 등
        1
    ) };

    ASSERT_CRASH(service->Start());

    for (int32 i = 0; i < 2; ++i)
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

    return 0;
}