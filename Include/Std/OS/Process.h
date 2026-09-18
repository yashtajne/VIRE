#ifndef Process_H
#define Process_H
#include "../Self.h"

#include "../../Core/Error.h"

struct _Process
{
#if IS_PLATFORM_WINDOWS
	HANDLE process_handle;
	HANDLE thread_handle;
	uint32_t process_id;
	charseq_t raw_invocation;
	charseq_t invocation;
	charseq_t* arguments;
	uint64_t argument_count;
	uint8_t exitcode;
	boolean has_started;
	boolean has_completed;
#elif IS_PLATFORM_LINUX
	uint32_t process_id;
	charseq_t invocation; // Owned
	charseq_t* arguments;
	uint64_t argument_count;
	uint8_t exitcode;
	boolean has_started;
	boolean has_completed;
#else
	#error "Not yet Implemented!"
#endif
};
typedef struct _Process process_t;

enum {
	PROCESS_CAPTURE_OUTPUT  = 1u << 0,
	PROCESS_INHERIT_STDIN   = 1u << 1,
	PROCESS_INHERIT_STDOUT  = 1u << 2,
	PROCESS_INHERIT_STDERR  = 1u << 3,
};

#define PROCESS_INHERIT_ALL \
	PROCESS_INHERIT_STDIN | PROCESS_INHERIT_STDOUT | PROCESS_INHERIT_STDERR

process_t* process_create(charseq_t invocation, err_t* occured);

err_t process_execute   (process_t* process, uint8_t flags);
err_t process_terminate (process_t* process);
err_t process_waitForCompletion (process_t* process);
err_t process_cleanup (process_t* process);

uint8_t process_getExitCode (process_t* process, err_t* occured);


#endif // Process_h
