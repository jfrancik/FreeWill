// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500		// Specifies that the minimum required platform is Windows 2000.
#endif

#pragma warning (disable:4996)

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
