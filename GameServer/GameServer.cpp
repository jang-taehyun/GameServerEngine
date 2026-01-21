#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>
#include <thread>
#include <chrono>

#include "ThreadManager.h"

#include "AccountManager.h"
#include "PlayerManager.h"

using std::operator""ms;

int main()
{
    using std::cout;
    using std::endl;

    GThreadManager->Launch(
        [=]()
        {
            while (true)
            {
                cout << "Player then Account" << endl;
                GPlayerManager.PlayerThenAccount();
                std::this_thread::sleep_for(100ms);
            }
        }
    );

    GThreadManager->Launch(
        [=]()
        {
            while (true)
            {
                cout << "Account then Player" << endl;
                GAccountManager.AccountThenPlayer();
                std::this_thread::sleep_for(100ms);
            }
        }
    );

    GThreadManager->Join();

    return 0;
}