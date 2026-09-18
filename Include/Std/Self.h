#ifndef StdSelf_h
#define StdSelf_h
#include "../../Pure.h"

#if IS_PLATFORM_WINDOWS
	#include "../../Platform/Windows/Kernel32.h"
	#include "../../Platform/Windows/IOCalls.h"
	#include "../../Platform/Windows/FileCalls.h"
	#include "../../Platform/Windows/ProcessCalls.h"
#endif

#if IS_PLATFORM_LINUX
	#include "../../Platform/Linux/Kernel.h"
#endif

typedef struct
{
	charseq_t* enviornment;
}
_Self;

extern _Self Self;

#define __PURESTART \
	_Self Self = { (charseq_t*)NULL };

static inline void exit(uint8_t exitcode)
{
#if IS_PLATFORM_WINDOWS
	ExitProcess( exitcode );
#elif IS_PLATFORM_LINUX
	linux_exit( exitcode );
#else
	#error "platform not supported!"
#endif
}

#endif // StdSelf_h