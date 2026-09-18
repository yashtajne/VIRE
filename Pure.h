#ifndef Pure_h
#define Pure_h

/*
 * Pure Runtime - Freestanding C Runtime Foundation
 * 
 * This header provides fundamental type definitions and macros
 * for freestanding C code without libc dependency.
 */

/*---- Platform Detection ----*/

#if defined(_WIN32) || defined(_WIN64)
    #define IS_PLATFORM_WINDOWS 1
    #define IS_PLATFORM_LINUX   0
#elif defined(__linux__)
    #define IS_PLATFORM_WINDOWS 0
    #define IS_PLATFORM_LINUX   1
#else
    #error "Unsupported platform"
#endif

/*---- NULL Definition ----*/

#define NULL ((void*)0)

/*---- Fundamental Types ----*/

/* Boolean type */
typedef unsigned char boolean;

/* Boolean constants */
#define TRUE  1
#define FALSE 0

/* Alternative lowercase macros (avoiding conflicts) */
#define pure_true  1
#define pure_false 0

/*---- Basic Integer Types ----*/

typedef signed char          int8_t;
typedef unsigned char        uint8_t;
typedef signed short         int16_t;
typedef unsigned short       uint16_t;
typedef signed int           int32_t;
typedef unsigned int         uint32_t;
typedef signed long long     int64_t;
typedef unsigned long long   uint64_t;

/*---- Pointer Types ----*/

typedef void*                voidptr_t;
typedef const void*          const_voidptr_t;

/*---- Character Types ----*/

typedef char                 char_t;
typedef char*                charseq_t;
typedef const char*          const_charseq_t;

/* Byte types */
typedef uint8_t              byte_t;
typedef uint8_t              uchar_t;

/*---- Size Type ----*/

typedef uint64_t             size_t;
typedef int64_t              ssize_t;

/*---- Error Type ----*/

typedef int                  err_t;

/*---- Useful Macros ----*/

#define PURE_UNUSED(x)           ((void)(x))
#define PURE_ARRAY_COUNT(arr)    (sizeof(arr) / sizeof((arr)[0]))
#define PURE_MIN(a, b)           ((a) < (b) ? (a) : (b))
#define PURE_MAX(a, b)           ((a) > (b) ? (a) : (b))
#define PURE_ABS(x)              ((x) < 0 ? -(x) : (x))

/* Alignment macros */
#define PURE_ALIGN_UP(x, align)  (((x) + ((align) - 1)) & ~((align) - 1))
#define PURE_ALIGN_DOWN(x, align) ((x) & ~((align) - 1))
#define PURE_IS_ALIGNED(x, align) (((x) & ((align) - 1)) == 0)

/* Bit manipulation macros */
#define PURE_BIT(n)              (1ULL << (n))
#define PURE_MASK(hi, lo)        ((PURE_BIT((hi) + 1) - 1) ^ (PURE_BIT(lo) - 1))
#define PURE_GET_BITS(val, hi, lo) (((val) & PURE_MASK(hi, lo)) >> (lo))
#define PURE_SET_BITS(val, hi, lo, new_val) \
    (((val) & ~PURE_MASK(hi, lo)) | (((new_val) << (lo)) & PURE_MASK(hi, lo)))

/* Stringify macros */
#define PURE_STRINGIFY(x)        #x
#define PURE_TOSTRING(x)         PURE_STRINGIFY(x)

/* Compiler hints */
#if defined(__GNUC__) || defined(__clang__)
    #define PURE_INLINE      __inline__
    #define PURE_NORETURN    __attribute__((noreturn))
    #define PURE_LIKELY(x)   __builtin_expect(!!(x), 1)
    #define PURE_UNLIKELY(x) __builtin_expect(!!(x), 0)
    #define PURE_PACKED      __attribute__((packed))
    #define PURE_ALIGNED(x)  __attribute__((aligned(x)))
#else
    #define PURE_INLINE      inline
    #define PURE_NORETURN
    #define PURE_LIKELY(x)   (x)
    #define PURE_UNLIKELY(x) (x)
    #define PURE_PACKED
    #define PURE_ALIGNED(x)
#endif

/* Static assertion (C11 style) */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
    #define PURE_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#else
    #define PURE_STATIC_ASSERT(cond, msg) typedef char PURE_CONCAT(static_assert_, __LINE__)[(cond) ? 1 : -1]
#endif

/* Concatenation macro */
#define PURE_CONCAT_IMPL(a, b) a##b
#define PURE_CONCAT(a, b)      PURE_CONCAT_IMPL(a, b)

#endif /* Pure_h */
