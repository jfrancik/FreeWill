// InternFrame.cpp : implementation file
//

#include "stdafx.h"
#include "fwview.h"
#include "InternFrame.h"


// CInternFrame

IMPLEMENT_DYNCREATE(CInternFrame, CFrameWnd)

CInternFrame::CInternFrame()
{
}

CInternFrame::~CInternFrame()
{
}


BEGIN_MESSAGE_MAP(CInternFrame, CFrameWnd)
END_MESSAGE_MAP()


// CInternFrame message handlers

BOOL CInternFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext *pContext)
{
	if (CFrameWnd::OnCreateClient(lpcs, pContext))
	{
		if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_BOTTOM
			| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
			!m_wndToolBar.LoadToolBar(IDR_TOOLBAR_LEFT))
		{
			TRACE0("Failed to create toolbar\n");
			return -1;      // fail to create
		}

		// TODO: Delete these three lines if you don't want the toolbar to be dockable
		m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
		EnableDocking(CBRS_ALIGN_ANY);
		DockControlBar(&m_wndToolBar);
		return TRUE;
	}
	else
		return FALSE;
}
