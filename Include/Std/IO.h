#ifndef StdIO_h
#define StdIO_h
#include "Self.h"

#include "../Core/String.h"
#include "../Core/ASCII.h"
#include "../Core/Char.h"

uint64_t __system_print (const char* chars, uint32_t count);
uint64_t __system_scan  (char* buffer, uint32_t count);

/*
Each format value is stored as one machine word.
%d reads that word as an integer; %s reads it as a charseq pointer.
Helpers are functions so _Generic does not type-check unused cast branches.
(x)+0 decays string literals (char[N]) to char*.
*/
static inline uintptr_t _fmt_word_ptr   (const void* p)          { return (uintptr_t)p; }
static inline uintptr_t _fmt_word_i32   (int v)                  { return (uintptr_t)(int64_t)v; }
static inline uintptr_t _fmt_word_u32   (unsigned int v)         { return (uintptr_t)v; }
static inline uintptr_t _fmt_word_long  (long v)                 { return (uintptr_t)(int64_t)v; }
static inline uintptr_t _fmt_word_ulong (unsigned long v)        { return (uintptr_t)v; }
static inline uintptr_t _fmt_word_i64   (long long v)            { return (uintptr_t)v; }
static inline uintptr_t _fmt_word_u64   (unsigned long long v)   { return (uintptr_t)v; }

#define _fmt_box(x) \
	_Generic((x)+0, \
		char*:              _fmt_word_ptr, \
		void*:              _fmt_word_ptr, \
		int:                _fmt_word_i32, \
		unsigned int:       _fmt_word_u32, \
		long:               _fmt_word_long, \
		unsigned long:      _fmt_word_ulong, \
		long long:          _fmt_word_i64, \
		unsigned long long: _fmt_word_u64 \
	)(x)

#define _fmt_concat(a, b) a##b
#define _fmt_concat2(a, b) _fmt_concat(a, b)
#define _fmt_narg(...) _fmt_narg_(__VA_ARGS__, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define _fmt_narg_(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, N, ...) N

#define _fmt_map_1(a) _fmt_box(a)
#define _fmt_map_2(a, ...) _fmt_box(a), _fmt_map_1(__VA_ARGS__)
#define _fmt_map_3(a, ...) _fmt_box(a), _fmt_map_2(__VA_ARGS__)
#define _fmt_map_4(a, ...) _fmt_box(a), _fmt_map_3(__VA_ARGS__)
#define _fmt_map_5(a, ...) _fmt_box(a), _fmt_map_4(__VA_ARGS__)
#define _fmt_map_6(a, ...) _fmt_box(a), _fmt_map_5(__VA_ARGS__)
#define _fmt_map_7(a, ...) _fmt_box(a), _fmt_map_6(__VA_ARGS__)
#define _fmt_map_8(a, ...) _fmt_box(a), _fmt_map_7(__VA_ARGS__)
#define _fmt_map_9(a, ...) _fmt_box(a), _fmt_map_8(__VA_ARGS__)
#define _fmt_map_10(a, ...) _fmt_box(a), _fmt_map_9(__VA_ARGS__)
#define _fmt_map_11(a, ...) _fmt_box(a), _fmt_map_10(__VA_ARGS__)
#define _fmt_map_12(a, ...) _fmt_box(a), _fmt_map_11(__VA_ARGS__)

#define _fmt_args(...) _fmt_concat2(_fmt_map_, _fmt_narg(__VA_ARGS__))(__VA_ARGS__)

#define format(E, F, ...) \
	__format( \
		F, \
		(uintptr_t[]){ _fmt_args(__VA_ARGS__) }, \
		sizeof((uintptr_t[]){ _fmt_args(__VA_ARGS__) }) / sizeof(uintptr_t), \
		E \
	)

string_t* __format (charseq_t format, uintptr_t* values, uint64_t count, err_t* occured);


#define print(s)                                       \
_Generic((s),                                          \
	voidptr_t : __print_voidptr,                       \
	boolean   : __print_Boolean,                       \
	boolean*  : __print_Boolean_ptr,                   \
	char  : __print_Char,                              \
	char* : __print_CharSeq,                           \
	struct StringBuilder  : __print_StringBuilder,     \
	struct StringBuilder* : __print_StringBuilder_ptr, \
	struct _String  : __print_String,                  \
	struct _String* : __print_String_ptr               \
)(s)

#define println(s) do { \
	print(s);           \
	print((char)'\n');  \
} while (0)

static inline uint32_t
__print_Boolean(boolean b)
{ return b ? __system_print( "true", 4 ) : __system_print( "false", 5 ); }

static inline uint32_t
__print_Boolean_ptr(boolean* b)
{ return *b ? __system_print( "true", 4 ) : __system_print( "false", 5 ); }

static inline uint32_t
__print_Char(char c)
{ return __system_print( &c, 1); }

static inline uint32_t
__print_CharSeq(char* charseq)
{ return __system_print( charseq, (uint32_t)charseq_countbytes( charseq ) ); }

static inline uint32_t
__print_StringBuilder(struct StringBuilder sb)
{ return __system_print( (char*)&sb.chars[0], (uint32_t)sb.size ); }

static inline uint32_t
__print_StringBuilder_ptr(struct StringBuilder* sb)
{ return __system_print( (char*)&sb->chars[0], (uint32_t)sb->size ); }

static inline uint32_t
__print_String(struct _String s)
{
	err_t err;
	charseq_t chars = string_getCharseq( &s, &err );
	if ( !err ) return __system_print( chars, s.size );
	return 0;
}

static inline uint32_t
__print_String_ptr(struct _String* s)
{
	err_t err;
	charseq_t chars = string_getCharseq( s, &err );
	if ( !err ) return __system_print( chars, s->size );
	return 0;
}

static inline uint32_t
__print_voidptr(voidptr_t b)
{
	char_t buffer[32];
	err_t err = pointer_to_ascii( b, (byte_t*)buffer, sizeof(buffer) );
	if ( err )
		return __system_print( "ERROR", 5 );
	return b == NULL ? __system_print( "NULL", 4 ) : print( buffer ) ;
}

#endif // IO_h