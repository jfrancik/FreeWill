// texture.cpp : Defines the texture object
//

#include "stdafx.h"
#include "texture.h"
#include <dxerr.h>

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}
/* IID_IDirect3DTexture9 */
/* {85C31227-3DE5-4f00-9B3A-F11AC38C18B5} */
MIDL_DEFINE_GUID(IID, IID_IDirect3DTexture9, 0x85c31227, 0x3de5, 0x4f00, 0x9b, 0x3a, 0xf1, 0x1a, 0xc3, 0x8c, 0x18, 0xb5);
#undef MIDL_DEFINE_GUID


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// Class CDX9Texture

#define D3D_ERROR ErrorSrc(__WFILE__, __LINE__), D3DError

HRESULT CDX9Texture::D3DError(HRESULT nD3DErrorCode)
{
	if (SUCCEEDED(nD3DErrorCode)) return nD3DErrorCode;
	wchar_t pBuf[256];
	const WCHAR *pStr1 = DXGetErrorString(nD3DErrorCode);
	const WCHAR *pStr2 = DXGetErrorDescription(nD3DErrorCode);
	_snwprintf(pBuf, 256, L"%ls (0x%x): %ls", pStr1, nD3DErrorCode, pStr2);
	FWULONG N = (FWULONG)(__int64)pBuf;
	return Error(MAT_E_PLATFORM_EX, 1, &N);
}

CDX9Texture::CDX9Texture():
	m_pDX9Texture(NULL),
	m_pDX9Device(NULL),
	m_fUTile(FWFLOAT(1.0)),
	m_fVTile(FWFLOAT(1.0))
{
}

CDX9Texture::~CDX9Texture()
{
	FreeData();
	if (m_pDX9Device != NULL)
		m_pDX9Device->Release();
}

HRESULT __stdcall CDX9Texture::FreeData()
{
	if (m_pDX9Texture != NULL)
		m_pDX9Texture->Release();
	m_pDX9Texture = NULL;
	return S_OK;
}

HRESULT __stdcall CDX9Texture::LoadFromFile(LPOLESTR szFileName)
{
	FreeData();
	if (m_pDX9Device != NULL)
	{
		//HRESULT h = D3DXCreateTextureFromFileEx(m_pDX9Device, sBuff, 
		//	D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, 
		//	D3DPOOL_MANAGED, D3DX_FILTER_TRIANGLE|D3DX_FILTER_MIRROR, 
		//	D3DX_FILTER_TRIANGLE|D3DX_FILTER_MIRROR, 0, NULL, NULL,
		//	&m_pDX9Texture);
		HRESULT h = D3DXCreateTextureFromFile(m_pDX9Device, szFileName, &m_pDX9Texture);
		if (FAILED(h)) return ERROR(MAT_TEXTURE_FROM_FILE_ERROR, 1, (FWULONG*)(&szFileName));
		// Error here is not critical...
		return S_OK;
	}
	return S_FALSE;
}

HRESULT __stdcall CDX9Texture::LoadFromFileInMemory(BYTE* pData, FWULONG nDataSize)
{
	FreeData();
	if (m_pDX9Device != NULL)
	{
		HRESULT h = D3DXCreateTextureFromFileInMemoryEx(
			m_pDX9Device, (LPVOID) pData, nDataSize, 
			D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, 
			D3DPOOL_MANAGED, D3DX_FILTER_TRIANGLE|D3DX_FILTER_MIRROR, 
			D3DX_FILTER_TRIANGLE|D3DX_FILTER_MIRROR, 0, NULL, NULL,
			&m_pDX9Texture);
		if (FAILED(h)) return ERROR(MAT_TEXTURE_ERROR);
		return S_OK;
	}
	return S_FALSE;
}

HRESULT __stdcall CDX9Texture::IsLoaded()
{
	return (m_pDX9Texture != NULL) ? S_OK : S_FALSE;
}

HRESULT __stdcall CDX9Texture::AsFileInMemory(BYTE** ppData, FWULONG* pnDataSize)
{
	if (ppData != NULL)
		*ppData = NULL;
	if (pnDataSize != NULL)
		*pnDataSize = 0;
	return ERROR(E_NOTIMPL);
}

HRESULT __stdcall CDX9Texture::GetContextObject(FWULONG index, IID *pIID, void **ppUnknown)
{
	if (ppUnknown != NULL)
	{
		if (pIID != NULL)
			*pIID = IID_IDirect3DTexture9;
		*ppUnknown = m_pDX9Texture;
		if (m_pDX9Texture != NULL)
			m_pDX9Texture->AddRef();
		return S_OK;
	}
	return S_FALSE;
}

HRESULT __stdcall CDX9Texture::PutContextObject(FWULONG index, REFIID iid, void *pUnknown)
{
	if (pUnknown != NULL)
	{ 
		if (iid == IID_IDirect3DTexture9)
		{
			FreeData();
			if FAILED(((IUnknown*)pUnknown)->QueryInterface(&m_pDX9Texture)) return ERROR(FW_E_POINTER);
			return S_OK;
		}
		if (iid == IID_IDirect3DDevice9)
		{
			if (m_pDX9Device != NULL)
				m_pDX9Device->Release();
			m_pDX9Device = NULL;
			if FAILED(((IUnknown*)pUnknown)->QueryInterface(&m_pDX9Device)) return ERROR(FW_E_POINTER);
			return S_OK;
		}
	}
	return S_FALSE;
}

HRESULT __stdcall CDX9Texture::SetUVTile(FWFLOAT fUTile, FWFLOAT fVTile)
{
	m_fUTile = fUTile;
	m_fVTile = fVTile;
	return S_OK;
}

HRESULT __stdcall CDX9Texture::GetUTile(FWFLOAT* pfResult)
{
	if (pfResult != NULL)
	{
		*pfResult = m_fUTile;
		return S_OK;
	}
	return S_FALSE;
}

HRESULT __stdcall CDX9Texture::GetVTile(FWFLOAT* pfResult)
{
	if (pfResult != NULL)
	{
		*pfResult = m_fVTile;
		return S_OK;
	}
	return S_FALSE;
}
