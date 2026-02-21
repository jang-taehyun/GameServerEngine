#include "pch.h"
#include "Player.h"
#include "GameSession.h"
#include "Room.h"

void Room::Enter(PlayerRef player)
{
	WRITE_LOCK;
	_players[player->playerID] = player;
}

void Room::Leave(PlayerRef player)
{
	WRITE_LOCK;
	_players.erase(player->playerID);
}

void Room::Broadcast(SendBufferRef sendBuffer)
{
	WRITE_LOCK;
	for (auto& p : _players)
	{
		p.second->ownerSession->Send(sendBuffer);
	}
}
