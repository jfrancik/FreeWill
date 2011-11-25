// sceneobj.h
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(__SCENEOBJ_H)
#define __SCENEOBJ_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\fwlib\factory.h"
#include "..\fwlib\fwunknown.h"
#include "sceneplus.h"
#include "kinetemplate.h"
#include <vector>

////////////////////////////////////////////////////////////////////////////////////////////////////
// CSceneObject

typedef CTKineNode<FWUNKNOWN<
									ISceneObject,
										IID_ISceneObject, ISceneObject,
										IID_ISceneRenderee, ISceneRenderee,
										IID_IKineNode, IKineNode,
										IID_IKineChild, IKineChild,
										IID_IKineObj3D, IKineObj3D>
										> __SCENEOBJECT_BASECLASS;

class CSceneObject : __SCENEOBJECT_BASECLASS
{
	// IKineObj3D
	virtual HRESULT _stdcall Reproduce(/*[out, retval]*/ IKineObj3D **p);

	// IKineNode
	virtual HRESULT _stdcall CreateChild(LPOLESTR pLabel, /*[out, retval]*/ IKineNode **p);

	// properties
	virtual HRESULT _stdcall GetRenderer(IRndrGeneric**);
	virtual HRESULT _stdcall PutRenderer(IRndrGeneric*);
	virtual HRESULT _stdcall GetDictionary(IMeshDictionary**);
	virtual HRESULT _stdcall PutDictionary(IMeshDictionary*);

	// Meterial Functions
	virtual HRESULT _stdcall GetMaterial(/*[out, retval]*/ IMaterial**);
	virtual HRESULT _stdcall PutMaterial(IMaterial*, BOOL bOverwrite);

	// Mesh List
	virtual HRESULT _stdcall NewMesh(FWSTRING pLabel, /*[out, retval]*/ IMesh**);
	virtual HRESULT _stdcall AddMesh(FWSTRING pLabel, IMesh*);

	// rendering (ISceneRenderee)
	virtual HRESULT _stdcall IsReady()					{ return (m_pRenderer) ? S_OK : S_FALSE; }		
	virtual HRESULT _stdcall IsVisible()				{ return m_bOn ? S_OK : S_FALSE; }
	virtual HRESULT _stdcall PutVisible(BOOL bOn)		{ m_bOn = bOn; return S_OK; }
	virtual HRESULT _stdcall PutRenderHint(BOOL b)		{ return S_FALSE; }
	virtual HRESULT _stdcall NeedsRendering(BOOL bReset){ return (m_bOn && m_pRenderer) ? S_OK : S_FALSE; }
	virtual HRESULT _stdcall GetRenderOrdinal(FWULONG *p)			{ if (p) *p = 200 + m_nOrdinalOffset; return S_OK; }	
	virtual HRESULT _stdcall PutRenderOrdinalOffset(FWULONG offs)	{ m_nOrdinalOffset = offs; return S_OK; }
	virtual HRESULT _stdcall Render(IRndrGeneric*);
	virtual HRESULT _stdcall Turnoff(IRndrGeneric*);

	DECLARE_FACTORY_CLASS(SceneObject, SceneObject)
	FW_RTTI(SceneObject)

private:
	void SetRenderInternalData();
	void ResetRenderInternalData();

protected:

	// properties
	BOOL m_bOn;

	IRndrGeneric *m_pRenderer;
	IMeshDictionary *m_pDictionary;

	IMaterial *m_pMaterial;
	bool m_bOverwriteMat;

	FWULONG m_nOrdinalOffset;			// set by PutRenderOrdinalOffset, used by GetRenderOrdinal

	// auxiliary rendering data
	std::vector<IMesh*>m_vecMeshes;		// vector of meshes
	FWULONG m_nBones;					// count of bones
	IKineChild **m_pBones;				// bones
	FWMATRIX *m_pFrameTransforms;		// the transforms

public:
	CSceneObject();
	~CSceneObject();
};

#endif
