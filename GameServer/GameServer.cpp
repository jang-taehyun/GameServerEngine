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
#include "Memory.h"

/**
* MS에서 제공하는 SLIST_ENTRY
* -> MS에서 제공하는 Lock-free List
* -> Lock-free stack을 만들때 사용된다.
* 
* MS에서 제공하는 SLIST_ENTRY를 사용하는 2가지 방법
* 1) SLIST_ENTRY를 상속
*   -> 포인터의 casting이 가능해짐
*   -> 상속을 받는다면, SLIST_ENTRY 안에 있는 멤버 변수들을 맨처음으로 받을 수 있음
* 2) SLIST_ENTRY를 멤버 변수의 맨처음 변수로 갖는다.
* 
* SLIST_ENTRY를 사용한다면,
* -> header를 만들고 초기화해줘야 한다.
*   -> PSLIST_HEADER Gheader;
* -> 반드시 데이터(객체)는 16byte로 정렬해야 한다.
*   -> DECLSPEC_ALIGN(16)
*/

using TL = TypeList<
    class Player,
    class Knight,
    class Mage,
    class Archer
>;

class Player
{
public:
    Player()
    {
        INIT_TL(Player);
    }

    DECLARE_TL;
};

class Knight : public Player
{
public:
    Knight()
    {
        INIT_TL(Knight);
    }
};

class Mage : public Player
{
public:
    Mage()
    {
        INIT_TL(Mage);
    }
};

class Archer : public Player
{
public:
    Archer()
    {
        INIT_TL(Archer);
    }
};

class Dog
{

};

int main()
{
    TypeList<Player, Knight, Mage>::Head whoAmI1;
    TypeList<Player, Knight, Mage>::Tail::Head whoAmI2;
    TypeList<Player, Knight, Mage>::Tail::Tail whoAmI3;

    TypeList<Player, TypeList<Knight, Mage>>::Head whoAmI4;
    TypeList<Player, TypeList<Knight, Mage>>::Tail::Tail whoAmI10;

    int32 n1 = Length<TL>::value;
    int32 n2 = Length<TypeList<Player, TypeList<Knight, Mage>>>::value;

    TypeAt<TL, 0>::Result whoAmI5;
    TypeAt<TL, 1>::Result whoAmI6;
    TypeAt<TL, 2>::Result whoAmI7;
    TypeAt<TL, 3>::Result whoAmI8;

    int32 idx1 = IndexOf<TL, Player>::value;
    int32 idx2 = IndexOf<TL, Knight>::value;
    int32 idx3 = IndexOf<TL, Archer>::value;
    int32 idx4 = IndexOf<TL, Mage>::value;
    int32 idx5 = IndexOf<TL, Dog>::value;

    bool IsCanCast1 = Conversion<Knight, Player>::exist;
    bool IsCanCast2 = Conversion<Player, Knight>::exist;
    bool IsCanCast3 = Conversion<Knight, Dog>::exist;
    bool IsCanCast4 = TypeConversion<TL>::CanConvert(0, 1);

    {
        Player* player = new Knight;

        bool canCast = CanCast<Knight*>(player);
        Knight* knight = TypeCast<Knight*>(player);
        if (knight)
        {

        }

        delete player;
    }

    {
        Player* player = new Player;

        bool canCast = CanCast<Knight*>(player);
        Knight* knight = TypeCast<Knight*>(player);

        delete player;
    }

    {
        std::shared_ptr<Knight> knight = std::make_shared<Knight>();

        bool canCast = CanCast<Player>(knight);
        std::shared_ptr<Player> player = TypeCast<Player>(knight);

        int32 BreakPoint = 1;
    }

    for (int32 i = 0; i < 5; ++i)
    {
        GThreadManager->Launch(
            []()
            {
                using namespace std::chrono;

                while (true)
                {
                }
            }
        );
    }

    GThreadManager->Join();

    return 0;
}