#ifndef CoreByte_h
#define CoreByte_h
#include "../../Pure.h"

#include "Error.h"

struct _ByteArray
{
	uint64_t size;
	byte_t*  buffer;
};
typedef struct _ByteArray bytearray_t;

#define bytearray_new(A, B) \
do {                        \
	(A).size = sizeof(B);   \
	(A).buffer = (B);       \
} while (0)


#endif // CoreByte_h