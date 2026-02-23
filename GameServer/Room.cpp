#include "pch.h"
#include "Player.h"
#include "GameSession.h"
#include "Room.h"

void Room::Enter(PlayerRef player)
{
	_players[player->playerID] = player;
}

void Room::Leave(PlayerRef player)
{
	_players.erase(player->playerID);
}

void Room::Broadcast(SendBufferRef sendBuffer)
{
	for (auto& p : _players)
	{
		p.second->ownerSession->Send(sendBuffer);
	}
}
