// reach.h
//
////////////////////////////////////////////////////////////////////////

#if !defined(__REACH_H)
#define __REACH_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../fwlib/factory.h"
#include "../fwlib/fwunknown.h"
#include <kineplus.h>
#include <bodyplus.h>
#include "action.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// CActionReach

class CActionReach : public CAction
{
public:

	// Overrides
	virtual HRESULT __stdcall HandleEvent(struct ACTION_EVENT *pEvent);
	virtual HRESULT __stdcall Create(IFWCreateContext*);

    DECLARE_FACTORY_CLASS(ActionReach, Action) 
	FW_RTTI(ActionReach)

protected:
	IKineObj3D *m_pArm1, *m_pArm2;			// arms
	IKineObj3D *m_pTerm, *m_pDest;			// terminator, destination
	FWVECTOR m_vDTerm, m_vDDest;				// vectors to be added to terminator & destination
	FWULONG m_bRight;							// right (or left) hand
	FWFLOAT m_fElbowCorrectionAngle;			// elbow correction angle

public:
	CActionReach();
	~CActionReach();
};

#endif
