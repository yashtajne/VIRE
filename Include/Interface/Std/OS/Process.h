#ifndef Process_H
#define Process_H
#include "../Self.h"
#include "../../Core/Error.h"

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
