// ChildView.h : interface of the CChildView class
//

#pragma once

#include <freewill.h>	// obligatory
#include <fwrender.h>	// to start the renderer
#include <fwaction.h>	// actions

class CCamera;

// CChildView window

class CChildView : public CWnd
{
	// FreeWill Objects
	IFWDevice *m_pFWDevice;					// FreeWill Device
	IRenderer *m_pRenderer;					// The Renderer
	IScene *m_pScene;						// The Scene
	ISceneLightDir *m_pLight1;				// light 1
	ISceneLightDir *m_pLight2;				// light 2
	IAction *m_pActionTick;					// The Clock Tick Action...
	CCamera *m_pCamera;						// camera
	bool m_bFWDone;							// true if FreeWill fully initialised...

	CPoint m_ptDrag;						// mouse initial position while dragging

	// Characters
	int m_nCurBody;							// index to the current character
	IBody *m_pBody[2];						// character bodies
	IAction *m_pWalkAction[2];				// character walking actions
	int m_nWalkState[2];					// current walking action - simply the key code W, A, S, D or X; 0 if none

// Construction
public:
	CChildView();

// Attributes
public:

// Operations
public:

// Overrides
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CChildView();

	// Generated message map functions
protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnActionsAction1();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnActionsAction2();
	afx_msg void OnActionsAction3();
	afx_msg void OnActionsAction4();
	afx_msg void OnActionsAction5();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnActionsAction6();
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};

