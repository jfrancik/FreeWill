// sceneobj.cpp : Defines the Scene Meshed Object
//

#include "stdafx.h"
#include "sceneobj.h"

#pragma warning (disable:4800)

///////////////////////////////////////////////////////////
// Class CSceneObject

CSceneObject::CSceneObject()
{ 
	m_bOn = TRUE;
	m_pRenderer = NULL;
	m_pDictionary = NULL;
	m_pMaterial = NULL;
	m_bOverwriteMat = FALSE;
	m_pBones = NULL;
	m_pFrameTransforms = NULL;
	m_nOrdinalOffset = 0;
}

CSceneObject::~CSceneObject()
{
	if (m_pRenderer) m_pRenderer->Release();
	if (m_pDictionary) m_pDictionary->Release();
	if (m_pMaterial) m_pMaterial->Release();
	ResetRenderInternalData();
}

// The inherited function does not add meshes to the reproduced object
// and does not initialise the object properly
HRESULT __stdcall CSceneObject::Reproduce(/*[out, retval]*/ IKineObj3D **p)
{
	// reproduce myself (as a IKineObj3D)
	IKineObj3D *pRepObj = NULL;
	HRESULT h = __SCENEOBJECT_BASECLASS::Reproduce(&pRepObj);
	if (FAILED(h)) return h;

	// query for ISceneObject
	ISceneObject *pRepSceneObj = NULL;
	h = pRepObj->QueryInterface(&pRepSceneObj);
	if (FAILED(h)) { pRepObj->Release(); return ERROR(FW_E_NOINTERFACE); }

	// SceneObject specific stuff
	IRndrGeneric *pRndr = NULL;
	GetRenderer(&pRndr);
	if (pRndr) pRepSceneObj->PutRenderer(pRndr);
	if (pRndr) pRndr->Release();
	IMeshDictionary *pDir = NULL;
	GetDictionary(&pDir);
	if (pDir) pRepSceneObj->PutDictionary(pDir);
	if (pDir) pDir->Release();
	pRepSceneObj->PutVisible(IsVisible() == S_OK ? TRUE : FALSE);
	pRepSceneObj->PutRenderHint(0);

	// enumerate my children
	IKineChild *pChild = NULL;
	IKineEnumChildren *pEnum = NULL;
	EnumChildren(&pEnum);
	while (pEnum->Next(&pChild) == S_OK)
	{
		// omit if I am not the primary parent
//		if (pChild->IsParent(this) == S_FALSE)
//			continue;

		// query for IMesh?
		IMesh *pMesh = NULL;
		h = pChild->QueryInterface(&pMesh);
		if (FAILED(h)) 
		{
			pChild->Release();
			continue;	// not a mesh...
		}

		// get label
		LPOLESTR pLabel = NULL;
		pChild->GetLabel(&pLabel);

		// add a mesh
		pRepSceneObj->AddMesh(pLabel, pMesh);
		pMesh->Release();

		pChild->Release();
	}
	pRepSceneObj->Release();
	pEnum->Release();
	*p = pRepObj;
	return S_OK;
}

HRESULT __stdcall CSceneObject::CreateChild(LPOLESTR pLabel, /*[out, retval]*/ IKineNode **p)
{
	if (!p) return ERROR(FW_E_POINTER);
	*p = NULL;
	FWDevice()->CreateObject(L"KineBone", IID_IKineNode, (IFWUnknown**)p);	
	if (!p) return ERROR(FW_E_OUTOFMEMORY, 0, NULL, NULL, 0);
	
	HRESULT h = AddChild(pLabel, *p);
	if (FAILED(h)) { (*p)->Release(); return h; }

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// properties

HRESULT CSceneObject::GetRenderer(IRndrGeneric **p)
{
	if (p)
	{
		*p = m_pRenderer;
		if (*p) (*p)->AddRef();
	}
	return S_OK;
}

HRESULT CSceneObject::PutRenderer(IRndrGeneric *p)
{
	if (!p) return S_OK;
	if (m_pRenderer) m_pRenderer->Release();
	m_pRenderer = p;
	m_pRenderer->AddRef();
	return S_OK;
}

HRESULT CSceneObject::GetDictionary(IMeshDictionary **p)
{
	*p = m_pDictionary;
	if (*p) (*p)->AddRef();
	return S_OK;
}

HRESULT CSceneObject::PutDictionary(IMeshDictionary *pDictionary)
{
	ResetRenderInternalData();
	if (m_pDictionary) m_pDictionary->Release();
	m_pDictionary = pDictionary;
	if (m_pDictionary) 
		m_pDictionary->AddRef();
	return S_OK;
}

HRESULT CSceneObject::GetMaterial(/*[out, retval]*/ IMaterial **p)
{
	*p = m_pMaterial;
	if (*p) (*p)->AddRef();
	return S_OK;
}

HRESULT CSceneObject::PutMaterial(IMaterial *pMaterial, BOOL bOverwrite)
{
	for (FWULONG i = 0; i < m_vecMeshes.size(); i++)
		if (bOverwrite)
			m_vecMeshes[i]->SetMaterial(pMaterial);
		else
		{
			IMaterial *pMeshMaterial = NULL;
			m_vecMeshes[i]->GetMaterial(&pMeshMaterial);
			if (pMeshMaterial == NULL || pMeshMaterial == m_pMaterial)
				m_vecMeshes[i]->SetMaterial(pMaterial);
			if (pMeshMaterial) pMeshMaterial->Release();
		}
	if (m_pMaterial) m_pMaterial->Release();
	m_pMaterial = pMaterial;
	if (m_pMaterial) m_pMaterial->AddRef();
	m_bOverwriteMat = (bool)bOverwrite;
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// meshes

HRESULT CSceneObject::NewMesh(FWSTRING pLabel, /*[out, retval]*/ IMesh **pMesh)
{
	HRESULT h = FWDevice()->CreateObject(L"Mesh",IID_IMesh, (IFWUnknown**)pMesh);
	if (FAILED(h)) return h;
	return AddMesh(pLabel, *pMesh);
}

HRESULT CSceneObject::AddMesh(FWSTRING pLabel, IMesh *pMesh)
{
	HRESULT h;
	if (!m_pRenderer)
		return ERROR(FW_E_NOTREADY);

	// ATTENTION: if no dictionary so far, create one!
	if (!m_pDictionary)
	{
		h = FWDevice()->CreateObject(L"MeshDictionary", IID_IMeshDictionary, (IFWUnknown**)&m_pDictionary);
		if (FAILED(h)) return h;
	}

	if (!pMesh) return ERROR(FW_E_POINTER);

	// if no buffers set, set renderer's buffer
	IMeshVertexBuffer *pVB = NULL;
	IMeshFaceBuffer *pFB = NULL;
	pMesh->GetBuffers(&pVB, &pFB);
	if (pVB) pVB->Release(); if (pFB) pFB->Release();
	if (pVB == NULL && pFB == NULL)
	{
		m_pRenderer->GetBuffers(&pVB, &pFB);
		pMesh->PutBuffers(pVB, pFB);
		pVB->Release(); pFB->Release();
	}

	IMeshDictionary *pDictionary = NULL;
	pMesh->GetDictionary(&pDictionary);
	if (pDictionary) pDictionary->Release();
	if (pDictionary == NULL)
		pMesh->PutDictionary(m_pDictionary);

	IKineChild *pChildMesh = NULL;
	h = pMesh->QueryInterface(&pChildMesh);
	if (FAILED(h)) return ERROR(FW_E_NOINTERFACE);
	FWSTRING pBuf = new wchar_t[wcslen(pLabel) + 1 + 6];
	wcscpy(pBuf, L"%mesh%");
	wcscat(pBuf, pLabel);
	bool bBlankNewMesh = (pChildMesh->GetParent(NULL) == S_FALSE);
    h = AddChildEx(pBuf, pChildMesh, bBlankNewMesh, NULL, NULL);
	delete [] pBuf;
	pChildMesh->Release();
	if (FAILED(h)) return h;

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// rendering

void CSceneObject::SetRenderInternalData()
{
	// Set following data:
	// m_nBones = number of bones
	// m_pBones = array of bones
	// m_pFrameTransforms = their global matrices (identities initially)
	// m_vecMeshes = vector of meshes...
	if (m_pBones) return;

	m_pDictionary->GetNum(&m_nBones);
	m_pBones = new IKineChild*[m_nBones];
	m_pFrameTransforms = new FWMATRIX[m_nBones];

	// set m_pBones to NULL, m_pFrameTransforms TO IDENTITY
	memset(m_pBones, 0, sizeof(IKineChild*) * m_nBones);
	memset(m_pFrameTransforms, 0, sizeof(FWMATRIX) * m_nBones);
	for (FWULONG i = 0; i < m_nBones; i++)
		m_pFrameTransforms[i][0][0] = m_pFrameTransforms[i][1][1] = 
			m_pFrameTransforms[i][2][2] = m_pFrameTransforms[i][3][3] = 1.0f;

	IKineChild *pChild = NULL;
	IKineEnumChildren *pEnum = NULL;
	EnumAllDescendants(&pEnum);
	while (pEnum->Next(&pChild) == S_OK)
	{
		IMesh *pMesh;
		if (pChild && SUCCEEDED(pChild->QueryInterface(&pMesh)))
			m_vecMeshes.push_back(pMesh);
		else
		{
			LPOLESTR pLabel;
			pChild->GetLabel(&pLabel);
			FWULONG i;
			if (m_pDictionary->GetIndexIfExists(pLabel, &i) == S_OK && m_pBones[i] == NULL)
			{
				m_pBones[i] = pChild;
				m_pBones[i]->AddRef();
			}
		}
		pChild->Release();
	}
	pEnum->Release();

	if (m_pMaterial)
		PutMaterial(m_pMaterial, m_bOverwriteMat);
}

void CSceneObject::ResetRenderInternalData()
{
	if (m_pBones) 
	{
		for (FWULONG i = 0; i < m_nBones; i++)
			if (m_pBones[i]) m_pBones[i]->Release();
		delete [] m_pBones;
	}

	if (m_pFrameTransforms) delete [] m_pFrameTransforms;
	m_pBones = NULL;
	m_pFrameTransforms = NULL;

	for (FWULONG i = 0; i < m_vecMeshes.size(); i++)
		if (m_vecMeshes[i]) m_vecMeshes[i]->Release();
}

HRESULT CSceneObject::Render(IRndrGeneric *pRenderer)
{
	if (!pRenderer) return S_OK;
	if (!m_pDictionary) return S_OK;
	SetRenderInternalData();
	if (m_vecMeshes.size() == 0) return S_FALSE;
	
	RNDR_MESHEDOBJ info;
	memset(&info, 0, sizeof(info));

	for (FWULONG i = 0; i < m_nBones; i++)
		if (m_pBones[i])
		{
			ITransform *pT = NULL, *pTGlob = NULL;
			m_pBones[i]->CreateCompatibleTransform(&pT);
			m_pBones[i]->GetRefTransform(pT);
			m_pBones[i]->CreateCompatibleTransform(&pTGlob);
			m_pBones[i]->GetGlobalTransform(pTGlob);
			pT->Inverse();
			pT->Multiply(pTGlob);
			pT->AsMatrix(m_pFrameTransforms[i]);
			pT->Release();
			pTGlob->Release();
		}

	info.nMatrixNum = m_nBones;
	info.pMatrices = m_pFrameTransforms;

	info.nMesh = (FWULONG)m_vecMeshes.size();
	for (FWULONG iMesh = 0; iMesh < m_vecMeshes.size(); iMesh++)
	{
		info.iMesh = iMesh;
		info.pMesh = m_vecMeshes[iMesh];
		if (info.pMesh->IsVisible() == S_FALSE) continue;
		info.pMesh->AddRef();
		info.pMesh->GetBuffers(&info.pVertexBuffer, &info.pFaceBuffer);
		info.pVertexBuffer->GetItemSize(&info.nVertexSize);
		info.pMesh->GetVertexFirst(&info.nVertexFirst);
		info.pMesh->GetVertexNum(&info.nVertexNum);
		info.pFaceBuffer->GetItemSize(&info.nFaceSize);
		info.pMesh->GetFaceFirst(&info.nFaceFirst);
		
		if (m_vecMeshes[iMesh]->SupportsSubmeshedVertexBlending() == S_OK)
		{
			// Submeshed Rendering

			// get submesh info
			FWULONG *pSubmeshLen;
			m_vecMeshes[iMesh]->GetSubmeshInfo(&info.nSubmesh, &info.nIndexNum, &pSubmeshLen, &info.pTransformIndices);


			for (FWULONG iSubmesh = 0; iSubmesh < info.nSubmesh; iSubmesh++)
			{
				info.iSubmesh = iSubmesh;
				info.nFaceNum = pSubmeshLen[iSubmesh];
				pRenderer->RenderMesh(&info);
				info.nFaceFirst += info.nFaceNum;
				info.pTransformIndices += info.nIndexNum;
			}
		}
		else
		{
			// TODO: indexed rendering
			info.nSubmesh = 0;
			info.iSubmesh = 0;
			info.pMesh->GetFaceNum(&info.nFaceNum);
	
			return ERROR(FW_E_FORMAT);
		}
		if (info.pMesh) info.pMesh->Release();
		if (info.pVertexBuffer) info.pVertexBuffer->Release();
		if (info.pFaceBuffer) info.pFaceBuffer->Release();
	}

	return S_OK;
}

HRESULT CSceneObject::Turnoff(IRndrGeneric *pRenderer)
{
	return S_OK;
}