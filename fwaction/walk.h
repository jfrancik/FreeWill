// walk.h
//
////////////////////////////////////////////////////////////////////////

#if !defined(__WALK_H)
#define __WALK_H

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
// CActionLegged - an abstract class

class CActionLegged : public CAction
{
protected:
	// Body
	PBODY m_pBody;			// body
	
	// Left/Right Resolution
	bool m_bRight;			// set if right leg goes ahead
	
	// Body measurements
	FWFLOAT m_fHeight;	// Height of the leg, from BODY_FOOT to BODY_LEG, unsigned
	FWFLOAT m_fSpan;	// Leg span, from BODY_PELVIS to BODY_LEG, positive on the left side (!)
	FWFLOAT m_fFoot;	// Foot step distance, BODY_FOOT to BODY_LEG casted to 2D floor, positive if forward

	void ResolveLeggedPose(bool bStepForward = true);

public:
	CActionLegged();
	~CActionLegged();
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// CActionStep

class CActionStep : public CActionLegged
{
public:
	// Overrides
	virtual HRESULT __stdcall HandleEvent(struct ACTION_EVENT *pEvent);
	virtual HRESULT __stdcall Create(IFWCreateContext*);

    DECLARE_FACTORY_CLASS(ActionStep, Action) 
	FW_RTTI(ActionStep)

protected:
	// Parameters
	FWFLOAT m_param1;
	enum PARAM1 { PARAM1_DEF, PARAM1_ANGLE, PARAM1_LEN, PARAM1_FD, PARAM1_ABS };
	PARAM1 m_param1Flag;
	
	FWFLOAT m_param2;
	enum PARAM2 { PARAM2_DEF, PARAM2_ANGLE, PARAM2_ASIDE, PARAM2_ABS };
	PARAM2 m_param2Flag;

	FWFLOAT m_param3;

	// Resolved Parameters
	FWFLOAT m_fStep;
	FWFLOAT m_fHead;

	// Other Params
	FWFLOAT m_fBend;

protected:
	// Tool Functions
	HRESULT ResolveAngleAngle(FWFLOAT fStep, FWFLOAT fHead);
	HRESULT ResolveAngleAside(FWFLOAT fStep, FWFLOAT fAside);
	HRESULT ResolveLenAngle(FWFLOAT fLen, FWFLOAT fHead);
	HRESULT ResolveAbsAbs(FWFLOAT x, FWFLOAT y);

	void SetParam1(PARAM1 flag);
	void SetParam2(PARAM2 flag);

public:
	CActionStep();
	~CActionStep();
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// CActionTurn

class CActionTurn : public CActionLegged
{
public:

	// Overrides
	virtual HRESULT __stdcall HandleEvent(struct ACTION_EVENT *pEvent);
	virtual HRESULT __stdcall Create(IFWCreateContext*);

    DECLARE_FACTORY_CLASS(ActionTurn, Action) 
	FW_RTTI(ActionTurn)

protected:
	FWFLOAT m_fRot;			// If not zero, Total Rotation in radians. If zero - means standard "turn back"
	FWULONG m_nSteps;		// Step Count:  default value taken so that rotation at each step is close to 60 degrees
	FWFLOAT m_fDist;		// Distance to proceed after completion the turn (0 by default)

public:
	CActionTurn();
	~CActionTurn();
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// CActionWalk

class CActionWalk : public CActionLegged
{
	IAction* MakeStep(FWULONG nTimeStamp);
public:

	// Overrides
	virtual HRESULT __stdcall HandleEvent(struct ACTION_EVENT *pEvent);
	virtual HRESULT __stdcall Create(IFWCreateContext*);

    DECLARE_FACTORY_CLASS(ActionWalk, Action) 
	FW_RTTI(ActionWalk)

protected:
	FWFLOAT m_x, m_y;		// destination coordinates
	FWFLOAT m_fLen;			// max length of step
	FWFLOAT m_fHead;		// max heading angle
	IAction *m_pActionStep;	// current step (a weak pointer)

	ACTION_SUBS *m_pSubs;	// EVENT_END subscription from the Step action

public:
	CActionWalk();
	~CActionWalk();
};



#endif
