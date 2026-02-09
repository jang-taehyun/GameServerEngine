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

int32 GameSession::OnRecvPacket(BYTE* buffer, int32 len)
{
    using namespace std;

    // Echo
    // cout << "OnRecv len : " << len << endl;

    PacketHeader header = *(reinterpret_cast<PacketHeader*>(buffer));
    cout << "Packet ID : " << (uint16)header.ID << "Size : " << header.size << endl;

    return len;
}

void GameSession::OnSend(int32 len)
{
    using namespace std;

    // Echo
    // cout << "OnSend len : " << len << endl;
}