// ChildView.h : interface of the CChildView class
//

#pragma once

#include <freewill.h>	// obligatory
#include <fwrender.h>	// to start the renderer
#include <fwaction.h>	// actions

// CChildView window

class CChildView : public CWnd
{
	// FreeWill Objects
	IFWDevice *m_pFWDevice;					// FreeWill Device
	IRenderer *m_pRenderer;					// The Renderer
	IScene *m_pScene;						// The Scene
	IBody *m_pBody;							// The Body
	ISceneLightDir *m_pLight1;				// light 1
	ISceneLightDir *m_pLight2;				// light 2
	IAction *m_pActionTick;					// The Clock Tick Action...
	bool m_bFWDone;

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
};

