#include "pch.h"

#include <chrono>

#include "ThreadManager.h"
#include "Service.h"
#include "Session.h"

char sendData[12] = "Hello World";

class ServerSession : public Session
{
public:
    virtual ~ServerSession()
    {
        using namespace std;

        cout << "~ServerSession()" << endl;
    }

    virtual void OnConnected() override
    {
        using namespace std;

        cout << "Connected To Server!!" << endl;
        
        SendBufferRef sendBuffer = GSendBufferManager->Open(4096);
        ::memcpy(sendBuffer->Buffer(), sendData, sizeof(sendData));
        sendBuffer->Close(sizeof(sendData));

        Send(sendBuffer);
    }

    virtual int32 OnRecv(BYTE* buffer, int32 len) override
    {
        using namespace std;

        // Echo
        cout << "OnRecv len : " << len << endl;

        this_thread::sleep_for(1s);

        SendBufferRef sendBuffer = GSendBufferManager->Open(4096);
        ::memcpy(sendBuffer->Buffer(), sendData, sizeof(sendData));
        sendBuffer->Close(sizeof(sendData));

        Send(sendBuffer);

        return len;
    }

    virtual void OnSend(int32 len) override
    {
        using namespace std;

        // Echo
        cout << "OnSend len : " << len << endl;
    }

    virtual void OnDisconnected() override
    {
        using namespace std;

        // Echo
        cout << "Disconnected!" << endl;
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
        5
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