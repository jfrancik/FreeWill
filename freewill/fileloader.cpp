// fileloader.cpp : Defines the File Loading Stuff

#include "stdafx.h"
#include "fileloader.h"
#include "boundplus.h"

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// Class CFileLoader

CFileLoader::CFileLoader() : 
		m_pFileIn(NULL), 
		m_pLoadScene(NULL), m_pLoadObject(NULL), m_pLoadName(NULL),
		m_pObject(NULL), m_pMesh(NULL), m_pMeshForMaterial(NULL), 
		m_pBone(NULL),
		m_bCutPrefixAuto(TRUE), m_pCutPrefix(NULL)
{ 
}

CFileLoader::~CFileLoader()
{ 
	if (m_pFileIn) m_pFileIn->Release();
	if (m_pLoadScene) m_pLoadScene->Release();
	if (m_pLoadObject) m_pLoadObject->Release();
	if (m_pLoadName) free(m_pLoadName);
	if (m_pObject) m_pObject->Release();
	if (m_pMesh) m_pMesh->Release();
	if (m_pMeshForMaterial) m_pMeshForMaterial->Release();
	if (m_pBone) m_pBone->Release();
	if (m_pCutPrefix) delete [] m_pCutPrefix;
}

/////////////////////////////////////////////////////////////////////////////
// IFileSink: Reading the entire scene

HRESULT _stdcall CFileLoader::OnBeginScene(LPOLESTR szName, LPOLESTR szFormat)
{
	return S_OK;
}

HRESULT _stdcall CFileLoader::OnEndScene()
{
	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// IFileSink: Reading the MeshObject main node

HRESULT _stdcall CFileLoader::OnBeginObject(LPOLESTR szName, LPOLESTR szClass)
{
	HRESULT h;
	if (m_pObject) m_pObject->Release(); m_pObject = NULL;
	if (m_pBone) m_pBone->Release(); m_pBone = NULL;

	if (m_pLoadScene)
	{
		// loading entire scene: create and initialise a new object...
		assert(!m_pLoadObject && !m_pLoadName);
		h = m_pLoadScene->NewObject(szName, &m_pObject);
		if (FAILED(h)) return h;
	}
	else
	if (!m_pLoadObject)
		// loading nothing more?
		return S_OK;	// ERROR(FW_E_POINTER); -- removed on 8 Dec 2008
	else
	if (m_pLoadName == NULL || wcscmp(szName, m_pLoadName) == 0)
	{
		// loading just this object
		m_pObject = m_pLoadObject;
		m_pLoadObject = NULL;
	}
	else
		// loading another object
		return S_OK;

	// establish a new bone system root - the object's...
	h = m_pObject->QueryInterface(&m_pBone);
	if (FAILED(h)) return ERROR(FW_E_NOINTERFACE);

	if (m_bCutPrefixAuto)
		PutCutPrefix(szName);

	return S_OK;
}

HRESULT _stdcall CFileLoader::OnEndObject()
{
//	if (m_pObject)
//		m_pObject->CreateFlatNamespace();
	if (m_pObject) m_pObject->Release(); m_pObject = NULL;
	if (m_pBone) m_pBone->Release(); m_pBone = NULL;
	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// IFileSink: Reading a kinematic or mesh object

HRESULT _stdcall CFileLoader::OnBeginNode(LPOLESTR szName, LPOLESTR szClass,
	FWMATRIX matrix, FWVECTOR vPos, FWQUAT qRot, FWVECTOR vScale, BOOL bVisible)
{
	if (!m_pBone) return S_OK;

	HRESULT h;
	IKineNode *q = NULL;
	ITransform *pT = NULL, *pU = NULL;
	ITransform *pTGB = NULL;

	h = FWDevice()->CreateObject(L"Transform", IID_ITransform, (IFWUnknown**)&pT); if (FAILED(h)) return h;
	h = FWDevice()->CreateObject(L"Transform", IID_ITransform, (IFWUnknown**)&pU); if (FAILED(h)) return h;

	// create the bone
	size_t nIndex = m_pCutPrefix ? wcslen(m_pCutPrefix) : 0;
	if (nIndex && wcsncmp(szName, m_pCutPrefix, nIndex) != 0) nIndex = 0;
	h = m_pBone->CreateChild(szName+nIndex, &q);
	if (FAILED(h)) return FW_E_CLASSSPECIFIC;
	m_pBone->Release();
	m_pBone = q;
	if (FAILED(h)) return ERROR(FW_E_NOINTERFACE);

	// apply geometry
	//memcpy(m_matrix, matrix, sizeof(FWMATRIX));	// for use in OnBeginMesh
	ITransform *pM = NULL;
	FWDevice()->CreateObject(L"Transform", IID_ITransform, (IFWUnknown**)&pM);
	pM->MulRotationQuat(&qRot);
	pM->MulScale(vScale.x, vScale.y, vScale.z);
	pM->MulTranslationVector(&vPos);
	pM->AsMatrix(m_matrix);
	pM->Release();


	pT->FromIdentity();
	m_pBone->PutBaseTransform(pT);
	m_pBone->CreateCompatibleTransform(&pTGB);
	h = m_pBone->GetGlobalTransform(pTGB); 
	if (FAILED(h))
	{
		FWDevice()->Recover(FW_SEV_WARNING);
		pTGB = (ITransform*)pT->Clone(IID_ITransform);
	}
	pU->FromTransform(pTGB);
	pU->Inverse();
	pT->FromIdentity();
	pT->MulRotationQuat(&qRot);
	pT->MulTranslationVector(&vPos);
	pT->Multiply(pU);
	m_pBone->PutBaseTransform(pT);
	m_pBone->FreezeRef();

	m_bVisible = bVisible;

	if (pT) pT->Release();
	if (pU) pU->Release();
	if (pTGB) pTGB->Release();

	return S_OK;
}

HRESULT _stdcall CFileLoader::OnEndNode()
{
	if (!m_pBone) return S_OK;
	IKineNode *p;
	HRESULT h = m_pBone->GetParent(&p);
	if (FAILED(h)) return FW_E_CLASSSPECIFIC;
	m_pBone->Release();
	m_pBone = p;
	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// IFileSink: Reading the bounding box data

HRESULT _stdcall CFileLoader::OnBB(FWVECTOR vMin, FWVECTOR vMax)
{
	if (m_pObject && m_pBone)
	{
		HRESULT h;

		// create bounding
		IBounding *pBounding = NULL;
		FWDevice()->CreateObject(L"Bounding", IID_IBounding, (IFWUnknown**)&pBounding);

		// add to the strucutre
		IKineChild *pChild = NULL;
		pBounding->QueryInterface(&pChild);
		m_pBone->AddChild(L"bound", pChild);
		pChild->Release();

		// prepare data (use the global transform)
		BOUND_AABB aabb = { vMin, vMax };

		// use the global transform
		ITransform *pTransform = NULL;
		h = FWDevice()->CreateObject(L"Transform", IID_ITransform, (IFWUnknown**)&pTransform);
		if (FAILED(h)) return h;
		pTransform->FromMatrix(m_matrix);
		pTransform->ApplyTo(&aabb.vecMin);
		pTransform->ApplyTo(&aabb.vecMax);
		pTransform->Release();

		pBounding->PutData(BOUND_FORMAT_AABB, sizeof(aabb), (BYTE*)&aabb);

		// release
		pBounding->Release();
	}
	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// IFileSink: Reading the mesh internals

HRESULT _stdcall CFileLoader::OnBeginMesh(FWULONG nVertsCount, FWULONG nFacesCount, FWULONG nTexVertsCount)	
{
	if (m_pObject)
	{
		HRESULT h = S_OK;
		// get the current label
		FWSTRING pLabel;
		assert(m_pBone);
		m_pBone->GetLabel(&pLabel);
		// create a new mesh
		h = m_pObject->NewMesh(pLabel, &m_pMesh);
		if (FAILED(h)) return h;
		m_pMesh->PutVisible(m_bVisible);
		// // set class
		// m_pMesh->PutAttrString(L"class", m_bVisible ? L"visible" : L"invisible");
		// open buffers
		h = m_pMesh->Open(NULL, NULL);
		if (FAILED(h)) return h;
		m_iV = m_iF = m_iTexV = 0;
		// set the global transform
		ITransform *pTransform = NULL;
		h = FWDevice()->CreateObject(L"Transform", IID_ITransform, (IFWUnknown**)&pTransform);
		if (FAILED(h)) return h;
		pTransform->FromMatrix(m_matrix);
		m_pMesh->PutTransform(pTransform);
		pTransform->Release();
	}
	return S_OK;
}

HRESULT _stdcall CFileLoader::OnEndMesh()
{
	if (m_pMesh)
	{
		// store the mesh for optional material...
		// ATTENTION: this id dependent on sequence of the input data
		// and will work only if material data come AFTER the mesh data
		// TO DO: fix it...
		if (m_pMeshForMaterial) m_pMeshForMaterial->Release();
		m_pMeshForMaterial = m_pMesh;
		m_pMeshForMaterial->AddRef();

		m_pMesh->Close();
		m_pMesh->Release();
		m_pMesh = NULL;
	}
	return S_OK;
}

HRESULT _stdcall CFileLoader::OnVertex(FWVECTOR vVertex, FWULONG nBonesCount, LPOLESTR *ppsBoneNames, FWFLOAT *pfWeights)	
{
	if (!m_pMesh) return S_OK;

	HRESULT h = S_OK;

	// TO DO: this depends on separating vertex loading and blending loading
	if (m_iF == 0)
	{
		// just regular vertices
		h = m_pMesh->SetVertexXYZVector(m_iV, vVertex);
		//h = m_pMesh->SetVertexDiffuse(m_iV, 0xffffffff);
		if (FAILED(h)) return h;
	}
	else
	{
		// blending!
		if (m_iV == 0)
		{
			h = m_pMesh->InitAdvVertexBlending(0.01f, 0);
//			m_iV += 2;
//			h = m_pMesh->AddBlendWeight(0, 1.0f, L"Pelvis");
//			h = m_pMesh->AddBlendWeight(1, 1.0f, L"Pelvis");
		}
		if (FAILED(h)) return h;

		if (nBonesCount == 0)
		{
			LPOLESTR pLabel;
			if (m_pBone)
				m_pBone->GetLabel(&pLabel);
			else
				pLabel = L"Pelvis";
			h = m_pMesh->AddBlendWeight(m_iV, 1.0f, pLabel);
			if (FAILED(h)) return h;
		}
		else
		{
			FWFLOAT W = 0.0f;
			for (FWULONG i = 0; i < nBonesCount; i++)
				W += pfWeights[i];
			FWFLOAT poprawka = (W < 0.001f) ? 0.1f : 0.0f;
			for (FWULONG i = 0; i < nBonesCount; i++)
			{
				size_t nIndex = m_pCutPrefix ? wcslen(m_pCutPrefix) : 0;
				if (nIndex && wcsncmp(ppsBoneNames[i], m_pCutPrefix, nIndex) != 0) nIndex = 0;
				h = m_pMesh->AddBlendWeight(m_iV, pfWeights[i] + poprawka, ppsBoneNames[i]+nIndex);
				if (FAILED(h)) return h;
			}
		}
	}
	m_iV++;

	return S_OK;
}

HRESULT _stdcall CFileLoader::OnTexVertex(FWVECTOR vTexVector)
{
	if (!m_pMesh) return S_OK;
	HRESULT h = m_pMesh->SetVertexTextureUV(m_iTexV, 0, vTexVector.x, vTexVector.y);
	m_iTexV++;
	return S_OK;
}

HRESULT _stdcall CFileLoader::OnFace(FWULONG iVertexA, FWULONG iVertexB, FWULONG iVertexC, 
					FWULONG iTexVertexA, FWULONG iTexVertexB, FWULONG iTexVertexC,
					FWVECTOR *pvNormals)
{
	if (!m_pMesh) return S_OK;

//	iVertexA += 2;
//	iVertexB += 2;
//	iVertexC += 2;

	HRESULT h = S_OK;

	if (m_iF == 0)
	{
		h = m_pMesh->InitAdvNormalSupport(0);
		if (FAILED(h)) return h;
		m_iV = 0;
	}
		
	h = m_pMesh->AddNormalVector(&iVertexA, pvNormals[0]); if (FAILED(h)) return h;
	h = m_pMesh->AddNormalVector(&iVertexB, pvNormals[1]); if (FAILED(h)) return h;
	h = m_pMesh->AddNormalVector(&iVertexC, pvNormals[2]); if (FAILED(h)) return h;
	h = m_pMesh->SetFace(m_iF, iVertexA, iVertexB, iVertexC); if (FAILED(h)) return h;

	m_iF++;

	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// IFileSink: Reading various objects

HRESULT _stdcall CFileLoader::OnLight(LPOLESTR szName, LPOLESTR szClass,
	BOOL bIsTarget, FWVECTOR vEye, FWVECTOR vAtDir, 
	FWCOLOR cColor, FWFLOAT fPower, BOOL bActive)
{
	HRESULT h;
	if (!m_pLoadScene) return S_OK;
	ISceneLightDir *pLight = NULL;
	h = FWDevice()->CreateObject(L"DirLight", IID_ISceneLightDir, (IFWUnknown**)&pLight);
	if (FAILED(h)) return h;

	m_pLoadScene->AddChild(szName, pLight);
	
	h = pLight->PutDiffuseColor(cColor);
	if (FAILED(h)) return h;
	if (bIsTarget)
		h = pLight->CreateEx(vEye, vAtDir);
	else
		h = pLight->Create(vAtDir);
	if (FAILED(h)) return h;

	pLight->Release();
	return S_OK;
}

HRESULT _stdcall CFileLoader::OnCamera(LPOLESTR szName, LPOLESTR szClass, 
	BOOL bIsTarget, FWVECTOR vEye, FWVECTOR vAtDir, FWVECTOR vUp,
	FWFLOAT fFOV, FWFLOAT fClipNear, FWFLOAT fClipFar, FWFLOAT fDistance, 
	BOOL bIsOrtho)
{
	HRESULT h;
	if (!m_pLoadScene) return S_OK;
	ISceneCamera *pCamera = NULL;
	h = FWDevice()->CreateObject(L"Camera", IID_ISceneCamera, (IFWUnknown**)&pCamera);
	if (FAILED(h)) return h;

	m_pLoadScene->AddChild(szName, pCamera);
	
	if (bIsTarget)
		h = pCamera->CreateEx(vEye, vAtDir, vUp);
	else
		h = pCamera->Create(vEye, vAtDir, vUp);
	if (FAILED(h)) return h;
	h = pCamera->PutPerspective(fFOV, fClipNear, fClipFar, 0.0f);
	if (FAILED(h)) return h;

	pCamera->Release();
	return S_OK;
}

HRESULT _stdcall CFileLoader::OnMaterial(FWCOLOR cAmbient, FWCOLOR cDiffuse, FWCOLOR cSpecular,
	FWFLOAT fShininess, FWFLOAT fShinStrength, FWFLOAT fOpacity, 
	FWFLOAT fSelfIllumination, BOOL bSelfIllumColorOn, FWCOLOR cSelfIllumColor, 
	BOOL bTwoSided, BOOL bTextured)
{
	if (!m_pMeshForMaterial) return S_OK;

	//m_pMeshForMaterial->PutAttrLong(L"colour", RGB((BYTE)(cDiffuse.r * 255), (BYTE)(cDiffuse.g * 255), (BYTE)(cDiffuse.b * 255)));
	
	IMaterial* pMaterial = NULL;
	HRESULT h = FWDevice()->CreateObject(L"Material", IID_IMaterial, (IFWUnknown**)&pMaterial);
	if (FAILED(h)) return h;
	cDiffuse.a = 1 - fOpacity;
	//if (fOpacity < 1) pMaterial->SetAlphaMode(MAT_ALPHA_MATERIAL);
	pMaterial->SetAmbientColor(cAmbient);
	pMaterial->SetDiffuseColor(cDiffuse);
	pMaterial->SetSpecularColor(cSpecular);
	pMaterial->SetShininess(fShininess);
	pMaterial->SetShininessStrength(fShinStrength);
	if (bSelfIllumColorOn)
		pMaterial->SetSelfIllumination(cSelfIllumColor, fSelfIllumination);
	else
		pMaterial->SetSelfIlluminationOff();
	pMaterial->SetTwoSided(bTwoSided);
//	pMaterial->SetTextured(bTextured);
	m_pMeshForMaterial->SetMaterial(pMaterial);
	pMaterial->Release();
	
	return S_OK;
}

HRESULT _stdcall CFileLoader::OnTexture(LPOLESTR szName, LPOLESTR szType, 
	BYTE* pData, FWULONG nSize, FWFLOAT fUTile, FWFLOAT fVTile)
{
	if (!m_pMeshForMaterial || !m_pLoadScene) return S_OK;

	ITexture* pTexture = NULL;
	IMaterial* pMaterial = NULL;
	IRndrGeneric* pRenderer = NULL;
	HRESULT h = m_pLoadScene->GetRenderer(&pRenderer);
	if (FAILED(h)) return h;
	h = m_pMeshForMaterial->GetMaterial(&pMaterial);
	if (h == S_OK && pMaterial)
	{
		h = pRenderer->CreateTexture(&pTexture);
		if (h == S_OK && pTexture)
		{
			if (pData != NULL)
				h = pTexture->LoadFromFileInMemory(pData, nSize);
			else
				h = pTexture->LoadFromFile(szName);
			if (h == S_OK) 
			{
				pTexture->SetUVTile(fUTile, fVTile);
				pMaterial->SetTexture(0, pTexture);
				FWCOLOR white = { 1, 1, 1, 1 };
				pMaterial->SetDiffuseColor(white);
			}
			pTexture->Release();
		}
		pMaterial->Release();
	}
	pRenderer->Release();
	return h == S_OK ? S_OK : S_FALSE;
}

HRESULT _stdcall CFileLoader::OnLongProperty(LPOLESTR szName, LONG)
{
	return S_OK;
}

HRESULT _stdcall CFileLoader::OnFloatProperty(LPOLESTR szName, FWFLOAT)
{
	return S_OK;
}

HRESULT _stdcall CFileLoader::OnStringProperty(LPOLESTR szName, LPOLESTR)
{
	return S_OK;
}

HRESULT _stdcall CFileLoader::OnVectorProperty(LPOLESTR szName, FWVECTOR)
{
	return S_OK;
}

HRESULT _stdcall CFileLoader::OnQuatProperty(LPOLESTR szName, FWQUAT)
{
	return S_OK;
}

HRESULT _stdcall CFileLoader::OnColorProperty(LPOLESTR szName, FWCOLOR)
{
	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// IFileLoader: Load Operations

HRESULT _stdcall CFileLoader::LoadScene(LPOLESTR pFilename, IScene *pScene)
{
	if (!pScene) return ERROR(FW_E_POINTER);
	if (pScene->IsReady() != S_OK) return ERROR(FW_E_NOTREADY);

	if (m_pLoadScene) m_pLoadScene->Release(); m_pLoadScene = NULL;
	if (m_pLoadObject) m_pLoadObject->Release(); m_pLoadObject = NULL;
	if (m_pLoadName) free(m_pLoadName); m_pLoadName = NULL;
	if (m_pObject) m_pObject->Release(); m_pObject = NULL;
	if (m_pMesh) m_pMesh->Release(); m_pMesh = NULL;
	if (m_pMeshForMaterial) m_pMeshForMaterial->Release(); m_pMeshForMaterial = NULL;
	if (m_pBone) m_pBone->Release(); m_pBone = NULL;

	m_pLoadScene = pScene;
	m_pLoadScene->AddRef();

	HRESULT h = CreateDefFile(pFilename);
	if (FAILED(h)) return h;

	assert(m_pFileIn);
	return m_pFileIn->Read(this, pFilename);
}

HRESULT _stdcall CFileLoader::LoadObject(LPOLESTR pFilename, LPOLESTR pName, ISceneObject *pObject)
{
	if (!pObject) return ERROR(FW_E_POINTER);
	if (pObject->IsReady() != S_OK) return ERROR(FW_E_NOTREADY);

	if (m_pLoadScene) m_pLoadScene->Release(); m_pLoadScene = NULL;
	if (m_pLoadObject) m_pLoadObject->Release(); m_pLoadObject = NULL;
	if (m_pLoadName) free(m_pLoadName); m_pLoadName = NULL;
	if (m_pObject) m_pObject->Release(); m_pObject = NULL;
	if (m_pMesh) m_pMesh->Release(); m_pMesh = NULL;
	if (m_pMeshForMaterial) m_pMeshForMaterial->Release(); m_pMeshForMaterial = NULL;
	if (m_pBone) m_pBone->Release(); m_pBone = NULL;

	m_pLoadObject = pObject;
	m_pLoadObject->AddRef();
	m_pLoadName = wcsdup(pName);

	HRESULT h = CreateDefFile(pFilename);
	if (FAILED(h)) return h;

	assert(m_pFileIn);
	return m_pFileIn->Read(this, pFilename);
}

/////////////////////////////////////////////////////////////////////////////
// IFileLoader: Configuring the Input File

HRESULT _stdcall CFileLoader::PutCutPrefix(LPOLESTR p)
{
	if (m_pCutPrefix) delete [] m_pCutPrefix;
	size_t n = wcslen(p);
	m_pCutPrefix = new wchar_t[n + 2];
	wcscpy(m_pCutPrefix, p);
	m_pCutPrefix[n] = L' ';
	m_pCutPrefix[n+1] = 0;
	return S_OK;
}

HRESULT _stdcall CFileLoader::GetCutPrefix(/*[out, retval]*/ LPOLESTR *p)
{
	assert(p);
	*p = m_pCutPrefix;
	return S_OK;
}

HRESULT _stdcall CFileLoader::PutCutPrefixAuto(BOOL b)
{
	m_bCutPrefixAuto = b;
	return S_OK;
}

HRESULT _stdcall CFileLoader::GetCutPrefixAuto(/*[out, retval]*/ BOOL *p)
{
	assert(p);
	*p = m_bCutPrefixAuto;
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// IFileLoader: Configuring the Input File

HRESULT _stdcall CFileLoader::CreateDefFile(LPOLESTR pFilename)
{
	if (m_pFileIn) 
		return S_OK;
	return FWDevice()->CreateObject(L"FileIn", IID_IFileIn, (IFWUnknown**)&m_pFileIn);
}

HRESULT _stdcall CFileLoader::GetFile(/*[out, retval]*/ IFileIn **pp)
{
	*pp = m_pFileIn;
	return S_OK;
}

HRESULT _stdcall CFileLoader::PutFile(IFileIn *p)
{
	if (m_pFileIn) m_pFileIn->Release();
	m_pFileIn = p;
	if (m_pFileIn) m_pFileIn->AddRef();
	return S_OK;
}

