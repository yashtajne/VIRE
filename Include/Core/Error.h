#ifndef CoreError_h
#define CoreError_h
#include "../../Pure.h"

typedef enum
{
#define PURE_ERROR(name, code, msg) name = code,
#include "../Errors.def"
#undef PURE_ERROR
} PureError;

charseq_t pure_error_string(err_t code);

#endif // Error_h