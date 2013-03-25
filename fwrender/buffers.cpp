// buffers.cpp : Defines DX-based buffers
// uses DirectX 9

#include "stdafx.h"
#include "renderer.h"
#include "buffers.h"

#include <D3d9.h>
#include <dxerr.h>

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}
/* IID_IDirect3DVertexBuffer9 */
/* {B64BB1B5-FD70-4df6-BF91-19D0A12455E3} */
MIDL_DEFINE_GUID(IID, IID_IDirect3DVertexBuffer9, 0xb64bb1b5, 0xfd70, 0x4df6, 0xbf, 0x91, 0x19, 0xd0, 0xa1, 0x24, 0x55, 0xe3);
/* IID_IDirect3DIndexBuffer9 */
/* {7C9DD65E-D3F7-4529-ACEE-785830ACDE35} */
MIDL_DEFINE_GUID(IID, IID_IDirect3DIndexBuffer9, 0x7c9dd65e, 0xd3f7, 0x4529, 0xac, 0xee, 0x78, 0x58, 0x30, 0xac, 0xde, 0x35);
#undef MIDL_DEFINE_GUID

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// Class CMeshDX9VertexBuffer

/////////////////////////////////////////////////////////////////////////////
// Error Handling

#define D3D_ERROR ErrorSrc(__WFILE__, __LINE__), D3DError

HRESULT CMeshDX9VertexBuffer::D3DError(HRESULT nD3DErrorCode)
{
	if (SUCCEEDED(nD3DErrorCode)) return nD3DErrorCode;
	wchar_t pBuf[256];
	_snwprintf(pBuf, 256, L"%ls (0x%x): %ls", DXGetErrorString(nD3DErrorCode), nD3DErrorCode, DXGetErrorDescription(nD3DErrorCode));
	FWULONG N = (FWULONG)(__int64)pBuf;
	return Error(REND_E_PLATFORM_EX, 1, &N);
}

///////////////////////////////////////////////////////////
// CMeshDX9VertexBuffer: IUnknown implementation

HRESULT _stdcall CMeshDX9VertexBuffer::PutContextObject(FWULONG index, REFIID iid, void *pUnknown)
{
	if (!pUnknown) return ERROR(FW_E_POINTER);
	if FAILED(((IUnknown*)pUnknown)->QueryInterface(&m_pDevice)) return ERROR(FW_E_POINTER);
	return S_OK;
}

HRESULT _stdcall CMeshDX9VertexBuffer::GetContextObject(FWULONG index, /*[out]*/ IID *pIID, /*[out]*/ void **pUnknown)
{
	if (pIID) *pIID = IID_IDirect3DVertexBuffer9;
	*pUnknown = m_pBuffer;
	if (m_pBuffer) m_pBuffer->AddRef();
	return S_OK;
}

HRESULT _stdcall CMeshDX9VertexBuffer::BeforeRendering()
{
	HRESULT h = S_OK;
	if (m_pDevice)
		m_pDevice->SetFVF(m_fvf); 
	if (FAILED(h)) return D3D_ERROR(h);
	return h;
}

HRESULT _stdcall CMeshDX9VertexBuffer::AfterRendering()
{
	return S_OK;
}

HRESULT _stdcall CMeshDX9VertexBuffer::Open(enum MESH_OPENMODES mode, FWULONG nFirst, FWULONG nSize, /*[out, retval]*/ BYTE **ppBytes)
{
	if (!m_pBuffer) return ERROR(FW_E_NOTREADY);
	DWORD nFlags = 0;
	switch (mode)
	{
	case MESH_OPEN_READ:
		if (nSize == 0) nSize = m_nBufSize - nFirst;
		nFlags = D3DLOCK_READONLY;
		break;
	case MESH_OPEN_APPEND:
		nFirst = m_nDataSize;
		nSize = m_nBufSize - m_nDataSize;
		nFlags = 0;
		break;
	case MESH_OPEN_MODIFY:
		nFlags = 0;
		break;
	default:
		return ERROR(FW_E_FORMAT);
	}
	return ERROR(m_pBuffer->Lock(m_nItemSize * nFirst, m_nItemSize * nSize, (void**)ppBytes, nFlags));
}

HRESULT _stdcall CMeshDX9VertexBuffer::Close(FWULONG nLength)
{
	if (!m_pBuffer) return ERROR(FW_E_NOTREADY);
	m_pBuffer->Unlock();
	m_nDataSize += nLength;
	return S_OK;
}

HRESULT _stdcall CMeshDX9VertexBuffer::Create(FWULONG nSize, FWULONG nFormat, FWULONG nBones, FWULONG nTextures)
{
	if (!m_pDevice) return ERROR(FW_E_NOTREADY);

	// MESH_VERTEX_??? -> D3DFVF convertion tables
	// !!! I cannot find D3DFVF_PSIZE in DX9!
	FWULONG kv[] = { MESH_VERTEX_XYZ,         MESH_VERTEX_BONEWEIGHT,   MESH_VERTEX_BONEINDEX,   MESH_VERTEX_NORMAL, 
		           MESH_VERTEX_POINTSIZE,   MESH_VERTEX_DIFFUSE,	  MESH_VERTEX_SPECULAR,    MESH_VERTEX_TEXTURE };
	FWULONG kx[] = { MESH_VERTEXID_XYZ,       MESH_VERTEXID_BONEWEIGHT, MESH_VERTEXID_BONEINDEX, MESH_VERTEXID_NORMAL, 
		           MESH_VERTEXID_POINTSIZE, MESH_VERTEXID_DIFFUSE,    MESH_VERTEXID_SPECULAR,  MESH_VERTEXID_TEXTURE };
	FWULONG d3[] = { D3DFVF_XYZ,              D3DFVF_XYZB1,             D3DFVF_LASTBETA_UBYTE4,  D3DFVF_NORMAL,
			       0,						D3DFVF_DIFFUSE,           D3DFVF_SPECULAR,         D3DFVF_TEX0 };
	FWULONG ln[] = { sizeof(D3DVECTOR),	    0,                        sizeof(DWORD),		   sizeof(D3DVECTOR),
			       0,						sizeof(FWULONG),            sizeof(FWULONG),           0 };
	FWULONG bones[] = { D3DFVF_XYZB1, D3DFVF_XYZB2, D3DFVF_XYZB3, D3DFVF_XYZB4, D3DFVF_XYZB5 }; 
	FWULONG textures[] = { D3DFVF_TEX0, D3DFVF_TEX1, D3DFVF_TEX2, D3DFVF_TEX3, D3DFVF_TEX4, D3DFVF_TEX5, D3DFVF_TEX6, D3DFVF_TEX7, D3DFVF_TEX8 };

	// check capabilities
	D3DCAPS9 caps;
	m_pDevice->GetDeviceCaps(&caps);
	if (caps.MaxVertexBlendMatrixIndex == 0)
		nFormat = nFormat & ~MESH_VERTEX_BONEINDEX;
	if (nBones > caps.MaxVertexBlendMatrices)
		nBones = caps.MaxVertexBlendMatrices;
	if (nBones <= 1)
	{
		nBones = 0;
		nFormat = nFormat & ~(MESH_VERTEX_BONEINDEX | MESH_VERTEX_BONEWEIGHT);
	}
    	
	// initialize data
	m_nBufSize = nSize;
	m_nDataSize = 0;
	m_nFormat = 0;									// will be updated later
	m_nBones = nBones;								// no more than 4 bones
	m_nTextures = (nTextures <= 8) ? nTextures : 8;	// no more than 8 textures
	memset(&m_description, 0, sizeof(m_description)); 

	// update conversion tables
	if (nBones > 1) ln[1] = (m_nBones - 1) * sizeof(FWFLOAT);
	if (nTextures)  ln[7] = nTextures * 2 * sizeof(FWFLOAT);	// restricted to double FWFLOAT per texture (D3DFVF_TEXTUREFORMAT2)

	// convert to FVF
	FWULONG nOffset = 0;
	FWULONG j = 0;
	m_fvf = 0;
	for (int i = 0; i < sizeof(kv) / sizeof(FWULONG); i++)
		if ((nFormat & kv[i]) && (d3[i] || kv[i] == MESH_VERTEX_TEXTURE))
		{
			// update internal data
			m_nFormat |= kv[i];
			m_description[kx[i]].offset = nOffset;
			m_description[kx[i]].size = ln[i];
			j++;

			// update FVF
			switch (d3[i])
			{
			case D3DFVF_XYZB1:	// important notice: XYZBn values overwrite XYZ
								// for indexed blend one additional position for indices
								m_fvf = bones[(nFormat & MESH_VERTEX_BONEINDEX) ? m_nBones-1 : m_nBones-2]; 
								break;
			case D3DFVF_TEX0:	m_fvf |= textures[m_nTextures]; break;
			default:			m_fvf |= d3[i]; break;
			}

			// update offset
			nOffset += ln[i];
		}
	m_nItemSize = nOffset;

	HRESULT h = m_pDevice->CreateVertexBuffer(m_nItemSize * m_nBufSize, /*D3DUSAGE_DYNAMIC*/0, m_fvf, D3DPOOL_MANAGED, &m_pBuffer, NULL);

	return ERROR(h);
}

HRESULT _stdcall CMeshDX9VertexBuffer::Destroy()
{
	if (!m_pBuffer) return S_OK;
	IDirect3DVertexBuffer9 *pBuffer = m_pBuffer;
	m_pBuffer = NULL;
	pBuffer->Release();
	return S_OK;
}

HRESULT _stdcall CMeshDX9VertexBuffer::GetFormat(/*[out]*/ FWULONG *nFormat, /*[out]*/ FWULONG *nBones, /*[out]*/ FWULONG *nTextures)
{
	if (!m_pBuffer) return ERROR(FW_E_NOTREADY);
	if (nFormat)   *nFormat = m_nFormat;
	if (nBones)    *nBones = m_nBones;
	if (nTextures) *nTextures = m_nTextures;
	return S_OK;
}

// substantially changed 23/03/2013 --- old version below
HRESULT _stdcall CMeshDX9VertexBuffer::GetCaps(enum MESH_VERTEXID nFlag, FWULONG nIndex, /*[out]*/ FWULONG *pOffset, /*[out,retval]*/ FWULONG *pSize)
{
	if (!m_pBuffer) return ERROR(FW_E_NOTREADY);

	FWULONG nOffset = m_description[nFlag].offset;
	FWULONG nSize = m_description[nFlag].size;

	if (nSize && (nFlag == MESH_VERTEXID_BONEWEIGHT || nFlag == MESH_VERTEXID_BONEINDEX) && nIndex < 0x80000000)
	{
		FWULONG nBones = min(3, m_nBones);
		if (nIndex >= nBones)
			nOffset = nSize = 0;
		else
		{
			nSize /= nBones;
			nOffset += nIndex * nSize;
		}
	}
	else
	if (nSize && nFlag == MESH_VERTEXID_TEXTURE && nIndex < 0x80000000)
	{
		nOffset += nIndex * 2 * sizeof(FWFLOAT);
		nSize = 2 * sizeof(FWFLOAT);
		if (nIndex >= m_nTextures) { nOffset = 0; nSize = 0; }
	}

	if (pOffset) *pOffset = nOffset;
	if (pSize) *pSize = nSize;
	return S_OK;
}

//HRESULT _stdcall CMeshDX9VertexBuffer::GetCaps(enum MESH_VERTEXID nFlag, FWULONG nIndex, /*[out]*/ FWULONG *pOffset, /*[out,retval]*/ FWULONG *pSize)
//{
//	if (!m_pBuffer) return ERROR(FW_E_NOTREADY);
//
//	if (pOffset) *pOffset = m_description[nFlag].offset;
//	if (pSize) *pSize = m_description[nFlag].size;
//	if (nFlag == MESH_VERTEXID_BONEWEIGHT && nIndex < 0x80000000)
//	{
//		if (pOffset) *pOffset += nIndex * sizeof(FWFLOAT);
//		if (pSize) *pSize = sizeof(FWFLOAT);
//		if (nIndex > m_nBones - 2) { if (pOffset) *pOffset = 0; if (pSize) *pSize = 0; }
//	}
//	if (nFlag == MESH_VERTEXID_BONEINDEX && nIndex < 0x80000000)
//	{
//		if (pOffset) *pOffset += nIndex;
//		if (pSize) *pSize = 1;
//		if (nIndex > 3) { if (pOffset) *pOffset = 0; if (pSize) *pSize = 0; }
//	}
//	if (nFlag == MESH_VERTEXID_TEXTURE && nIndex < 0x80000000 && *pSize)
//	{
//		if (pOffset) *pOffset += nIndex * 2 * sizeof(FWFLOAT);
//		if (pSize) *pSize = 2 * sizeof(FWFLOAT);
//		if (nIndex >= m_nTextures) { if (pOffset) *pOffset = 0; if (pSize) *pSize = 0; }
//	}
//	return S_OK;
//}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// Class CMeshDX9FaceBuffer

HRESULT _stdcall CMeshDX9FaceBuffer::PutContextObject(FWULONG index, REFIID iid, void *pUnknown)
{
	if (!pUnknown) return ERROR(FW_E_POINTER);
	if (FAILED(((IUnknown*)pUnknown)->QueryInterface(&m_pDevice))) return ERROR(FW_E_POINTER);
	return S_OK;
}

HRESULT _stdcall CMeshDX9FaceBuffer::GetContextObject(FWULONG index, /*[out]*/ IID *pIID, /*[out]*/ void **pUnknown)
{
	if (pIID) *pIID = IID_IDirect3DIndexBuffer9;
	*pUnknown = m_pBuffer;
	m_pBuffer->AddRef();
	return S_OK;
}

HRESULT _stdcall CMeshDX9FaceBuffer::BeforeRendering()
{
	return S_OK;
}

HRESULT _stdcall CMeshDX9FaceBuffer::AfterRendering()
{
	return S_OK;
}

HRESULT _stdcall CMeshDX9FaceBuffer::Open(enum MESH_OPENMODES mode, FWULONG nFirst, FWULONG nSize, /*[out, retval]*/ BYTE **ppBytes)
{
	if (!m_pBuffer) return ERROR(FW_E_NOTREADY);
	DWORD nFlags = 0;
	switch (mode)
	{
	case MESH_OPEN_READ:
		if (nSize == 0) nSize = m_nBufSize - nFirst;
		nFlags = D3DLOCK_READONLY;
		break;
	case MESH_OPEN_APPEND:
		nFirst = m_nDataSize;
		nSize = m_nBufSize - m_nDataSize;
		nFlags = 0;
		break;
	case MESH_OPEN_MODIFY:
		nFlags = 0;
		break;
	default:
		return ERROR(FW_E_FORMAT);
	}
	return ERROR(m_pBuffer->Lock(m_nItemSize * nFirst, m_nItemSize * nSize, (void**)ppBytes, nFlags));
}

HRESULT _stdcall CMeshDX9FaceBuffer::Close(FWULONG nLength)
{
	if (!m_pBuffer) return ERROR(FW_E_NOTREADY);
	m_pBuffer->Unlock();
	m_nDataSize += nLength;
	return S_OK;
}

HRESULT _stdcall CMeshDX9FaceBuffer::Create(FWULONG nSize)
{
	if (!m_pDevice) return ERROR(FW_E_NOTREADY);
	m_nBufSize = nSize;
	m_nDataSize = 0;
	D3DCAPS9 caps;
	m_pDevice->GetDeviceCaps(&caps);
	if (caps.MaxVertexIndex > 0xffff)
	{	// 32-bit indices
		m_nItemSize = 3 * sizeof(FWULONG);
        return ERROR(m_pDevice->CreateIndexBuffer(m_nItemSize * m_nBufSize, /*D3DUSAGE_DYNAMIC*/0, D3DFMT_INDEX32, D3DPOOL_MANAGED, &m_pBuffer, NULL));
	}
	else
	{	// 16-bit indices
		m_nItemSize = 3 * sizeof(short);
        return ERROR(m_pDevice->CreateIndexBuffer(m_nItemSize * m_nBufSize, /*D3DUSAGE_DYNAMIC*/0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pBuffer, NULL));
	}
}

HRESULT _stdcall CMeshDX9FaceBuffer::Destroy()
{
	if (!m_pBuffer) return S_OK;
	IDirect3DIndexBuffer9 *pBuffer = m_pBuffer;
	m_pBuffer = NULL;
	pBuffer->Release();
	return S_OK;
}

HRESULT _stdcall CMeshDX9FaceBuffer::GetFormat(FWULONG *nBitsPerIndex)
{
	if (nBitsPerIndex) *nBitsPerIndex = 8 * m_nItemSize / 3;
	return S_OK;
}

