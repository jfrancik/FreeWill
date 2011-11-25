// FreeWillView.h : interface of the CFreeWillView class
//
#pragma once

//#define TEST_SAVEAVI


#include <freewill.h>
#include <fwrender.h>
#include <fwaction.h>

class CFreeWillView : public CView
{
protected: // create from serialization only
	CFreeWillView();
	DECLARE_DYNCREATE(CFreeWillView)

// Attributes
public:
	CFreeWillDoc *GetDocument() const;

	IFWDevice *m_pFWDevice;	// FreeWill Device
	IRenderer *m_pRenderer;			// The Renderer
	IScene *m_pScene;				// The Scene
	IBody *m_pBody;
	IAction *m_pActionTick;			// The Clock Tick Action...

	// Direct Manipulation Parameters...
	UINT m_nManipMode;						// direct manipulation mode
	FWFLOAT m_x, m_y;							// direct manipulation params
	FWFLOAT m_x0, m_y0;						// direct manipulation initial params
	FWFLOAT m_dx, m_dy;						// factors for dx, dy: may force horizontal/vertical only modes
	ITransform *m_pAxisT, *m_pAxisInvT;		// Transforms for various Axis Modes

// Operations
public:

// Overrides
	public:
	virtual void OnDraw(CDC *pDC);  // overridden to draw this view
virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CFreeWillView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
	afx_msg void OnSize(UINT nType, int cx, int cy);
//	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
protected:
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
public:
	enum HITVAL { HIT_IN, HIT_H, HIT_V, HIT_OUT };
	afx_msg void OnUpdateIndicatorTest(CCmdUI *pCmdUI);
	afx_msg void OnUpdateIndicatorOp(CCmdUI *pCmdUI);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	enum HITVAL HitTest(CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnTest1();
	afx_msg void OnTest2();
	afx_msg void OnTest3();
	afx_msg void OnTest4();
	afx_msg void OnTest5();
	afx_msg void OnTest6();
	afx_msg void OnTest7();
	afx_msg void OnTest8();
	afx_msg void OnTestSetA();
	afx_msg void OnUpdateTestSetA(CCmdUI *pCmdUI);
	afx_msg void OnTestSetB();
	afx_msg void OnUpdateTestSetB(CCmdUI *pCmdUI);
	afx_msg void OnTestSetC();
	afx_msg void OnUpdateTestSetC(CCmdUI *pCmdUI);
	afx_msg void OnTestSetD();
	afx_msg void OnUpdateTestSetD(CCmdUI *pCmdUI);
	afx_msg void OnTestX();
	afx_msg void OnTestY();
	afx_msg void OnTestZ();

	// TEST & DEMO FUNCTIONS
	// Common
	FWULONG m_nTestSet;
	BOOL m_bStoreAVI;
	void OnTestPrepare(LPCTSTR fname, bool bResetBones = true);
	void SaveAVI(LPCTSTR fname);
	// Set A
	void OnTestA1();
	void OnTestA2();
	void OnTestA3();
	void OnTestA4();
	void OnTestA5();
	void OnTestA6();
	void OnTestA7();
	void OnTestA8();
	// Set B
	void OnTestB1();
	void OnTestB2();
	void OnTestB3();
	void OnTestB4();
	void OnTestB5();
	void OnTestB6();
	void OnTestB7();
	void OnTestB8();
	// Set C
	void OnTestC1();
	void OnTestC2();
	void OnTestC3();
	void OnTestC4();
	void OnTestC5();
	void OnTestC6();
	void OnTestC7();
	void OnTestC8();
	// Set D
	void OnTestD1();
	void OnTestD2();
	void OnTestD3();
	void OnTestD4();
	void OnTestD5();
	void OnTestD6();
	void OnTestD7();
	void OnTestD8();
	afx_msg void OnFileSaveStill();
	afx_msg void OnTestStoremode();
	afx_msg void OnUpdateTestStoremode(CCmdUI *pCmdUI);
	afx_msg void OnTimer(UINT nIDEvent);

	// Helpers for the Lobby Scene
	IKineNode *Reproduce(LPOLESTR pLabel);
	void MoveNode(IKineChild *pNode, FWFLOAT x, FWFLOAT y, FWFLOAT z);
	void RotNodeX(IKineChild *pNode, FWFLOAT rot);
	void RotNodeY(IKineChild *pNode, FWFLOAT rot);
	void RotNodeZ(IKineChild *pNode, FWFLOAT rot);
	FWULONG Walk(LPOLESTR pLabel, FWULONG time, FWFLOAT rt1, FWFLOAT fd, FWFLOAT rt2);
	afx_msg void OnViewReset();
	afx_msg void OnViewFullscreen();
};

#ifndef _DEBUG  // debug version in FreeWillView.cpp
inline CFreeWillDoc *CFreeWillView::GetDocument() const
   { return reinterpret_cast<CFreeWillDoc*>(m_pDocument); }
#endif

