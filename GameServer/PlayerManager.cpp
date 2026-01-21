#include "pch.h"
#include "PlayerManager.h"
#include "AccountManager.h"

#include <thread>
using std::operator""s;

PlayerManager GPlayerManager;

void PlayerManager::PlayerThenAccount()
{
	WRITE_LOCK;

	std::this_thread::sleep_for(1s);

	GAccountManager.Lock();
}

void PlayerManager::Lock()
{
	WRITE_LOCK;
}