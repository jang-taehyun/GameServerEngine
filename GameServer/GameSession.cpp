#include "pch.h"
#include "GameSession.h"
#include "GameSessionManager.h"

GameSession::~GameSession()
{
    using namespace std;

    cout << "~GameSession()" << endl;
}

void GameSession::OnConnected()
{
    GSessionManager->Add(std::static_pointer_cast<GameSession>(shared_from_this()));
}

void GameSession::OnDisconnected()
{
    GSessionManager->Remove(std::static_pointer_cast<GameSession>(shared_from_this()));
}

int32 GameSession::OnRecv(BYTE* buffer, int32 len)
{
    using namespace std;

    // Echo
    cout << "OnRecv len : " << len << endl;

    SendBufferRef sendBuffer = GSendBufferManager->Open(4096);
    ::memcpy(sendBuffer->Buffer(), buffer, len);
    sendBuffer->Close(len);

    GSessionManager->BroadCast(sendBuffer);

    return len;
}

void GameSession::OnSend(int32 len)
{
    using namespace std;

    // Echo
    cout << "OnSend len : " << len << endl;
}