// body.h
//
////////////////////////////////////////////////////////////////////////

#if !defined(__BODY_H)
#define __BODY_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\fwlib\factory.h"
#include "..\fwlib\fwunknown.h"
#include "bodyplus.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// CBody

class CBody : public FWUNKNOWN<IBody, IID_IBody, IBody>
{
private:
	int CalcIndex(FWULONG);

public:
	// Loading body parts from an external model
	virtual HRESULT __stdcall LoadBody(IKineNode *pBody, FWULONG nSchema);
	virtual HRESULT __stdcall LoadBodyPart(IKineNode *pNode, FWSTRING label, FWULONG nIndex);
	virtual HRESULT __stdcall RemoveAll();

	// Getting body parts

	virtual HRESULT __stdcall GetBodyPart(FWULONG nIndex, REFIID iid, /*[out, retval, iid_is(iid)]*/ IFWUnknown **p);
	virtual IFWUnknown *__stdcall BodyPart(FWULONG nIndex, REFIID iid);
	virtual IKineChild *__stdcall BodyChild(FWULONG nIndex);
	virtual IKineNode  *__stdcall BodyNode(FWULONG nIndex);
	virtual IBounding  *__stdcall BodyBounding(FWULONG nIndex);

	// Queries
	virtual HRESULT __stdcall GetInfo(FWULONG nQuery, FWULONG nIndex, /*[out, retval]*/ FWULONG*);
	virtual HRESULT __stdcall GetFInfo(FWULONG nQuery, FWULONG nIndex, /*[out, retval]*/ FWFLOAT*);
	virtual HRESULT __stdcall GetXYZ(FWULONG nIndex, /*[out, retval]*/ FWVECTOR *pv);

	// Helpers
	virtual HRESULT __stdcall CreateCompatibleTransform(/*[out, retval]*/ ITransform**);

	DECLARE_FACTORY_CLASS(Body, Body)
	FW_RTTI(Body)

private:
	IKineChild **m_ppParts;

public:
	CBody();
	~CBody();
};

#endif