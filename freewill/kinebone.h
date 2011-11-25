// kinebone.h
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(_KINEOBJ3D__)
#define _KINEOBJ3D__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\fwlib\factory.h"
#include "..\fwlib\fwunknown.h"
#include "kineplus.h"
#include "kinetemplate.h"

class CKineBone : public CTKineNode<FWUNKNOWN<
								IKineNode,
								IID_IKineChild, IKineChild,
								IID_IKineNode, IKineNode,
								IID_IKineObj3D, IKineObj3D> >
{
public:
	// IKineNode implementation 
	virtual HRESULT __stdcall CreateChild(LPOLESTR pLabel, /*[out, retval]*/ IKineNode**);

	DECLARE_FACTORY_CLASS(KineBone, KineObj3D)
	FW_RTTI(KineBone)

public:
	CKineBone()		{ }
	~CKineBone()	{ }
};

#endif