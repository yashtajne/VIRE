#ifndef StdSelf_h
#define StdSelf_h
#include "../../Pure.h"

typedef struct
{
	charseq_t* enviornment;
}
_Self;

extern _Self Self;

#define __PURESTART \
	_Self Self = { (charseq_t*)NULL };

void exit(uint8_t exitcode);

#endif // StdSelf_h