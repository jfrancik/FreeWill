
// FWStarterView.cpp : implementation of the CFWStarterView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "FWStarter.h"
#endif

#include "FWStarterDoc.h"
#include "FWStarterView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFWStarterView

IMPLEMENT_DYNCREATE(CFWStarterView, CView)

BEGIN_MESSAGE_MAP(CFWStarterView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CFWStarterView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CFWStarterView construction/destruction

CFWStarterView::CFWStarterView()
{
	// TODO: add construction code here

}

CFWStarterView::~CFWStarterView()
{
}

BOOL CFWStarterView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CFWStarterView drawing

void CFWStarterView::OnDraw(CDC* /*pDC*/)
{
	CFWStarterDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CFWStarterView printing


void CFWStarterView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CFWStarterView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CFWStarterView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CFWStarterView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CFWStarterView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CFWStarterView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CFWStarterView diagnostics

#ifdef _DEBUG
void CFWStarterView::AssertValid() const
{
	CView::AssertValid();
}

void CFWStarterView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CFWStarterDoc* CFWStarterView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFWStarterDoc)));
	return (CFWStarterDoc*)m_pDocument;
}
#endif //_DEBUG


// CFWStarterView message handlers
