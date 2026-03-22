/*
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Copyright (c) Arvid Gerstmann. All rights reserved.
 */
#ifndef _WINDOWS_
#ifndef WINDOWS_WINDOW_H
#define WINDOWS_WINDOW_H

/* Disable all warnings */
#if defined(_MSC_VER)
    #pragma warning(push, 0)
#endif

#ifndef WINDOWS_BASE_H
#include "windows_base.h"
#endif
#if defined(__cplusplus)
extern "C" {
#endif

typedef struct DECLSPEC_ALIGN(16) _M128A
{
	ULONGLONG Low;
	LONGLONG High;
} M128A, * PM128A;
typedef struct DECLSPEC_ALIGN(16) _XSAVE_FORMAT
{
	WORD ControlWord;
	WORD StatusWord;
	BYTE TagWord;
	BYTE Reserved1;
	WORD ErrorOpcode;
	DWORD ErrorOffset;
	WORD ErrorSelector;
	WORD Reserved2;
	DWORD DataOffset;
	WORD DataSelector;
	WORD Reserved3;
	DWORD MxCsr;
	DWORD MxCsr_Mask;
	M128A FloatRegisters[8];

	M128A XmmRegisters[16];
	BYTE Reserved4[96];
} XSAVE_FORMAT, * PXSAVE_FORMAT;
typedef XSAVE_FORMAT XMM_SAVE_AREA32, * PXMM_SAVE_AREA32;
typedef struct DECLSPEC_ALIGN(16) _CONTEXT
{
	DWORD64 P1Home;
	DWORD64 P2Home;
	DWORD64 P3Home;
	DWORD64 P4Home;
	DWORD64 P5Home;
	DWORD64 P6Home;

	DWORD ContextFlags;
	DWORD MxCsr;

	WORD SegCs;
	WORD SegDs;
	WORD SegEs;
	WORD SegFs;
	WORD SegGs;
	WORD SegSs;
	DWORD EFlags;

	DWORD64 Dr0;
	DWORD64 Dr1;
	DWORD64 Dr2;
	DWORD64 Dr3;
	DWORD64 Dr6;
	DWORD64 Dr7;

	DWORD64 Rax;
	DWORD64 Rcx;
	DWORD64 Rdx;
	DWORD64 Rbx;
	DWORD64 Rsp;
	DWORD64 Rbp;
	DWORD64 Rsi;
	DWORD64 Rdi;
	DWORD64 R8;
	DWORD64 R9;
	DWORD64 R10;
	DWORD64 R11;
	DWORD64 R12;
	DWORD64 R13;
	DWORD64 R14;
	DWORD64 R15;
	DWORD64 Rip;

	union
	{
		XMM_SAVE_AREA32 FltSave;
		struct
		{
			M128A Header[2];
			M128A Legacy[8];
			M128A Xmm0;
			M128A Xmm1;
			M128A Xmm2;
			M128A Xmm3;
			M128A Xmm4;
			M128A Xmm5;
			M128A Xmm6;
			M128A Xmm7;
			M128A Xmm8;
			M128A Xmm9;
			M128A Xmm10;
			M128A Xmm11;
			M128A Xmm12;
			M128A Xmm13;
			M128A Xmm14;
			M128A Xmm15;
		} DUMMYSTRUCTNAME;
	} DUMMYUNIONNAME;

	M128A VectorRegister[26];
	DWORD64 VectorControl;

	DWORD64 DebugControl;
	DWORD64 LastBranchToRip;
	DWORD64 LastBranchFromRip;
	DWORD64 LastExceptionToRip;
	DWORD64 LastExceptionFromRip;
} CONTEXT, * PCONTEXT;
typedef PCONTEXT LPCONTEXT;

void WINAPI DebugBreak(void);

BOOL WINAPI IsDebuggerPresent(void);

BOOL WINAPI CheckRemoteDebuggerPresent(
		HANDLE hProcess,
		PBOOL hbDebuggerPresent);

void WINAPI OutputDebugStringA(
		LPCSTR lpOutputString);

void WINAPI OutputDebugStringW(
		LPCWSTR lpOutputString);

BOOL WINAPI GetThreadContext(
		HANDLE hThread,
		LPCONTEXT lpContext);

BOOL WINAPI DebugActiveProcess(
		DWORD dwProcessId);

BOOL WINAPI DebugActiveProcessStop(
		DWORD dwProcessId);

#if defined(__cplusplus)
}
#endif

/* Enable all warnings */
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif /* WINDOWS_WINDOW_H */
#endif /* _WINDOWS_ */