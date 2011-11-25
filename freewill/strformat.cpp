//------------------------------------------------------------------------------
// C-string formatting routines
// Module:  Tools.cpp
// Author:  Marek Mittmann
// Date:    4.02.2003 (last modification: 22.09.2005)
// Desc.:   
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "strformat.h"

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Buffer for string formating routines
const int _STR_BUFFER_SIZE	= 0x200;	// this values should
const int _STR_BUFFER_COUNT	= 16;		// be power of 2

char szFmtBuffer[_STR_BUFFER_SIZE * _STR_BUFFER_COUNT];
size_t nFmtBufferPos = 0;
 
inline void MoveFmtBufferPos()
{
	nFmtBufferPos += _STR_BUFFER_SIZE;
	nFmtBufferPos &= _STR_BUFFER_SIZE * _STR_BUFFER_COUNT - 1;
}

inline char* FmtBufferPos()
{
	return szFmtBuffer + nFmtBufferPos;
}

char* TmpStrBuff()
{
	MoveFmtBufferPos();
	return FmtBufferPos();
}

int TmpStrBuffSize()
{
	return _STR_BUFFER_SIZE-sizeof(char);
}

const char* ConcatStrs(const char* szStr1, const char* szStr2)
{
	MoveFmtBufferPos();
	strncpy(FmtBufferPos(), szStr1, _STR_BUFFER_SIZE);
	*(FmtBufferPos()+_STR_BUFFER_SIZE-sizeof(char)) = '\0';
	strncat(FmtBufferPos(), szStr2, _STR_BUFFER_SIZE-1);
	return FmtBufferPos();
}
 
const char* FormatStr(const char* szFormat, ...)
{
	va_list pArgList;

	MoveFmtBufferPos();
	va_start(pArgList, szFormat);
	if (_vsnprintf(FmtBufferPos(), _STR_BUFFER_SIZE-sizeof(char), szFormat, pArgList) < 0)
		*(FmtBufferPos()+_STR_BUFFER_SIZE-sizeof(char)) = '\0';
	va_end(pArgList);
	
	return FmtBufferPos();
}

const char* IntToStr(long iValue, int nBase)
{
	MoveFmtBufferPos();
	ltoa(iValue, FmtBufferPos(), nBase);
	return FmtBufferPos();		
}

const char* UIntToStr(unsigned long uValue, int nBase)
{
	MoveFmtBufferPos();
	ultoa(uValue, FmtBufferPos(), nBase);
	return FmtBufferPos();		
}

const char* FloatToStr(double dValue, int nDigits)
{
	MoveFmtBufferPos();
	_gcvt(dValue, nDigits, FmtBufferPos());
	return FmtBufferPos();			
}

const char* BoolToStr(bool bValue)
{
	MoveFmtBufferPos();
	strcpy(FmtBufferPos(), bValue ? "true" : "false");
	return FmtBufferPos();					
}

const char* WStrToStr(const LPOLESTR szwSource)
{
	MoveFmtBufferPos();
	if (_snprintf(FmtBufferPos(), _STR_BUFFER_SIZE, "%S", szwSource) < 0)
		*(FmtBufferPos()+_STR_BUFFER_SIZE-sizeof(char)) = '\0';
	return FmtBufferPos();
}

const LPOLESTR StrToWStr(const char* szSource)
{
	MoveFmtBufferPos();
	if (_snwprintf((LPOLESTR)FmtBufferPos(), _STR_BUFFER_SIZE / sizeof(wchar_t), L"%S", szSource) < 0)
		*(FmtBufferPos()+_STR_BUFFER_SIZE-sizeof(wchar_t)) = L'\0';
	return (LPOLESTR)FmtBufferPos();
}

const char* ExtractFilePath(const char* szFileName, bool bIncTrailingBackslash)
{
	MoveFmtBufferPos();
	strncpy(FmtBufferPos(), szFileName, _STR_BUFFER_SIZE);
	*(FmtBufferPos()+_STR_BUFFER_SIZE-sizeof(char)) = '\0';
	char* pPos = strrchr(FmtBufferPos(), '\\');
	if (pPos != NULL)
		*(pPos + (bIncTrailingBackslash ? 1 : 0)) = '\0';
	return FmtBufferPos();
}

const char* ExtractFileName(const char* szFileName, bool bWithExt)
{
	MoveFmtBufferPos();
	const char* pPos = strrchr(szFileName, '\\');
	strncpy(FmtBufferPos(), pPos == NULL ? szFileName : pPos+1, _STR_BUFFER_SIZE);
	*(FmtBufferPos()+_STR_BUFFER_SIZE-sizeof(char)) = '\0';
	if (!bWithExt)
	{
		pPos = strrchr(FmtBufferPos(), '.');
//		if (pPos != NULL)		commented out by JF on 6 Nov 2008
//			*pPos = '\0';		caused compilation error!
	}
	return FmtBufferPos();
}

const char* ExtractFileExt(const char* szFileName)
{
	MoveFmtBufferPos();
	const char* pPos = strrchr(szFileName, '.');
	strncpy(FmtBufferPos(), pPos == NULL ? "" : pPos+1, _STR_BUFFER_SIZE);
	*(FmtBufferPos()+_STR_BUFFER_SIZE-sizeof(char)) = '\0';
	return FmtBufferPos();
}

const char* IncludeFileExt(const char* szFileName, const char* szExt)
{
	MoveFmtBufferPos();
	strncpy(FmtBufferPos(), szFileName, _STR_BUFFER_SIZE);
	*(FmtBufferPos()+_STR_BUFFER_SIZE-sizeof(char)) = '\0';
	if (strchr(FmtBufferPos(), '.') == NULL)
	{
		if (szExt[0] != '.')
			strncat(FmtBufferPos(), ".", _STR_BUFFER_SIZE-1);
		strncat(FmtBufferPos(), szExt, _STR_BUFFER_SIZE-1);
	}
	return FmtBufferPos();
}