// meshedobj.cpp : Defines the Meshed Object
//

#include "stdafx.h"
#include "sceneacc.h"

// Global Light Index

static FWULONG g_nIndex = 0;

///////////////////////////////////////////////////////////
// Class CSceneLightPoint

CSceneLightPoint::CSceneLightPoint() : m_bReady(FALSE), m_bOn(TRUE), m_bDirty(TRUE), m_nOrdinalOffset(0)
{
	m_nIndex = g_nIndex++;
	m_colorDiffuse.r = 1.0f; m_colorDiffuse.g = 1.0f; m_colorDiffuse.b = 1.0f; m_colorDiffuse.a = 1.0f;
	m_colorSpecular.r = 0.0f; m_colorSpecular.g = 0.0f; m_colorSpecular.b = 0.0f; m_colorSpecular.a = 1.0f;
	m_colorAmbient.r = 0.0f; m_colorAmbient.g = 0.0f; m_colorAmbient.b = 0.0f; m_colorAmbient.a = 1.0f;
}

CSceneLightPoint::~CSceneLightPoint()
{
}

HRESULT CSceneLightPoint::Create(FWVECTOR vPosition, FWFLOAT fRange, FWFLOAT fAtten0, FWFLOAT fAtten1, FWFLOAT fAtten2)
{
	m_fRange = fRange;
	m_fAtten0 = fAtten0; m_fAtten1 = fAtten1; m_fAtten2 = fAtten2;
	FWVECTOR v = { 0.0f, 0.0f, 0.0f };
	LtoG(&v);
	v.x = vPosition.x - v.x; v.y = vPosition.y - v.y; v.z = vPosition.z - v.z;
	ITransform *pT = NULL;
	HRESULT h = FWDevice()->CreateObject(L"Transform", IID_ITransform, (IFWUnknown**)&pT); if (FAILED(h)) return h;
	pT->FromTranslationVector(&v);
	Transform(pT, KINE_BASE);
	pT->Release();
	m_bReady = TRUE;
	return S_OK;
}

HRESULT CSceneLightPoint::GetCreateParams(FWVECTOR *vPosition, FWFLOAT *fRange)
{
	if (vPosition)
	{
		vPosition->x = vPosition->y = vPosition->z = 0.0f;
		LtoG(vPosition);
	}
	if (fRange) *fRange = m_fRange;
	return S_OK;
}

HRESULT CSceneLightPoint::Render(IRndrGeneric *pRenderer)
{
	m_bDirty = FALSE;
	pRenderer->SetLight(m_nIndex, m_bOn);
	if (!m_bOn) return S_OK;
	FWVECTOR vPos;
	vPos.x = vPos.y = vPos.z = 0.0f;
	LtoG(&vPos);
	pRenderer->SetPointLight(m_nIndex, m_colorDiffuse, m_colorSpecular, m_colorAmbient, &vPos, m_fRange, m_fAtten0, m_fAtten1, m_fAtten2);
	return S_OK;
}

HRESULT CSceneLightPoint::Turnoff(IRndrGeneric *pRenderer)
{
	pRenderer->SetLight(m_nIndex, FALSE);
	return S_OK;
}

HRESULT __stdcall CSceneLightPoint::CreateChild(LPOLESTR pLabel, /*[out, retval]*/ IKineNode **p)
{
	if (!p) return ERROR(FW_E_POINTER);
	*p = NULL;
	FWDevice()->CreateObject(L"KineBone", IID_IKineNode, (IFWUnknown**)p);	
	if (!p) return ERROR(FW_E_OUTOFMEMORY, 0, NULL, NULL, 0);
	
	HRESULT h = AddChild(pLabel, *p);
	if (FAILED(h)) { (*p)->Release(); *p = NULL; return h; }

	return S_OK;
}

///////////////////////////////////////////////////////////
// Class CSceneLightDir

CSceneLightDir::CSceneLightDir() : m_bReady(FALSE), m_bOn(TRUE), m_bDirty(TRUE), m_nOrdinalOffset(0)
{ 
	m_nIndex = g_nIndex++;
	m_colorDiffuse.r = 1.0f; m_colorDiffuse.g = 1.0f; m_colorDiffuse.b = 1.0f; m_colorDiffuse.a = 1.0f;
	m_colorSpecular.r = 0.0f; m_colorSpecular.g = 0.0f; m_colorSpecular.b = 0.0f; m_colorSpecular.a = 1.0f;
	m_colorAmbient.r = 0.0f; m_colorAmbient.g = 0.0f; m_colorAmbient.b = 0.0f; m_colorAmbient.a = 1.0f;
}

CSceneLightDir::~CSceneLightDir()
{
}

HRESULT CSceneLightDir::Create(FWVECTOR vDirection)
{
	m_bReady = TRUE;
	PutDirection(vDirection);
	return S_OK;
}

HRESULT CSceneLightDir::CreateEx(FWVECTOR vPosition, FWVECTOR vTarget)
{
	PutPosition(vPosition);
	PutTarget(vTarget);
	m_bReady = TRUE;
	return S_OK;
}

HRESULT CSceneLightDir::GetCreateParams(FWVECTOR *vDirection)
{
	if (vDirection) GetDirection(vDirection);
	return S_OK;
}

HRESULT CSceneLightDir::Render(IRndrGeneric *pRenderer)
{
	m_bDirty = FALSE;
	pRenderer->SetLight(m_nIndex, m_bOn);
	if (!m_bOn) return S_OK;
	FWVECTOR vDir;
	GetDirection(&vDir);
	pRenderer->SetDirLight(m_nIndex, m_colorDiffuse, m_colorSpecular, m_colorAmbient, &vDir);
	return S_OK;
}

HRESULT CSceneLightDir::Turnoff(IRndrGeneric *pRenderer)
{
	pRenderer->SetLight(m_nIndex, FALSE);
	return S_OK;
}

HRESULT __stdcall CSceneLightDir::CreateChild(LPOLESTR pLabel, /*[out, retval]*/ IKineNode **p)
{
	if (!p) return ERROR(FW_E_POINTER);
	*p = NULL;
	FWDevice()->CreateObject(L"KineBone", IID_IKineNode, (IFWUnknown**)p);	
	if (!p) return ERROR(FW_E_OUTOFMEMORY, 0, NULL, NULL, 0);
	
	HRESULT h = AddChild(pLabel, *p);
	if (FAILED(h)) { (*p)->Release(); return h; }

	return S_OK;
}

///////////////////////////////////////////////////////////
// Class CSceneLightSpot

CSceneLightSpot::CSceneLightSpot() : m_bReady(FALSE), m_bOn(TRUE), m_bDirty(TRUE), m_nOrdinalOffset(0)
{ 
	m_nIndex = g_nIndex++;
	m_colorDiffuse.r = 1.0f; m_colorDiffuse.g = 1.0f; m_colorDiffuse.b = 1.0f; m_colorDiffuse.a = 1.0f;
	m_colorSpecular.r = 0.0f; m_colorSpecular.g = 0.0f; m_colorSpecular.b = 0.0f; m_colorSpecular.a = 1.0f;
	m_colorAmbient.r = 0.0f; m_colorAmbient.g = 0.0f; m_colorAmbient.b = 0.0f; m_colorAmbient.a = 1.0f;
}

CSceneLightSpot::~CSceneLightSpot()
{
}

HRESULT CSceneLightSpot::Create(FWVECTOR vPosition, FWVECTOR vDirection, FWFLOAT fTheta, FWFLOAT fPhi, FWFLOAT fFalloff)
{
	PutPosition(vPosition);
	PutDirection(vDirection);
	m_fTheta = fTheta;
	m_fPhi = fPhi;
	m_fFalloff = fFalloff;
	m_bReady = TRUE;
	return S_OK;
}

HRESULT CSceneLightSpot::CreateEx(FWVECTOR vPosition, FWVECTOR vTarget, FWFLOAT fTheta, FWFLOAT fPhi, FWFLOAT fFalloff)
{
	PutPosition(vPosition);
	PutTarget(vTarget);
	m_fTheta = fTheta;
	m_fPhi = fPhi;
	m_fFalloff = fFalloff;
	m_bReady = TRUE;
	return S_OK;
}

HRESULT CSceneLightSpot::GetCreateParams(FWVECTOR *vPosition, FWVECTOR *vDirection, FWFLOAT *fTheta, FWFLOAT *fPhi, FWFLOAT *fFalloff)
{
	if (vPosition) GetPosition(vPosition);
	if (vDirection) GetDirection(vDirection);
	if (fTheta) *fTheta = m_fTheta;
	if (fPhi) *fPhi = m_fPhi;
	if (fFalloff) *fFalloff = m_fFalloff;
	return S_OK;
}

HRESULT CSceneLightSpot::Render(IRndrGeneric *pRenderer)
{
	m_bDirty = FALSE;
	pRenderer->SetLight(m_nIndex, m_bOn);
	if (!m_bOn) return S_OK;
	FWVECTOR vPos;
	GetPosition(&vPos);
	FWVECTOR vDir;
	GetDirection(&vDir);
	pRenderer->SetSpotLight(m_nIndex, m_colorDiffuse, m_colorSpecular, m_colorAmbient, &vPos, &vDir, m_fTheta, m_fPhi, m_fFalloff);
	return S_OK;
}

HRESULT CSceneLightSpot::Turnoff(IRndrGeneric *pRenderer)
{
	pRenderer->SetLight(m_nIndex, FALSE);
	return S_OK;
}

HRESULT __stdcall CSceneLightSpot::CreateChild(LPOLESTR pLabel, /*[out, retval]*/ IKineNode **p)
{
	if (!p) return ERROR(FW_E_POINTER);
	*p = NULL;
	FWDevice()->CreateObject(L"KineBone", IID_IKineNode, (IFWUnknown**)p);	
	if (!p) return ERROR(FW_E_OUTOFMEMORY, 0, NULL, NULL, 0);
	
	HRESULT h = AddChild(pLabel, *p);
	if (FAILED(h)) { (*p)->Release(); return h; }

	return S_OK;
}

///////////////////////////////////////////////////////////
// Class CSceneCamera

ISceneCamera *CSceneCamera::c_CurCamera = NULL;

CSceneCamera::CSceneCamera() : m_bReady(FALSE), m_bDirty(TRUE), m_fFOV(3.141592654f/4.0f), m_fClipNear(1.0f), m_fClipFar(1000.0f), m_fAspect(0.0f), m_nOrdinalOffset(0)
{
}

CSceneCamera::~CSceneCamera()
{
	if (c_CurCamera == this) c_CurCamera = NULL;
}

HRESULT CSceneCamera::Create(FWVECTOR vPosition, FWVECTOR vDirection, FWVECTOR vUp)
{
	PutPosition(vPosition);
	PutDirection(vDirection);
	PutUpVector(vUp);
	m_bReady = TRUE;
	return S_OK;
}

HRESULT CSceneCamera::CreateEx(FWVECTOR vPosition, FWVECTOR vTarget, FWVECTOR vUp)
{
	PutPosition(vPosition);
	PutTarget(vTarget);
	PutUpVector(vUp);
	m_bReady = TRUE;
	return S_OK;
}


HRESULT CSceneCamera::GetCreateParams(FWVECTOR *vPosition, FWVECTOR *vDirection, FWVECTOR *vUp)
{
	if (vPosition) GetPosition(vPosition);
	if (vDirection) GetDirection(vDirection);
	if (vUp) GetUpVector(vUp);
	return S_OK;
}

HRESULT CSceneCamera::PutPerspective(FWFLOAT fFOV, FWFLOAT fClipNear, FWFLOAT fClipFar, FWFLOAT fAspect)
{
	m_fFOV = fFOV;
	m_fClipNear = fClipNear;
	m_fClipFar = fClipFar;
	m_fAspect = fAspect;
	return S_OK;
}

HRESULT CSceneCamera::GetPerspective(FWFLOAT *pfFOV, FWFLOAT *pfClipNear, FWFLOAT *pfClipFar, FWFLOAT *pfAspect)
{
	if (pfFOV) *pfFOV = m_fFOV;
	if (pfClipNear) *pfClipNear = m_fClipNear;
	if (pfClipFar) *pfClipFar = m_fClipFar;
	if (pfAspect) *pfAspect = m_fAspect;
	return S_OK;
}

HRESULT CSceneCamera::Render(IRndrGeneric *pRenderer)
{
	m_bDirty = FALSE;
	HRESULT h;
	ITransform *pT;
	h = GetLookAtLHTransform(&pT);
	if (FAILED(h)) return h;

	h = pRenderer->PutViewTransform(pT);

	if (SUCCEEDED(h))
	{
		FWFLOAT fAspect;
		if (m_fAspect == 0.0f)
			pRenderer->GetAspectRatio(&fAspect);
		else
			fAspect = m_fAspect;
		pT->FromPerspectiveLH(m_fFOV, m_fClipNear, m_fClipFar, fAspect);
//pT->FromScaling(m_fFOV/100.0f/fAspect, m_fFOV/100.0f, 0.0f);
		pRenderer->PutProjectionTransform(pT);
	}

	pT->Release();

	if (FAILED(h)) return FW_E_CLASSSPECIFIC;
	return S_OK;
}

HRESULT CSceneCamera::Turnoff(IRndrGeneric *pRenderer)
{
	if (c_CurCamera == this) c_CurCamera = NULL;
	return S_OK;
}

HRESULT __stdcall CSceneCamera::CreateChild(LPOLESTR pLabel, /*[out, retval]*/ IKineNode **p)
{
	if (!p) return S_OK;

	*p = NULL;
	FWDevice()->CreateObject(L"KineBone", IID_IKineNode, (IFWUnknown**)p);	
	if (!p) return ERROR(FW_E_OUTOFMEMORY, 0, NULL, NULL, 0);
	
	HRESULT h = AddChild(pLabel, *p);
	if (FAILED(h)) { (*p)->Release(); return h; }

	return S_OK;
}
