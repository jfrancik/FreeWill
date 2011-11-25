//------------------------------------------------------------------------------
// C-string formatting routines
// Module:  strformat.h
// Author:  Marek Mittmann
// Date:    4.02.2003 (last modification: 22.09.2005)
// Desc.:   
//------------------------------------------------------------------------------

#ifndef _STRFORMAT_H__INCLUDED_
#define _STRFORMAT_H__INCLUDED_

#include <wtypes.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

const char* ConcatStrs(const char* szStr1, const char* szStr2);
const char* FormatStr(const char* szFormat, ...);

const char* IntToStr(long iValue, int nBase = 10);
const char* UIntToStr(unsigned long uValue, int nBase = 10);
const char* FloatToStr(double dValue, int nDigits = 4);
const char* BoolToStr(bool bValue);

char* TmpStrBuff();
int TmpStrBuffSize();

const char* WStrToStr(const LPOLESTR szwSource);
const LPOLESTR StrToWStr(const char* szSource);

const char* ExtractFilePath(const char* szFileName, bool bIncTrailingBackslash = true);
const char* ExtractFileName(const char* szFileName, bool bWithExt = false);
const char* ExtractFileExt(const char* szFileName);
const char* IncludeFileExt(const char* szFileName, const char* szExt);

#endif // _STRFORMAT_H__INCLUDED_