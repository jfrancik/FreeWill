// fwview.h : main header file for the fwview application
//
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols


// CFreeWillApp:
// See fwview.cpp for the implementation of this class
//

class CFreeWillApp : public CWinApp
{
public:
	CFreeWillApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnIdle(LONG lCount);

private:
	CWnd *m_pSplashWnd;
public:
	void ShowSplashWindow();
	void HideSplashWindow();
};

extern CFreeWillApp theApp;