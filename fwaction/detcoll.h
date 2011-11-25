// detcoll.h
//
////////////////////////////////////////////////////////////////////////

#if !defined(__DETCOLL_H)
#define __DETCOLL_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../fwlib/factory.h"
#include "../fwlib/fwunknown.h"
#include <kineplus.h>
#include <boundplus.h>
#include "action.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// CActionDetColl

class CActionDetColl : public CAction
{
public:

	// Overrides
	virtual HRESULT __stdcall HandleEvent(struct ACTION_EVENT *pEvent);
	virtual HRESULT __stdcall Create(IFWCreateContext*);

	BOOL Test();

    DECLARE_FACTORY_CLASS(ActionDetColl, Action) 
	FW_RTTI(ActionDetColl)

protected:
	IBounding **m_ppBox1, **m_ppBox2;
	FWULONG m_nLen1, m_nLen2;
	BOOL m_bColl;

public:
	CActionDetColl();
	~CActionDetColl();
};

#endif
