#ifndef StdString_h
#define StdString_h
#include "Self.h"

#include "../Core/String.h"

err_t initString (string_t*, uint32_t capacity);

string_t* newString    (uint32_t capacity, err_t* occured);
err_t     freeString   (string_t*);
err_t     resizeString (string_t*, uint32_t new_capacity);
err_t     ensureStringCapacity(string_t* string, uint64_t extra);

err_t heapstring_fromCharseq         (string_t* string, charseq_t seq);
err_t heapstring_fromCharseqWithSize (string_t* string, charseq_t seq, uint32_t length);

#endif // StdString_h