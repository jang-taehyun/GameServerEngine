#pragma once


class Player
{
public:

	uint64 playerID = 0;
	std::string name;
	Protocol::PlayerType type = Protocol::PLAYER_TYPE_NONE;
	GameSessionRef ownerSession = nullptr;		// TODO: Cycle ²÷±â
};

