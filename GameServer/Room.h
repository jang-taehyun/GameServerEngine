#pragma once



class Room
{
public:
	void Enter(PlayerRef player);
	void Leave(PlayerRef player);
	void Broadcast(SendBufferRef sendBuffer);


private:
	USE_LOCK;
	Map<uint64, PlayerRef> _players;
};