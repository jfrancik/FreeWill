// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define _CRTDBG_MAP_ALLOC

#pragma warning (disable:4996)

// Specify that the minimum required platform is Windows 2000
#ifndef WINVER
#define WINVER 0x0500
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0500
#endif


#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#ifdef _DEBUG
#define _USE_MFC
#endif

// Windows Header Files: 
#ifdef USE_MFC
#include <afxwin.h>
#else
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <windows.h>
#ifdef _DEBUG
inline void _trace(LPCWSTR fmt, ...)
{
	WCHAR out[1024];
	va_list body;
	va_start(body, fmt);
	vswprintf(out, fmt, body);
	va_end(body);
	OutputDebugString(out);
}
#define ASSERT(x) {if(!(x)) _asm{int 0x03}}
#define VERIFY(x) {if(!(x)) _asm{int 0x03}}
#define TRACE _trace
#else
#define ASSERT(x)
#define VERIFY(x) x
inline void _trace(LPCTSTR fmt, ...) { }
#define TRACE  1 ? (void)0 : _trace
#endif
#endif 

#include <assert.h>
