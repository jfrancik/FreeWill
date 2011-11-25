// manip.h
//
////////////////////////////////////////////////////////////////////////

#if !defined(__MANIP_H)
#define __MANIP_H

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
// CActionGrasp

class CActionGrasp : public CAction
{
public:

	// Overrides
	virtual HRESULT __stdcall HandleEvent(struct ACTION_EVENT *pEvent);
	virtual HRESULT __stdcall Create(IFWCreateContext*);

    DECLARE_FACTORY_CLASS(ActionGrasp, Action) 
	FW_RTTI(ActionGrasp)

protected:
	PBODY m_pBody;

public:
	CActionGrasp();
	~CActionGrasp();
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// CActionTake

class CActionTake : public CAction
{
public:

	// Overrides
	virtual HRESULT __stdcall HandleEvent(struct ACTION_EVENT *pEvent);
	virtual HRESULT __stdcall Create(IFWCreateContext*);

    DECLARE_FACTORY_CLASS(ActionTake, Action) 
	FW_RTTI(ActionTake)

protected:
	PBODY m_pBody;
	PBONE m_pGoal;

public:
	CActionTake();
	~CActionTake();
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// CActionPoint

class CActionPoint : public CAction
{
public:

	// Overrides
	virtual HRESULT __stdcall HandleEvent(struct ACTION_EVENT *pEvent);
	virtual HRESULT __stdcall Create(IFWCreateContext*);

    DECLARE_FACTORY_CLASS(ActionPoint, Action) 
	FW_RTTI(ActionPoint)

protected:
	PBODY m_pBody;
	PBONE m_pGoal;
	FWVECTOR m_vec;

public:
	CActionPoint();
	~CActionPoint();
};

#endif
