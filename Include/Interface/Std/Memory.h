#ifndef StdMemory_h
#define StdMemory_h
#include "Self.h"

#include "../Core/Error.h"
#include "../Core/Memory.h"

err_t heap_allocate   (uint64_t bytes, voidptr_t* out_pointer);
err_t heap_reallocate (voidptr_t pointer, uint64_t bytes, voidptr_t* out_pointer);
err_t heap_deallocate (voidptr_t pointer);

voidptr_t allocate   (uint64_t bytes, err_t* occured);
voidptr_t reallocate (voidptr_t pointer, uint64_t bytes, err_t* occured);
void      deallocate (voidptr_t pointer, err_t* occured);

#endif // Memory_h