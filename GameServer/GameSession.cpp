#include "pch.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "ServerPacketHandler.h"

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

void GameSession::OnRecvPacket(BYTE* buffer, int32 len)
{
    ServerPacketHandler::HandlePacket(buffer, len);
}

void GameSession::OnSend(int32 len)
{
    // Echo
    // using namespace std;
    // cout << "OnSend len : " << len << endl;
}