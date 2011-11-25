// kinebone.cpp : Defines 3D objects - IKineObj3D, IKineChild & IKineNode implementation
//

#include "stdafx.h"
#include "kinebone.h"

///////////////////////////////////////////////////////////
//
// CKineBone implementation

HRESULT __stdcall CKineBone::CreateChild(LPOLESTR pLabel, /*[out, retval]*/ IKineNode **p)
{
	HRESULT h = GetClone(IID_IKineNode, (IFWUnknown**)p);
	if (FAILED(h)) return h;

	h = AddChild(pLabel, *p);
	if (FAILED(h)) { (*p)->Release(); return h; }

	return S_OK;
}

