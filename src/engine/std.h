#ifndef TYPES_H
#define TYPES_H

#if defined(_WIN32) && defined(ENGINE_BUILD_DLL)
 #define ENGINE_API __declspec(dllexport)
#elif defined(_WIN32) && defined(ENGINE_DLL)
 #define ENGINE_API __declspec(dllimport)
#elif defined(__GNUC__) && defined(ENGINE_BUILD_DLL)
 #define ENGINE_API __attribute__((visibility("default")))
#else
#define ENGINE_API
#endif

#ifndef _STDINT_H
#define _STDINT_H
typedef signed char        int8_t;
typedef unsigned char      uint8_t;
typedef short              int16_t;
typedef unsigned short     uint16_t;
typedef int                int32_t;
typedef unsigned           uint32_t;
typedef long long          int64_t;
typedef unsigned long long uint64_t;

#define INT8_MIN (-128)
#define INT16_MIN (-32768)
#define INT32_MIN (-2147483647 - 1)
#define INT64_MIN  (-9223372036854775807LL - 1)

#define INT8_MAX 127
#define INT16_MAX 32767
#define INT32_MAX 2147483647
#define INT64_MAX 9223372036854775807LL

#define UINT8_MAX 255
#define UINT16_MAX 65535
#define UINT32_MAX 0xffffffffU
#define UINT64_MAX 0xffffffffffffffffULL
#endif // _STDINT_H

#ifndef _STDBOOL_H
#define _STDBOOL_H
#define bool	_Bool
#define true	1
#define false	0
#endif // _STDBOOL_H

#ifndef NULL
#define NULL ((void*)0)
#endif

#endif // TYPES_H