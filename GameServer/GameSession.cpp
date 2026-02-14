#include "pch.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "ClientPacketHandler.h"

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
    PacketSessionRef session = GetPacketSessionRef();
    PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

    // TODO: packetID 대역 체크
    ClientPacketHandler::HandlePacket(session, buffer, len);
}

void GameSession::OnSend(int32 len)
{
    // Echo
    // using namespace std;
    // cout << "OnSend len : " << len << endl;
}