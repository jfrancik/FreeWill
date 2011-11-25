// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "fwview.h"

#include "MainFrm.h"
#include "InternFrame.h"
#include "LeftView.h"
#include "FreeWillView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Define this to create a smaller display window to capture avis
//#define SMALL_WINDOW

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
//	ID_INDICATOR_CAPS,
//	ID_INDICATOR_NUM,
//	ID_INDICATOR_SCRL,
	ID_INDICATOR_TEST,
	ID_INDICATOR_NAME,
	ID_INDICATOR_OP,
	ID_INDICATOR_PLAYTIME,
};


// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	// TODO: Delete these three lines if you don't want the toolbar to be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/,
	CCreateContext *pContext)
{
	// create splitter window
	if (!m_wndSplitter.CreateStatic(this, 1, 2))
		return FALSE;

	// left pane: CItemsView
	// right pane: CInternFrame (automatically creates palette bar)
#ifdef SMALL_WINDOW
	if (!m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CInternFrame), CSize(200, 200), pContext) ||
#else
	if (!m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CInternFrame), CSize(500, 500), pContext) ||
#endif
		!m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CFreeWillView), CSize(500, 500), pContext))
	{
		m_wndSplitter.DestroyWindow();
		return FALSE;
	}

	// create view in the frame in right splitter
	CView *pView = (CView*)RUNTIME_CLASS(CLeftView)->CreateObject();
	CRect rect(0, 0, 0, 0);
	pView->Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER,
		rect, m_wndSplitter.GetPane(0, 0), AFX_IDW_PANE_FIRST+1, pContext);

	SetActiveView((CView*)m_wndSplitter.GetPane(0, 1));

	return TRUE;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;

#ifdef SMALL_WINDOW
	cs.style = WS_OVERLAPPED | WS_CAPTION | FWS_ADDTOTITLE
		 | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU;
#else
	cs.style = WS_OVERLAPPED | WS_CAPTION | FWS_ADDTOTITLE
		 | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_MAXIMIZE | WS_SYSMENU;
#endif

	return TRUE;
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame message handlers

