#pragma once

#include "JobSerializer.h"


/*------------
	 Room
------------*/

class Room : public JobSerializer
{
public:
	// 멀티쓰레드 환경에서는 job으로 접근
	virtual void FlushJob() override;

public:
	// 싱글쓰레드 환경인 마냥 코딩해도 됨.
	void Enter(PlayerRef player);
	void Leave(PlayerRef player);
	void Broadcast(SendBufferRef sendBuffer);

private:
	Map<uint64, PlayerRef> _players;
};