#pragma once

#include <thread>
#include <functional>

using std::vector;
using std::thread;
using std::function;

/*------------------
	ThreadManager
------------------*/

class ThreadManager
{
public:
	ThreadManager();
	~ThreadManager();

	// 쓰레드 생성 및 실행 //
	void	Launch(function<void(void)> callback);

	// 쓰레드 join //
	void	Join();

	// TLS 초기화 //
	static void InitTLS();

	// TLS 해제 //
	static void DestroyTLS();

	// global queue에 있는 job queue 실행 //
	static void DoGlobalQueueWork();

	// job timer에 예약된 job들을 실행 //
	static void DistributeReserveJobs();

private:
	Mutex _lock;
	vector<thread>	_threads;
};

