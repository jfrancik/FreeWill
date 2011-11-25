// bsphere.h
//
////////////////////////////////////////////////////////////////////////

#if !defined(__BSPHERE_H)
#define __BSPHERE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\fwlib\factory.h"
#include "..\fwlib\fwunknown.h"
#include "boundplus.h"
#include "kinetemplate.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// CBoundingSphere

class CBoundingSphere : public CTKineNode<FWUNKNOWN<
							BASECLASS<IBounding, IKineNode>,
							IID_IBounding, IBounding,
							IID_IKineNode, IKineNode,
							IID_IKineChild, IKineChild,
							IID_IKineObj3D, IKineObj3D> >
{
public:

	// IKineNode: CreateChild returns E_NOTIMPL
	virtual HRESULT _stdcall CreateChild(LPOLESTR pLabel, /*[out, retval]*/ IKineNode **p);

	// Data Transfer
	virtual HRESULT _stdcall PutData(enum BOUND_FORMAT nFormat, FWULONG nSize, /*[in, size_is(nSize)]*/ BYTE *pBuf);
	virtual HRESULT _stdcall GetData(enum BOUND_FORMAT nFormat, FWULONG nSize, /*[out, size_is(nSize)]*/ BYTE *pBuf);

	// Query about Supported Formats
	HRESULT _InternalQueryInputFormat(enum BOUND_FORMAT, /*[out, retval]*/ enum BOUND_PREFERENCE*);
	HRESULT _InternalQueryOutputFormat(enum BOUND_FORMAT, /*[out, retval]*/ enum BOUND_PREFERENCE*);
	virtual HRESULT _stdcall QueryInputFormat(enum BOUND_FORMAT, /*[out, retval]*/ enum BOUND_PREFERENCE*);
	virtual HRESULT _stdcall QueryOutputFormat(enum BOUND_FORMAT, /*[out, retval]*/ enum BOUND_PREFERENCE*);
	virtual HRESULT _stdcall QueryInputFormatEx(FWULONG nLen, /*[in, size_is(nLen)]*/ enum BOUND_FORMAT*, /*[out, size_is(nLen)]*/ enum BOUND_PREFERENCE*);
	virtual HRESULT _stdcall QueryOutputFormatEx(FWULONG nLen, /*[in, size_is(nLen)]*/ enum BOUND_FORMAT*, /*[out, size_is(nLen)]*/ enum BOUND_PREFERENCE*);
	virtual HRESULT _stdcall QueryDetect(IBounding *pWith, /*[out]*/ enum BOUND_FORMAT*, /*[out]*/ enum BOUND_PREFERENCE*, /*[out]*/ enum BOUND_PRECISION*);

	// Collision Detection Test
	virtual HRESULT _stdcall Detect(IBounding *pWith);
	virtual HRESULT _stdcall DetectEx(IBounding *pWith, BOUND_FORMAT fmt);

	// Sub-Boundings (with Hierarchical Tree Algorithms only)
	virtual HRESULT _stdcall GetSubBoundings(/*[in, out]*/ FWULONG *nSize, /*[out, size_is(*nSize)]*/ IBounding**)
	{ return ERROR(E_NOTIMPL); }

	DECLARE_FACTORY_CLASS(BoundingSphere, Bounding)
	FW_RTTI(BoundingSphere)

	CBoundingSphere();
	~CBoundingSphere();

protected:
	BOUND_SPHERE m_sphere;
};

#endif