#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>
#include <thread>
#include <chrono>

#include "RefCounting.h"

class Wraight : public RefCountable
{
public:
    int _hp = 150;
    int _posX = 0;
    int _posY = 0;
};

using WraightRef = TSharedPtr<Wraight>;
class Missile : public RefCountable
{
public:
    void SetTarget(WraightRef target)
    {
        _target = target;
    }

    bool Update()
    {
        if (_target == nullptr)
            return false;

        int posX = _target->_posX;
        int posY = _target->_posY;


        // TODO: 쫓아간다


        if (0 == _target->_hp)
        {
            _target->ReleaseRef();
            _target = nullptr;
            return false;
        }

        return true;
    }

private:
    WraightRef _target = nullptr;
};

using MissileRef = TSharedPtr<Missile>;

int main()
{
    WraightRef wraight(new Wraight);
    wraight->ReleaseRef();
    MissileRef missile(new Missile);
    missile->ReleaseRef();
    missile->SetTarget(wraight);

    // 레이스가 피격 당함
    wraight->_hp = 0;
    wraight = nullptr;

    while (true)
    {
        if (missile != nullptr)
        {
            if (false == missile->Update())
            {
                missile = nullptr;
                break;
            }
        }
    }

    return 0;
}