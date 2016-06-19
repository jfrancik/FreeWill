// buffers.cpp : Defines DX-based buffers
// uses DirectX 9

#include "stdafx.h"
#include "renderer.h"
#include "buffers.h"
#include "texture.h"

#include <assert.h>
#include <cvt/wstring>
#include <codecvt>

#include <D3d9.h>
#include <d3dx9tex.h>
#include <d3dx9math.h>
#include <dxerr.h>

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}
/* IID_IDirect3DDevice9 */
// {D0223B96-BF7A-43fd-92BD-A43B0D82B9EB} */
MIDL_DEFINE_GUID(IID, IID_IDirect3DDevice9, 0xd0223b96, 0xbf7a, 0x43fd, 0x92, 0xbd, 0xa4, 0x3b, 0xd, 0x82, 0xb9, 0xeb);
#undef MIDL_DEFINE_GUID


//// REMARKS:
// D3DDEVTYPE_HAL fixed now!

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// Class CDX9Renderer

/////////////////////////////////////////////////////////////////////////////
// Error Handling

#define D3D_ERROR ErrorSrc(__WFILE__, __LINE__), D3DError

HRESULT CDX9Renderer::D3DError(HRESULT nD3DErrorCode)
{
	if (SUCCEEDED(nD3DErrorCode)) return nD3DErrorCode;
	wchar_t pBuf[256];
	_snwprintf(pBuf, 256, L"%ls (0x%x): %ls", DXGetErrorString(nD3DErrorCode), nD3DErrorCode, DXGetErrorDescription(nD3DErrorCode));
	FWULONG N = (FWULONG)(__int64)pBuf;
	return Error(REND_E_PLATFORM_EX, 1, &N);
}

void __MatrixCopy(D3DMATRIX *pd3dm, FWMATRIX *pm)
{
//	memcpy(pd3dm, pm, sizeof(D3DXMATRIX));

	FWDOUBLE *pLong = (FWDOUBLE*)pm;
	float *pShort = (float*)pd3dm;
	for (ULONG i = 0; i < 16; i++)
		*pShort++ = (float)(*pLong++);
}


/////////////////////////////////////////////////////////////////////////////
// IRndrGeneric implementation

HRESULT _stdcall CDX9Renderer::GetBuffers(/*[out]*/ IMeshVertexBuffer **pV, /*[out]*/ IMeshFaceBuffer **pF)
{
	HRESULT h = CreateBuffers(m_pVertexBuffer == NULL, m_pFaceBuffer == NULL);
	if (FAILED(h)) return h;
	assert(m_pVertexBuffer && m_pFaceBuffer);

	if (pV)
	{
		*pV = m_pVertexBuffer;
		(*pV)->AddRef();
	}
	if (pF)
	{
		*pF = m_pFaceBuffer;
		(*pF)->AddRef();
	}
	return S_OK;
}

HRESULT _stdcall CDX9Renderer::CreateBuffers(BOOL bVertexBuf, BOOL bFaceBuf)
{
	if (bVertexBuf)
	{
		// release the old buffer
		if (m_pVertexBuffer) m_pVertexBuffer->Release();
		// create a new buffer
		m_pVertexBuffer = new CMeshDX9VertexBuffer;
		FWDevice()->RegisterObject(m_pVertexBuffer);
		// set platfotm-dependent contents
		HRESULT h;
		h = m_pVertexBuffer->PutContextObject(0, IID_IDirect3DDevice9, m_pDevice);
	}
	if (bFaceBuf)
	{
		// release the old buffer
		if (m_pFaceBuffer) m_pFaceBuffer->Release();
		// create a new buffer
		m_pFaceBuffer = new CMeshDX9FaceBuffer;
		FWDevice()->RegisterObject(m_pFaceBuffer);
		// set platfotm-dependent contents
		HRESULT h;
		h = m_pFaceBuffer->PutContextObject(0, IID_IDirect3DDevice9, m_pDevice);
	}
	return S_OK;
}

HRESULT _stdcall CDX9Renderer::GetTargetSize(FWULONG *pWidth, FWULONG *pHeight)
{
	if (!m_bOnScreen)
		return GetBackBufferSize(pWidth, pHeight);
	RECT rect;
	::GetClientRect(m_hWnd, &rect);
	if (pWidth) *pWidth = rect.right-rect.left;
	if (pHeight) *pHeight = rect.bottom-rect.top;
	return S_OK;
}

HRESULT _stdcall CDX9Renderer::GetBackBufferSize(FWULONG *pWidth, FWULONG *pHeight)
{
	if (m_bOnScreen)
	{
		if (pWidth) *pWidth = m_d3dpp.BackBufferWidth;
		if (pHeight) *pHeight = m_d3dpp.BackBufferHeight;
	}
	else
	{
		if (pWidth) *pWidth = m_nOffsW;
		if (pHeight) *pHeight = m_nOffsH;
	}
	return S_OK;
}

HRESULT _stdcall CDX9Renderer::GetViewport(FWULONG *pX, FWULONG *pY, FWULONG *pWidth, FWULONG *pHeight)
{
	D3DVIEWPORT9 vp;
	m_pDevice->GetViewport(&vp);
	if (pX) *pX = vp.X;
	if (pY) *pY = vp.Y;
	if (pWidth) *pWidth = vp.Width;
	if (pHeight) *pHeight = vp.Height;
	return S_OK;
}
	
HRESULT _stdcall CDX9Renderer::GetTargetViewport(FWULONG *pX, FWULONG *pY, FWULONG *pWidth, FWULONG *pHeight)
{
	FWULONG nWTarget, nHTarget, nWBack, nHBack;
	GetTargetSize(&nWTarget, &nHTarget);
	GetBackBufferSize(&nWBack, &nHBack);
	
	D3DVIEWPORT9 vp;
	m_pDevice->GetViewport(&vp);
	if (pX) *pX = vp.X * nWTarget / nWBack;
	if (pY) *pY = vp.Y * nHTarget / nHBack;
	if (pWidth) *pWidth = vp.Width * nWTarget / nWBack;
	if (pHeight) *pHeight = vp.Height * nHTarget / nHBack;
	return S_OK;
}
	
HRESULT _stdcall CDX9Renderer::PutViewport(FWFLOAT fX, FWFLOAT fY, FWFLOAT fWidth, FWFLOAT fHeight, FWLONG nX, FWLONG nY, FWLONG nWidth, FWLONG nHeight)
{
	FWULONG nWBack, nHBack;
	FWULONG nWSize, nHSize;
	GetBackBufferSize(&nWBack, &nHBack);
	GetViewSize(&nWSize, &nHSize);

	if (nWSize)
	{
		nX *= (FWLONG)nWBack; nX /= (FWLONG)nWSize;
		nWidth *= (FWLONG)nWBack; nWidth /= (FWLONG)nWSize;
	}
	if(nHSize)
	{
		nY *= (FWLONG)nHBack; nY /= (FWLONG)nHSize;
		nHeight *= (FWLONG)nHBack; nHeight /= (FWLONG)nHSize;
	}

	D3DVIEWPORT9 vp;
	vp.X = (DWORD)(nWBack * fX) + nX;
	vp.Y = (DWORD)(nHBack * fY) + nY;
	vp.Width = (DWORD)(nWBack * fWidth + nWidth);
	vp.Height = (DWORD)(nHBack * fHeight + nHeight);
	vp.MinZ = 0.0f;
	vp.MaxZ = 1.0f;
	
	HRESULT h = m_pDevice->SetViewport(&vp);
	if (FAILED(h)) return D3D_ERROR(h);

	return S_OK;
}

HRESULT _stdcall CDX9Renderer::PutTargetViewport(FWFLOAT fX, FWFLOAT fY, FWFLOAT fWidth, FWFLOAT fHeight, FWLONG nX, FWLONG nY, FWLONG nWidth, FWLONG nHeight)
{
	FWULONG nWTarget, nHTarget, nWBack, nHBack;
	GetTargetSize(&nWTarget, &nHTarget);
	GetBackBufferSize(&nWBack, &nHBack);

	D3DVIEWPORT9 vp;
	vp.X = (DWORD)(nWBack * fX + (FWFLOAT)nX * nWBack / nWTarget);
	vp.Y = (DWORD)(nHBack * fY + (FWFLOAT)nY * nHBack / nHTarget);
	vp.Width = (DWORD)(nWBack * fWidth + (FWFLOAT)nWidth * nWBack / nWTarget);
	vp.Height = (DWORD)(nHBack * fHeight + (FWFLOAT)nHeight * nHBack / nHTarget);
	vp.MinZ = 0.0f;
	vp.MaxZ = 1.0f;
	
	HRESULT h = m_pDevice->SetViewport(&vp);
	if (FAILED(h)) return D3D_ERROR(h);

	return S_OK;
}

HRESULT _stdcall CDX9Renderer::GetViewSize(FWULONG *pX, FWULONG *pY)
{
	return GetTargetSize(pX, pY);
}

HRESULT _stdcall CDX9Renderer::GetAspectRatio(FWFLOAT *p)
{
	if (!p) return S_OK;
	FWULONG w, h;
	GetTargetViewport(NULL, NULL, &w, &h);
	*p = (FWFLOAT)w / (FWFLOAT)h;
	return S_OK;
}

HRESULT _stdcall CDX9Renderer::GetResetFlag(/*[out, retval]*/ BOOL *bReset)
{
	if (bReset) *bReset = m_bReset;
	return S_OK;
}

HRESULT _stdcall CDX9Renderer::RenderMesh(struct RNDR_MESHEDOBJ *pInfo)
{
	HRESULT h;

	// set vertex buffer
	IMeshVertexBuffer *pVertexBuffer = NULL;
	IDirect3DVertexBuffer9 *pDXVertices = NULL;
	pVertexBuffer = pInfo->pVertexBuffer;
	pVertexBuffer->BeforeRendering();
	h = pVertexBuffer->GetContextObject(0, NULL, (void**)&pDXVertices); if (FAILED(h)) return h;
	h = m_pDevice->SetStreamSource(0, pDXVertices, 0, pInfo->nVertexSize); if (FAILED(h)) return D3D_ERROR(h);

	// set face buffer
	IMeshFaceBuffer *pFaceBuffer = NULL;
	IDirect3DIndexBuffer9 *pDXIndices = NULL;
	pFaceBuffer = pInfo->pFaceBuffer;
	pFaceBuffer->BeforeRendering();
	h = pFaceBuffer->GetContextObject(0, NULL, (void**)&pDXIndices); if (FAILED(h)) return h;
	h = m_pDevice->SetIndices(pDXIndices); if (FAILED(h)) return D3D_ERROR(h);

	// set colouring, materials and textures
	D3DMATERIAL9 dxMaterial;
	dxMaterial.Ambient.r = dxMaterial.Diffuse.r = dxMaterial.Specular.r = 1.0f;
	dxMaterial.Ambient.g = dxMaterial.Diffuse.g = dxMaterial.Specular.g = 1.0f;
	dxMaterial.Ambient.b = dxMaterial.Diffuse.b = dxMaterial.Specular.b = 1.0f;
	dxMaterial.Ambient.a = dxMaterial.Diffuse.a = dxMaterial.Specular.a = 1.0f;

	FWULONG nAlphaMode = MAT_ALPHA_DISABLE;
	FWULONG nCullingMode = MAT_CULLING_DISABLE;
	bool bTextured = false;

	IMaterial *pMaterial = NULL;
	h = pInfo->pMesh->GetMaterial(&pMaterial);
	if (h == S_OK && pMaterial)
	{
		pMaterial->GetAlphaMode(&nAlphaMode);
		pMaterial->GetCullingMode(&nCullingMode);

		pMaterial->GetAmbientColor((FWCOLOR*)(&dxMaterial.Ambient));
		pMaterial->GetDiffuseColor((FWCOLOR*)(&dxMaterial.Diffuse));
		pMaterial->GetSpecularColor((FWCOLOR*)(&dxMaterial.Specular));
		pMaterial->GetSelfIlluminationColor((FWCOLOR*)(&dxMaterial.Emissive));

		ITexture *pTexture = NULL;
		h = pMaterial->GetTexture(0, &pTexture);

		if (h == S_OK && pTexture)
		{
			IDirect3DTexture9 *pDXTexture0 = NULL;
			h = pTexture->GetContextObject(0, NULL, (void**)&pDXTexture0);
			if (SUCCEEDED(h) && pDXTexture0)
			{
				bTextured = true;
				m_pDevice->SetTexture(0, pDXTexture0);
				FWFLOAT fUTile, fVTile;
				pTexture->GetUTile(&fUTile); pTexture->GetVTile(&fVTile);
				D3DXMATRIX mTex;
				D3DXMatrixIdentity(&mTex);
				D3DXMatrixScaling(&mTex, fUTile, fVTile, 1.0f);
				m_pDevice->SetTransform(D3DTS_TEXTURE0, &mTex);
				m_pDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
				pDXTexture0->Release();
			}
			pTexture->Release();
		}
		pMaterial->Release();
	}

	// Set Texture Stages and Render States
	DWORD nColorMode = bTextured ? D3DTOP_MODULATE : D3DTOP_SELECTARG1;
	m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	m_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, nColorMode);

	m_pDevice->SetTextureStageState(0,D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	m_pDevice->SetTextureStageState(0,D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
	switch (nAlphaMode)
	{
	case MAT_ALPHA_DISABLE: m_pDevice->SetTextureStageState(0,D3DTSS_ALPHAOP, D3DTOP_DISABLE); break;
	case MAT_ALPHA_MATERIAL: m_pDevice->SetTextureStageState(0,D3DTSS_ALPHAOP, D3DTOP_SELECTARG1); break;
	case MAT_ALPHA_TEXTURE: m_pDevice->SetTextureStageState(0,D3DTSS_ALPHAOP, D3DTOP_SELECTARG2); break;
	}

	if (nAlphaMode == MAT_ALPHA_DISABLE)
	{
		m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
		m_pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	}
	else
	{
		m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		m_pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	}

	h = m_pDevice->SetMaterial(&dxMaterial);

	if (pInfo->nMatrixNum)
	{
		D3DMATRIX *pMatrices = new D3DMATRIX[pInfo->nMatrixNum];
		for (FWULONG i = 0; i < pInfo->nMatrixNum; i++)
			__MatrixCopy(&pMatrices[i], &pInfo->pMatrices[i]);
		
		// set bone matrices
		for (FWULONG j = 0; j < pInfo->nIndexNum; j++)
			if (pInfo->pTransformIndices[j] != 0xffffffff)
			{
				assert(pInfo->pTransformIndices[j] < pInfo->nMatrixNum);
				h = m_pDevice->SetTransform(D3DTS_WORLDMATRIX(j), pMatrices + pInfo->pTransformIndices[j]);
			}
		delete [] pMatrices;
	}
	
	switch (nCullingMode)
	{
	case MAT_CULLING_DISABLE:
		h = m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		if SUCCEEDED(h) h = m_pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, pInfo->nVertexFirst, 0, pInfo->nVertexNum, pInfo->nFaceFirst*3, pInfo->nFaceNum); 
		break;
	case MAT_CULLING_CW:
		h = m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
		if SUCCEEDED(h) h = m_pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, pInfo->nVertexFirst, 0, pInfo->nVertexNum, pInfo->nFaceFirst*3, pInfo->nFaceNum); 
		break;
	case MAT_CULLING_CCW:
		h = m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		if SUCCEEDED(h) h = m_pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, pInfo->nVertexFirst, 0, pInfo->nVertexNum, pInfo->nFaceFirst*3, pInfo->nFaceNum); 
		break;
	case MAT_CULLING_CW_CCW:
		h = m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
		if SUCCEEDED(h) h = m_pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, pInfo->nVertexFirst, 0, pInfo->nVertexNum, pInfo->nFaceFirst*3, pInfo->nFaceNum); 
		if SUCCEEDED(h) h = m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		if SUCCEEDED(h) h = m_pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, pInfo->nVertexFirst, 0, pInfo->nVertexNum, pInfo->nFaceFirst*3, pInfo->nFaceNum); 
		break;
	case MAT_CULLING_CCW_CW:
		h = m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		if SUCCEEDED(h) h = m_pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, pInfo->nVertexFirst, 0, pInfo->nVertexNum, pInfo->nFaceFirst*3, pInfo->nFaceNum); 
		if SUCCEEDED(h) h = m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
		if SUCCEEDED(h) h = m_pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, pInfo->nVertexFirst, 0, pInfo->nVertexNum, pInfo->nFaceFirst*3, pInfo->nFaceNum); 
		break;
	}
    
	h = m_pDevice->SetStreamSource(0, NULL, 0, pInfo->nVertexSize); if (FAILED(h)) return D3D_ERROR(h);	// added 18 June 2016
	h = m_pDevice->SetIndices(NULL); if (FAILED(h)) return D3D_ERROR(h);								// added 18 June 2016

	if (FAILED(h)) return D3D_ERROR(h);

	// release texture
	m_pDevice->SetTexture(0, NULL);

	if (pDXVertices) pDXVertices->Release();
	if (pDXIndices) pDXIndices->Release();

	pVertexBuffer->AfterRendering();
	pFaceBuffer->AfterRendering();

	return S_OK;
}

HRESULT _stdcall CDX9Renderer::SetLight(FWULONG iIndex, BOOL bOn)
{
	return D3D_ERROR(m_pDevice->LightEnable(iIndex, bOn));
}

HRESULT _stdcall CDX9Renderer::GetLight(FWULONG iIndex)
{
	BOOL b;
	m_pDevice->GetLightEnable(iIndex, &b);
	return b ? S_OK : S_FALSE;
}

HRESULT _stdcall CDX9Renderer::SetPointLight(FWULONG iIndex, FWCOLOR clrDiff, FWCOLOR clrSpec, FWCOLOR clrAmb, FWVECTOR *pPos, FWFLOAT fRange, FWFLOAT fAtten0, FWFLOAT fAtten1, FWFLOAT fAtten2)
{
	HRESULT h;
	D3DLIGHT9 light;
	ZeroMemory(&light, sizeof(D3DLIGHT9));
	light.Type       = D3DLIGHT_POINT;
	memcpy(&light.Diffuse, &clrDiff, sizeof(light.Diffuse));
	memcpy(&light.Specular, &clrSpec, sizeof(light.Specular));
	memcpy(&light.Ambient, &clrAmb, sizeof(light.Ambient));
	light.Position.x = pPos->x;
	light.Position.y = pPos->y;
	light.Position.z = pPos->z;
	light.Range       = fRange;
	light.Attenuation0 = fAtten0;
	light.Attenuation1 = fAtten1;
	light.Attenuation2 = fAtten2;
	h = m_pDevice->SetLight(iIndex, &light); if (FAILED(h)) return D3D_ERROR(h);
	h = m_pDevice->LightEnable(iIndex, TRUE); if (FAILED(h)) return D3D_ERROR(h);
	h = m_pDevice->SetRenderState(D3DRS_LIGHTING, TRUE); if (FAILED(h)) return D3D_ERROR(h);

	return S_OK;
}

HRESULT _stdcall CDX9Renderer::SetDirLight(FWULONG iIndex, FWCOLOR clrDiff, FWCOLOR clrSpec, FWCOLOR clrAmb, FWVECTOR *pDir)
{
	HRESULT h;

	D3DXVECTOR3 vecDir(pDir->x, pDir->y, pDir->z);
	D3DLIGHT9 light;
	ZeroMemory(&light, sizeof(D3DLIGHT9));
	light.Type       = D3DLIGHT_DIRECTIONAL;
	memcpy(&light.Diffuse, &clrDiff, sizeof(light.Diffuse));
	memcpy(&light.Specular, &clrSpec, sizeof(light.Specular));
	memcpy(&light.Ambient, &clrAmb, sizeof(light.Ambient));
	D3DXVec3Normalize((D3DXVECTOR3*)&light.Direction, &vecDir);
	light.Range       = 1000.0f;
	light.Attenuation1 = 1.0f;
	h = m_pDevice->SetLight(iIndex, &light); if (FAILED(h)) return D3D_ERROR(h);
	h = m_pDevice->LightEnable(iIndex, TRUE); if (FAILED(h)) return D3D_ERROR(h);
	h = m_pDevice->SetRenderState(D3DRS_LIGHTING, TRUE); if (FAILED(h)) return D3D_ERROR(h);

	return S_OK;
}

HRESULT _stdcall CDX9Renderer::SetSpotLight(FWULONG iIndex, FWCOLOR clrDiff, FWCOLOR clrSpec, FWCOLOR clrAmb, FWVECTOR *pPos, FWVECTOR *pDir, FWFLOAT fTheta, FWFLOAT fPhi, FWFLOAT fFalloff)
{
	HRESULT h;

	D3DLIGHT9 light;
	ZeroMemory(&light, sizeof(D3DLIGHT9));
	light.Type       = D3DLIGHT_SPOT;
	memcpy(&light.Diffuse, &clrDiff, sizeof(light.Diffuse));
	memcpy(&light.Specular, &clrSpec, sizeof(light.Specular));
	memcpy(&light.Ambient, &clrAmb, sizeof(light.Ambient));
	light.Position.x = pPos->x;
	light.Position.y = pPos->y;
	light.Position.z = pPos->z;
	D3DXVECTOR3 vecDir(pDir->x, pDir->y, pDir->z);
	D3DXVec3Normalize((D3DXVECTOR3*)&light.Direction, &vecDir);
	light.Range       = 1000.0f;
	light.Attenuation1 = 1.0f;
	light.Theta = fTheta;
	light.Phi = fPhi;
	light.Falloff = fFalloff;
	h = m_pDevice->SetLight(iIndex, &light); if (FAILED(h)) return D3D_ERROR(h);
	h = m_pDevice->LightEnable(iIndex, TRUE); if (FAILED(h)) return D3D_ERROR(h);
	h = m_pDevice->SetRenderState(D3DRS_LIGHTING, TRUE); if (FAILED(h)) return D3D_ERROR(h);

	return S_OK;
}

HRESULT _stdcall CDX9Renderer::SetAmbientLight(FWCOLOR clrAmb)
{
	m_pDevice->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_COLORVALUE(clrAmb.r, clrAmb.g, clrAmb.b, clrAmb.a));
	return S_OK;
}

HRESULT _stdcall CDX9Renderer::GetViewTransform(/*[out, retval]*/ ITransform **pVal)
{
	assert(pVal);
	*pVal = m_pITransformView;
	if (*pVal) (*pVal)->AddRef();
	return S_OK;
}

HRESULT _stdcall CDX9Renderer::PutViewTransform(ITransform *p)
{
	HRESULT h = S_OK;
	assert(p);
	if (m_pITransformView) m_pITransformView->Release();
	m_pITransformView = NULL;
	if (p)
	{
		m_pITransformView = (ITransform*)p->Clone(IID_ITransform);
		p->AsTransform(m_pITransformView);
	}

	if (m_pDevice)
	{
		FWMATRIX matrix;
		D3DXMATRIX m;
		m_pITransformView->AsMatrix(matrix);
		__MatrixCopy(&m, &matrix);
		//m_pITransformView->AsMatrix(*(FWMATRIX*)&m);
		h = m_pDevice->SetTransform(D3DTS_VIEW, &m);
		if (FAILED(h)) return D3D_ERROR(h);
	}

	return S_OK;
}

HRESULT _stdcall CDX9Renderer::GetProjectionTransform(/*[out, retval]*/ ITransform **pVal)
{
	assert(pVal);
	*pVal = m_pITransformProjection;
	if (*pVal) (*pVal)->AddRef();
	return S_OK;
}

HRESULT _stdcall CDX9Renderer::PutProjectionTransform(ITransform *p)
{
	HRESULT h = S_OK;
	assert(p);
	if (m_pITransformProjection) m_pITransformProjection->Release();
	m_pITransformProjection = NULL;
	if (p)
	{
		m_pITransformProjection = (ITransform*)p->Clone(IID_ITransform);
		p->AsTransform(m_pITransformProjection);
	}

	if (m_pDevice)
	{
		FWMATRIX matrix;
		D3DXMATRIX m;
		m_pITransformProjection->AsMatrix(matrix);
		__MatrixCopy(&m, &matrix);
		//m_pITransformProjection->AsMatrix(*(FWMATRIX*)&m);
		h = m_pDevice->SetTransform(D3DTS_PROJECTION, &m);
		if (FAILED(h)) return D3D_ERROR(h);
	}

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// attributes

HRESULT _stdcall CDX9Renderer::GetBackColor(/*[out, retval]*/ FWCOLOR *pVal)
{
	*pVal = m_colorBack;
	return S_OK;
}

HRESULT _stdcall CDX9Renderer::PutBackColor(FWCOLOR newVal)
{
	m_colorBack = newVal;
	m_DXBackColor = D3DCOLOR_XRGB((FWULONG)(m_colorBack.r * 255), (FWULONG)(m_colorBack.g * 255), (FWULONG)(m_colorBack.b * 255));
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// drawing functions

HRESULT _stdcall CDX9Renderer::InitDisplay(HWND hWnd, FWULONG nWidth, FWULONG nHeight)
{
	if (m_pD3D) m_pD3D->Release(); m_pD3D = NULL;
	if (m_pDevice) m_pDevice->Release(); m_pDevice = NULL;

	m_hWnd = hWnd;
	bool bFullScreen = (m_hWnd == NULL);

	HRESULT h;

	// Create D3D Device
	m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);

	// Get the current desktop display mode
	D3DDISPLAYMODE d3ddm;
	h = m_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm);
	if (FAILED(h)) return D3D_ERROR(h);

	if (nWidth) d3ddm.Width = nWidth;
	if (nHeight) d3ddm.Height = nHeight;

	// Full Screen Option
	if (bFullScreen)
	{
		h = m_pD3D->CheckDeviceType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, d3ddm.Format, FALSE);
		if FAILED(h)
		{
			d3ddm.Format = D3DFMT_R5G6B5;
			h = m_pD3D->CheckDeviceType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, d3ddm.Format, FALSE);
		}
		if (FAILED(h)) return D3D_ERROR(h);
	
		m_hWnd = ::GetDesktopWindow();
	}

	// check the format of ZBuffer
	D3DFORMAT AutoDepthStencilFormat = D3DFMT_D32;
	if FAILED(m_pD3D->CheckDepthStencilMatch(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, d3ddm.Format, AutoDepthStencilFormat))
		AutoDepthStencilFormat = D3DFMT_D24X8;
	if FAILED(m_pD3D->CheckDepthStencilMatch(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, d3ddm.Format, AutoDepthStencilFormat))
		AutoDepthStencilFormat = D3DFMT_D16;

	// Set up the structure used to create the D3DDevice
	ZeroMemory(&m_d3dpp, sizeof(m_d3dpp));
	m_d3dpp.Windowed = !bFullScreen;
	m_d3dpp.hDeviceWindow  = m_hWnd;
	m_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	m_d3dpp.EnableAutoDepthStencil = TRUE;
	m_d3dpp.AutoDepthStencilFormat = AutoDepthStencilFormat;
	m_d3dpp.BackBufferCount= 1;
	m_d3dpp.BackBufferFormat = d3ddm.Format;
	m_d3dpp.BackBufferWidth = d3ddm.Width;
	m_d3dpp.BackBufferHeight = d3ddm.Height;

	m_pDevice = NULL;

	D3DCAPS9 caps;
	m_pD3D->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);

	DWORD dwBehaviourFlags;
	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
		if (caps.DevCaps & D3DDEVCAPS_PUREDEVICE)
			dwBehaviourFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE;
		else
			dwBehaviourFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	else
		dwBehaviourFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

//	dwBehaviourFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	h = m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hWnd,
                                    D3DCREATE_FPU_PRESERVE | dwBehaviourFlags, &m_d3dpp, &m_pDevice);

	// HERE error is raised if system does not have Direct3D!!!
	if (FAILED(h)) return D3D_ERROR(h);

	// some initial settings
	__SetInitialRenderStates();

	h = Clear();
	if (FAILED(h)) return h;

	return S_OK;
}

HRESULT CDX9Renderer::__SetInitialRenderStates()
{
	HRESULT h;
	h = m_pDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_3WEIGHTS); if (FAILED(h)) return D3D_ERROR(h);
    h = m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    h = m_pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	h = m_pDevice->SetRenderState(D3DRS_NORMALIZENORMALS, 0x00202020);
	h = m_pDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, 0);
	h = m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	return S_OK;
}

HRESULT _stdcall CDX9Renderer::InitOffScreen(FWULONG nWidth, FWULONG nHeight)
{
	DoneOffScreen();

	HRESULT h;

	FWULONG nW, nH;
	GetTargetSize(&nW, &nH);
	m_nOffsW = nWidth ? nWidth : nW;
	m_nOffsH = nHeight ? nHeight : nH;

	// store the Back Buffer, set new render surface
	h = m_pDevice->GetRenderTarget(0, &m_pBackSurface);
	if (FAILED(h)) return D3D_ERROR(h);
	IDirect3DTexture9 *pTexture = NULL;
	h = m_pDevice->CreateTexture(m_nOffsW, m_nOffsH, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pTexture, NULL);
	if (FAILED(h)) return D3D_ERROR(h);
	h = pTexture->GetSurfaceLevel(0, &m_pOffsSurface);
	if (FAILED(h)) return D3D_ERROR(h);
	h = m_pDevice->SetRenderTarget(0, m_pOffsSurface);
	if (FAILED(h)) return D3D_ERROR(h);

	pTexture->Release();	// added 18 June 2016
	
	// store the Depth Stencil Buffer, set new Depth Stencil Surface
	h = m_pDevice->GetDepthStencilSurface(&m_pBackDS);
	if (FAILED(h)) return D3D_ERROR(h);
	h = m_pDevice->CreateDepthStencilSurface(m_nOffsW, m_nOffsH, D3DFMT_D16, D3DMULTISAMPLE_NONE, 0, false, &m_pOffsDS, NULL);
	if (FAILED(h)) return D3D_ERROR(h);
	h = m_pDevice->SetDepthStencilSurface(m_pOffsDS);
	if (FAILED(h)) return D3D_ERROR(h);

	m_bOnScreen = false;

	return S_OK;
}

HRESULT _stdcall CDX9Renderer::DoneDisplay()
{
	DoneOffScreen();
	if (m_pVertexBuffer) m_pVertexBuffer->Release(); m_pVertexBuffer = NULL;
	if (m_pFaceBuffer) m_pFaceBuffer->Release(); m_pFaceBuffer = NULL;
	if (m_pITransformView) m_pITransformView->Release(); m_pITransformView = NULL;
	if (m_pITransformProjection) m_pITransformProjection->Release(); m_pITransformProjection = NULL;
	//if (m_pAviFile) delete m_pAviFile; m_pAviFile = NULL;
	if (m_pMedia) delete m_pMedia; m_pMedia = NULL;
	if (m_pDevice) m_pDevice->Release(); m_pDevice = NULL;
	if (m_pD3D) m_pD3D->Release(); m_pD3D = NULL;

	return S_OK;
}

HRESULT _stdcall CDX9Renderer::DoneOffScreen()
{
	if (m_pOffsSurface && m_pOffsDS && m_pBackSurface && m_pBackDS)
	{
		SetTargetToScreen();
		m_pOffsSurface->Release(); m_pOffsSurface = NULL;
		m_pOffsDS->Release(); m_pOffsDS = NULL;
		m_pBackSurface->Release(); m_pBackSurface = NULL;
		m_pBackDS->Release(); m_pBackDS = NULL;
	}
	else
	{
		assert(!m_pOffsSurface && !m_pOffsDS && !m_pBackSurface && !m_pBackDS);
	}
	return S_OK;
}

HRESULT _stdcall CDX9Renderer::GetWindow(HWND *phWnd)
{
	if (phWnd) *phWnd = m_hWnd;
	return S_OK;
}

HRESULT _stdcall CDX9Renderer::PutWindow(HWND hWnd)
{
	m_hWnd = hWnd;
	return S_OK;
}

HRESULT _stdcall CDX9Renderer::SetCallback(enum FW_RENDER_CB_TYPE type, FW_RENDER_CB_HOOK fn, FWULONG nParam, void *pParam)
{
	m_ppfnCB[(FWULONG)type].pFn = fn;
	m_ppfnCB[(FWULONG)type].nParam = nParam;
	m_ppfnCB[(FWULONG)type].pParam = pParam;
	return S_OK;
}

HRESULT _stdcall CDX9Renderer::IsDeviceAvailable()
{
	return SUCCEEDED(m_pDevice->TestCooperativeLevel()) ? S_OK : S_FALSE;
}

HRESULT _stdcall CDX9Renderer::ResetDevice()
{
	HRESULT h = m_pDevice->Reset(&m_d3dpp);
	if FAILED(h) return D3D_ERROR(h);
	m_bReset = true;
	return S_OK;
}

HRESULT _stdcall CDX9Renderer::ResetDeviceEx(HWND hWnd, FWULONG nWidth, FWULONG nHeight)
{
	if (nWidth) m_d3dpp.BackBufferWidth = nWidth;
	if (nHeight) m_d3dpp.BackBufferHeight = nHeight;
	if (hWnd == NULL)
	{
		m_d3dpp.Windowed = false;
		m_d3dpp.hDeviceWindow = ::GetDesktopWindow();
	}
	else
	{
		m_d3dpp.Windowed = true;
		m_d3dpp.hDeviceWindow = hWnd;
	}
	m_hWnd = m_d3dpp.hDeviceWindow;
	return ResetDevice();
}

HRESULT _stdcall CDX9Renderer::Clear()
{
	return D3D_ERROR(m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, m_DXBackColor, 1.0f, 0));
}

HRESULT _stdcall CDX9Renderer::BeginFrame()
{
	if (m_bOnScreen && !m_hWnd) return ERROR(FW_E_NOTREADY);
	HRESULT h = S_OK;

	h = m_pDevice->TestCooperativeLevel();
	switch (h)
	{
		case D3DERR_DEVICELOST:
			TRACE(L"DX Device lost at %d\n", GetTickCount() - m_nStartTime);
			m_bFrozen = true;
			Sleep(200); //Wait a bit so we don't burn through cycles for no reason
			return h;
		case D3DERR_DEVICENOTRESET:
			if (m_ppfnCB[FW_CB_LOSTDEVICE].pFn) m_ppfnCB[FW_CB_LOSTDEVICE].pFn(this, m_ppfnCB[FW_CB_LOSTDEVICE].nParam, m_ppfnCB[FW_CB_LOSTDEVICE].pParam);
			h = ResetDevice();
			if FAILED(h) return D3D_ERROR(h);
			__SetInitialRenderStates();
			if (m_ppfnCB[FW_CB_RESETDEVICE].pFn) m_ppfnCB[FW_CB_RESETDEVICE].pFn(this, m_ppfnCB[FW_CB_RESETDEVICE].nParam, m_ppfnCB[FW_CB_RESETDEVICE].pParam);
			TRACE(L"DX Device reset at %d\n", GetTickCount() - m_nStartTime);
			//return D3DERR_DEVICENOTRESET;
			break;
		case D3DERR_DRIVERINTERNALERROR:
			return D3D_ERROR(h);
		default:
			if FAILED(h) return D3D_ERROR(h);
			break;
	}

	h = m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, m_DXBackColor, 1.0f, 0);
	if (FAILED(h)) return D3D_ERROR(h);

	h = m_pDevice->BeginScene(); 
	if (FAILED(h)) return D3D_ERROR(h);

	return S_OK;
}

HRESULT _stdcall CDX9Renderer::EndFrame()
{
	HRESULT h;
	h = m_pDevice->EndScene(); 
	if (FAILED(h)) 
		return h;

	if (m_bOnScreen)
	{
		// the screen
		h = m_pDevice->Present(NULL, NULL, m_hWnd, NULL); 

		if (h == D3DERR_DEVICELOST)
			return h;
		else
		if (FAILED(h)) 
			return D3D_ERROR(h);
		
		// after successful device reset/unfreeze
		m_bReset = false;
		if (m_bFrozen && IsDeviceAvailable() == S_OK)
		{
			m_bFrozen = false;
			if (!m_bPause && IsPlaying() == S_OK)
				m_nStartTime += ::GetTickCount() - m_nLastTime;		// put start time forward by the length of the freeze
		}

		// everything is successful - store the time for pausing/freezing
		if (!m_bPause)
			m_nLastTime = ::GetTickCount();
	}
	else
	{
		// off-screen
		if (m_pStillFilename)
		{
			// save a still frame
			h = D3DXSaveSurfaceToFileW(m_pStillFilename, (D3DXIMAGE_FILEFORMAT)m_fmtStill, m_pOffsSurface, NULL, NULL);
			if (FAILED(h)) return D3D_ERROR(h);
			free(m_pStillFilename);
			m_pStillFilename = NULL;
		}
		if (m_pMedia)
		{
			// add to an avi file...
			D3DLOCKED_RECT lockedRect;
			RECT rect = { 0, 0, m_nOffsW, m_nOffsH };

			HRESULT h;
			IDirect3DSurface9 *pSurface = NULL;
			D3DDISPLAYMODE ddm;
			h = m_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &ddm);
			if (FAILED(h)) return D3D_ERROR(h);
			h = m_pDevice->CreateOffscreenPlainSurface(m_nOffsW, m_nOffsH, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pSurface, NULL);
			if (FAILED(h)) return D3D_ERROR(h);

			h = m_pDevice->GetRenderTargetData(m_pOffsSurface, pSurface);
			if (FAILED(h)) return D3D_ERROR(h);

			h = pSurface->LockRect(&lockedRect, &rect, D3DLOCK_NO_DIRTY_UPDATE|D3DLOCK_NOSYSLOCK|D3DLOCK_READONLY);
			if (FAILED(h)) return D3D_ERROR(h);
			
			
			// OPTIMISED RGB -> YUV CONVERTER
			int nSrcLineSize = lockedRect.Pitch;
			int n2SrcLineSize = nSrcLineSize + nSrcLineSize;
			BYTE *pSrcLine0 = (BYTE*)(lockedRect.pBits);
			BYTE *pSrcLine1 = pSrcLine0 + nSrcLineSize;

			uint8_t **ppData;
			int *pLinesize;
			m_pMedia->get_video_frame(ppData, pLinesize);
			int nYLineSize = pLinesize[0];
			int n2YLineSize = nYLineSize + nYLineSize;
			int nULineSize = pLinesize[1];
			int nVLineSize = pLinesize[2];
			BYTE *pYLine0 = ppData[0];
			BYTE *pYLine1 = pYLine0 + nYLineSize;
			BYTE *pULine = ppData[1];
			BYTE *pVLine = ppData[2];

			for (FWULONG i = 0; i < m_nOffsH / 2; i++)
			{
				BYTE *pSrc0 = pSrcLine0;
				BYTE *pSrc1 = pSrcLine1;
				BYTE *pY0 = pYLine0;
				BYTE *pY1 = pYLine1;
				BYTE *pU = pULine;
				BYTE *pV = pVLine;
				for (FWULONG j = 0; j < m_nOffsW / 2; j++)
				{
					int r, g, b;
					int Y, U, V;
					int x = j + j;
					int y = i + i;

					// Y[0, 0]
					b = *pSrc0++;
					g = *pSrc0++;
					r = *pSrc0++;
					pSrc0++;	// jump over alpha
					Y = ( (  66 * r + 129 * g +  25 * b + 128) >> 8) +  16;
					*pY0++ = Y;

					// U/V
					U = ( ( -43 * r -  85 * g + 128 * b + 128) >> 8) + 128;
					V = ( ( 128 * r - 107 * g -  21 * b + 128) >> 8) + 128;
					*pU++ = U;
					*pV++ = V;

					// Y[1, 0]
					b = *pSrc0++;
					g = *pSrc0++;
					r = *pSrc0++;
					pSrc0++;	// jump over alpha
					Y = ( (  66 * r + 129 * g +  25 * b + 128) >> 8) +  16;
					*pY0++ = Y;

					// Y[0, 1]
					b = *pSrc1++;
					g = *pSrc1++;
					r = *pSrc1++;
					pSrc1++;	// jump over alpha
					Y = ( (  66 * r + 129 * g +  25 * b + 128) >> 8) +  16;
					*pY1++ = Y;

					// Y[1, 1]
					b = *pSrc1++;
					g = *pSrc1++;
					r = *pSrc1++;
					pSrc1++;	// jump over alpha
					Y = ( (  66 * r + 129 * g +  25 * b + 128) >> 8) +  16;
					*pY1++ = Y;
				}
				pSrcLine0 += n2SrcLineSize;
				pSrcLine1 += n2SrcLineSize;
				pYLine0 += n2YLineSize;
				pYLine1 += n2YLineSize;
				pULine += nULineSize;
				pVLine += nULineSize;
			}
										
			pSurface->UnlockRect();
			pSurface->Release();

			m_pMedia->write_video_frame();

//			SetTargetToScreen();
//			h = m_pAviFile->AppendNewFrame(m_nOffsW, m_nOffsH, pBits);
//			SetTargetOffScreen();

			if (FAILED(h)) return D3D_ERROR(h);

			return S_OK;
		}
	}
	return S_OK;
}



//HRESULT _stdcall CDX9Renderer::SaveMovieFrame()
//{
//	HRESULT h;
//	IDirect3DSurface9 *pSurface = NULL;
//	D3DDISPLAYMODE ddm;
//	h = m_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &ddm); 
//	if (FAILED(h)) return D3D_ERROR(h);
//	h = m_pDevice->CreateOffscreenPlainSurface(ddm.Width, ddm.Height, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pSurface, NULL); 
//	if (FAILED(h)) return D3D_ERROR(h);
//	RECT rect;
//	::GetWindowRect(m_hWnd, &rect);
//	UINT nWidth = rect.right - rect.left;
//	UINT nHeight = rect.bottom - rect.top;
//	BYTE *pBits = new BYTE[nWidth * nHeight * 4];
//	h = m_pDevice->GetFrontBufferData(0, pSurface);
//	if (FAILED(h)) return D3D_ERROR(h);
//	D3DLOCKED_RECT lockedRect;
//	h = pSurface->LockRect(&lockedRect, &rect, D3DLOCK_NO_DIRTY_UPDATE|D3DLOCK_NOSYSLOCK|D3DLOCK_READONLY);
//	if (FAILED(h)) return D3D_ERROR(h);
//	for(UINT i = 0; i < nHeight; i++)
//		memcpy((BYTE*)pBits + (nHeight-i-1) * nWidth * 4, (BYTE*)lockedRect.pBits + i * lockedRect.Pitch, nWidth * 4);
//	pSurface->UnlockRect();
//	h = m_pAviFile->AppendNewFrame(nWidth, nHeight, pBits);
//	if (FAILED(h)) return D3D_ERROR(h);
//	delete [] pBits;
//	pSurface->Release();
//	return S_OK;
//}



HRESULT _stdcall CDX9Renderer::SetTargetToScreen()
{
	if (m_bOnScreen) return S_OK;
	m_bOnScreen = true;

	assert(m_pBackSurface); assert(m_pBackDS);
	HRESULT h;
	h = m_pDevice->SetRenderTarget(0, m_pBackSurface); if (FAILED(h)) return D3D_ERROR(h);
	h = m_pDevice->SetDepthStencilSurface(m_pBackDS); if (FAILED(h)) return D3D_ERROR(h);
	return S_OK;
}

HRESULT _stdcall CDX9Renderer::SetTargetOffScreen()
{
	if (!m_bOnScreen) return S_OK;
	m_bOnScreen = false;

	assert(m_pOffsSurface); assert(m_pOffsDS);
	HRESULT h;
	h = m_pDevice->SetRenderTarget(0, m_pOffsSurface); if (FAILED(h)) return D3D_ERROR(h);
	h = m_pDevice->SetDepthStencilSurface(m_pOffsDS); if (FAILED(h)) return D3D_ERROR(h);
	return S_OK;
}

HRESULT _stdcall CDX9Renderer::OpenStillFile(LPCTSTR pFilename, enum FW_RENDER_BITMAP fmt)
{
	m_pStillFilename = (FWSTRING)_wcsdup(pFilename);
	m_fmtStill = fmt;
	return S_OK;
}

HRESULT _stdcall CDX9Renderer::CloseStillFile()
{
	return S_OK;
}

HRESULT _stdcall CDX9Renderer::OpenMovieFile(LPCTSTR pFilename, FWULONG nFramesPerSecond, FWULONG nBitrate)
{
	// convert from UNICODE to ASCII
	std::string filename = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(pFilename);

	FWULONG width, height;
	GetBackBufferSize(&width, &height);

	try
	{
		m_pMedia = new CMediaFile;
		m_pMedia->open(filename.c_str());
		m_pMedia->add_video_stream(width, height, nFramesPerSecond, 0*nBitrate);
		m_pMedia->disable_audio_stream();
	}
	catch (MEDIA_ERROR e)
	{
	}

	return S_OK;
}

//HRESULT _stdcall CDX9Renderer::OpenMovieFileWithCodec(LPCTSTR pFilename, FWULONG nFramesPerSecond, signed char *fccCodec)
//{
////	if (fccCodec)
////		m_pAviFile = new CAviFile(pFilename, nFramesPerSecond, mmioFOURCC(fccCodec[0], fccCodec[1], fccCodec[2], fccCodec[3]));
////	else
////		m_pAviFile = new CAviFile(pFilename, nFramesPerSecond);
//	return S_OK;
//}

HRESULT _stdcall CDX9Renderer::CloseMovieFile()
{
	if (m_pMedia)
	{
		m_pMedia->finish_and_close();
		delete m_pMedia;
	}
	m_pMedia = NULL;

	//if (m_pAviFile) delete m_pAviFile;
	//m_pAviFile = NULL;
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// animation control

HRESULT CDX9Renderer::Play()
{
	m_nStartTime = m_nLastTime = ::GetTickCount();
	m_bPause = m_bFrozen = false;
	m_bPlay = true;
	return S_OK;
}

HRESULT CDX9Renderer::IsPlaying()
{
	if (!m_bPlay) return S_FALSE;

	FWLONG nPlayTime;
	GetPlayTime(&nPlayTime);
	if (m_bTotal && nPlayTime >= m_nTotalTime) return S_FALSE;
	return S_OK;
}

HRESULT CDX9Renderer::Pause()
{
	m_bPause = !m_bPause;

	if (!m_bPause)	// if resume
	{
		m_nStartTime += ::GetTickCount() - m_nLastTime;		// put start time forward by the length of the pause
		m_nLastTime = ::GetTickCount();
	}

	return S_OK;
}

HRESULT CDX9Renderer::IsPaused()
{
	return (m_bPause || m_bFrozen) ? S_OK : S_FALSE;
}

HRESULT CDX9Renderer::Stop()
{
	m_nStartTime = m_nLastTime = 0;
	m_bPlay = m_bPause = m_bFrozen = false;
	return S_OK;
}

HRESULT CDX9Renderer::GetAccel(FWFLOAT *pA)
{
	if (pA) *pA = m_fAccel;
	return S_OK; 
}

HRESULT CDX9Renderer::PutAccel(FWFLOAT nA)
{
	FWLONG nPlayTime;
	GetPlayTime(&nPlayTime);

	m_fAccel = nA;

	if (m_bPlay)
		if (m_bPause || m_bFrozen)
			m_nStartTime = m_nLastTime - (FWLONG)(nPlayTime / m_fAccel);
		else
			m_nStartTime = ::GetTickCount() - (FWLONG)(nPlayTime / m_fAccel);

	return S_OK; 
}

HRESULT CDX9Renderer::GetTotalPlayingTime(FWLONG *pnMSec)
{
	if (pnMSec) *pnMSec = m_nTotalTime;
	return S_OK;
}

HRESULT CDX9Renderer::PutTotalPlayingTime(FWLONG nMSec)
{
	m_nTotalTime = nMSec;
	m_bTotal = true;
	return S_OK;
}

HRESULT CDX9Renderer::ClearTotalPlayingTime()
{
	m_nTotalTime = 0;
	m_bTotal = false;
	return S_OK;
}

HRESULT CDX9Renderer::GetPlayTime(FWLONG *pnMSec)
{
	FWLONG nPlayTime = 0;
	if (m_bPlay)
		if (m_bPause || m_bFrozen)
			nPlayTime = (FWLONG)((FWLONG)(m_nLastTime - m_nStartTime) * m_fAccel);
		else
			nPlayTime = (FWLONG)((FWLONG)(GetTickCount() - m_nStartTime) * m_fAccel);

	if (m_bTotal) 
		nPlayTime = min(nPlayTime, m_nTotalTime);

	if (pnMSec) *pnMSec = nPlayTime;
	return S_OK;
}

HRESULT CDX9Renderer::PutPlayTime(FWLONG nMSec)
{
	if (nMSec == 0)
		m_nStartTime = m_nLastTime = 0;
	else if (m_bPause || m_bFrozen)
		m_nStartTime = m_nLastTime - nMSec;
	else
		m_nStartTime = GetTickCount() - nMSec;
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// Low level access

HRESULT CDX9Renderer::GetDeviceHandle(FWULONG nId, /*[out, retval]*/ FWHANDLE *pHandle)
{
	switch (nId)
	{
	case 0: if (m_pD3D) m_pD3D->AddRef(); *pHandle = (FWHANDLE)m_pD3D; return S_OK;
	case 1: if (m_pDevice) m_pDevice->AddRef(); *pHandle = (FWHANDLE)m_pDevice; return S_OK;
	default: return Error(FW_E_BADINDEX);
	}
}

/////////////////////////////////////////////////////////////////////////////
// texture function

HRESULT _stdcall CDX9Renderer::CreateTexture(ITexture** ppTexture)
{
	if (ppTexture != NULL)
	{
		*ppTexture = new CDX9Texture;
		FWDevice()->RegisterObject(*ppTexture);
		HRESULT h = (*ppTexture)->PutContextObject(0, IID_IDirect3DDevice9, m_pDevice);
		return h;
	}
	return S_FALSE;
}