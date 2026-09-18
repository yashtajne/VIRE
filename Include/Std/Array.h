#ifndef StdArray_h
#define StdArray_h
#include "Self.h"

#include "../Core/Array.h"

array_t* newArray    (uint64_t size_of_type, uint64_t capacity, err_t* occured);
err_t    freeArray   (array_t* array);
err_t    resizeArray (array_t** array, uint64_t new_capacity);

#define heaparray(T, C, E) (newArray( sizeof(T), (C), (E) ))

#endif // StdArray_h