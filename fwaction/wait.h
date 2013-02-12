// wait.h
//
////////////////////////////////////////////////////////////////////////

#if !defined(__WAIT_H)
#define __WAIT_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../fwlib/factory.h"
#include "../fwlib/fwunknown.h"
#include <kineplus.h>
#include <bodyplus.h>
#include "action.h"

interface IKineChild;
interface IBody;
typedef IKineChild *PBONE;
typedef IBody *PBODY;

////////////////////////////////////////////////////////////////////////////////////////////////////
// CActionWait

class CActionWait : public CAction
{
public:
	// Overrides
	virtual HRESULT __stdcall HandleEvent(struct ACTION_EVENT *pEvent);
	virtual HRESULT __stdcall Create(IFWCreateContext*);

	virtual HRESULT __stdcall GetCompleteTime(FWLONG *p)		{ if (p) *p = m_nTimeStamp; return S_OK; }
	virtual FWLONG __stdcall CompleteTime()						{ return m_nTimeStamp; }

	DECLARE_FACTORY_CLASS(ActionWait, Action) 
	FW_RTTI(ActionWait)

protected:

	IBody *m_pBody;
	FWLONG m_nTimeStamp;

public:
	CActionWait();
	~CActionWait();
};

#endif
