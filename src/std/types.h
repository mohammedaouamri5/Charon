#ifndef KERNEL_TYPES_H
#define KERNEL_TYPES_H

/* Fixed-width unsigned integers */
typedef __UINT8_TYPE__    u8;
typedef __UINT16_TYPE__   u16;
typedef __UINT32_TYPE__   u32;
typedef __UINT64_TYPE__   u64;

/* Fixed-width signed integers */
typedef __INT8_TYPE__     i8;
typedef __INT16_TYPE__    i16;
typedef __INT32_TYPE__    i32;
typedef __INT64_TYPE__    i64;

/* Pointer-sized integers */
typedef __UINTPTR_TYPE__  uptr;
typedef __INTPTR_TYPE__   iptr;

/* Sizes and pointer differences */
typedef __SIZE_TYPE__     usize;
typedef __PTRDIFF_TYPE__  isize;

/* Boolean (C23 already has bool as a keyword) */
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 202311L
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

/* Limits: unsigned */
#define U8_MAX     __UINT8_MAX__
#define U16_MAX    __UINT16_MAX__
#define U32_MAX    __UINT32_MAX__
#define U64_MAX    __UINT64_MAX__
#define UPTR_MAX   __UINTPTR_MAX__
#define USIZE_MAX  __SIZE_MAX__

/* Limits: signed */
#define I8_MAX     __INT8_MAX__
#define I16_MAX    __INT16_MAX__
#define I32_MAX    __INT32_MAX__
#define I64_MAX    __INT64_MAX__
#define IPTR_MAX   __INTPTR_MAX__
#define ISIZE_MAX  __PTRDIFF_MAX__

#define I8_MIN     (-I8_MAX - 1)
#define I16_MIN    (-I16_MAX - 1)
#define I32_MIN    (-I32_MAX - 1)
#define I64_MIN    (-I64_MAX - 1)
#define IPTR_MIN   (-IPTR_MAX - 1)
#define ISIZE_MIN  (-ISIZE_MAX - 1)

#ifdef __cplusplus
  #define STATIC_ASSERT(cond, msg) static_assert(cond, msg)
#else
  #define STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#endif

STATIC_ASSERT(sizeof(u8)    == 1, "u8 must be 1 byte");
STATIC_ASSERT(sizeof(u16)   == 2, "u16 must be 2 bytes");
STATIC_ASSERT(sizeof(u32)   == 4, "u32 must be 4 bytes");
STATIC_ASSERT(sizeof(u64)   == 8, "u64 must be 8 bytes");
STATIC_ASSERT(sizeof(i8)    == 1, "i8 must be 1 byte");
STATIC_ASSERT(sizeof(i16)   == 2, "i16 must be 2 bytes");
STATIC_ASSERT(sizeof(i32)   == 4, "i32 must be 4 bytes");
STATIC_ASSERT(sizeof(i64)   == 8, "i64 must be 8 bytes");
STATIC_ASSERT(sizeof(uptr)  == sizeof(void *), "uptr must fit a pointer");
STATIC_ASSERT(sizeof(iptr)  == sizeof(void *), "iptr must fit a pointer");
STATIC_ASSERT(sizeof(usize) == sizeof(void *), "usize must match pointer size");
#endif /* KERNEL_TYPES_H */
