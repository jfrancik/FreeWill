// mesh.cpp : Defines the mesh object
//

#include "stdafx.h"
#include "mesh.h"

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// Class CMesh

CMesh::CMesh() 
{ 
	m_bOn = TRUE;
	m_pTransform = NULL;
	m_pVertexBuffer = NULL;
	m_pFaceBuffer = NULL;
	m_pDictionary = NULL;
	m_bHasBytes = false;
	m_pVertexBytes = m_pFaceBytes = NULL;
	m_nVertexFirst = m_nVertexNum = m_nVertexMaxSize  = m_nFaceFirst = m_nFaceNum = m_nFaceMaxSize = 0;
	m_pCtrl = NULL;
	m_bBWOn = false;
	m_pBWWeights = NULL;
	m_pBWIndices = NULL;
	m_nSubmeshNum = 0;
	m_pSubmeshLen = m_pSubmeshBones = NULL;
	m_pMaterial = NULL;
}

CMesh::~CMesh()
{ 
	if (m_pTransform) m_pTransform->Release();
	if (m_bHasBytes)
		Close();
	if (m_pVertexBuffer) m_pVertexBuffer->Release();
	if (m_pFaceBuffer) m_pFaceBuffer->Release();
	m_bHasBytes = false;
	BWDisposeStuff();
	if (m_pCtrl) delete [] m_pCtrl;
	if (m_pDictionary) m_pDictionary->Release();
	if (m_pSubmeshLen) delete [] m_pSubmeshLen;
	if (m_pSubmeshBones) delete [] m_pSubmeshBones;
	if (m_pBWWeights) delete [] m_pBWWeights; m_pBWWeights = NULL;
	if (m_pBWIndices) delete [] m_pBWIndices; m_pBWIndices = NULL;
	if (m_pMaterial) m_pMaterial->Release();
}

/////////////////////////////////////////////////////////////////////////////
// buffer initialization & query

HRESULT CMesh::GetBuffers(IMeshVertexBuffer **pV, IMeshFaceBuffer **pF)
{
	if (pV)
	{
		*pV = m_pVertexBuffer;
		if (*pV) (*pV)->AddRef();
	}
	if (pF)
	{
		*pF = m_pFaceBuffer;
		if (*pF) (*pF)->AddRef();
	}
	return S_OK;
}

HRESULT CMesh::PutBuffers(IMeshVertexBuffer *pVertexBuffer, IMeshFaceBuffer *pFaceBuffer)
{
	if (m_bHasBytes) return ERROR(FW_E_NOTREADY);	// cannot change buffers when bytes allocated

	if (pVertexBuffer) 
	{
		if (m_pVertexBuffer) m_pVertexBuffer->Release();
		m_pVertexBuffer = pVertexBuffer;
		m_pVertexBuffer->AddRef();
		m_pVertexBytes = NULL;
		m_nVertexFirst = m_nVertexNum = m_nVertexMaxSize  = 0;
		m_pVertexBuffer->GetCaps(MESH_VERTEXID_XYZ, 0, &m_offsetXYZ, &m_sizeXYZ);
		m_pVertexBuffer->GetCaps(MESH_VERTEXID_POINTSIZE, 0, &m_offsetPointSize, &m_sizePointSize);
		m_pVertexBuffer->GetCaps(MESH_VERTEXID_DIFFUSE, 0, &m_offsetDiffuse, &m_sizeDiffuse);
		m_pVertexBuffer->GetCaps(MESH_VERTEXID_SPECULAR, 0, &m_offsetSpecular, &m_sizeSpecular);
		for (int i = 0; i < 16; i++)
			m_pVertexBuffer->GetCaps(MESH_VERTEXID_TEXTURE, i, &m_offsetTexture[i], &m_sizeTexture[i]);
	}
	if (pFaceBuffer) 
	{
		if (m_pFaceBuffer) m_pFaceBuffer->Release();
		m_pFaceBuffer = pFaceBuffer;
		m_pFaceBuffer->AddRef();
		m_pFaceBytes = NULL;
		m_nFaceFirst = m_nFaceNum = m_nFaceMaxSize  = 0;
		FWULONG nFaceFormat;
		GetFaceFormat(&nFaceFormat);
		m_bFace32 = nFaceFormat == 4;
	}
	return S_OK;
}

HRESULT CMesh::GetDictionary(IMeshDictionary **p)
{
	*p = m_pDictionary;
	if (*p) (*p)->AddRef();
	return S_OK;
}

HRESULT CMesh::PutDictionary(IMeshDictionary *pDictionary)
{
	if (m_pDictionary) m_pDictionary->Release();
	m_pDictionary = pDictionary;
	if (m_pDictionary) 
		m_pDictionary->AddRef();
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// caps

HRESULT CMesh::GetVertexFormat(/*[out]*/ FWULONG *nFormat, /*[out]*/ FWULONG *nBones, /*[out]*/ FWULONG *nTextures, /*[out]*/ FWULONG *nVertexSize)
{
	if (!m_pVertexBuffer) return ERROR(FW_E_NOTREADY);
	m_pVertexBuffer->GetItemSize(nVertexSize);
	return m_pVertexBuffer->GetFormat(nFormat, nBones, nTextures);
}

HRESULT CMesh::GetVertexCaps(enum MESH_VERTEXID nFlag, FWULONG nIndex, /*[out]*/ FWULONG *pOffset, /*[out,retval]*/ FWULONG *pSize)
{
	if (!m_pVertexBuffer) return ERROR(FW_E_NOTREADY);
	return m_pVertexBuffer->GetCaps(nFlag, nIndex, pOffset, pSize);
}

HRESULT CMesh::GetFaceFormat(/*[out]*/ FWULONG *nSizeOfVertexIndex)
{
	if (!m_pFaceBuffer) return ERROR(FW_E_NOTREADY);
	if (!nSizeOfVertexIndex) return S_OK;
	m_pFaceBuffer->GetItemSize(nSizeOfVertexIndex);
	*nSizeOfVertexIndex /= 3;
	return S_OK;
}

HRESULT CMesh::SupportsVertexBlending()
{
	if (!m_pVertexBuffer) return ERROR(FW_E_NOTREADY);
	FWULONG nFormat;
	m_pVertexBuffer->GetFormat(&nFormat, NULL, NULL);
	if (nFormat & MESH_VERTEX_BONEWEIGHT)
		return S_OK;
	else
		return S_FALSE;
}

HRESULT CMesh::SupportsIndexedVertexBlending()
{
	if (!m_pVertexBuffer) return ERROR(FW_E_NOTREADY);
	FWULONG nFormat;
	m_pVertexBuffer->GetFormat(&nFormat, NULL, NULL);
	return ((nFormat & MESH_VERTEX_BONEWEIGHT) && (nFormat & MESH_VERTEX_BONEINDEX)) ? S_OK : S_FALSE;
}

HRESULT CMesh::SupportsSubmeshedVertexBlending()
{
	if (!m_pVertexBuffer) return ERROR(FW_E_NOTREADY);
	FWULONG nFormat;
	m_pVertexBuffer->GetFormat(&nFormat, NULL, NULL);
	return ((nFormat & MESH_VERTEX_BONEWEIGHT) && !(nFormat & MESH_VERTEX_BONEINDEX)) ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// support for submeshes

HRESULT CMesh::GetSubmeshInfo(FWULONG *pnSubmeshNum, FWULONG *pnBoneNum, FWULONG **pSubmeshLen, FWULONG **pSubmeshBones)
{
	if (SupportsSubmeshedVertexBlending() != S_OK)
		return ERROR(FW_E_FORMAT);
	if (!m_pSubmeshLen || !m_pSubmeshBones)
		return ERROR(FW_E_NOTREADY);

	if (pnSubmeshNum) *pnSubmeshNum = m_nSubmeshNum;
	if (pnBoneNum) *pnBoneNum = m_nBoneNum;
	if (pSubmeshLen) *pSubmeshLen = m_pSubmeshLen;
	if (pSubmeshBones) *pSubmeshBones = m_pSubmeshBones;
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// operations

HRESULT CMesh::Open(/*[out]*/ BYTE **ppVertexBytes, /*[out]*/ BYTE **ppFaceBytes)
{
	if (m_bHasBytes)
	{
		if (ppVertexBytes) *ppVertexBytes = m_pVertexBytes;
		if (ppFaceBytes) *ppFaceBytes = m_pFaceBytes;
		return S_OK;
	}

	if (!m_pVertexBuffer || !m_pFaceBuffer || m_bHasBytes) 
		return ERROR(FW_E_NOTREADY);
	
	HRESULT h;

	// get vertex bytes
	h = m_pVertexBuffer->OpenAppend(&m_pVertexBytes);
	if (FAILED(h)) return FW_E_CLASSSPECIFIC;

	// get face bytes
	h = m_pFaceBuffer->OpenAppend(&m_pFaceBytes);
	if (FAILED(h)) { m_pVertexBuffer->Close(0); return FW_E_CLASSSPECIFIC; }

	// set vertex params
	m_pVertexBuffer->GetDataSize(&m_nVertexFirst);
	m_nVertexNum = 0;
	m_pVertexBuffer->GetFreeSize(&m_nVertexMaxSize);
	m_pVertexBuffer->GetItemSize(&m_nVertexItemSize);
	if (ppVertexBytes) *ppVertexBytes = m_pVertexBytes;

	// set face params
	m_pFaceBuffer->GetDataSize(&m_nFaceFirst);
	m_nFaceNum = 0;
	m_pFaceBuffer->GetFreeSize(&m_nFaceMaxSize);
	m_pFaceBuffer->GetItemSize(&m_nFaceItemSize);
	if (ppFaceBytes) *ppFaceBytes = m_pFaceBytes;

	m_bHasBytes = true;

	return S_OK;
}

HRESULT CMesh::Close()
{
	HRESULT h = S_OK, h1;

	if (!m_bHasBytes) return ERROR(FW_E_NOTREADY);

	if (m_bBWOn) h = BWFinalize();	// h is not critical error here!

	h1 = m_pVertexBuffer->Close(m_nVertexNum); if (FAILED(h1)) return FW_E_CLASSSPECIFIC;
	h1 = m_pFaceBuffer->Close(m_nFaceNum); if (FAILED(h1)) return FW_E_CLASSSPECIFIC;
	if (m_pCtrl) delete [] m_pCtrl;
	m_pCtrl = NULL;

	m_nVertexMaxSize = m_nFaceMaxSize = 0;
	m_pVertexBytes = m_pFaceBytes = NULL;
	m_bHasBytes = false;

	return h;
}

/////////////////////////////////////////////////////////////////////////////
// The Mesh Transform

HRESULT CMesh::GetTransform(ITransform **pVal)
{
	assert(pVal);
	*pVal = m_pTransform;
	if (*pVal) (*pVal)->AddRef();
	return S_OK;
}

HRESULT CMesh::PutTransform(ITransform *p)
{
	assert(p);
	if (m_pTransform) m_pTransform->Release();
	m_pTransform = NULL;
	if (!p) return S_OK;
	m_pTransform = (ITransform*)p->Clone(IID_ITransform);
	return p->AsTransform(m_pTransform);
}

/////////////////////////////////////////////////////////////////////////////
// support for general data

HRESULT CMesh::SetVertexXYZ(FWULONG iVertex, FWFLOAT x, FWFLOAT y, FWFLOAT z)
{
	FWVECTOR v = { x, y, z};
	return SetVertexXYZVector(iVertex, v);
}

HRESULT CMesh::SetVertexXYZVector(FWULONG iVertex, FWVECTOR v)
{
	if (!m_bHasBytes) return ERROR(FW_E_NOTREADY);
	FWFLOAT *p;
	switch (m_sizeXYZ)
	{
	case 12:
		p = (FWFLOAT*)(m_pVertexBytes + iVertex * m_nVertexItemSize + m_offsetXYZ);
		if (m_pTransform) m_pTransform->ApplyTo(&v);
		*p++ = v.x;
		*p++ = v.y;
		*p++ = v.z;
		m_nVertexNum = max(m_nVertexNum, iVertex + 1);
		break;
	default:
		return ERROR(FW_E_FORMAT);
	}
	return S_OK;
}

HRESULT CMesh::SetVertexPointSize(FWULONG iVertex, FWULONG val)
{
	if (!m_bHasBytes) return ERROR(FW_E_NOTREADY);
	switch (m_sizePointSize)
	{
	case 4:
		*((FWULONG*)(m_pVertexBytes + iVertex * m_nVertexItemSize + m_offsetPointSize)) = val;
		m_nVertexNum = max(m_nVertexNum, iVertex + 1);
		break;
	case 2:
		*((USHORT*)(m_pVertexBytes + iVertex * m_nVertexItemSize + m_offsetPointSize)) = (USHORT)val;
		m_nVertexNum = max(m_nVertexNum, iVertex + 1);
		break;
	default:
		return ERROR(FW_E_FORMAT);
	}
	return S_OK;
}

HRESULT CMesh::SetVertexDiffuse(FWULONG iVertex, FWULONG val)
{
	if (!m_bHasBytes) return ERROR(FW_E_NOTREADY);
	switch (m_sizeDiffuse)
	{
	case 4:
		*((FWULONG*)(m_pVertexBytes + iVertex * m_nVertexItemSize + m_offsetDiffuse)) = val;
		m_nVertexNum = max(m_nVertexNum, iVertex + 1);
		break;
	default:
		return ERROR(FW_E_FORMAT);
	}
	return S_OK;
}

HRESULT CMesh::SetVertexSpecular(FWULONG iVertex, FWULONG val)
{
	if (!m_bHasBytes) return ERROR(FW_E_NOTREADY);
	switch (m_sizeSpecular)
	{
	case 4:
		*((FWULONG*)(m_pVertexBytes + iVertex * m_nVertexItemSize + m_offsetSpecular)) = val;
		m_nVertexNum = max(m_nVertexNum, iVertex + 1);
		break;
	default:
		return ERROR(FW_E_FORMAT);
	}
	return S_OK;
}

HRESULT CMesh::SetVertexTexture(FWULONG iVertex, FWULONG iTexture, FWULONG val)
{
	if (!m_bHasBytes) return ERROR(FW_E_NOTREADY);
	switch (m_sizeTexture[iTexture])
	{
	case 4:
		*((FWULONG*)(m_pVertexBytes + iVertex * m_nVertexItemSize + m_offsetTexture[iTexture])) = val;
		m_nVertexNum = max(m_nVertexNum, iVertex + 1);
		break;
	case 2:
		*((USHORT*)(m_pVertexBytes + iVertex * m_nVertexItemSize + m_offsetTexture[iTexture])) = (USHORT)val;
		m_nVertexNum = max(m_nVertexNum, iVertex + 1);
		break;
	default:
		return ERROR(FW_E_FORMAT);
	}
	return S_OK;
}

HRESULT CMesh::SetVertexTextureUV(FWULONG iVertex, FWULONG iTexture, FWFLOAT u, FWFLOAT v)
{
	if (m_sizeTexture[iTexture] < 2*sizeof(FWFLOAT))
		return ERROR(FW_E_FORMAT);		
	if (iTexture < 16)
	{
		FWFLOAT* pTexUV = (FWFLOAT*)
			(m_pVertexBytes + iVertex*m_nVertexItemSize + m_offsetTexture[iTexture]);
		pTexUV[0] = u;
		pTexUV[1] = v;
		return S_OK;
	}
	return S_FALSE;
}

HRESULT CMesh::SetMaterial(IMaterial *pMaterial)
{
	if (m_pMaterial)
	{
		m_pMaterial->Release();
		m_pMaterial = NULL;
	}
	if (pMaterial != NULL)
	{
		m_pMaterial = pMaterial;
		m_pMaterial->AddRef();
	}
	return S_OK;
}

HRESULT CMesh::GetMaterial(IMaterial **ppMaterial)
{
	if (ppMaterial)
	{
		*ppMaterial = m_pMaterial;
		if (*ppMaterial != NULL)
			(*ppMaterial)->AddRef();
		return S_OK;
	}
	return S_FALSE;
}

HRESULT CMesh::SetFace(FWULONG iFace, FWULONG iVertexA, FWULONG iVertexB, FWULONG iVertexC)
{
	if (!m_bHasBytes) return ERROR(FW_E_NOTREADY);
	if (m_bFace32)
	{
		FWULONG *p = ((FWULONG*)m_pFaceBytes) + 3 * iFace;
		*p++ = iVertexA;
		*p++ = iVertexB;
		*p++ = iVertexC;
		m_nFaceNum = max(m_nFaceNum, iFace + 1);
	}
	else
	{
		USHORT *p = ((USHORT*)m_pFaceBytes) + 3 * iFace;
		*p++ = (USHORT)iVertexA;
		*p++ = (USHORT)iVertexB;
		*p++ = (USHORT)iVertexC;
		m_nFaceNum = max(m_nFaceNum, iFace + 1);
	}
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// support for normals

HRESULT CMesh::SupportNormal(FWULONG nLimit)
{
	if (!m_bHasBytes) return ERROR(FW_E_NOTREADY);

	// set vertex size & normal offset
	m_pVertexBuffer->GetCaps(MESH_VERTEXID_NORMAL, 0, &m_nNormalOffset, NULL);
	// determine control buffer length
	m_nCtrlSize = nLimit;
	if (m_nCtrlSize == 0) m_nCtrlSize = m_nVertexMaxSize;
	if (m_nCtrlSize < m_nVertexNum) return ERROR(FW_E_INVALIDARG);
	if (m_nCtrlSize > m_nVertexMaxSize) return ERROR(FW_E_MEMOVERRUN);
	m_pCtrl = new FWULONG[m_nCtrlSize];
	if (!m_pCtrl) return ERROR(FW_E_OUTOFMEMORY);
	memset(m_pCtrl, 0xff, m_nCtrlSize * sizeof(FWULONG));
	return S_OK;
}

HRESULT CMesh::AddNormal(/*[in, out]*/ FWULONG *index, FWFLOAT x, FWFLOAT y, FWFLOAT z)
{
	FWVECTOR v = { x, y, z};
	return AddNormalVector(index, v);
}

HRESULT CMesh::AddNormalVector(/*[in, out]*/ FWULONG *index, FWVECTOR v)
{
	if (!m_bHasBytes || m_pCtrl == NULL) return ERROR(FW_E_NOTREADY);

	if (m_pTransform) m_pTransform->ApplyRotationTo(&v);

	BYTE *pVertex = m_pVertexBytes + *index * m_nVertexItemSize;
	FWFLOAT *p = (FWFLOAT*)(pVertex + m_nNormalOffset);

	while (m_pCtrl[*index] != 0xffffffff)
	{
		// check the normal already written
		if (p[0] == v.x && p[1] == v.y && p[2] == v.z)
			return S_OK;	// this normal already added; nothing more to do

		// test: create a new vertex synonim?
		if (m_pCtrl[*index] == *index)
		{
			// it was the last synonim; create a new one
			if (m_nVertexNum + 1 > m_nCtrlSize)
				return ERROR(FW_E_MEMOVERRUN);
			m_pCtrl[*index] = m_nVertexNum;
			*index = m_nVertexNum;
			m_nVertexNum++;
			BYTE *qVertex = m_pVertexBytes + *index * m_nVertexItemSize;
			FWFLOAT *q = (FWFLOAT*)(qVertex + m_nNormalOffset);
			memcpy(qVertex, pVertex, m_nVertexItemSize);
			p = q; pVertex = qVertex;
			break;
		}
		else
		{
			*index = m_pCtrl[*index];
			pVertex = m_pVertexBytes + *index * m_nVertexItemSize;
			p = (FWFLOAT*)(pVertex + m_nNormalOffset);
		}
	}
	*p++ = v.x;	// we assume that normal x, y, z are stored as three consecutive floats
	*p++ = v.y;
	*p++ = v.z;
	m_pCtrl[*index] = *index;
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// support for blend weights

HRESULT CMesh::SupportBlendWeight(FWFLOAT fMinWeight, FWULONG nMinVertexNum)
{
	if (!m_bHasBytes) return ERROR(FW_E_NOTREADY);

	BWDisposeStuff();
	FWULONG nFormat, nBones;
	m_pVertexBuffer->GetFormat(&nFormat, &nBones, NULL);
	if (!(nFormat & MESH_VERTEX_BONEWEIGHT) || nBones < 2)
		return ERROR(FW_E_FORMAT);

	m_bBWOn = SupportsVertexBlending() == S_OK;
	if (!m_bBWOn) return ERROR(FW_E_FORMAT);

	m_nBWVertexNum = max(nMinVertexNum, m_nVertexNum);
	m_nBoneNum = nBones;
	m_pBWWeights = new FWFLOAT[m_nBWVertexNum * m_nBoneNum];
	memset(m_pBWWeights, 0, sizeof(FWFLOAT) * m_nBWVertexNum * m_nBoneNum);
	m_pBWIndices = new FWULONG[m_nBWVertexNum * m_nBoneNum];
	m_fBWMinWeight = fMinWeight;
	return S_OK;
}

HRESULT CMesh::AddBlendWeight(FWULONG iVertex, FWFLOAT fWeight, LPOLESTR pBoneName)
{
	if (!m_bBWOn || !m_pDictionary) return ERROR(FW_E_NOTREADY);
	if (iVertex >= m_nBWVertexNum) return ERROR(FW_E_MEMOVERRUN);
	if (fWeight <= m_fBWMinWeight) return S_OK;	// ignore very small weights

	// add new weight/index in a free place at pWeights/pIndices[iVertex]
	// if no free place: remove the minimum

	// determine bone index
	FWULONG iBone;
	m_pDictionary->GetIndex(pBoneName, &iBone);

	// find minimum (it'll contain 0 if it there's a free place)
	FWFLOAT *pWeight = m_pBWWeights + iVertex * m_nBoneNum, *pMinWeight = pWeight, fMin = *pMinWeight;
	FWULONG *pIndex  = m_pBWIndices + iVertex * m_nBoneNum, *pMinIndex  = pIndex;
	if (*pIndex == iBone)
	{	// earlier entry for the same bone - will be cumulated... @@@ 25/06/2004
		*pWeight += fWeight;
		return S_OK;
	}
	pWeight++, pIndex++;
	for (FWULONG i = 1; fMin > 0.0f && i < m_nBoneNum; i++, pWeight++, pIndex++)
		if (*pIndex == iBone)
		{	// earlier entry for the same bone - will be cumulated...  @@@ 25/06/2004
			*pWeight += fWeight;
			return S_OK;
		}
		else
		if (*pWeight < fMin)
		{
			pMinWeight = pWeight;
			pMinIndex = pIndex;
			fMin = *pMinWeight;
		}
	if (fWeight > fMin) // if fWeight is min - ignore
	{
		*pMinWeight = fWeight;
		*pMinIndex  = iBone;
	}

	return S_OK;
}

HRESULT CMesh::BWFinalize()
{
	////////////////////////////
	// General-Purpose Variables
	FWULONG i, j, k, l;
	FWFLOAT *pFloat, *qFloat;
	FWULONG *pULong, *qULong;
	const FWULONG nFaceSize = m_bFace32 ? 12 : 6;					// size of face item
	const FWULONG nBonesFloatSize = sizeof(FWFLOAT) * m_nBoneNum;	// face of FWFLOAT array of bones
	const FWULONG nBonesULongSize = sizeof(FWULONG) * m_nBoneNum;	// face of FWULONG array of bones

	///////////////////////////
	// Introductory Preparation

	// normalize bone weight sums to 1.0f
	for (i = 0; i < m_nBWVertexNum; i++)
		BWNormalizeVertex(i);

	// reallocate buffers if necessary
	if (m_nVertexNum > m_nBWVertexNum)
	{
		pFloat = new FWFLOAT[m_nVertexNum * m_nBoneNum];
		memcpy(pFloat, m_pBWWeights, m_nBWVertexNum * nBonesFloatSize);
		delete [] m_pBWWeights; 
		m_pBWWeights = pFloat;
		pULong = new FWULONG[m_nVertexNum * m_nBoneNum];
		memcpy(pULong, m_pBWIndices, m_nBWVertexNum * nBonesULongSize);
		delete [] m_pBWIndices; 
		m_pBWIndices = pULong;
	}

	// this is the number of vertices used throughout
	m_nBWVertexNum = m_nVertexNum;

	// copy blend data for vertices multiplied during AddNormal
	// use m_pCtrl buffer
	if (m_pCtrl)
	{
		FWULONG nCtrlSize = min(m_nCtrlSize, m_nBWVertexNum);
		FWULONG *pCtrl = m_pCtrl;
		for (i = 0; i < nCtrlSize; i++, pCtrl++)
		{
			pULong = pCtrl;
			j = i;
			while (*pULong != j && *pULong != 0xffffffff)
			{
				// vertex[*pULong] is a copy of vertex[j]
				memcpy(m_pBWWeights + *pULong * m_nBoneNum, m_pBWWeights + j * m_nBoneNum, nBonesFloatSize);
				memcpy(m_pBWIndices + *pULong * m_nBoneNum, m_pBWIndices + j * m_nBoneNum, nBonesULongSize);
				j = *pULong;
				pULong = m_pCtrl + j;
			}
		}
	}

	//////////////////////////////////////
	// For Indexed Blending That's The End
	if (SupportsIndexedVertexBlending() == S_OK)
	{
		HRESULT h1 = BWCopyToBufIndexed();
		HRESULT h2 = BWDisposeStuff();
		return FAILED(h1) ? h1 : h2;
	}

	////////////////////////////////////////////////////////////
	// Normalize Faces: reduce number of bones for each triangle
	// Max number of bones PER triangle is m_nBoneNum, but no less than three

	FWULONG nMaxBonesPerTriangle = (m_nBoneNum >= 3) ? m_nBoneNum : 3;

	pFloat = qFloat = new FWFLOAT[m_nBoneNum * 3];	// pFloat..qFloat - array of weights used by this triangle
	pULong = qULong = new FWULONG[m_nBoneNum * 3];	// pULong..qULong - array of indices used by this triangle

	for (i = 0; i < m_nFaceNum; i++)	// for each triangle face
	{
		// the normalization loop - repeat until the face depends on no more than m_nBoneNum
		do
		{
			// scan thru all three vertices & their all bones to collect all distinctive bones
			// in result we obtain the number of bones this face depends and a list of them
			// pFloat/pULong = start the arrays of bones; qFloat/qLong = consecutive array ends
			qFloat = pFloat;
			qULong = pULong;
			for (j = 0; j < 3; j++)	// for each vertex
			{
				FWULONG iVertex = BWVertexFromFace(i, j);
				for (k = 0; k < m_nBoneNum; k++)	// for each bone
				{
					FWFLOAT fWeight = m_pBWWeights[iVertex * m_nBoneNum + k];
					FWULONG nIndex  = m_pBWIndices[iVertex * m_nBoneNum + k];
					if (fWeight == 0) continue;
					// do we already have this bone index?
					FWFLOAT *pF;
					FWULONG *pU;
					for (pU = pULong, pF = pFloat; pU < qULong && *pU != nIndex; pU++, pF++)
						;
					if (pU < qULong)
					{		// yes, we have; replace if weight higher than stored
						if (fWeight <= *pF)
							continue;
					}
					else	// no, that's a new bone to this triangle
						qULong++, qFloat++;
					*pF = fWeight;
					*pU = nIndex;
				}
			}

			// if the triangle does not exceed acceptable number of bones
			if ((FWULONG)(qFloat - pFloat) <= nMaxBonesPerTriangle)
				break;	// break the while(1) loop -> task is done for this face

			// find the minmum weight
			FWFLOAT *pMin = pFloat;
			for (FWFLOAT *pF = pFloat + 1; pF < qFloat; pF++)
				if (*pF && *pF < *pMin) 
					pMin = pF;
			assert(*pMin < 1.0f || pMin == pFloat);

			// at what index to delete?
			FWULONG iToDel = *(pULong + (pMin - pFloat));

			// if the last bone of a vertex is to be reduced...
			assert(*pMin < 1.0f);

			// replace delete the min weight in all vertices
			for (j = 0; j < 3; j++)	// for each vertex
			{
				// @@@ One usefull idea would be to delete here all bones [significantly] 
				// weaker then the minimum taken from the triangle - why to leave it if sth stronger is deleted...
				FWULONG iVertex = BWVertexFromFace(i, j);
				for (k = 0; k < m_nBoneNum; k++)	// for each bone
					if (m_pBWIndices[iVertex * m_nBoneNum + k] == iToDel)
						m_pBWWeights[iVertex * m_nBoneNum + k] = 0.0f;
				BWNormalizeVertex(iVertex);
			}
			qFloat--;
		} while ((FWULONG)(qFloat - pFloat) > nMaxBonesPerTriangle);
	}
	delete [] pFloat;
	delete [] pULong;

	////////////////////////
	// Prepare Per-Face Data

	// This is a very similar step as the face normalization.
	// This time the indices are stored in a global arrays below; weights are out of interest
	// We can assert that faces have no more triangles than they should have (they've been normalized)
	// This step is indispensable: the task could not be completed during normalization, as
	// the normalization of consecutive faces could affect data belonging to previous faces.

	FWULONG *pPerFaceNum = new FWULONG[m_nFaceNum];					// number of different bone indices
	FWULONG *pPerFaceInd = new FWULONG[m_nFaceNum * m_nBoneNum];	// bone indices used in this face
	memset(pPerFaceInd, 0xffffffff, m_nFaceNum * nBonesULongSize);
		// notice: faces are already normalised, therefore it is safe to use m_nBoneNum positions per face

	FWULONG *pNum = pPerFaceNum;
	for (i = 0; i < m_nFaceNum; i++, pNum++)	// for each triangle face
	{
		// scan thru all three vertices & their all bones to collect all distinctive bones
		FWULONG *pInd = pPerFaceInd + i * m_nBoneNum, *qInd = pInd;
		*pNum = 0;
		for (j = 0; j < 3; j++)	// for each vertex
		{
			FWULONG iVertex = BWVertexFromFace(i, j);
			for (k = 0; k < m_nBoneNum; k++)	// for each bone
				if (m_pBWWeights[iVertex * m_nBoneNum + k])		// positive weight
				{
					FWULONG nIndex  = m_pBWIndices[iVertex * m_nBoneNum + k];
					FWULONG *p;
					for (p = pInd; p < qInd && *p != nIndex; p++)
						;
					if (p >= qInd)
						*qInd++ = nIndex, (*pNum)++;
				}
		}
		assert(*pNum <= m_nBoneNum);	// was deleted, i dont know why
	}

	/////////////////
	// Find Submeshes

	// faces are sorted in order to form a sequence of submeshes using no more than m_nBoneNum bones each
	// Input: 
	//	m_pFaceBytes - buffer of faces
	//	pPerFaceNum, pPerFaceInd - per face data (total number and indices of bones)
	// Output:
	//	pSubmeshLen, pSubmeshInd - submeshes: number of faces, indices of bones

	// output buffers are filled not faster than input buffers are used;
	// therefore input buffers may be re-used; no need to allocate more memory
	FWULONG *pSubmeshLen = pPerFaceNum;	// number of faces per submesh
	FWULONG *pSubmeshInd = pPerFaceInd;	// bone indices per submesh
	FWULONG nCurBones = 0;				// number of bones in currently collected mesh

	// current pointers (currently collected submesh)
	FWULONG *qSubmeshLen = pSubmeshLen;
	FWULONG *qSubmeshInd = pSubmeshInd;

	i = 0;								// first unsorted face
	FWULONG *iPerFaceNum = pPerFaceNum;	// ...its per-face data
	FWULONG *iPerFaceInd = pPerFaceInd;

	// create the first submesh using the i'th face
	i++, iPerFaceNum++, iPerFaceInd += m_nBoneNum;
	*qSubmeshLen = 1;
	// memcpy(qSubmeshInd, iPerFaceInd, nBonesULongSize);
	// notice that the last line is unnecessary as qSubmeshInd == pSubmeshInd
	nCurBones = *pPerFaceNum;
	FWULONG iCandidate = 0;				// index of a candidate face
	FWULONG nCandScore = 0;				// score of the candidate face

	while (i < m_nFaceNum)
	{
		FWULONG *jPerFaceNum = iPerFaceNum;
		FWULONG *jPerFaceInd = iPerFaceInd;

		// browse all unsorted faces
		for (j = i; j < m_nFaceNum; j++, jPerFaceNum++, jPerFaceInd += m_nBoneNum)
		{
			// measure compatibility factor for the current face
			FWULONG nCompIn = 0;	// how many compatible bones
			FWULONG nCompEx = 0;	// how many incompatible bones
			FWULONG nCompScore;	// overall score
			for (k = 0; k < *jPerFaceNum; k++)
				for (l = 0; l < nCurBones; l++)
					if (jPerFaceInd[k] == qSubmeshInd[l])
					{
						nCompIn++;
						break;
					}
			nCompEx = *jPerFaceNum - nCompIn;
			nCompScore = nCompIn * 256 + nCompEx;
			if (nCurBones + nCompEx > m_nBoneNum) nCompScore = 0;

			// check if the face may be a part of the current submesh
			if (nCompEx == 0)
			{
				// if the face belongs to the submesh, add it...
				if (i != j)
				{
					// swap i and j
					static BYTE pAux[3 * 4];	// enough space for 32-bit face item
					memcpy(pAux, m_pFaceBytes + i * nFaceSize, nFaceSize);
					memcpy(m_pFaceBytes + i * nFaceSize, m_pFaceBytes + j * nFaceSize, nFaceSize);
					memcpy(m_pFaceBytes + j * nFaceSize, pAux, nFaceSize);
					*jPerFaceNum = *iPerFaceNum;
					memcpy(jPerFaceInd, iPerFaceInd, nBonesULongSize);
					if (i == iCandidate) iCandidate = j;	// correct iCandidate if necessary
				}
				i++, iPerFaceNum++, iPerFaceInd += m_nBoneNum;
				(*qSubmeshLen)++;
			}
			else 
			// check: maybe it's a good candidate for mesh extension
			if (nCompScore > nCandScore)
				iCandidate = j, nCandScore = nCompScore;
		}

		// if there is a candidate for submesh extension
		if (nCurBones < m_nBoneNum && nCandScore)
		{
			// extend the submesh with the candidate definition
			FWULONG nCandNum = pPerFaceNum[iCandidate];					// candidate's no of bones
			FWULONG *pCandInd = pPerFaceInd + iCandidate * m_nBoneNum;	// candidate's bone indices
			for (k = 0; k < nCandNum; k++)
			{
				for (l = 0; l < nCurBones && pCandInd[k] != qSubmeshInd[l]; l++)
					;
				if (l >= nCurBones)
					qSubmeshInd[nCurBones++] = pCandInd[k];
			}
		}
		else
		if (i < m_nFaceNum)	// @@@ this line added: 3 June 2005
		{
			// close this submesh
			qSubmeshLen++;
			qSubmeshInd += m_nBoneNum;
			// open a new one
			*qSubmeshLen = 0;
			memcpy(qSubmeshInd, iPerFaceInd, nBonesULongSize);
			nCurBones = *iPerFaceNum;
		}

		assert(i < m_nFaceNum || nCandScore == 0);

		iCandidate = 0;
		nCandScore = 0;
	}

	// close the final submesh
	qSubmeshLen++;
	qSubmeshInd += m_nBoneNum;

	/////////////////////////
	// Reconfigure Blend Data

	m_nSubmeshNum = (FWULONG)(qSubmeshLen - pSubmeshLen);
	m_pSubmeshLen = new FWULONG[m_nSubmeshNum];
	m_pSubmeshBones = new FWULONG[m_nSubmeshNum * m_nBoneNum];
	memcpy(m_pSubmeshLen, pSubmeshLen, m_nSubmeshNum * sizeof(FWULONG));
	memcpy(m_pSubmeshBones, pSubmeshInd, m_nSubmeshNum * nBonesULongSize);

	// release
	delete [] pPerFaceNum;
	delete [] pPerFaceInd;

	//////////////////////////////////////
	// Finally: Copy data & Dispose
	HRESULT h1 = BWCopyToBufSubmeshed();
	HRESULT h2 = BWDisposeStuff();
	return FAILED(h1) ? h1 : h2;
}

HRESULT CMesh::BWCopyToBufIndexed()
{
	m_pVertexBuffer->GetItemSize(&m_nVertexItemSize);

//	FWULONG *pWeightsOffs;	// weight position offsets in vertex structure (nBones-1 positions)
//	FWULONG *pIndicesOffs;	// index  position offsets in vertex structure (nBones-1 positions)
//	FWULONG nIndexSize;		// size of index position in a vertex structure // note: weight size assumed as sizeof(FWFLOAT)

	return ERROR(E_NOTIMPL);
}

HRESULT CMesh::BWCopyToBufSubmeshed()
{
	FWULONG i, j, k, l;
	m_pVertexBuffer->GetItemSize(&m_nVertexItemSize);

	// find addresses for bone weights
	FWULONG *pOffset = new FWULONG[m_nBoneNum];
	for (i = 0; i < m_nBoneNum; i++)
	{
		FWULONG nOffset, nSize;
		m_pVertexBuffer->GetCaps(MESH_VERTEXID_BONEWEIGHT, i, &nOffset, &nSize);
		pOffset[i] = nSize ? nOffset : 0xffffffff;
	}

	// control buffers - for consistency check
	// indicates submesh for which the value has been set; 0xff to indicate no value yet
	FWULONG *pCtrl = new FWULONG[m_nVertexMaxSize];
	memset(pCtrl, 0xff, sizeof(FWULONG) * m_nVertexMaxSize);
	// creates a list of synonims, 0 used as nil
	FWULONG *pSynm = new FWULONG[m_nVertexMaxSize];
	memset(pSynm, 0x00, sizeof(FWULONG) * m_nVertexMaxSize);

	FWULONG *pSubmeshLen = m_pSubmeshLen;
	FWULONG *pSubmeshBones = m_pSubmeshBones;
	FWULONG iFace = 0;
	// for each submesh
	for (i = 0; i < m_nSubmeshNum; i++)
	{
		// map: bone index to offset
		static FWULONG map[256];
		memset(map, 0xff, sizeof(map));
		FWULONG *p = pSubmeshBones;
		for (j = 0; j < m_nBoneNum; j++, p++)
			if (*p < 256)
				map[*p] = pOffset[j];

		// for each face
		for (j = 0; j < *pSubmeshLen; j++)
		{
			// for each vertex
			for (k = 0; k < 3; k++)
			{
				FWULONG iVertex = BWVertexFromFace(iFace, k);
				BYTE *pVertex = m_pVertexBytes +  iVertex * m_nVertexItemSize;

				FWFLOAT *pWeights = m_pBWWeights + iVertex * m_nBoneNum;
				FWULONG *pIndices = m_pBWIndices + iVertex * m_nBoneNum;

				// check if consistent
				while (pCtrl[iVertex] != 0xffffffff && pCtrl[iVertex] != i)
					if (pSynm[iVertex])
						iVertex = pSynm[iVertex];
					else
					{
						// create synonim
						if (m_nVertexNum + 1 > m_nVertexMaxSize)
						{
							delete [] pOffset; delete [] pCtrl; delete [] pSynm;
							return ERROR(FW_E_MEMOVERRUN);
						}
						pSynm[iVertex] = m_nVertexNum;
						iVertex = m_nVertexNum;
						m_nVertexNum++;
						BYTE *p = m_pVertexBytes +  iVertex * m_nVertexItemSize;
						memcpy(p, pVertex, m_nVertexItemSize);
						assert(pCtrl[iVertex] == 0xffffffff);
					}

				pVertex = m_pVertexBytes +  iVertex * m_nVertexItemSize;
				BWVertexToFace(iFace, k, iVertex);

				if (pCtrl[iVertex] != 0xffffffff)
					continue;
				pCtrl[iVertex] = i;

				// reset all bone weights
				for (l = 0; l < m_nBoneNum; l++)
					if (pOffset[l] != 0xffffffff)
						*(FWFLOAT*)(pVertex + pOffset[l]) = 0.0f;

				// set weight for each submesh bone
				for (l = 0; l < m_nBoneNum; l++, pWeights++, pIndices++)
				{
					if (*pWeights == 0.0f) continue;
					FWULONG offset = map[*pIndices];
					if (offset == 0xffffffff) continue;
					*(FWFLOAT*)(pVertex + map[*pIndices]) = *pWeights;
				}
			}
			iFace++;
		}
		pSubmeshLen++;
		pSubmeshBones += m_nBoneNum;
	}

	delete [] pOffset;
	delete [] pCtrl;
	delete [] pSynm;

	return S_OK;
}

HRESULT CMesh::BWDisposeStuff()
{
	if (!m_bBWOn) return S_OK;
	m_bBWOn = false;
	if (m_pBWWeights) delete [] m_pBWWeights; m_pBWWeights = NULL;
	if (m_pBWIndices) delete [] m_pBWIndices; m_pBWIndices = NULL;
	return S_OK;
}

void CMesh::BWNormalizeVertex(FWULONG iVertex)
{
	FWFLOAT norm = 0.0f;
	FWFLOAT *p = m_pBWWeights + iVertex * m_nBoneNum;
	FWFLOAT *q = p;
	FWULONG j;
	for (j = 0; j < m_nBoneNum; j++) 
		norm += *p++;
	if (norm - 1.0f < 4.0e-7f && norm - 1.0f > -4.0e-7f) 
		return;
	norm = 1.0f / norm;
	for (j = 0; j < m_nBoneNum; j++) 
	{
		*q *= norm;
		if (*q > 0.99999 && *q < 1.000001) *q = 1.0f;
		q++;
	}
}

FWULONG CMesh::BWVertexFromFace(FWULONG iFace, FWULONG iVertex)
{
	if (m_bFace32)
		return ((FWULONG*)m_pFaceBytes)[iFace * 3 + iVertex];
	else
		return ((short*)m_pFaceBytes)[iFace * 3 + iVertex];
}

void CMesh::BWVertexToFace(FWULONG iFace, FWULONG iVertex, FWULONG iValue)
{
	if (m_bFace32)
		((FWULONG*)m_pFaceBytes)[iFace * 3 + iVertex] = iValue;
	else
		((short*)m_pFaceBytes)[iFace * 3 + iVertex] = (short)iValue;
}

