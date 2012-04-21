// Camera.cpp

#include "StdAfx.h"

#include <freewill.h>
#include <fwaction.h>
#include <fwrender.h>

#include "Camera.h"
#include "Vector.h"
#include "freewilltools.h"

#pragma warning (disable:4995)
#pragma warning (disable:4996)
#pragma warning (disable:4244)
 
#define NOBONE ((IKineNode*)NULL)

CCamera::CCamera()
{
	m_bMoved = m_bRotated = m_bZoomed = false;
	m_pBaseBone = NULL;
	m_pHandleBone = NULL;
	m_pCamera = NULL;
}

CCamera::~CCamera()
{
	Destroy();
}

void CCamera::SetBaseBone(IKineNode *pNode, bool bKeepCoord)
{
	ASSERT(pNode); if (!pNode) return;

	if (bKeepCoord && m_pHandleBone)
	{
		// modify coordinates so that to keep the camera intact
		ITransform *pT;
		m_pHandleBone->CreateCompatibleTransform(&pT);
		pNode->GetGlobalTransform(pT);
		m_pHandleBone->Transform(pT, KINE_LOCAL | KINE_INVERTED);
		m_pHandleBone->GetParentTransform(pT);
		m_pHandleBone->Transform(pT, KINE_LOCAL | KINE_REGULAR);
		pT->Release();
	}

	// deconstruct the camera from wherever it is now
	if (m_pBaseBone && m_pHandleBone)
		m_pBaseBone->DelChildPtr(m_pHandleBone);
	if (m_pBaseBone) 
		m_pBaseBone->Release();

	// get storey bone
	m_pBaseBone = pNode;
	m_pBaseBone->AddRef();

	// install the camera on the bone - if camera already created
	if (m_pHandleBone)
	{
		OLECHAR buf[257];
		m_pBaseBone->CreateUniqueLabel(L"CameraHandle", 256, buf);
		m_pBaseBone->AddChild(buf, m_pHandleBone);
	}
}

bool CCamera::Create()
{
	ASSERT(m_pHandleBone == NULL && m_pCamera == NULL);

	if (!m_pBaseBone)
		return false;

	OLECHAR buf[257];
	m_pBaseBone->CreateUniqueLabel(L"CameraHandle", 256, buf);
	m_pBaseBone->CreateChild(buf, &m_pHandleBone);

	m_pHandleBone->FWDevice()->CreateObject(L"Camera", IID_ISceneCamera, (IFWUnknown**)&m_pCamera);
	m_pHandleBone->AddChild(L"Camera", m_pCamera);

	m_pCamera->Create(__FW_Vector(0, 0, 0), __FW_Vector(0, 1, 0), __FW_Vector(0, 0, 1.0f));
	m_pCamera->PutPerspective((FWFLOAT)M_PI / 4, 20.0f, 10000.0f, 0.0f);
	m_pCamera->PutVisible(TRUE);

	IKineTargetedObj *pTO = NULL;
	m_pCamera->QueryInterface(&pTO);
	pTO->PutConfig(KINE_TARGET_ORBITING, NULL);
	pTO->Release();

	return true;
}

bool CCamera::Destroy()
{
	if (!m_pBaseBone && !m_pHandleBone && !m_pCamera)
		return true;
	if (m_pBaseBone && m_pHandleBone)
		m_pBaseBone->DelChildPtr(m_pHandleBone);
	if (m_pCamera) m_pCamera->Release();
	if (m_pHandleBone) m_pHandleBone->Release();
	if (m_pBaseBone) m_pBaseBone->Release();
	m_pBaseBone = NULL;
	m_pHandleBone = NULL;
	m_pCamera = NULL;
	return true;
}

void CCamera::GetCurPos(FWVECTOR &pos)
{
	pos.x = pos.y = pos.z = 0;
	m_pHandleBone->LtoG((FWVECTOR*)&pos);
}

void CCamera::GetCurLocalPos(FWVECTOR &pos)
{
	pos.x = pos.y = pos.z = 0;
	m_pHandleBone->LtoG((FWVECTOR*)&pos);
	m_pBaseBone->GtoL((FWVECTOR*)&pos);
}

void CCamera::Reset()
{
	m_pHandleBone->Reset();
	m_pCamera->Reset();
}

void CCamera::Move(FWFLOAT x, FWFLOAT y, FWFLOAT z, IKineNode *pRef)
{
	if (!m_pCamera || !m_pHandleBone) return;

	ITransform *pT = NULL;
	m_pHandleBone->CreateCompatibleTransform(&pT);
	pT->FromTranslationXYZ(x, y, z);
	m_pHandleBone->Transform(pT, KINE_RIGHT_SIDE);
	pT->Release();
	m_bMoved = true;
}

void CCamera::Pan(FWFLOAT f)
{
	if (!m_pCamera || !m_pHandleBone) return;

	ITransform *pT = NULL;
	m_pHandleBone->CreateCompatibleTransform(&pT);
	pT->FromRotationZ(f);
	m_pHandleBone->Transform(pT, KINE_RIGHT_SIDE);
	pT->Release();
	m_bRotated = true;
}

void CCamera::Tilt(FWFLOAT f)
{
	if (!m_pCamera) return;

	ITransform *pT = NULL;
	m_pCamera->CreateCompatibleTransform(&pT);
	pT->FromRotationX(f);
	m_pCamera->Transform(pT, KINE_RIGHT_SIDE);
	pT->Release();
	m_bRotated = true;
}

void CCamera::Zoom(FWFLOAT f)
{
	if (!m_pCamera) return;

	//m_cp.fZoom -= f;
//	m_cp.fHFOV -= f;
//	m_cp.fVFOV -= f;
//	m_pCamera->PutPerspective(m_cp.FOV(), m_cp.fClipNear, m_cp.fClipFar, m_cp.fAspectRatio);

//	m_bZoomed = true;


	//FWFLOAT fFOV, fNear, fFar, fAspect;
	//m_pCamera->GetPerspective(&fFOV, &fNear, &fFar, &fAspect);
	//fFOV -= f;
	//m_pCamera->PutPerspective(fFOV, fNear, fFar, fAspect);
}

void CCamera::Adjust(FWFLOAT fNewAspectRatio)
{
//	m_cp.fHFOV = 2 * atan(tan(m_cp.fHFOV/2) * m_cp.fAspectRatio / fNewAspectRatio);
//	m_cp.fAspectRatio = fNewAspectRatio;
//	m_pCamera->PutPerspective(m_cp.FOV(), m_cp.fClipNear, m_cp.fClipFar, m_cp.fAspectRatio);
}

