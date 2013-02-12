// action.h
//
////////////////////////////////////////////////////////////////////////

#if !defined(__ACTION_H)
#define __ACTION_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../fwlib/factory.h"
#include "../fwlib/fwunknown.h"
#include "actionplus.h"
#include <list>
#include <set>


////////////////////////////////////////////////////////////////////////////////////////////////////
// CAction

class CAction : public FWUNKNOWN<IAction, IID_IAction, IAction>
{
protected:

	// Activity Flag - used by Suspend and Resume
	FWULONG m_nSuspended;

	// Time Data
	FWLONG m_nStartTime;			// start time for the action (considering suspend times)
	FWLONG m_nPeriod;				// the period (total time) of action
	FWLONG m_nSuspendTime;			// time when action suspended or zero
	FWFLOAT m_fTimePrev;			// time stamp for the GetProgPhase function

	// Style info (optional)
	FWSTRING m_pStyle;
	std::set<std::wstring> m_styles;

	// Envelope Data
	struct ACTION_ENVELOPE_DATA m_env;

	// Current Event
	ACTION_EVENT *m_pCurEvent;

	// Handle Event Hook Function
	HANDLE_EVENT_HOOK_FUNC m_pfHook;
	FWULONG m_nParam;
	void *m_pParam;

	// Typedefs
	struct ACTION_SUBS_EX;
	struct COLL;
	typedef std::list<ACTION_SUBS_EX>		LIST_OF_SUBS;
	typedef std::map<INT, LIST_OF_SUBS>		MAP_INT_LIST_OF_SUBS;
	typedef std::map<FWULONG, COLL>			MAP_OF_COLL;

	// Extended version of ACTION_SUBS
	// Contains C++ specific info: unsubscribtion iterator and sorting operators
	struct ACTION_SUBS_EX : public ACTION_SUBS
	{
		ACTION_SUBS_EX(IAction *pOriginator, IAction *pSubscriber, FWULONG nEvent, FWULONG nFlags, FWLONG nTrigger = 0, FWULONG nId = 0);
		LIST_OF_SUBS *pColl;
		LIST_OF_SUBS::iterator iter;
		friend bool operator <(const ACTION_SUBS_EX &i1, const ACTION_SUBS_EX &i2) { return i1.nTrigger < i2.nTrigger; }
		friend bool operator >(const ACTION_SUBS_EX &i1, const ACTION_SUBS_EX &i2) { return i1.nTrigger > i2.nTrigger; }
	};

	// Subscription Lists
	struct COLL			// contains collections of all subscribers of a given event code
	{
		LIST_OF_SUBS lstAny;			// subscriptions with the ACTION_ANY flag
		MAP_INT_LIST_OF_SUBS mapEq;		// subscriptions with the ACTION_EQ flag
		LIST_OF_SUBS lstLte;			// subscriptions with the ACTION_LTE flag
		LIST_OF_SUBS lstGte;			// subscriptions with the ACTION_GTE flag
	};
	
	MAP_OF_COLL m_Subsribers;	// the Subscription List: maps event code to COLL structure

	//////////////////////////////////////////////
	// Construction & Deconstruction

public:
	CAction();
	~CAction();

	DECLARE_FACTORY_CLASS(Action, Action)
	FW_RTTI(Action)

	//////////////////////////////////////////////
	// Error Definitions

	FW_ERROR_BEGIN
		FW_ERROR_ENTRY(ACTION_E_CANNOTSUBSCRIBE,	L"Action cannot subscribe for the given event", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(ACTION_E_CANNOTSETUP,		L"Action cannot set-up for the given verb", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(ACTION_E_INVALIDPERIOD,		L"Action timing function called with an invalid value of action period", FW_SEV_CRITICAL)
	FW_ERROR_END

	//////////////////////////////////////////////
	// Generic Overridables (usually overwritten)

	virtual HRESULT __stdcall HandleEvent(struct ACTION_EVENT *pEvent);
	virtual HRESULT __stdcall Create(IFWCreateContext*);

	//////////////////////////////////////////////
	// Helpers for Implementation of Derivated Classes
protected:
	HRESULT QueryStdParams(IFWEnumParams *pEnum, struct ACTION_SUBS **ppSubs = NULL);
	HRESULT ErrorStdParams(IFWEnumParams *pEnum, HRESULT h);

	//////////////////////////////////////////////
	// Subscription

public:
	virtual HRESULT __stdcall Subscribe(IAction *pSubscriber, FWULONG nEvent, FWULONG nFlags, FWLONG nTrigger, FWULONG nId, struct ACTION_SUBS **pHandle);
	virtual HRESULT __stdcall UnSubscribe(FWLONG nTimeStamp, ACTION_SUBS *pSubs);
	virtual HRESULT __stdcall UnSubscribeAll();
	virtual HRESULT __stdcall GetSubscriptionCount(/*[out, retval]*/ FWULONG *p);
	virtual FWULONG __stdcall SubscriptionCount();
	virtual BOOL __stdcall AnySubscriptionsLeft();
	virtual HRESULT __stdcall IsSubscriptionCount();

	//////////////////////////////////////////////
	// Style Functions

	virtual HRESULT __stdcall SetStyleString(FWSTRING p);
	virtual HRESULT __stdcall GetStyleString(FWSTRING *p);
	virtual HRESULT __stdcall IsStyle(FWSTRING strStyle);
	
	//////////////////////////////////////////////
	// Life Cycle

	virtual HRESULT __stdcall Suspend(FWLONG nTimeStamp);
	virtual HRESULT __stdcall Resume(FWLONG nTimeStamp);
	virtual HRESULT __stdcall IsSuspended();

	//////////////////////////////////////////////
	// Time and Phase Functions

	virtual HRESULT __stdcall SetStartTime(FWLONG nStartTime)	{ m_nStartTime = nStartTime; return S_OK; }
	virtual HRESULT __stdcall GetStartTime(FWLONG *p)			{ if (p) *p = m_nStartTime; return S_OK; }
	virtual FWLONG  __stdcall StartTime()						{ return m_nStartTime; }
	virtual HRESULT __stdcall GetPeriod(FWLONG *p)				{ if (p) *p = m_nPeriod; return S_OK; }
	virtual FWLONG  __stdcall Period()							{ return m_nPeriod; }
	virtual HRESULT __stdcall GetCompleteTime(FWLONG *p)		{ if (p) *p = StartTime() + Period(); return S_OK; }
	virtual FWLONG  __stdcall CompleteTime()					{ return StartTime() + Period(); }

	virtual HRESULT __stdcall IsStarted(struct ACTION_EVENT *pEvent);
	virtual HRESULT __stdcall IsOverdue(struct ACTION_EVENT *pEvent);
	virtual HRESULT __stdcall IsMorituri(struct ACTION_EVENT *pEvent);

	virtual HRESULT __stdcall Die(struct ACTION_EVENT *pEvent);

	virtual HRESULT __stdcall GetTime(struct ACTION_EVENT *pEvent, FWLONG *p);
	virtual HRESULT __stdcall GetPhase(struct ACTION_EVENT *pEvent, FWFLOAT *p);

	virtual FWLONG  __stdcall Time(struct ACTION_EVENT *pEvent)		{ FWLONG t; GetTime(pEvent, &t); return t; }
	virtual FWFLOAT __stdcall Phase(struct ACTION_EVENT *pEvent)	{ FWFLOAT ph; GetPhase(pEvent, &ph); return ph; }

	virtual HRESULT __stdcall GetDeltaPhase(struct ACTION_EVENT *pEvent, FWFLOAT *p);
	virtual HRESULT __stdcall GetProgPhase(struct ACTION_EVENT *pEvent, FWFLOAT *p);

	//////////////////////////////////////////////
	// Time Envelope Functions

	virtual HRESULT __stdcall SetSinusoidalEnvelope(FWFLOAT fEaseIn, FWFLOAT fEaseOut)			{ m_env.type = ACTION_ENV_SIN; m_env.fEaseIn = fEaseIn; m_env.fEaseOut = fEaseOut; return S_OK; }
	virtual HRESULT __stdcall SetParabolicEnvelope(FWFLOAT fEaseIn, FWFLOAT fEaseOut)			{ m_env.type = ACTION_ENV_PARA; m_env.fEaseIn = fEaseIn; m_env.fEaseOut = fEaseOut; return S_OK; }
	virtual HRESULT __stdcall SetSinusoidalEnvelopeT(FWULONG timeEaseIn, FWULONG timeEaseOut)	{ m_env.type = ACTION_ENV_SIN_TIME; m_env.timeEaseIn = timeEaseIn; m_env.timeEaseOut = timeEaseOut; return S_OK; }
	virtual HRESULT __stdcall SetParabolicEnvelopeT(FWULONG timeEaseIn, FWULONG timeEaseOut)	{ m_env.type = ACTION_ENV_PARA_TIME; m_env.timeEaseIn = timeEaseIn; m_env.timeEaseOut = timeEaseOut; return S_OK; }
	virtual HRESULT __stdcall SetPhysicalEnvelopeT(FWFLOAT s, FWFLOAT v, FWFLOAT a, FWFLOAT j)	{ m_env.type = ACTION_ENV_PHYSICAL; m_env.fS = s; m_env.fV = v; m_env.fA = a; m_env.fJ = j; return S_OK; }
	virtual HRESULT __stdcall SetNoneEnvelope()													{ m_env.type = ACTION_ENV_NONE; return S_OK; }
	virtual HRESULT __stdcall SetEnvelope(enum ACTION_ENVELOPE t, FWFLOAT fEaseIn, FWFLOAT fEaseOut)	{ m_env.type = t; m_env.fEaseIn = fEaseIn; m_env.fEaseOut = fEaseOut; return S_OK; }

	virtual HRESULT __stdcall SetEnvelopeEx(struct ACTION_ENVELOPE_DATA *p)						{ m_env = *p; return S_OK; }

	virtual HRESULT __stdcall GetEnvelope(struct ACTION_EVENT *pEvent, /*[out]*/ struct ACTION_ENVELOPE_DATA *p);
	virtual HRESULT __stdcall ApplyEnvelope(struct ACTION_EVENT *pEvent, FWULONG timeIn, FWULONG timeRef, FWFLOAT *pfOut);

	//////////////////////////////////////////////
	// Events

	virtual HRESULT __stdcall SendEvent(FWLONG nTimeStamp, FWULONG nEvent, FWLONG nSubCode, FWLONG nReserved);
	virtual HRESULT __stdcall SendEventEx(struct ACTION_EVENT *pEvent);
	virtual HRESULT __stdcall RaiseEvent(FWLONG nTimeStamp, FWULONG nEvent, FWLONG nSubCode, FWLONG nReserved);
	virtual HRESULT __stdcall RaiseEventEx(struct ACTION_EVENT *pEvent);
	virtual HRESULT __stdcall SetHandleEventHook(HANDLE_EVENT_HOOK_FUNC pfHook, FWULONG nParam, void *pParam);
};

#endif
