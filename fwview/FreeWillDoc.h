// FreeWillDoc.h : interface of the CFreeWillDoc class
//
#pragma once

#include <freewill.h>
#include <fwrender.h>

#define RELOAD_SCENE_STRUCT		1089


class CFreeWillDoc : public CDocument
{
protected: // create from serialization only
	CFreeWillDoc();
	DECLARE_DYNCREATE(CFreeWillDoc)

// Attributes
private:
	IFWDevice *m_pFWDevice;	// FreeWill Device
	IRenderer *m_pRenderer;			// The Renderer
	IScene *m_pScene;				// The Scene
	IBody *m_pBody;					// The Body

	BYTE *m_pSceneBuf;				// Data buffer for Store/RetrieveState functions
	FWULONG m_nSceneBufCount;

	IKineNode *m_pCurrent;			// The node currently selected
	FWSTRING m_poleLabel;				// The name of the node currently selected (IKineChild::GetLabel)

	UINT m_nManipMode;				// The manipulation mode: ID_MANIPULATE_POINT, ID_MANIPULATE_MOVE or ID_MANIPULATE_ROTATE
	UINT m_nManipAxisMode;			// The manipulation axis mode: ID_MANIPULATE_LOCAL, ID_MANIPULATE_VIEW or ID_MANIPULATE_ARBITRARY
	IKineChild *m_pManipAxis;		// The manipulation arbitrary axis object
	FWSTRING m_poleManipAxis;			// The manipulation arbitrary axis object label

public:
	void GetFreeWillObjects(IFWDevice **ppFWDevice, IRenderer **ppRenderer, IScene **ppScene, IBody **ppBody);
	void SetFreeWillObjects(IFWDevice *pFWDevice, IRenderer *pRenderer, IScene *pScene, IBody *pBody);

	IKineNode *GetCurrentNode();
	void SetCurrentNode(IKineNode *p);

	UINT GetManipulationMode()			{ return m_nManipMode; }
	void SetManipulationMode(UINT n)	{ m_nManipMode = n; }
	UINT GetManipAxisMode()				{ return m_nManipAxisMode; }
	void SetManipAxisMode(UINT n)		{ m_nManipAxisMode = n; }
	void GetManipArbitraryAxis(IKineChild **p);
	void SetManipArbitraryAxis(IKineChild *p);	

// Operations
public:

	void LoadFromFile(LPCTSTR fname);
	void StoreSceneState();
	void ResetSceneState();

// Overrides
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CFreeWillDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPlayPlay();
	afx_msg void OnUpdatePlayPlay(CCmdUI *pCmdUI);
	afx_msg void OnPlayPause();
	afx_msg void OnUpdatePlayPause(CCmdUI *pCmdUI);
	afx_msg void OnPlayStop();
	afx_msg void OnUpdatePlayStop(CCmdUI *pCmdUI);
	afx_msg void OnPlayRewind();
	afx_msg void OnUpdatePlayRewind(CCmdUI *pCmdUI);
	afx_msg void OnPlayForward();
	afx_msg void OnUpdatePlayForward(CCmdUI *pCmdUI);
	afx_msg void OnManipulateMove();
	afx_msg void OnUpdateManipulateMove(CCmdUI *pCmdUI);
	afx_msg void OnManipulateRotate();
	afx_msg void OnUpdateManipulateRotate(CCmdUI *pCmdUI);
	afx_msg void OnManipulateReset();
	afx_msg void OnUpdateManipulateReset(CCmdUI *pCmdUI);
	afx_msg void OnManipulateResetAll();
	afx_msg void OnUpdateIndicatorPlaytime(CCmdUI *pCmdUI);
	afx_msg void OnUpdateIndicatorName(CCmdUI *pCmdUI);
	afx_msg void OnManipulatePoint();
	afx_msg void OnUpdateManipulatePoint(CCmdUI *pCmdUI);
	afx_msg void OnManipulateLocal();
	afx_msg void OnUpdateManipulateLocal(CCmdUI *pCmdUI);
	afx_msg void OnManipulateView();
	afx_msg void OnUpdateManipulateView(CCmdUI *pCmdUI);
	afx_msg void OnManipulateArbitrary();
	afx_msg void OnUpdateManipulateArbitrary(CCmdUI *pCmdUI);
	afx_msg void OnManipulateSetArbitrary();
	afx_msg void OnUpdateManipulateSetArbitrary(CCmdUI *pCmdUI);
	afx_msg void OnAccelaration2xfaster();
	afx_msg void OnUpdateAccelaration2xfaster(CCmdUI *pCmdUI);
	afx_msg void OnAccelaration2xslower();
	afx_msg void OnUpdateAccelaration2xslower(CCmdUI *pCmdUI);
	afx_msg void OnAccelaration4xfaster();
	afx_msg void OnUpdateAccelaration4xfaster(CCmdUI *pCmdUI);
	afx_msg void OnAccelaration4xslower();
	afx_msg void OnUpdateAccelaration4xslower(CCmdUI *pCmdUI);
	afx_msg void OnAccelarationNormal();
	afx_msg void OnUpdateAccelarationNormal(CCmdUI *pCmdUI);
};


