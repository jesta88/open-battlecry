#pragma once

#include <stddef.h>

typedef int BOOL;
typedef char CHAR;
typedef short SHORT;
typedef int INT;
typedef long LONG;
typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef unsigned int UINT;
typedef unsigned long ULONG;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
#ifndef HAVE_WCHAR_T
#define HAVE_WCHAR_T
typedef unsigned short wchar_t;
#endif
typedef wchar_t WCHAR;
typedef wchar_t* PWCHAR;
typedef size_t SIZE_T;
typedef WORD ATOM;
typedef unsigned int ULONG32;
typedef unsigned __int64 DWORD64;
typedef unsigned __int64 ULONG64;
typedef unsigned __int64 DWORDLONG;

typedef CHAR* LPSTR;
typedef WCHAR* LPWSTR;
typedef const CHAR* LPCSTR;
typedef const WCHAR* LPCWSTR;

typedef __int64 LONGLONG;
typedef unsigned __int64 ULONGLONG;

typedef void* LPVOID;
typedef BOOL* PBOOL;
typedef LPVOID HANDLE;

#define _CRTALLOC(x)        __declspec(allocate(x))
#if _MSC_VER
#define DECLSPEC_ALIGN(x)   __declspec(align(x))
#else
#define DECLSPEC_ALIGN(x)    __declspec(aligned(x))
#endif

#define WINAPI __stdcall