// bodymove.h
//
////////////////////////////////////////////////////////////////////////

#if !defined(__BODYMOVE_H)
#define __BODYMOVE_H

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
// CActionBend

class CActionBend : public CAction
{
public:

	// Overrides
	virtual HRESULT __stdcall HandleEvent(struct ACTION_EVENT *pEvent);
	virtual HRESULT __stdcall Create(IFWCreateContext*);

    DECLARE_FACTORY_CLASS(ActionBend, Action) 
	FW_RTTI(ActionBend)

protected:
	PBODY m_pBody;

public:
	CActionBend();
	~CActionBend();
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// CActionSquat

class CActionSquat : public CAction
{
public:

	// Overrides
	virtual HRESULT __stdcall HandleEvent(struct ACTION_EVENT *pEvent);
	virtual HRESULT __stdcall Create(IFWCreateContext*);

    DECLARE_FACTORY_CLASS(ActionSquat, Action) 
	FW_RTTI(ActionSquat)

protected:
	PBODY m_pBody;

public:
	CActionSquat();
	~CActionSquat();
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// CActionSwing

class CActionSwing : public CAction
{
public:

	// Overrides
	virtual HRESULT __stdcall HandleEvent(struct ACTION_EVENT *pEvent);
	virtual HRESULT __stdcall Create(IFWCreateContext*);

    DECLARE_FACTORY_CLASS(ActionSwing, Action) 
	FW_RTTI(ActionSwing)

protected:
	PBODY m_pBody;

public:
	CActionSwing();
	~CActionSwing();
};


#endif
