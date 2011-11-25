// LeftView.h : interface of the CLeftView class
//
#pragma once

#include <freewill.h>

class CFreeWillDoc;

class CLeftView : public CTreeView
{
protected: // create from serialization only
	CLeftView();
	DECLARE_DYNCREATE(CLeftView)

// Attributes
public:
	CFreeWillDoc *GetDocument();
	IKineNode *m_pCurrent;

// Operations
public:

// Overrides
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void OnInitialUpdate(); // called first time after construct

	HTREEITEM CLeftView::InsertItem(IKineChild *pChild, HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST);

// Implementation
public:
	virtual ~CLeftView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTvnSelchanged(NMHDR *pNMHDR, LRESULT *pResult);
protected:
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
public:
	afx_msg void OnLeftExpandAll();
	afx_msg void OnLeftExpandBipedHands();
	afx_msg void OnLeftExpandBipedFingers();
	afx_msg void OnLeftExpandNum(UINT);
	afx_msg void OnTvnGetInfoTip(NMHDR *pNMHDR, LRESULT *pResult);
};

#ifndef _DEBUG  // debug version in LeftView.cpp
inline CFreeWillDoc *CLeftView::GetDocument()
   { return reinterpret_cast<CFreeWillDoc*>(m_pDocument); }
#endif

