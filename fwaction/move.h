// move.h
//
////////////////////////////////////////////////////////////////////////

#if !defined(__MOVE_H)
#define __MOVE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../fwlib/factory.h"
#include "../fwlib/fwunknown.h"
#include <kineplus.h>
#include <boundplus.h>
#include "action.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// CActionMove

class CActionMove : public CAction
{
public:

	// Overrides
	virtual HRESULT __stdcall HandleEvent(struct ACTION_EVENT *pEvent);
	virtual HRESULT __stdcall Create(IFWCreateContext*);

    DECLARE_FACTORY_CLASS(ActionMove, Action) 
	FW_RTTI(ActionMove)

protected:
	IKineObj3D *m_pObj;				// the object
	FWVECTOR m_v;

public:
	CActionMove();
	~CActionMove();
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// CActionMoveTo

class CActionMoveTo : public CAction
{
public:

	// Overrides
	virtual HRESULT __stdcall HandleEvent(struct ACTION_EVENT *pEvent);
	virtual HRESULT __stdcall Create(IFWCreateContext*);

    DECLARE_FACTORY_CLASS(ActionMoveTo, Action) 
	FW_RTTI(ActionMoveTo)

protected:
	IKineObj3D *m_pObj;				// the object
	IKineChild *m_pRefObj;			// the reference object for the m_v vector (may be NULL)
	FWVECTOR m_v;

public:
	CActionMoveTo();
	~CActionMoveTo();
};

#endif
