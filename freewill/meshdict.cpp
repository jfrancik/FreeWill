// mesh.cpp : Defines the mesh object
//

#include "stdafx.h"
#include "meshdict.h"

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// Class CMeshDictionary

CMeshDictionary::CMeshDictionary()
{ 
}

CMeshDictionary::~CMeshDictionary()
{
	for (BONEVECTOR::iterator i = m_vector.begin(); i != m_vector.end(); i++)
		free (*i);
	for (SYNMAP::iterator i = m_synmap.begin(); i != m_synmap.end(); i++)
	{
		free(i->first);
		free(i->second);
	}
}

///////////////////////////////////////////////////////////
// main functionality

HRESULT CMeshDictionary::GetNum(FWULONG *p)
{
	*p = (FWULONG)m_vector.size();
	return S_OK;
}

HRESULT CMeshDictionary::GetName (FWULONG i, LPOLESTR *pName)
{
	*pName = NULL;
	if (i >= m_vector.size())
		return ERROR(FW_E_BADINDEX);
	else
		*pName = m_vector[i];
	return S_OK;
}

HRESULT CMeshDictionary::GetIndex(LPOLESTR pName, FWULONG *pI)
{
	GetSynonim(pName, &pName);	// optionally change the name into its synonim
	BONEMAP::iterator i = m_map.find(pName);
	if (i != m_map.end())
	{
		if (pI) *pI = i->second;
		return S_OK;
	}
	else
	{
		// new name
		LPOLESTR pCopy = _wcsdup(pName);
		m_map[pCopy] = (FWULONG)m_vector.size();
		if (pI) *pI = (FWULONG)m_vector.size();
		m_vector.push_back(pCopy);
		return S_FALSE;
	}
}

HRESULT CMeshDictionary::GetIndexIfExists(LPOLESTR pName, FWULONG *pI)
{
	GetSynonim(pName, &pName);	// optionally change the name into its synonim
	BONEMAP::iterator i = m_map.find(pName);
	if (i != m_map.end())
	{
		if (pI) *pI = i->second;
		return S_OK;
	}
	else
	{
		if (pI) *pI = NULL;
		return S_FALSE;
	}
}

/////////////////////////////////////////////////////////////////////////////
// synonims

HRESULT CMeshDictionary::PutSynonim(LPOLESTR pFor, LPOLESTR pSynonim)
{
	SYNMAP::iterator i = m_synmap.find(pFor);
	if (i == m_synmap.end())
	{
		// new synonim
		LPOLESTR pCopy = _wcsdup(pFor);
		m_synmap[pCopy] = _wcsdup(pSynonim);
	}
	return S_OK;
}

HRESULT CMeshDictionary::GetSynonim(LPOLESTR pFor, LPOLESTR *ppSynonim)
{
	SYNMAP::iterator i = m_synmap.find(pFor);
	if (i == m_synmap.end())
		*ppSynonim = pFor;
	else
		*ppSynonim = i->second;
	return S_OK;
}

HRESULT CMeshDictionary::GetSynonimNum(FWULONG *p)
{
	*p = (FWULONG)m_synmap.size();
	return S_OK;
}

HRESULT CMeshDictionary::GetSynonimAt(FWULONG *pi, /*[out]*/ LPOLESTR*, LPOLESTR*)
{
	return ERROR(E_NOTIMPL);
}

