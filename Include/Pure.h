#ifndef Pure_h
#define Pure_h

#if defined(_WIN32) || defined(_WIN64)
	#define IS_PLATFORM_WINDOWS 1
#else
	#define IS_PLATFORM_WINDOWS 0
#endif

#if defined(__linux__)
	#define IS_PLATFORM_LINUX 1
#else
	#define IS_PLATFORM_LINUX 0
#endif

#if defined(__APPLE__)
	#define IS_PLATFORM_APPLE 1
#else
	#define IS_PLATFORM_APPLE 0
#endif

#define ONE_B  1ULL
#define ONE_KB 1000ULL
#define ONE_MB (1000ULL * 1000ULL)
#define ONE_GB (1000ULL * 1000ULL * 1000ULL)

#define ONE_KiB (1024ULL)
#define ONE_MiB (1024ULL * 1024ULL)
#define ONE_GiB (1024ULL * 1024ULL * 1024ULL)

enum Directions
{
	LEFT = 1,
	RIGHT,
	UP,
	DOWN,
};

/*
	int8           : -128 to 127
	unsigned int8  : 0 to 255

	int16          : -32,768 to 32,767
	unsigned int16 : 0 to 65,535

	int32          : -2,147,483,648 to 2,147,483,647
	unsigned int32 : 0 to 4,294,967,295

	int64          : -9,223,372,036,854,775,808 to 9,223,372,036,854,775,807
	unsigned int64 : 0 to 18,446,744,073,709,551,615
*/
#define INT8_MIN   (-128)
#define INT8_MAX   127
#define UINT8_MAX  255U

#define INT16_MIN  (-32768)
#define INT16_MAX  32767
#define UINT16_MAX 65535U

#define INT32_MIN  (-2147483647 - 1)
#define INT32_MAX  2147483647
#define UINT32_MAX 4294967295U

#define INT64_MIN  (-9223372036854775807LL - 1)
#define INT64_MAX  9223372036854775807LL
#define UINT64_MAX 18446744073709551615ULL

#if defined(_MSC_VER)
	typedef __int8           int8_t;
	typedef __int16          int16_t;
	typedef __int32          int32_t;
	typedef __int64          int64_t;

	typedef unsigned __int8  uint8_t;
	typedef unsigned __int16 uint16_t;
	typedef unsigned __int32 uint32_t;
	typedef unsigned __int64 uint64_t;

#elif defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
	typedef __INT8_TYPE__    int8_t;
	typedef __INT16_TYPE__   int16_t;
	typedef __INT32_TYPE__   int32_t;
	typedef __INT64_TYPE__   int64_t;

	typedef __UINT8_TYPE__   uint8_t;
	typedef __UINT16_TYPE__  uint16_t;
	typedef __UINT32_TYPE__  uint32_t;
	typedef __UINT64_TYPE__  uint64_t;

#else
	#error "Unknown compiler. Cannot define exact-width integer types."
#endif

/*
#if   defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 202311L)
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
	typedef _Bool bool;
	#define true 1
	#define false 0
#else
	typedef unsigned char bool;
	#define true 1
	#define false 0
#endif
*/

/* ------- boolean ------- */
typedef uint8_t boolean;
#define true 1
#define false 0

/* ------- Pointers ------- */
typedef void* voidptr_t;

#if IS_PLATFORM_WINDOWS
	#if defined(_WIN64)
		typedef uint64_t uintptr_t;
	#elif defined(_WIN32)
		typedef uint32_t uintptr_t;
	#endif
#elif IS_PLATFORM_LINUX
	#if __SIZEOF_POINTER__ == 8
		typedef uint64_t uintptr_t;
	#elif __SIZEOF_POINTER__ == 4
		typedef uint32_t uintptr_t;
	#endif
#else
	#error "Unsupported pointer size"
#endif

/* ------- Chars & Bytes ------- */
typedef           uint64_t size_t; // <-- remove this soon!!!!
typedef           char     char_t;
typedef unsigned  char     uchar_t;
typedef unsigned  char     byte_t;

typedef int32_t unicode_t;

typedef           char*    charseq_t;
typedef unsigned  char*    byteseq_t;

typedef           char*    charptr_t;
typedef           byte_t*  byteptr_t;

typedef int8_t errorcode_t;
typedef errorcode_t err_t;

typedef boolean result_t;
#define failure false
#define success true

#define NULL ((void*)0)

#define STR_(x) #x
#define STR(x) STR_(x)

static inline boolean is_ok(err_t e)
{
	return e == 0;
}

#endif // Pure_h