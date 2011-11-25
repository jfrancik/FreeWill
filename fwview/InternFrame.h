#pragma once


// CInternFrame frame

class CInternFrame : public CFrameWnd
{
	DECLARE_DYNCREATE(CInternFrame)
protected:
	CInternFrame();           // protected constructor used by dynamic creation
	virtual ~CInternFrame();

// Attributes
public:
	CToolBar m_wndToolBar;

protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext *pContext);
};


