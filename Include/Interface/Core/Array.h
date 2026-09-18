#ifndef CoreArray_h
#define CoreArray_h
#include "../../Pure.h"

#include "Error.h"
#include "Memory.h"

struct _Array
{
	uint64_t  length;    // Number of items stored
	uint64_t  capacity;  // Max items it can hold
	uint64_t  type_size; // Size of one element
	byteptr_t elements;  // Actual Elements
};
typedef struct _Array array_t;
typedef struct _Array arr_t;

#define array_at(A, I) \
    ((A)->elements + ((A)->type_size * (I)))

array_t* mempool_newArray    (struct MemoryPoolHeader*, uint64_t sizeof_type, uint64_t capacity, err_t* occured);
err_t    mempool_freeArray   (struct MemoryPoolHeader*, array_t* array);
err_t    mempool_resizeArray (struct MemoryPoolHeader*, array_t** array, uint64_t new_capacity);

err_t array_reset(array_t* array);

voidptr_t array_elementAt(array_t* array, uint64_t index, err_t* occured);

err_t array_insert    (array_t* array, void* element);
err_t array_insertAt (array_t* array, void* element, uint64_t index);
err_t array_removeAt (array_t* array, uint64_t index);

uint64_t sizeof_used_array_t();  // <-- yet to implement
uint64_t sizeof_whole_array_t(); // <-- yet to implement

err_t array_merge (array_t* a1, array_t* a2, array_t** out_array); // <-- yet to implement
err_t array_split (array_t* array, uint64_t index, array_t** out_array); // <-- yet to implement

#endif // CoreArray_h