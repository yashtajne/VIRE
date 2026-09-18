#ifndef CoreASCII_h
#define CoreASCII_h
#include "../../Pure.h"

#include "Error.h"

err_t ascii_to_integer (charseq_t ascii, int64_t* out_integer);

err_t integer_to_ascii (int64_t integer, byte_t* buffer, uint8_t buffer_size);
err_t pointer_to_ascii (voidptr_t pointer, byte_t* buffer, uint8_t buffer_size);

boolean ascii_isSpace (char_t ascii);

#endif // ASCII_h