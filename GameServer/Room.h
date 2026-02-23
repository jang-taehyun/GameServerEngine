#pragma once

#include "JobQueue.h"


/*------------
	 Room
------------*/

class Room : public JobQueue
{
public:
	// 싱글쓰레드 환경인 마냥 코딩해도 됨.
	void Enter(PlayerRef player);
	void Leave(PlayerRef player);
	void Broadcast(SendBufferRef sendBuffer);

private:
	Map<uint64, PlayerRef> _players;
};