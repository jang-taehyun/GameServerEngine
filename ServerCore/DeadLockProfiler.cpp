#include "pch.h"
#include "DeadLockProfiler.h"

/*----------------------------------
		Dead Lock Profiler
----------------------------------*/

void DeadLockProfiler::PushLock(const char* name)
{
	LockGuard guard(_lock);

	// ID를 찾거나 발급 //
	int32 lockID = 0;
	auto findIt = _nameToID.find(name);
	if (_nameToID.end() == findIt)
	{
		// 처음 발견했기 때문에 ID 발급 및 등록
		lockID = static_cast<int32>(_nameToID.size());
		_nameToID[name] = lockID;
		_IDToName[lockID] = name;
	}
	else
	{
		// 이미 있기 때문에 ID 추출
		lockID = findIt->second;
	}

	// 기존에 잡고 있는 락이 있다면
	if (false == _lockStack.empty())
	{
		// 기존에 발견되지 않은 케이스라면 데드락 여부 다시 확인
		const int32 prevID = _lockStack.top();
		if (prevID != lockID)
		{
			// 새로운 간선을 발견했다면, cycle이 생기는지 검사
			set<int32>& history = _lockHistory[prevID];
			if (history.end() == history.find(lockID))
			{
				history.insert(lockID);
				CheckCycle();
			}
		}

		// else의 경우는 고려하지 않음
		// -> recursive하게 lock을 잡을 수 있기 때문
	}

	// lock 등록 //
	_lockStack.push(lockID);
}

void DeadLockProfiler::PopLock(const char* name)
{
	LockGuard guard(_lock);

	// 여러번 unlock을 시도
	if (_lockStack.empty())
		CRASH("MULTIPLE_UNLOCK");

	// pop 순서가 꼬인 경우
	int32 lockID = _nameToID[name];
	if (_lockStack.top() != lockID)
		CRASH("INVALID_UNLOCK");

	_lockStack.pop();
}

void DeadLockProfiler::CheckCycle()
{
	// 초기화
	const int32 lockCount = static_cast<int32>(_nameToID.size());
	_discoverOrder = vector<int32>(lockCount, -1);
	_discoverCount = 0;
	_finished = vector<bool>(lockCount, false);
	_parent = vector<int32>(lockCount, -1);

	// cycle 검사
	for (int32 lockID = 0; lockID < lockCount; ++lockID)
	{
		DFS(lockID);
	}

	// 해제
	_discoverOrder.clear();
	_finished.clear();
	_parent.clear();
}

void DeadLockProfiler::DFS(int32 here)
{
	// 이미 방문
	if (-1 != _discoverOrder[here])
		return;

	// DFS 탐색 순번 지정
	_discoverOrder[here] = _discoverCount++;

	// history에 해당 노드가 있는지 확인
	auto findIt = _lockHistory.find(here);
	if (_lockHistory.end() == findIt)
	{
		// 없으면 DFS 종료
		_finished[here] = true;
		return;
	}

	// 역방향 간선이 있는지 판별
	set<int32>& nextSet = findIt->second;
	for (int32 there : nextSet)
	{
		if (-1 == _discoverOrder[there])
		{
			// 탐색하지 않았으면 현재 노드를 parent로 지정하고 방문
			_parent[there] = here;
			DFS(there);
			continue;
		}

		// here가 there보다 먼저 발견되었다면, there은 here의 자손이다. (순방향 간선)
		if (_discoverOrder[here] < _discoverOrder[there])
		{
			continue;
		}

		// 순방향 간선이 아니고, DFS(there)이 아직 끝나지 않았다면, there의 here의 조상이다. (역방향 간선)
		if (false == _finished[there])
		{
			printf("Dead lock detected!!\n");
			printf("%s -> %s\n", _IDToName[here], _IDToName[there]);

			int32 now = here;
			do
			{
				printf("%s -> %s\n", _IDToName[_parent[now]], _IDToName[now]);
				now = _parent[now];
			}
			while (now != there);

			CRASH("DEADLOCK_DETECTED");
		}
	}

	_finished[here] = true;
}
