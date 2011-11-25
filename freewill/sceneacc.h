// sceneacc.h
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(__SCENEACC_H)
#define __SCENEACCOBJ_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\fwlib\factory.h"
#include "..\fwlib\fwunknown.h"
#include "sceneplus.h"
#include "kinetemplate.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// CSceneTarget - auxiliary (internal, not exported) class for target objects
// sets external BOOL flag each time is invalidated.

typedef CTKineNode<FWUNKNOWN<
			IKineNode,
			IID_IKineChild, IKineChild,
			IID_IKineNode, IKineNode,
			IID_IKineObj3D, IKineObj3D> > TARGETBASECLASS;

class CAuxSceneTarget : public TARGETBASECLASS
{
public:
	// IKineNode implementation 
	virtual HRESULT __stdcall CreateChild(LPOLESTR pLabel, /*[out, retval]*/ IKineNode **p) { return ERROR(E_NOTIMPL); }
	virtual HRESULT __stdcall Invalidate() { if (m_pFlag) *m_pFlag = TRUE; return TARGETBASECLASS::Invalidate(); }

	FW_RTTI(AuxSceneTarget)

public:
	CAuxSceneTarget(BOOL *pFlag = NULL) : m_pFlag(pFlag)	{ }
	~CAuxSceneTarget()	{ }
private:
	BOOL *m_pFlag;	// external Dirty flag, set when object invalidated
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// CSceneLightPoint

typedef CTKineNode<FWUNKNOWN<
			ISceneLightPoint,
			IID_ISceneLightPoint, ISceneLightPoint,
			IID_ISceneLight, ISceneLight,
			IID_ISceneRenderee, ISceneRenderee,
			IID_IKineNode, IKineNode,
			IID_IKineChild, IKineChild,
			IID_IKineObj3D, IKineObj3D> > LIGHTPOINTBASECLASS;

class CSceneLightPoint : LIGHTPOINTBASECLASS
{
	// ISceneLightPoint
	virtual HRESULT _stdcall Create(FWVECTOR vPosition, FWFLOAT fRange, FWFLOAT fAtten0, FWFLOAT fAtten1, FWFLOAT fAtten2);
	virtual HRESULT _stdcall GetCreateParams(FWVECTOR *vPosition, FWFLOAT *fRange);

	// ISceneLight
	virtual HRESULT _stdcall GetDiffuseColor(FWCOLOR *p)	{ if (p) memcpy(p, &m_colorDiffuse, sizeof(FWCOLOR)); return S_OK; }
	virtual HRESULT _stdcall PutDiffuseColor(FWCOLOR c)	{ memcpy(&m_colorDiffuse, &c, sizeof(FWCOLOR)); return S_OK; }
	virtual HRESULT _stdcall GetSpecularColor(FWCOLOR *p)	{ if (p) memcpy(p, &m_colorSpecular, sizeof(FWCOLOR)); return S_OK; }
	virtual HRESULT _stdcall PutSpecularColor(FWCOLOR c)	{ memcpy(&m_colorSpecular, &c, sizeof(FWCOLOR)); return S_OK; }
	virtual HRESULT _stdcall GetAmbientColor(FWCOLOR *p)	{ if (p) memcpy(p, &m_colorAmbient, sizeof(FWCOLOR)); return S_OK; }
	virtual HRESULT _stdcall PutAmbientColor(FWCOLOR c)	{ memcpy(&m_colorAmbient, &c, sizeof(FWCOLOR)); return S_OK; }
	
	// rendering (ISceneRenderee)
	virtual HRESULT _stdcall IsReady()					{ return m_bReady ? S_OK : S_FALSE; }
	virtual HRESULT _stdcall IsVisible()				{ return m_bOn ? S_OK : S_FALSE; }
	virtual HRESULT _stdcall PutVisible(BOOL bOn)		{ m_bOn = bOn; m_bDirty = TRUE; return S_OK; }
	virtual HRESULT _stdcall PutRenderHint(BOOL b)		{ m_bDirty = b; return S_OK; }
	virtual HRESULT _stdcall NeedsRendering(BOOL bReset){ return (m_bReady && (m_bDirty || bReset)) ? S_OK : S_FALSE; }
	virtual HRESULT _stdcall GetRenderOrdinal(FWULONG *p)			{ if (p) *p = 20 + m_nOrdinalOffset; return S_OK; }	
	virtual HRESULT _stdcall PutRenderOrdinalOffset(FWULONG offs)	{ m_nOrdinalOffset = offs; return S_OK; }
	virtual HRESULT _stdcall Render(IRndrGeneric*);
	virtual HRESULT _stdcall Turnoff(IRndrGeneric*);

	// IKineNode/IKineObj3D
	virtual HRESULT _stdcall CreateChild(LPOLESTR pLabel, /*[out, retval]*/ IKineNode **p);
	virtual HRESULT _stdcall Invalidate()	{ m_bDirty = TRUE; return LIGHTPOINTBASECLASS::Invalidate(); }

	DECLARE_FACTORY_CLASS(SceneLightPoint, SceneLightPoint)
	FW_RTTI(SceneLightPoint)

protected:
	FWULONG m_nIndex;					// light index
	BOOL m_bReady, m_bOn, m_bDirty;		// flags
	FWCOLOR m_colorDiffuse, m_colorSpecular, m_colorAmbient;
	FWFLOAT m_fRange;
	FWFLOAT m_fAtten0, m_fAtten1, m_fAtten2;
	FWULONG m_nOrdinalOffset;			// set by PutRenderOrdinalOffset, used by GetRenderOrdinal

public:
	CSceneLightPoint();
	~CSceneLightPoint();
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// CSceneLightDir

typedef CTKineTargetedObject<FWUNKNOWN<
			BASECLASS<ISceneLightDir, IKineTargetedObj>,
			IID_ISceneLightDir, ISceneLightDir,
			IID_ISceneLight, ISceneLight,
			IID_ISceneRenderee, ISceneRenderee,
			IID_IKineTargetedObj, IKineTargetedObj,
			IID_IKineNode, IKineNode,
			IID_IKineChild, IKineChild,
			IID_IKineObj3D, IKineObj3D> > LIGHTDIRBASECLASS;

class CSceneLightDir : LIGHTDIRBASECLASS
{
	// ISceneLightDir
	virtual HRESULT _stdcall Create(FWVECTOR vDirection);
	virtual HRESULT _stdcall CreateEx(FWVECTOR vPosition, FWVECTOR vTarget);
	virtual HRESULT _stdcall GetCreateParams(FWVECTOR *vDirection);

	// ISceneLight
	virtual HRESULT _stdcall GetDiffuseColor(FWCOLOR *p)	{ if (p) memcpy(p, &m_colorDiffuse, sizeof(FWCOLOR)); return S_OK; }
	virtual HRESULT _stdcall PutDiffuseColor(FWCOLOR c)	{ memcpy(&m_colorDiffuse, &c, sizeof(FWCOLOR)); return S_OK; }
	virtual HRESULT _stdcall GetSpecularColor(FWCOLOR *p)	{ if (p) memcpy(p, &m_colorSpecular, sizeof(FWCOLOR)); return S_OK; }
	virtual HRESULT _stdcall PutSpecularColor(FWCOLOR c)	{ memcpy(&m_colorSpecular, &c, sizeof(FWCOLOR)); return S_OK; }
	virtual HRESULT _stdcall GetAmbientColor(FWCOLOR *p)	{ if (p) memcpy(p, &m_colorAmbient, sizeof(FWCOLOR)); return S_OK; }
	virtual HRESULT _stdcall PutAmbientColor(FWCOLOR c)	{ memcpy(&m_colorAmbient, &c, sizeof(FWCOLOR)); return S_OK; }
	
	// rendering (ISceneRenderee)
	virtual HRESULT _stdcall IsReady()					{ return m_bReady ? S_OK : S_FALSE; }
	virtual HRESULT _stdcall IsVisible()				{ return m_bOn ? S_OK : S_FALSE; }
	virtual HRESULT _stdcall PutVisible(BOOL bOn)		{ m_bOn = bOn; m_bDirty = TRUE; return S_OK; }
	virtual HRESULT _stdcall PutRenderHint(BOOL b)		{ m_bDirty = b; return S_OK; }
	virtual HRESULT _stdcall NeedsRendering(BOOL bReset){ return (m_bReady && (m_bDirty || bReset)) ? S_OK : S_FALSE; }
	virtual HRESULT _stdcall GetRenderOrdinal(FWULONG *p)			{ if (p) *p = 20 + m_nOrdinalOffset; return S_OK; }	
	virtual HRESULT _stdcall PutRenderOrdinalOffset(FWULONG offs)	{ m_nOrdinalOffset = offs; return S_OK; }
	virtual HRESULT _stdcall Render(IRndrGeneric*);
	virtual HRESULT _stdcall Turnoff(IRndrGeneric*);

	// IKineNode/IKineObj3D/IKineTargetedObj
	virtual HRESULT _stdcall CreateChild(LPOLESTR pLabel, /*[out, retval]*/ IKineNode **p);
	virtual HRESULT _stdcall Invalidate()	{ m_bDirty = TRUE; return LIGHTDIRBASECLASS::Invalidate(); }
	virtual HRESULT CreatePairNode(IKineNode **p)	{ *p = new CAuxSceneTarget(&m_bDirty); FWDevice()->RegisterObject(*p); return S_OK; }

	DECLARE_FACTORY_CLASS(SceneLightDir, SceneLightDir)
	FW_RTTI(SceneLightDir)

protected:
	FWULONG m_nIndex;					// light index
	BOOL m_bReady, m_bOn, m_bDirty;		// flags
	FWCOLOR m_colorDiffuse, m_colorSpecular, m_colorAmbient;
	FWULONG m_nOrdinalOffset;			// set by PutRenderOrdinalOffset, used by GetRenderOrdinal

public:
	CSceneLightDir();
	~CSceneLightDir();
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// CSceneLightSpot

typedef CTKineTargetedObject<FWUNKNOWN<
			BASECLASS<ISceneLightSpot, IKineTargetedObj>,
			IID_ISceneLightSpot, ISceneLightSpot,
			IID_ISceneLight, ISceneLight,
			IID_ISceneRenderee, ISceneRenderee,
			IID_IKineTargetedObj, IKineTargetedObj,
			IID_IKineNode, IKineNode,
			IID_IKineChild, IKineChild,
			IID_IKineObj3D, IKineObj3D> > LIGHTSPOTBASECLASS;

class CSceneLightSpot : LIGHTSPOTBASECLASS
{
	// ISceneLightSpot
	virtual HRESULT _stdcall Create(FWVECTOR vPosition, FWVECTOR vDirection, FWFLOAT fTheta, FWFLOAT fPhi, FWFLOAT fFalloff);
	virtual HRESULT _stdcall CreateEx(FWVECTOR vPosition, FWVECTOR vTarget, FWFLOAT fTheta, FWFLOAT fPhi, FWFLOAT fFalloff);
	virtual HRESULT _stdcall GetCreateParams(FWVECTOR *vPosition, FWVECTOR *vDirection, FWFLOAT *fTheta, FWFLOAT *fPhi, FWFLOAT *fFalloff);

	// ISceneLight
	virtual HRESULT _stdcall GetDiffuseColor(FWCOLOR *p)	{ if (p) memcpy(p, &m_colorDiffuse, sizeof(FWCOLOR)); return S_OK; }
	virtual HRESULT _stdcall PutDiffuseColor(FWCOLOR c)	{ memcpy(&m_colorDiffuse, &c, sizeof(FWCOLOR)); return S_OK; }
	virtual HRESULT _stdcall GetSpecularColor(FWCOLOR *p)	{ if (p) memcpy(p, &m_colorSpecular, sizeof(FWCOLOR)); return S_OK; }
	virtual HRESULT _stdcall PutSpecularColor(FWCOLOR c)	{ memcpy(&m_colorSpecular, &c, sizeof(FWCOLOR)); return S_OK; }
	virtual HRESULT _stdcall GetAmbientColor(FWCOLOR *p)	{ if (p) memcpy(p, &m_colorAmbient, sizeof(FWCOLOR)); return S_OK; }
	virtual HRESULT _stdcall PutAmbientColor(FWCOLOR c)	{ memcpy(&m_colorAmbient, &c, sizeof(FWCOLOR)); return S_OK; }
	
	// rendering (ISceneRenderee)
	virtual HRESULT _stdcall IsReady()					{ return m_bReady ? S_OK : S_FALSE; }
	virtual HRESULT _stdcall IsVisible()				{ return m_bOn ? S_OK : S_FALSE; }
	virtual HRESULT _stdcall PutVisible(BOOL bOn)		{ m_bOn = bOn; m_bDirty = TRUE; return S_OK; }
	virtual HRESULT _stdcall PutRenderHint(BOOL b)		{ m_bDirty = b; return S_OK; }
	virtual HRESULT _stdcall NeedsRendering(BOOL bReset){ return (m_bReady && (m_bDirty || bReset)) ? S_OK : S_FALSE; }
	virtual HRESULT _stdcall GetRenderOrdinal(FWULONG *p)			{ if (p) *p = 20 + m_nOrdinalOffset; return S_OK; }	
	virtual HRESULT _stdcall PutRenderOrdinalOffset(FWULONG offs)	{ m_nOrdinalOffset = offs; return S_OK; }
	virtual HRESULT _stdcall Render(IRndrGeneric*);
	virtual HRESULT _stdcall Turnoff(IRndrGeneric*);

	// IKineNode/IKineObj3D/IKineTargetedObj
	virtual HRESULT _stdcall CreateChild(LPOLESTR pLabel, /*[out, retval]*/ IKineNode **p);
	virtual HRESULT _stdcall Invalidate()	{ m_bDirty = TRUE; return LIGHTSPOTBASECLASS::Invalidate(); }
	virtual HRESULT CreatePairNode(IKineNode **p)	{ *p = new CAuxSceneTarget(&m_bDirty); FWDevice()->RegisterObject(*p); return S_OK; }

	DECLARE_FACTORY_CLASS(SceneLightSpot, SceneLightSpot)
	FW_RTTI(SceneLightSpot)

protected:
	FWULONG m_nIndex;					// light index
	BOOL m_bReady, m_bOn, m_bDirty;		// flags
	FWCOLOR m_colorDiffuse, m_colorSpecular, m_colorAmbient;
	FWFLOAT m_fTheta, m_fPhi, m_fFalloff;
	FWULONG m_nOrdinalOffset;			// set by PutRenderOrdinalOffset, used by GetRenderOrdinal

public:
	CSceneLightSpot();
	~CSceneLightSpot();
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// CSceneCamera

typedef CTKineTargetedObject<FWUNKNOWN<
			BASECLASS<ISceneCamera, IKineTargetedObj>,
			IID_ISceneCamera, ISceneCamera,
			IID_ISceneRenderee, ISceneRenderee,
			IID_IKineTargetedObj, IKineTargetedObj,
			IID_IKineNode, IKineNode,
			IID_IKineChild, IKineChild,
			IID_IKineObj3D, IKineObj3D> > CAMERABASECLASS;

class CSceneCamera : CAMERABASECLASS
{
	// ISceneCamera
	virtual HRESULT _stdcall Create(FWVECTOR vPosition, FWVECTOR vDirection, FWVECTOR vUp);
	virtual HRESULT _stdcall CreateEx(FWVECTOR vPosition, FWVECTOR vTarget, FWVECTOR vUp);
	virtual HRESULT _stdcall GetCreateParams(FWVECTOR *vPosition, FWVECTOR *vDirection, FWVECTOR *vUp);
	virtual HRESULT _stdcall PutPerspective(FWFLOAT fFOV, FWFLOAT fClipNear, FWFLOAT fClipFar, FWFLOAT fAspect);
	virtual HRESULT _stdcall GetPerspective(FWFLOAT *pFOV, FWFLOAT *pfClipNear, FWFLOAT *pfClipFar, FWFLOAT *pAspect);
	virtual HRESULT _stdcall GetCurrentCamera(ISceneCamera **p)	{ if (p) { *p = c_CurCamera; if (c_CurCamera) c_CurCamera->AddRef(); } return S_OK; }

	// rendering (ISceneRenderee)
	virtual HRESULT _stdcall IsReady()					{ return m_bReady ? S_OK : S_FALSE; }
	virtual HRESULT _stdcall IsVisible()				{ if (!c_CurCamera) PutVisible(TRUE); return c_CurCamera == this ? S_OK : S_FALSE; }
	virtual HRESULT _stdcall PutVisible(BOOL bOn)		{ if (!bOn) return S_FALSE; c_CurCamera = this; return S_OK; }
	virtual HRESULT _stdcall PutRenderHint(BOOL b)		{ m_bDirty = b; return S_OK; }
	virtual HRESULT _stdcall NeedsRendering(BOOL bReset){ return (m_bReady && IsVisible() == S_OK && (m_bDirty || bReset)) ? S_OK : S_FALSE; }
	virtual HRESULT _stdcall GetRenderOrdinal(FWULONG *p)			{ if (p) *p = 20 + m_nOrdinalOffset; return S_OK; }	
	virtual HRESULT _stdcall PutRenderOrdinalOffset(FWULONG offs)	{ m_nOrdinalOffset = offs; return S_OK; }
	virtual HRESULT _stdcall Render(IRndrGeneric*);
	virtual HRESULT _stdcall Turnoff(IRndrGeneric*);

	// IKineNode/IKineObj3D/IKineTargetedObj
	virtual HRESULT _stdcall CreateChild(LPOLESTR pLabel, /*[out, retval]*/ IKineNode **p);
	virtual HRESULT _stdcall Invalidate()	{ m_bDirty = TRUE; return CAMERABASECLASS::Invalidate(); }
	virtual HRESULT CreatePairNode(IKineNode **p)	{ *p = new CAuxSceneTarget(&m_bDirty); FWDevice()->RegisterObject(*p); return S_OK; }
	virtual HRESULT CreateUpNode(IKineNode **p)		{ *p = new CAuxSceneTarget(&m_bDirty); FWDevice()->RegisterObject(*p); return S_OK; }

	DECLARE_FACTORY_CLASS(SceneCamera, SceneCamera)
	FW_RTTI(SceneCamera)

protected:
	static ISceneCamera *c_CurCamera;	// the only active camera instance
	BOOL m_bReady, m_bDirty;			// flags
	BOOL m_bPerspective;				// flag for the perspective camera; affine otherwise
	FWFLOAT m_fFOV, m_fClipNear, m_fClipFar, m_fAspect;	// perspective params
	FWULONG m_nOrdinalOffset;			// set by PutRenderOrdinalOffset, used by GetRenderOrdinal

public:
	CSceneCamera();
	~CSceneCamera();
};

#endif
