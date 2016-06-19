#pragma once

#ifndef __AVIFILE_H
#define __AVIFILE_H

#include <vfw.h>

class CAviFile
{
	HDC				m_hAviDC;
	HANDLE			m_hHeap;
	LPVOID			m_lpBits;
	LONG			m_lSample;
	TCHAR			m_szFileName[MAX_PATH];
	DWORD			m_nRate;
	PAVIFILE		m_pAviFile;
	PAVISTREAM		m_pAviStream;
	PAVISTREAM		m_pAviCompressedStream;
	AVISTREAMINFO	m_AviStreamInfo;
	AVICOMPRESSOPTIONS	m_AviCompressOptions;
	DWORD m_fccHandler;

	int		nAppendFuncSelector;		//0=Dummy	1=FirstTime	2=Usual
	HRESULT	AppendFrameFirstTime(HBITMAP );
	HRESULT	AppendFrameUsual(HBITMAP);
	HRESULT	AppendDummy(HBITMAP);
	HRESULT	(CAviFile::*pAppendFrame[3])(HBITMAP hBitmap);

	HRESULT	AppendFrameFirstTime(int, int, LPVOID,int );
	HRESULT	AppendFrameUsual(int, int, LPVOID,int );
	HRESULT	AppendDummy(int, int, LPVOID,int );
	HRESULT	(CAviFile::*pAppendFrameBits[3])(int, int, LPVOID,int );
	void	ReleaseMemory();
public:
	CAviFile(LPCTSTR lpszFileName = L"Output.avi", DWORD nRate = 25, DWORD fccHandler = 0);
	~CAviFile(void);
	HRESULT	AppendNewFrame(HBITMAP hBitmap);
	HRESULT	AppendNewFrame(int nWidth, int nHeight, LPVOID pBits,int nBitsPerPixel=32);
};

#endif