// texture.h
//
////////////////////////////////////////////////////////////////////////

#if !defined(__TEXTURE_H)
#define __TEXTURE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\fwlib\factory.h"
#include "..\fwlib\fwunknown.h"
#include "matplus.h"
#include <d3dx9.h>

class CDX9Texture: public FWUNKNOWN<ITexture, IID_ITexture, ITexture>
{
protected:
	IDirect3DTexture9* m_pDX9Texture;
	IDirect3DDevice9* m_pDevice;
	FWFLOAT m_fUTile, m_fVTile;
	
public:
	virtual HRESULT __stdcall LoadFromFile(LPOLESTR szFileName);
	virtual HRESULT __stdcall LoadFromFileInMemory(BYTE* pData, FWULONG nDataSize);
	virtual HRESULT __stdcall IsLoaded();
	virtual HRESULT __stdcall FreeData();

	virtual HRESULT __stdcall AsFileInMemory(BYTE** ppData, FWULONG* pnDataSize);
	
	virtual HRESULT __stdcall SetUVTile(FWFLOAT fUTile, FWFLOAT fVTile);
	virtual HRESULT __stdcall GetUTile(FWFLOAT* pfResult);
	virtual HRESULT __stdcall GetVTile(FWFLOAT* pfResult);

	virtual HRESULT __stdcall GetContextObject(FWULONG index, IID *pIID, void **ppUnknown);
	virtual HRESULT __stdcall PutContextObject(FWULONG index, REFIID iid, void *pUnknown);

	// error helper
	HRESULT D3DError(HRESULT nD3DErrorCode);

	DECLARE_FACTORY_CLASS(DX9Texture, Texture)
	FW_RTTI(DX9Texture)
	FW_ERROR_BEGIN
		FW_ERROR_ENTRY(MAT_E_PLATFORM,					L"DirectX Error", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(MAT_E_PLATFORM_EX,				L"DirectX Error: \"%s\"", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(MAT_TEXTURE_ERROR,				L"An error occured when creating a texture", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(MAT_TEXTURE_FROM_FILE_ERROR,		L"An error occured when loading a texture from file: %s", FW_SEV_CRITICAL)
	FW_ERROR_END

	CDX9Texture();
	~CDX9Texture();
};

#endif
