// meshdict.h
//
////////////////////////////////////////////////////////////////////////

#if !defined(__MESHDICT_H)
#define __MESHDICT_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\fwlib\factory.h"
#include "..\fwlib\fwunknown.h"
#include "meshplus.h"
#include <map>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////////////////////
// CMeshDictionary

class CMeshDictionary : public FWUNKNOWN<IMeshDictionary, IID_IMeshDictionary, IMeshDictionary >
{
	// main dictionary
	virtual HRESULT _stdcall GetNum(FWULONG*);
	virtual HRESULT _stdcall GetName (FWULONG i, LPOLESTR*);
	virtual HRESULT _stdcall GetIndex(LPOLESTR pName, FWULONG*);
	virtual HRESULT _stdcall GetIndexIfExists(LPOLESTR pName, FWULONG*);

	// synonims
	virtual HRESULT _stdcall PutSynonim(LPOLESTR pFor, LPOLESTR pSynonim);
	virtual HRESULT _stdcall GetSynonim(LPOLESTR pFor, LPOLESTR*);
	virtual HRESULT _stdcall GetSynonimNum(FWULONG*);
	virtual HRESULT _stdcall GetSynonimAt(FWULONG *pi, /*[out]*/ LPOLESTR*, LPOLESTR*);

	DECLARE_FACTORY_CLASS(MeshDictionary, MeshDictionary)
	FW_RTTI(MeshDictionary)

protected:
	struct STRLESS { bool operator()(LPOLESTR p, LPOLESTR q) const { return wcscmp(p, q) < 0; } };
	typedef std::map<LPOLESTR, FWULONG, STRLESS> BONEMAP;
	typedef std::vector<LPOLESTR> BONEVECTOR;
	typedef std::map<LPOLESTR, LPOLESTR, STRLESS> SYNMAP;
	BONEMAP m_map;
	BONEVECTOR m_vector;
	SYNMAP m_synmap;

public:
	CMeshDictionary();
	~CMeshDictionary();
};

#endif
