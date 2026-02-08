#include "pch.h"
#include "CoreTLS.h"

thread_local uint32				LThreadID = 0;

// 현재까지 등록된 lock의 모음(Lock이 실행되는 것을 추적) //
// DeadLockProfiler에서 사용
thread_local std::stack<int32>	LLockStack;

thread_local SendBufferChunkRef	LSendBufferChunk = nullptr;