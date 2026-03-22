/*
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Copyright (c) Arvid Gerstmann. All rights reserved.
 */
#ifndef _WINDOWS_
#ifndef WINDOWS_BASE_H
#define WINDOWS_BASE_H

#if !defined(_WIN64)
#error "32 bit Windows is not supported"
#endif

/* _WIN32_WINNT version constants: */
#define _WIN32_WINNT_NT4          0x0400 // Windows NT 4.0
#define _WIN32_WINNT_WIN2K        0x0500 // Windows 2000
#define _WIN32_WINNT_WINXP        0x0501 // Windows XP
#define _WIN32_WINNT_WS03         0x0502 // Windows Server 2003
#define _WIN32_WINNT_WIN6         0x0600 // Windows Vista
#define _WIN32_WINNT_VISTA        0x0600 // Windows Vista
#define _WIN32_WINNT_WS08         0x0600 // Windows Server 2008
#define _WIN32_WINNT_LONGHORN     0x0600 // Windows Vista
#define _WIN32_WINNT_WIN7         0x0601 // Windows 7
#define _WIN32_WINNT_WIN8         0x0602 // Windows 8
#define _WIN32_WINNT_WINBLUE      0x0603 // Windows 8.1
#define _WIN32_WINNT_WINTHRESHOLD 0x0A00 // Windows 10
#define _WIN32_WINNT_WIN10        0x0A00 // Windows 10

#ifndef _WIN32_WINNT
#define _WIN32_WINNT _WIN32_WINNT_WIN10
#endif

/* Magic: */
#define _CRTALLOC(x)      __declspec(allocate(x))
#define DECLSPEC_ALIGN(x) __declspec(align(x))

/* Basic Defines: */
#define NTAPI    __stdcall
#define WINAPI   __stdcall
#define APIENTRY __stdcall
#define CALLBACK __stdcall
#define TRUE     (1)
#define FALSE    (0)
#ifndef NULL
#define NULL ((void*)0)
#endif
#ifndef FORCEINLINE
#define FORCEINLINE __forceinline
#endif
#ifdef UNICODE
#define __TEXT(x) L##x
#define TEXT(x)   __TEXT(x)
#else
#define TEXT(x) x
#endif
#define PATH_MAX 260
#define MAX_PATH 260

#define MINCHAR  0x80
#define MAXCHAR  0x7f
#define MINSHORT 0x8000
#define MAXSHORT 0x7fff
#define MINLONG  0x80000000
#define MAXLONG  0x7fffffff
#define MAXBYTE  0xff
#define MAXWORD  0xffff
#define MAXDWORD 0xffffffff

#define MAKEWORD(a, b)                          \
    ((WORD)(((BYTE)(((DWORD_PTR)(a)) & 0xff)) | \
            ((WORD)((BYTE)(((DWORD_PTR)(b)) & 0xff))) << 8))
#define MAKELONG(a, b)                            \
    ((LONG)(((WORD)(((DWORD_PTR)(a)) & 0xffff)) | \
            ((DWORD)((WORD)(((DWORD_PTR)(b)) & 0xffff))) << 16))
#define LOWORD(l) ((WORD)(((DWORD_PTR)(l)) & 0xffff))
#define HIWORD(l) ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))
#define LOBYTE(w) ((BYTE)(((DWORD_PTR)(w)) & 0xff))
#define HIBYTE(w) ((BYTE)((((DWORD_PTR)(w)) >> 8) & 0xff))

#if !defined(_68K_) && !defined(_MPPC_) && !defined(_X86_) && !defined(_IA64_) && !defined(_AMD64_) && defined(_M_AMD64)
#define _AMD64_
#endif

#define STDMETHODCALLTYPE  __stdcall
#define STDMETHODVCALLTYPE __cdecl
#define STDAPICALLTYPE     __stdcall
#define STDAPIVCALLTYPE    __cdecl

#define interface struct
#define PURE
#define THIS_                     INTERFACE *This,
#define THIS                      INTERFACE* This
#define STDMETHOD(method)         HRESULT(STDMETHODCALLTYPE* method)
#define STDMETHOD_(type, method)  type(STDMETHODCALLTYPE* method)
#define STDMETHODV(method)        HRESULT(STDMETHODVCALLTYPE* method)
#define STDMETHODV_(type, method) type(STDMETHODVCALLTYPE* method)

#define IFACEMETHOD(method)         __override STDMETHOD(method)
#define IFACEMETHOD_(type, method)  __override STDMETHOD_(type, method)
#define IFACEMETHODV(method)        __override STDMETHODV(method)
#define IFACEMETHODV_(type, method) __override STDMETHODV_(type, method)

#define BEGIN_INTERFACE
#define END_INTERFACE

#ifdef CONST_VTABLE
#undef CONST_VTBL
#define CONST_VTBL const
#define DECLARE_INTERFACE(iface)                  \
    typedef interface iface {                     \
        const struct iface##Vtbl* lpVtbl;         \
    } iface;                                      \
    typedef const struct iface##Vtbl iface##Vtbl; \
    const struct iface##Vtbl
#else
#undef CONST_VTBL
#define CONST_VTBL
#define DECLARE_INTERFACE(iface)            \
    typedef interface iface {               \
        struct iface##Vtbl* lpVtbl;         \
    } iface;                                \
    typedef struct iface##Vtbl iface##Vtbl; \
    struct iface##Vtbl
#endif /* CONST_VTABLE */

#define DECLARE_INTERFACE_(iface, baseiface) DECLARE_INTERFACE(iface)

#define HRESULT_IS_WIN32(x) \
    ((((x) >> 16) & 0xFFFF) == 0x8)
#define HRESULT_IS_FAILURE(x) \
    ((((x) >> 31) & 0x1) == 0x1)
#define HRESULT_FACILITY(x) \
    (((x) >> 16) & 0xFFFF)
#define HRESULT_CODE(x) \
    ((x)&0xFFFF)
#define HRESULT_FROM_WIN32(x) \
    (0x80070000 | (x))

/* ========================================================================== */
/* Errors: */
/* ========================================================================== */
#define ERROR_SUCCESS             0L
#define ERROR_FILE_NOT_FOUND      2L
#define ERROR_PATH_NOT_FOUND      3L
#define ERROR_TOO_MANY_OPEN_FILES 4L
#define ERROR_ACCESS_DENIED       5L
#define ERROR_NO_MORE_FILES       18L
#define ERROR_SHARING_VIOLATION   32L
#define ERROR_FILE_EXISTS         80L
#define ERROR_INSUFFICIENT_BUFFER 122L
#define ERROR_ALREADY_EXISTS      183L
#define ERROR_MORE_DATA           234L

#define SUCCEEDED(hr)             (((HRESULT)(hr)) >= 0)
#define FAILED(hr)                (((HRESULT)(hr)) < 0)

/* ========================================================================== */
/* Misc                                                                      */
/* ========================================================================== */
#define STANDARD_RIGHTS_REQUIRED 0x000F0000L

/* ========================================================================== */
/* Enums                                                                      */
/* ========================================================================== */
#define DLL_PROCESS_ATTACH (1)
#define DLL_PROCESS_DETACH (0)
#define DLL_THREAD_ATTACH  (2)
#define DLL_THREAD_DETACH  (3)

/* ========================================================================== */
/* Types                                                                      */
/* ========================================================================== */
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
typedef float FLOAT;
typedef unsigned long DWORD;
#ifndef HAVE_WCHAR_T
#define HAVE_WCHAR_T
typedef unsigned short wchar_t;
#endif
typedef wchar_t WCHAR;
typedef wchar_t* PWCHAR;
typedef WORD ATOM;
typedef unsigned int ULONG32;
typedef unsigned __int64 DWORD64;
typedef unsigned __int64 ULONG64;
typedef signed int INT32;
typedef signed __int64 INT64;
typedef unsigned __int64 DWORDLONG;

typedef CHAR* PCHAR;
typedef ULONG* PULONG;
typedef BYTE* PBYTE;
typedef ULONG64* PULONG64;
typedef DWORD64* PDWORD64;

typedef signed __int64 LONGLONG;
typedef unsigned __int64 ULONGLONG;

typedef void VOID;
typedef void* PVOID;
typedef void* LPVOID;
typedef BOOL* PBOOL;
typedef BOOL* LPBOOL;
typedef WORD* PWORD;
typedef LONG* PLONG;
typedef LONG* LPLONG;
typedef DWORD* PDWORD;

typedef LPVOID HANDLE;
typedef HANDLE HINSTANCE;
typedef HANDLE HWND;
typedef HINSTANCE HMODULE;
typedef HANDLE HDC;
typedef HANDLE HGLRC;
typedef HANDLE HMENU;
typedef HANDLE* PHANDLE;
typedef HANDLE* LPHANDLE;

#define DECLARE_HANDLE(name) \
    struct name##__          \
    {                        \
        int unused;          \
    };                       \
    typedef struct name##__* name

typedef WCHAR* PWSTR;
typedef BYTE* LPBYTE;
typedef long* LPLONG;
typedef DWORD* LPDWORD;
typedef const void* LPCVOID;

typedef signed __int64 INT_PTR;
typedef signed __int64 LONG_PTR;
typedef unsigned __int64 UINT_PTR;
typedef unsigned __int64 ULONG_PTR;
typedef ULONG_PTR DWORD_PTR;
typedef DWORD_PTR* PDWORD_PTR;

typedef ULONG_PTR SIZE_T;
typedef LONG_PTR SSIZE_T;

typedef CHAR* LPSTR;
typedef WCHAR* LPWSTR;
typedef const CHAR* LPCSTR;
typedef const WCHAR *LPCWSTR, *PCWSTR;
#if defined(UNICODE)
typedef WCHAR TCHAR;
typedef WCHAR TBYTE;
typedef LPCWSTR LPCTSTR;
typedef LPWSTR LPTSTR;
#else
typedef char TCHAR;
typedef unsigned char TBYTE;
typedef LPCSTR LPCTSTR;
typedef LPSTR LPTSTR;
#endif

typedef INT_PTR(__stdcall* FARPROC)(void);
typedef INT_PTR(__stdcall* NEARPROC)(void);
typedef INT_PTR(__stdcall* PROC)(void);

typedef DWORD ACCESS_MASK;
typedef ACCESS_MASK* PACCESS_MASK;

typedef HANDLE HICON;
typedef HANDLE HBRUSH;
typedef HICON HCURSOR;

typedef LONG HRESULT;
typedef LONG_PTR LRESULT;
typedef LONG_PTR LPARAM;
typedef UINT_PTR WPARAM;

typedef void* HGDIOBJ;

typedef HANDLE HKEY;
typedef HKEY* PHKEY;
typedef ACCESS_MASK REGSAM;

/* ========================================================================== */
/* Structures: */
/* ========================================================================== */
typedef struct _OVERLAPPED
{
    ULONG_PTR Internal;
    ULONG_PTR InternalHigh;
    union
    {
        struct
        {
            DWORD Offset;
            DWORD OffsetHigh;
        };
        PVOID Pointer;
    };
    HANDLE hEvent;
} OVERLAPPED, *LPOVERLAPPED;

typedef struct _SECURITY_ATTRIBUTES
{
    DWORD nLength;
    LPVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

typedef struct tagRECT
{
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
} RECT, *PRECT, *NPRECT, *LPRECT;

typedef const RECT* LPCRECT;

typedef union _LARGE_INTEGER
{
    struct
    {
        DWORD LowPart;
        LONG HighPart;
    };
    struct
    {
        DWORD LowPart;
        LONG HighPart;
    } u;
    LONGLONG QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

typedef union _ULARGE_INTEGER
{
    struct
    {
        DWORD LowPart;
        DWORD HighPart;
    };
    struct
    {
        DWORD LowPart;
        DWORD HighPart;
    } u;
    ULONGLONG QuadPart;
} ULARGE_INTEGER, *PULARGE_INTEGER;

/* Filetime: */
typedef struct _FILETIME
{
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;

#endif /* WINDOWS_BASE_H */
#endif /* _WINDOWS_ */