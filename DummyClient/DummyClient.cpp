#include "pch.h"

#include <chrono>

#include "ThreadManager.h"
#include "Service.h"
#include "Session.h"
#include "BufferReader.h"

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

    virtual int32 OnRecvPacket(BYTE* buffer, int32 len) override
    {
        using namespace std;

        BufferReader br{ buffer, (uint32)len };
        PacketHeader header;
        br >> header;

        uint64 ID = 0;
        uint32 hp = 0;
        uint16 attack = 0;
        br >> ID >> hp >> attack;

        cout << "ID : " << ID << ", hp : " << hp << ", attack : " << attack << endl;
        char recvBuffer[0x1000] = { 0, };
        br.Read(recvBuffer, header.size - sizeof(PacketHeader) - sizeof(uint64) - sizeof(uint32) - sizeof(uint16));
        cout << "recv str : " << recvBuffer << endl;

        return len;
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

    ClientServiceRef service{ MakeShared<ClientService>(
        NetworkAddress(L"127.0.0.1", 7777),
        MakeShared<IOCPCore>(),
        MakeShared<ServerSession>,                // TODO: Session manager µî
        1000
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