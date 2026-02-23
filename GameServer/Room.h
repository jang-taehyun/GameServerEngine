#pragma once

#include "Job.h"


/*------------
	 Room
------------*/

class Room
{
	friend class EnterJob;
	friend class LeaveJob;
	friend class BroadcastJob;

public:
	// 멀티쓰레드 환경에서는 일감으로 접근
	void PushJob(JobRef job) { _jobs.Push(job); }
	void FlushJob();

private:
	// 싱글쓰레드 환경인 마냥 코딩해도 됨.
	void Enter(PlayerRef player);
	void Leave(PlayerRef player);
	void Broadcast(SendBufferRef sendBuffer);

private:
	Map<uint64, PlayerRef> _players;
	JobQueue _jobs;
};


/*-----------------
	 Enter Job
-----------------*/

class EnterJob : public IJob
{
public:
	EnterJob(Room& room, PlayerRef player) : _room(room), _player(player)
	{

	}

	virtual void Execute() override
	{
		_room.Enter(_player);
	}

private:
	Room& _room;
	PlayerRef _player;
};


/*-----------------
	 Leave Job
-----------------*/

class LeaveJob : public IJob
{
public:
	LeaveJob(Room& room, PlayerRef player) : _room(room), _player(player)
	{

	}

	virtual void Execute() override
	{
		_room.Leave(_player);
	}

private:
	Room& _room;
	PlayerRef _player;
};


/*---------------------
	 Broadcast Job
---------------------*/

class BroadcastJob : public IJob
{
public:
	BroadcastJob(Room& room, SendBufferRef sendBuffer) : _room(room), _sendBuffer(sendBuffer)
	{

	}

	virtual void Execute() override
	{
		_room.Broadcast(_sendBuffer);
	}

private:
	Room& _room;
	SendBufferRef _sendBuffer;
};