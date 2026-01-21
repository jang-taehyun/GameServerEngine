#include "pch.h"
#include "AccountManager.h"
#include "PlayerManager.h"

#include <thread>
using std::operator""s;

AccountManager GAccountManager;

void AccountManager::AccountThenPlayer()
{
	WRITE_LOCK;

	std::this_thread::sleep_for(1s);

	GPlayerManager.Lock();
}

void AccountManager::Lock()
{
	WRITE_LOCK;
}