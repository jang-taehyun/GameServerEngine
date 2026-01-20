#pragma once

#include "Types.h"

/*------------
	Crash
------------*/

// 인위적인 크래시 발생 매크로 //
#define CRASH(cause)						\
{											\
	uint32* crash = nullptr;				\
	__analysis_assume(crash != nullptr);	\
	*crash = (uint32)0xDEADBEEF;			\
}											\

// 조건부 크래시 발생 매크로 //
#define ASSERT_CRASH(expr)					\
{											\
	if (!(expr))							\
	{										\
		CRASH("ASSERT_CRASH");				\
		__analysis_assume(expr);			\
	}										\
}											\