#ifndef _ULTRA64_TYPES_H_
#define _ULTRA64_TYPES_H_

#ifndef NULL
#define NULL    (void *)0
#endif

#define TRUE 1
#define FALSE 0

/* For PSP and other non-N64 targets, use stdint.h types to avoid conflicts with PSP SDK */
#ifdef TARGET_PSP
#include <stddef.h>
#include <stdint.h>
typedef int8_t   s8;
typedef uint8_t  u8;
typedef int16_t  s16;
typedef uint16_t u16;
typedef int32_t  s32;
typedef uint32_t u32;
typedef int64_t  s64;
typedef uint64_t u64;
#else
/* Original N64 style type definitions */
#ifndef s8
typedef signed char            s8;
#endif
#ifndef u8
typedef unsigned char          u8;
#endif
#ifndef s16
typedef signed short int       s16;
#endif
#ifndef u16
typedef unsigned short int     u16;
#endif
#ifndef s32
typedef signed int             s32;
#endif
#ifndef u32
typedef unsigned int           u32;
#endif

#ifdef TARGET_N64
#ifndef s64
typedef signed long long int   s64;
#endif
#ifndef u64
typedef unsigned long long int u64;
#endif
#else
#include <stddef.h>
#include <stdint.h>
#ifndef s64
typedef int64_t s64;
#endif
#ifndef u64
typedef uint64_t u64;
#endif
#endif
#endif

typedef float  f32;
typedef double f64;

#ifdef TARGET_N64
typedef u32 size_t;
typedef s32 ssize_t;
typedef u32 uintptr_t;
typedef s32 intptr_t;
typedef s32 ptrdiff_t;
#else
typedef ptrdiff_t ssize_t;
#endif

typedef volatile u8   vu8;
typedef volatile u16 vu16;
typedef volatile u32 vu32;
typedef volatile u64 vu64;
typedef volatile s8   vs8;
typedef volatile s16 vs16;
typedef volatile s32 vs32;
typedef volatile s64 vs64;

#endif
