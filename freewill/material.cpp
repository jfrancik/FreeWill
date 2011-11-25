// material.cpp : Defines the texture object
//

#include "stdafx.h"
#include "material.h"

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// Class CMaterial

CMaterial::CMaterial()
{
	for (int i=0; i < MAX_MATERIAL_TEXTURES_COUNT; i++)
		m_pTextures[i] = NULL;
	m_colorAmbient.r = m_colorDiffuse.r = m_colorSpecular.r = 1.0f;
	m_colorAmbient.g = m_colorDiffuse.g = m_colorSpecular.g = 1.0f;
	m_colorAmbient.b = m_colorDiffuse.b = m_colorSpecular.b = 1.0f;
	m_colorAmbient.a = m_colorDiffuse.a = m_colorSpecular.a = 1.0f;
	m_fShininess = 0.0f;
	m_fShininessStrength = 0.0f;
	m_fSelfIllumination = 0;
	memset(&m_colorSelfIllumination, 0, sizeof(m_colorSelfIllumination));;
	m_bTwoSided = FALSE;
	m_nAlphaMode = MAT_ALPHA_DISABLE;
}

CMaterial::~CMaterial()
{
	for (int i=0; i < MAX_MATERIAL_TEXTURES_COUNT; i++)
		if (m_pTextures[i]) 
			m_pTextures[i]->Release();
}

HRESULT __stdcall CMaterial::SetTexture(FWULONG iTexture, ITexture* pTexture)
{
	if (iTexture < MAX_MATERIAL_TEXTURES_COUNT)
	{
		if (m_pTextures[iTexture])
		{
			m_pTextures[iTexture]->Release();
			m_pTextures[iTexture] = NULL;
		}
		if (pTexture != NULL)
		{
			m_pTextures[iTexture] = pTexture;
			m_pTextures[iTexture]->AddRef();
		}
		return S_OK;
	}
	return S_FALSE;
}

HRESULT __stdcall CMaterial::GetTexture(FWULONG iTexture, ITexture** ppTexture)
{
	if (iTexture < MAX_MATERIAL_TEXTURES_COUNT && ppTexture)
	{
		*ppTexture = m_pTextures[iTexture];
		if (*ppTexture != NULL)
			(*ppTexture)->AddRef();
		return S_OK;
	}
	return S_FALSE;
}