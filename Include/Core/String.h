#ifndef String_h
#define String_h
#include "../../Pure.h"

#include "Error.h"
#include "Memory.h"

/*---- String Utility Functions (libc replacement) ----*/

static inline uint64_t string_length(const char* str)
{
    if (str == NULL) return 0;
    uint64_t len = 0;
    while (str[len] != '\0')
        len++;
    return len;
}

static inline int string_compare(const char* s1, const char* s2)
{
    if (s1 == NULL || s2 == NULL) return -1;
    while (*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    return *(uint8_t*)s1 - *(uint8_t*)s2;
}

static inline char* string_copy(char* dest, const char* src)
{
    if (dest == NULL || src == NULL) return NULL;
    char* orig_dest = dest;
    while ((*dest++ = *src++) != '\0');
    return orig_dest;
}

static inline char* string_copy_n(char* dest, const char* src, uint64_t n)
{
    if (dest == NULL || src == NULL) return NULL;
    char* orig_dest = dest;
    uint64_t i = 0;
    while (i < n && src[i] != '\0')
    {
        dest[i] = src[i];
        i++;
    }
    while (i < n)
        dest[i++] = '\0';
    return orig_dest;
}

static inline boolean string_is_space(char c)
{
    return c == ' ' || (c >= '\t' && c <= '\r');
}

/*---- StringBuilder ----*/
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

/*---- String ----*/
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
err_t     mempool_resizeString (struct MemoryPoolHeader*, string_t*);

err_t string_fromInteger         (string_t* string, uint64_t integer);

err_t poolstring_fromCharseq         (struct MemoryPoolHeader*, string_t* string, charseq_t seq);
err_t poolstring_fromCharseqWithSize (struct MemoryPoolHeader*, string_t* string, charseq_t seq, uint32_t length);

err_t string_appendChar    (string_t* string, char ch);
err_t string_appendCharseq (string_t* string, charseq_t seq);

err_t string_toUppercase (string_t* string);
err_t string_toLowercase (string_t* string);

err_t string_shrink (string_t* string, uint32_t new_size);
err_t string_trim   (string_t* string, uint8_t from);

#endif // String_h