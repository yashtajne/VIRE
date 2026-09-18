#ifndef String_h
#define String_h
#include "../../Pure.h"

#include "Error.h"
#include "Memory.h"

/*
StringBuilder
	- alloc only object
*/
struct StringBuilder
{
	uint64_t size;
	uint64_t capacity;
	uchar_t  chars[];
};

struct StringBuilder* mempool_newStringBuilder  (struct MemoryPoolHeader*, uint64_t capacity, err_t* occured);
err_t                 mempool_freeStringBuilder (struct MemoryPoolHeader*, struct StringBuilder* string_builder);

err_t StringBuilder_reset  (struct StringBuilder*);
err_t StringBuilder_append (struct StringBuilder*, charseq_t seq);

/*
String
	- semialloc object
	- inline buffer that can hold 24 bytes
*/
#define STRING_INLINE_BUFFER_SIZE 24

#define empty_string                          \
	(string_t) {                              \
		.characters = { 0 },                  \
		.size = 0,                            \
		.capacity = STRING_INLINE_BUFFER_SIZE \
}

struct _String
{
	union
	{
		char_t    inline_buffer[STRING_INLINE_BUFFER_SIZE];
		charseq_t allocated_buffer;
	} characters;
	uint32_t size;
	uint32_t capacity;
};
typedef struct _String string_t;
typedef struct _String str_t;

charseq_t string_getCharseq(string_t* string, err_t* occured);

#define string_sizeOfFreeBytes(s) (((s)->capacity - (s)->size) > 0 ? ((s)->capacity - (s)->size - 1) : 0)
#define string_isInline(s) ((s)->capacity <= STRING_INLINE_BUFFER_SIZE)
#define string_charseq(s) (string_isInline((s)) ? (s)->characters.inline_buffer : (s)->characters.allocated_buffer)

static inline boolean string_hasSpaceFor(string_t* string, uint64_t size)
{
	return string_sizeOfFreeBytes(string) >= size;
}

err_t mempool_initString(struct MemoryPoolHeader*, string_t*, uint32_t capacity);

string_t* mempool_newString    (struct MemoryPoolHeader*, uint32_t capacity, err_t* occured);
err_t     mempool_freeString   (struct MemoryPoolHeader*, string_t*);
err_t     mempool_resizeString (struct MemoryPoolHeader*, string_t*); // <-- yet to implement !!!

err_t string_fromInteger         (string_t* string, uint64_t integer);

err_t poolstring_fromCharseq         (struct MemoryPoolHeader*, string_t* string, charseq_t seq);
err_t poolstring_fromCharseqWithSize (struct MemoryPoolHeader*, string_t* string, charseq_t seq, uint32_t length);

err_t string_appendChar    (string_t* string, char ch);
err_t string_appendCharseq (string_t* string, charseq_t seq);

err_t string_toUppercase (string_t* string);
err_t string_toLowercase (string_t* string);

err_t string_shrink (string_t* string, uint32_t new_size);
err_t string_trim   (string_t* string, uint8_t from);

// err_t string_split (string_t* string, char separator, array_t** out_array);
// err_t string_merge (string_t* s1, string_t* s2); // <-- Yet to implement

#endif // String_h