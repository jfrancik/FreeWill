// material.h
//
////////////////////////////////////////////////////////////////////////

#if !defined(__MATERIAL_H)
#define __MATERIAL_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\fwlib\factory.h"
#include "..\fwlib\fwunknown.h"
#include "matplus.h"

const
	int MAX_MATERIAL_TEXTURES_COUNT = 8;	

class CMaterial: public FWUNKNOWN<IMaterial, IID_IMaterial, IMaterial>
{
protected:
	FWCOLOR m_colorAmbient;
	FWCOLOR m_colorDiffuse;
	FWCOLOR m_colorSpecular;
	FWFLOAT m_fShininess;
	FWFLOAT m_fShininessStrength;
	FWFLOAT m_fSelfIllumination;
	FWCOLOR m_colorSelfIllumination;
	BOOL m_bTwoSided;
	FWULONG m_nAlphaMode;
	ITexture *m_pTextures[MAX_MATERIAL_TEXTURES_COUNT];
	
public:
	virtual HRESULT __stdcall SetAmbientColor(FWCOLOR val)			{ m_colorAmbient = val; return S_OK; }
	virtual HRESULT __stdcall SetDiffuseColor(FWCOLOR val)			{ m_colorDiffuse = val; return S_OK; }
	virtual HRESULT __stdcall SetSpecularColor(FWCOLOR val)			{ m_colorSpecular = val; return S_OK; }
	virtual HRESULT __stdcall SetShininess(FWFLOAT fVal)			{ m_fShininess = fVal; return S_OK; }
	virtual HRESULT __stdcall SetShininessStrength(FWFLOAT fVal)	{ m_fShininessStrength = fVal; return S_OK; }
	virtual HRESULT __stdcall SetSelfIlluminationOff()				{ memset(&m_colorSelfIllumination, 0, sizeof(FWCOLOR)); m_fSelfIllumination = 0; return S_OK; }
	virtual HRESULT __stdcall SetSelfIllumination(FWCOLOR color, FWFLOAT fStrength)
																	{ m_colorSelfIllumination = color;
																	  m_fSelfIllumination = fStrength; 
																	  return S_OK; }
	virtual HRESULT __stdcall SetSelfIlluminationColor(FWCOLOR val)	{ m_colorSelfIllumination = val; return S_OK; }
	virtual HRESULT __stdcall SetTwoSided(BOOL bVal)				{ m_bTwoSided = bVal; return S_OK; }
	virtual HRESULT __stdcall SetAlphaMode(FWULONG val)				{ m_nAlphaMode = val; return S_OK; }
	virtual HRESULT __stdcall SetAlpha(FWFLOAT fAlpha)				{ m_nAlphaMode = (fAlpha == 1) ? MAT_ALPHA_DISABLE : MAT_ALPHA_MATERIAL; m_colorDiffuse.a = fAlpha; return S_OK; }



	virtual HRESULT __stdcall GetAmbientColor(FWCOLOR* p)			{ if (p) *p = m_colorAmbient; return S_OK; }
	virtual HRESULT __stdcall GetDiffuseColor(FWCOLOR* p)			{ if (p) *p = m_colorDiffuse; return S_OK; }
	virtual HRESULT __stdcall GetSpecularColor(FWCOLOR* p)			{ if (p) *p = m_colorSpecular; return S_OK; }
	virtual HRESULT __stdcall GetShininess(FWFLOAT* p)				{ if (p) *p = m_fShininess; return S_OK; }
	virtual HRESULT __stdcall GetShininessStrength(FWFLOAT* p)		{ if (p) *p = m_fShininessStrength; return S_OK; }
	virtual HRESULT __stdcall GetSelfIlluminationStrength(FWFLOAT* p){ if (p) *p = m_fSelfIllumination; return S_OK; }
	virtual HRESULT __stdcall GetSelfIlluminationColor(FWCOLOR* p)	{ if (p) *p = m_colorSelfIllumination; return S_OK; }
	virtual HRESULT __stdcall GetTwoSided(BOOL* p)					{ if (p) *p = m_bTwoSided; return S_OK; }
	virtual HRESULT __stdcall GetAlphaMode(FWULONG *p)				{ if (p) *p = m_nAlphaMode; return S_OK; }

	virtual HRESULT __stdcall SetTexture(FWULONG iTexture, ITexture* pTexture);
	virtual HRESULT __stdcall GetTexture(FWULONG iTexture, ITexture** ppTexture);

	DECLARE_FACTORY_CLASS(Material, Material)
	FW_RTTI(Material)

	CMaterial();
	~CMaterial();
};

#endif
