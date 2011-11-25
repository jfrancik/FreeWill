// scene.h
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(__SCENE_H)
#define __SCENE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\fwlib\factory.h"
#include "..\fwlib\fwunknown.h"
#include "sceneplus.h"
#include "kinetemplate.h"

#include <list>

////////////////////////////////////////////////////////////////////////////////////////////////////
// CScene

typedef CTKineNode<FWUNKNOWN<
			IScene, 
			IID_IScene, IScene,
			IID_ISceneRenderee, ISceneRenderee,
			IID_IKineNode, IKineNode,
			IID_IKineChild, IKineChild,
			IID_IKineObj3D, IKineObj3D> > SCENEBASE;

class CScene : SCENEBASE
{
	// IKineNode
	// CreateChild creates a new SceneObject
	virtual HRESULT _stdcall CreateChild(LPOLESTR pLabel, /*[out, retval]*/ IKineNode **p);
	// AddChildEx / DelChildAt ensure updating m_displist
	virtual HRESULT _stdcall DelChildAt(FWULONG nId);
	virtual HRESULT _stdcall AddChildEx(LPOLESTR pLabel, IKineChild *p, BOOL bSetParentalDep, IKineNode **pParentNode, FWULONG *pnId);


	// Properties
	virtual HRESULT _stdcall GetRenderer(IRndrGeneric**);
	virtual HRESULT _stdcall PutRenderer(IRndrGeneric*);

	// Facility Functions
	virtual HRESULT _stdcall NewObject(FWSTRING pLabel, /*[out, retval]*/ ISceneObject**);
	virtual HRESULT _stdcall AddObject(FWSTRING pLabel, ISceneObject*);
	virtual HRESULT _stdcall GetCurrentCamera(ISceneCamera **p);
	virtual HRESULT _stdcall PutCamera(ISceneCamera *p);

	// rendering  (ISceneRenderee)
	virtual HRESULT _stdcall IsReady()					{ return (m_pRenderer) ? S_OK : S_FALSE; }		
	virtual HRESULT _stdcall IsVisible()				{ return m_bOn ? S_OK : S_FALSE; }		
	virtual HRESULT _stdcall PutVisible(BOOL bOn)		{ m_bOn = bOn; return S_OK; }
	virtual HRESULT _stdcall PutRenderHint(BOOL b)		{ return S_FALSE; }
	virtual HRESULT _stdcall NeedsRendering(BOOL bReset){ return (m_bOn && m_pRenderer) ? S_OK : S_FALSE; }
	virtual HRESULT _stdcall GetRenderOrdinal(FWULONG *p)			{ if (p) *p = 0 + m_nOrdinalOffset; return S_OK; }	
	virtual HRESULT _stdcall PutRenderOrdinalOffset(FWULONG offs)	{ m_nOrdinalOffset = offs; return S_OK; }
	virtual HRESULT _stdcall Render(IRndrGeneric*);
	virtual HRESULT _stdcall Turnoff(IRndrGeneric*);

	virtual HRESULT _stdcall SortDisplayList();

	DECLARE_FACTORY_CLASS(Scene, Scene)
	FW_RTTI(Scene)

protected:

	// properties
	BOOL m_bOn;
	IRndrGeneric *m_pRenderer;

	FWULONG m_nOrdinalOffset;			// set by PutRenderOrdinalOffset, used by GetRenderOrdinal

	// the display list
	typedef std::list<ISceneRenderee*> DISPLIST;
	DISPLIST m_displist;

	// the current camera - for GetCurrentCamera
	ISceneCamera *m_pCamera;

public:
	CScene();
	~CScene();
};

#endif
