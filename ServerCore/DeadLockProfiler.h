#pragma once

/*----------------------------------
		Dead Lock Profiler
----------------------------------*/

using std::unordered_map;
using std::vector;
using std::stack;
using std::map;
using std::set;

class DeadLockProfiler
{
public:
	// Lock 등록 //
	void PushLock(const char* name);	// name : Lock의 이름
	
	// Lock 해제 //
	void PopLock(const char* name);
	
	// cycle 판별 //
	void CheckCycle();

private:
	void DFS(int32 here);

private:
	// Lock ID -> Lock 이름 //
	unordered_map<const char*, int32>	_nameToID;

	// Lock 이름 -> Lock ID //
	unordered_map<int32, const char*>	_IDToName;

	// 현재까지 등록된 lock의 모음(Lock이 실행되는 것을 추적) //
	stack<int32>						_lockStack;

	// Lock의 history(간선 정보) //
	map<int32, set<int32>>				_lockHistory;	// 어떤 lock이 몇 번째 lock을 잡았는지 기록

	Mutex								_lock;

// DFS 탐색시 필요한 리소스 //
private:

	// DFS 방문 순서 //
	vector<int32>						_discoverOrder;

	// 노드가 발견된 횟수 //
	int32								_discoverCount;

	// i번째 DFS 탐색이 종료되었는지 확인하는 플래그 배열 //
	vector<bool>						_finished;

	// DFS 탐색 시 발견되는 노드의 부모 //
	vector<int32>						_parent;
};

