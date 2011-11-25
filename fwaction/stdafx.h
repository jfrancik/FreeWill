// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

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
#define USE_MFC
#endif

// Windows Header Files: 
#ifdef USE_MFC
#define _AFXDLL
#include <afxwin.h>
#else
#include <windows.h>
#define TRACE
#endif 

#include <assert.h>