#pragma once

/**
* 3.5세대 방식
* - Lamda와 std::function을 이용하는 방법
* - 여기에 더해 첫번째 job을 넣은 쓰레드가 실행까지 담당하는 방법
*
* 장점
* - 자동으로 functor를 만들 수 있다.
*
* 단점
* - job이 너~~무 몰리면? job이 한 쓰레드한테만 몰려 쓰레드가 절대로 끝나지 않는 상황이 발생
* 
* 사용
* - 심리스 mmo에서 사용
*   - 심리스 mmo에서는 액터 단위로 job queue를 배치하는 경우도 있음
*   - 때문에 flush를 해야 되는 대상이 엄청나게 많아질 수 있음(거의 몇십만 단위로)
*/

#include <functional>


/*-----------
	 Job
-----------*/

using CallbackType = std::function<void()>;
class Job
{
public:
    Job(CallbackType&& callback) : _callback(std::move(callback))
    {

    }

    template<typename T, typename Ret, typename... Args>
    Job(std::shared_ptr<T> owner, Ret(T::*memFunc)(Args...), Args... args)
    {
        _callback =
            [owner, memFunc, args...]()
            {
                (owner.get()->*memFunc)(args...);
            };
    }

    void Execute()
    {
        _callback();
    }

private:
    CallbackType _callback;
};

/**
* std::functional 템플릿 객체
* - 온갖 callable 타입을 받아줄 수 있음
*	- callable 타입 : 람다, 전역 함수
* 
* 
* callable
* 
* 
* Lamda 함수
* - 익명 함수
* - 람다 캡처 기능
*	- functor랑 비슷
*	- 람다 정의 시, 컴파일러가 내부적으로 functor와 같은 클래스를 만들어서, 캡처한다.
* 
* - 단점
*	- 캡처한 모든 객체가 살아있다는 보장이 없다.
*		- 레퍼런스 카운트와 관련된 문제 : job 방식처럼 요청과 실행이 분리되어 실행되는 구조에서 shared_ptr를 레퍼런스로 들고있는 경우, 람다를 실행할 때 shared_ptr이 해제되었으면 오염된 메모리를 참조할 수 있다.
*	    - 클래스 멤버 함수 안에 람다를 정의하고 자기 자신을 캡처하는 경우,
*		    자기 자신이 메모리에서 해제된 이후에 람다가 실행될 수 있다.(댕글링 포인터가 발생할 수 있음)
*		    - 문제 상황 예시 코드
		    	class Knight
                {
                public:
                    void HealMe(int32 value)
                    {
                        using namespace std;
                        cout << "heal me!" << value << endl;
                
                        _hp += value;
                    }
                
                    void Test()
                    {
                        auto job =
                            [=]()               // 위험한 부분(해당 부분에서 this 포인터를 복사해서 가지고 있어 shared_ptr이 사라지면 해당 객체도 메모리에서 해제된다.)
                            {
                                HealMe(_hp);
                            };
                    }
                
                private:
                    int32 _hp = 100;
                };

            - 개선한 코드
                class Knight : public std::enable_shared_from_this<Knight>
                {
                public:
                    void HealMe(int32 value)
                    {
                        using namespace std;
                        cout << "heal me!" << value << endl;
                
                        _hp += value;
                    }
                
                    void Test()
                    {
                        auto job =
                            [self = shared_from_this()]()       // self가 reference count를 1증가 시켰기 때문에, 객체가 소멸하지 않았다는 것을 보장받을 수 있다.
                            {
                                self->HealMe(self->_hp);
                            };
                    }
                
                private:
                    int32 _hp = 100;
                };
* 
* - 때문에 람다를 쓸 때는 모든 객체를 복사해서는 안된다.
*   - shared_ptr을 캡처하는 경우, shared_ptr 변수를 만들어서 ref count를 1 증가시킨 형태로 복사해서 사용한다.
*       - 예시
*           PlayerRef player = std::make_shared<Player>();
            std::function<void()> func =
                [self = GRoom, &player]()
                {
                    self->Enter(player);
                };
* 
* closure(클로저)
* 
*/

/**
* shared_ptr을 사용하는 경우, this 포인터랑 같이 쓰면 안된다.
*/