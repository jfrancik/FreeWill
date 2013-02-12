// action.cpp : Defines Action object
//

#include "stdafx.h"
#include "action.h"

#include <algorithm>
#include <functional>
#include <string>
#define _USE_MATH_DEFINES
#include <math.h>
#include "motion3.h"

#pragma warning (disable:4503)	// decorated name length exceeded

using namespace std;

#define TOKENPASTE2(x, y) x ## y
#define TOKENPASTE(x, y) TOKENPASTE2(x, y)
#define for_each_1(Z, TYPE, COLL, VAR)	\
	bool Z;\
	for (TYPE::iterator i = COLL.begin(); i != COLL.end(); i++)		\
		for (TYPE::value_type &VAR = (Z = true , *i); Z; Z = false)
#define for_each(TYPE, COLL, VAR)	for_each_1(TOKENPASTE(x_var_unique_, __COUNTER__), TYPE, COLL, VAR)

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// Class CAction

CAction::ACTION_SUBS_EX::ACTION_SUBS_EX(IAction *pOriginator, IAction *pSubscriber, FWULONG nEvent, FWULONG nFlags, FWLONG nTrigger, FWULONG nId)
{ 
	this->pOriginator = pOriginator;
	this->pSubscriber = pSubscriber;
	this->nEvent = nEvent;
	this->nFlags = nFlags;
	this->nTrigger = nTrigger;
	this->nId = nId;
	this->pColl = NULL;
}

//////////////////////////////////////////////
// Construction & Deconstruction

CAction::CAction() 
:	m_nSuspended(0), m_pStyle(NULL),
	m_nStartTime(0), m_nSuspendTime(0), m_nPeriod(0), m_fTimePrev(0.0f),
	m_pfHook(NULL), m_nParam(0), m_pParam(NULL)
{ 
	m_env.type = ACTION_ENV_REWRITE_FROM_SENDER;
	m_pCurEvent = NULL;
}

CAction::~CAction()
{
	if (m_pStyle) free(m_pStyle);
	UnSubscribeAll();
}

//////////////////////////////////////////////
// Generic Overridables (usually overwritten)

HRESULT CAction::HandleEvent(struct ACTION_EVENT *pEvent)
{
	return S_OK;
}

HRESULT CAction::Create(IFWCreateContext *p)
{
	HRESULT h;
	IFWEnumParams *pEnum = NULL;

	p->EnumParams(&pEnum);

	try
	{
		h = QueryStdParams(pEnum); if (FAILED(h)) throw(h);
	}
	catch (HRESULT h)
	{
		return ErrorStdParams(pEnum, h);
	}

	pEnum->Release();
	return S_OK;
}

//////////////////////////////////////////////
// Helpers for Implementation of Derivated Classes

HRESULT CAction::QueryStdParams(IFWEnumParams *pEnum, struct ACTION_SUBS **ppSubs)
{
	HRESULT h = S_OK;
	IAction *pTickSource = NULL;
	IAction *pPrevAction = NULL;
	FWSTRING pStyle = NULL;
	m_nStartTime = m_nPeriod = 0;

	// 1st: pTickSource, obligatory, may be NULL
	h = pEnum->QueryPUNKNOWN(IID_IAction, (FWPUNKNOWN*)&pTickSource);
	if FAILED(h) return ERROR(h);	// This parameter is obligatory

	// 2nd, option 1: Start Time
	h = pEnum->QueryLONG(&m_nStartTime);

	// 2nd, option 2: Previous Action
	if FAILED(h) h = pEnum->QueryPUNKNOWN(IID_IAction, (FWPUNKNOWN*)&pPrevAction);

	// 3rd, Period, optional, only taken if 2nd successfully read
	if SUCCEEDED(h) pEnum->QueryLONG(&m_nPeriod);

	// 4th, Style, optional
	pEnum->QuerySTRING(&pStyle);

	// Standard operations now...
	// Subscribe, Suspend and SetStyleString never fails in this implementation so no checks are done...
	if (pPrevAction) 
	{
		// modified: 12/02/13 - actions hung on previous action go as ACTION_ANY rather than ACTION_GTE
		if (pTickSource) 
			pTickSource->Subscribe(this, EVENT_TICK, ACTION_ANY, 0, 0, ppSubs);
		pPrevAction->Subscribe(this, EVENT_END, ACTION_ANY | ACTION_ONCE | ACTION_RESUME, 0, 0, NULL);
		pPrevAction->Release();
		h = Suspend(0);
	}
	else
	if (pTickSource) 
		pTickSource->Subscribe(this, EVENT_TICK, ACTION_GTE, m_nStartTime, 0, ppSubs);

	if (pTickSource) 
		pTickSource->Release();

	SetStyleString(pStyle);

	return S_OK;
}

HRESULT CAction::ErrorStdParams(IFWEnumParams *pEnum, HRESULT h)
{
	m_nPeriod = 0;	// prevents running the action... (shall not always run...)
	pEnum->ErrorEx(__WFILE__, __LINE__, h);
	pEnum->Release();
	return h;
}

//////////////////////////////////////////////
// Subscription

HRESULT CAction::Subscribe(IAction *pSubscriber, FWULONG nEvent, FWULONG nFlags, FWLONG nTrigger, FWULONG nId, struct ACTION_SUBS **pHandle)
{
	if (pSubscriber == this)
		nFlags |= ACTION_WEAKPTR;	// auto-references are always weak - to avoid looped refs
		
	ACTION_SUBS_EX subs(this, pSubscriber, nEvent, nFlags, nTrigger, nId);

	if ((nFlags & ACTION_WEAKPTR) == 0)
		pSubscriber->AddRef();	

	LIST_OF_SUBS *pColl;
	LIST_OF_SUBS::iterator iter;

	COLL &coll = m_Subsribers[nEvent];

	switch (nFlags & ACTION_MASK_TRIGGER)
	{
	case ACTION_ANY:
		pColl = &coll.lstAny;
		iter = coll.lstAny.insert(coll.lstAny.end(), subs);
		break;
	case ACTION_EQ:
		pColl = &coll.mapEq[nTrigger];
		iter = pColl->insert(pColl->end(), subs);
		break;
	case ACTION_LTE:
		{
			pColl = &coll.lstLte;
			LIST_OF_SUBS::iterator pos = lower_bound(coll.lstLte.begin(), coll.lstLte.end(), subs, std::greater<ACTION_SUBS_EX>());
			iter = coll.lstLte.insert(pos, subs);
			break;
		}
	case ACTION_GTE:
		{
			pColl = &coll.lstGte;
			LIST_OF_SUBS::iterator pos = lower_bound(coll.lstGte.begin(), coll.lstGte.end(), subs);
			iter = coll.lstGte.insert(pos, subs);
			break;
		}
	}
	(*iter).pColl = pColl;
	(*iter).iter = iter;

	if (pHandle) *pHandle = &(*iter);
		
	return S_OK;
}

HRESULT CAction::UnSubscribe(FWLONG nTimeStamp, ACTION_SUBS *pSubs)
{
	if (pSubs->pOriginator != this)
		return pSubs->pOriginator->UnSubscribe(nTimeStamp, pSubs);
	else
	{
		pSubs->pSubscriber->SendEvent(nTimeStamp, EVENT_UNSUBSCRIBE, 0, (FWLONG)pSubs);
		if ((pSubs->nFlags & ACTION_WEAKPTR) == 0)
			pSubs->pSubscriber->Release();

		((ACTION_SUBS_EX*)pSubs)->pColl->erase(((ACTION_SUBS_EX*)pSubs)->iter);

		return S_OK;
	}
}

HRESULT CAction::UnSubscribeAll()
{
	for_each(MAP_OF_COLL, m_Subsribers, v)
	{
		for_each(LIST_OF_SUBS, v.second.lstAny, v)
			if (v.pSubscriber && (v.nFlags & ACTION_WEAKPTR) == 0) 
				v.pSubscriber->Release();
		for_each(MAP_INT_LIST_OF_SUBS, v.second.mapEq, v)
		{
			for_each(LIST_OF_SUBS, v.second, v)
				if (v.pSubscriber && (v.nFlags & ACTION_WEAKPTR) == 0) 
					v.pSubscriber->Release();
		}
		for_each(LIST_OF_SUBS, v.second.lstLte, v)
			if (v.pSubscriber && (v.nFlags & ACTION_WEAKPTR) == 0) 
				v.pSubscriber->Release();
		for_each(LIST_OF_SUBS, v.second.lstGte, v)
			if (v.pSubscriber && (v.nFlags & ACTION_WEAKPTR) == 0) 
				v.pSubscriber->Release();
	}
	m_Subsribers.clear();
	return S_OK;
}

HRESULT CAction::GetSubscriptionCount(/*[out, retval]*/ FWULONG *p)
{
	if (p) *p = SubscriptionCount();
	return S_OK;
}

FWULONG CAction::SubscriptionCount()
{
	FWULONG nCount = 0;
	for_each(MAP_OF_COLL, m_Subsribers, v)
	{
		nCount += v.second.lstAny.size();
		for_each(MAP_INT_LIST_OF_SUBS, v.second.mapEq, v)
			nCount += v.second.size();
		nCount += v.second.lstLte.size();
		nCount += v.second.lstGte.size();
	}
	return nCount;
}

BOOL CAction::AnySubscriptionsLeft()
{
	for_each(MAP_OF_COLL, m_Subsribers, v)
	{
		if (v.second.lstGte.size() || v.second.lstAny.size() || v.second.lstLte.size()) return true;
		for_each(MAP_INT_LIST_OF_SUBS, v.second.mapEq, v)
			if (v.second.size()) return true;
	}
	return false;
}

HRESULT CAction::IsSubscriptionCount()
{
	return AnySubscriptionsLeft() ? S_OK : S_FALSE;
}

//////////////////////////////////////////////
// Style Functions

HRESULT CAction::SetStyleString(FWSTRING p)
{
	if (m_pStyle) free(m_pStyle);
	m_styles.clear();
	m_pStyle = wcsdup(p);

	FWSTRING pSep = L"; ";
	while (p && *p)
	{
		p += wcsspn(p, pSep);
		ULONG i = wcscspn(p, pSep);
		wstring token(p, i);
		if (i) m_styles.insert(token);
		p += i;
	}
	return S_OK;
}

HRESULT CAction::GetStyleString(FWSTRING *p)
{
	if (p) *p = m_pStyle;
	return S_OK;
}

HRESULT CAction::IsStyle(FWSTRING strStyle)
{
	if (m_styles.find(strStyle) != m_styles.end()) return S_OK;
	else return S_FALSE;
}
	
//////////////////////////////////////////////
// Life Cycle

HRESULT CAction::Suspend(FWLONG nTimeStamp)
{
	m_nSuspended++;
	if (m_nSuspended == 1)
	{
		// just suspended
		SendEvent(nTimeStamp, EVENT_SUSPENDED, 0, 0);
		m_nSuspendTime = nTimeStamp;
	}
	return S_OK;
}

HRESULT CAction::Resume(FWLONG nTimeStamp)
{
	if (m_nSuspended == 0) return S_OK;		// already resumed
	m_nSuspended--;
	if (m_nSuspended == 0)
	{
		// just resumed
		//if (m_nSuspendTime > m_nStartTime)
		//	m_nStartTime += nTimeStamp - m_nSuspendTime;
		m_nStartTime = nTimeStamp;

		SendEvent(nTimeStamp, EVENT_RESUMED, 0, 0);
	}
	return S_OK;
}

HRESULT CAction::IsSuspended()
{
	if (m_nSuspended != 0) return S_OK;
	else return S_FALSE;
}

//////////////////////////////////////////////
// Time and Phase Functions

HRESULT CAction::IsStarted(struct ACTION_EVENT *pEvent)
{
	return (pEvent->nTimeStamp >= StartTime()) ? S_OK : S_FALSE;
}

HRESULT CAction::IsOverdue(struct ACTION_EVENT *pEvent)
{
	return (pEvent->nTimeStamp >= CompleteTime()) ? S_OK : S_FALSE;
}

HRESULT CAction::IsMorituri(struct ACTION_EVENT *pEvent)
{
	switch (pEvent->pSubs->nFlags & ACTION_MASK_MODE)
	{
	case ACTION_AUTO:		return IsOverdue(pEvent);
	case ACTION_ONCE:		return S_OK;
	case ACTION_MANUAL:		return S_FALSE;
	case ACTION_MORITURI:	return S_OK;
	}
	return S_FALSE;
}

HRESULT CAction::Die(struct ACTION_EVENT *pEvent)
{
	if (pEvent->pSubs == NULL)
		return S_FALSE;	// cannot die if not sunscribed
	if ((pEvent->pSubs->nFlags & ACTION_MASK_MODE) == ACTION_ONCE) 
		return S_OK;	// will soon die anyway... change to MORITURI would trigger EVENT_END - not desired with ACTION_ONCE events.
	pEvent->pSubs->nFlags &= ~ACTION_MASK_MODE; 
	pEvent->pSubs->nFlags |= ACTION_MORITURI; return S_OK;
}

HRESULT CAction::GetTime(struct ACTION_EVENT *pEvent, FWLONG *p)
{
	if (!p) return S_OK;
	*p = pEvent->nTimeStamp - StartTime();
	return S_OK;
}

HRESULT CAction::GetPhase(struct ACTION_EVENT *pEvent, FWFLOAT *p)
{
	if (!p) return S_OK;
	if (m_nPeriod == 0)
	{
		*p = 1;
		return S_OK;
	}
	return ApplyEnvelope(pEvent, Time(pEvent), Period(), p);
}

HRESULT CAction::GetDeltaPhase(struct ACTION_EVENT *pEvent, FWFLOAT *p)
{
	FWFLOAT fT;
	GetPhase(pEvent, &fT);
	if (p) *p = fT - m_fTimePrev;
	m_fTimePrev = fT;
	return S_OK;
}

HRESULT CAction::GetProgPhase(struct ACTION_EVENT *pEvent, FWFLOAT *p)
{
	FWFLOAT fT;
	GetPhase(pEvent, &fT);
	if (p)
		if (m_fTimePrev == 1.0)
			*p = 1.0;
		else
			*p = (FWFLOAT)(fT - m_fTimePrev) / (FWFLOAT)(1.0 - m_fTimePrev);
	m_fTimePrev = fT;
	return S_OK;
}

//////////////////////////////////////////////
// Time Envelope                 Functions

HRESULT CAction::GetEnvelope(struct ACTION_EVENT *pEvent, /*[out]*/ struct ACTION_ENVELOPE_DATA *p)
{
//		if (m_env.type == ACTION_ENV_REWRITE_FROM_SENDER && pEvent && pEvent->pSender)
//			pEvent->pSender->GetEnvelope(NULL, &m_env);
//		if (m_env.type == ACTION_ENV_REWRITE_FROM_SENDER && pEvent && pEvent->pOriginator)
//			pEvent->pOriginator->GetEnvelope(NULL, &m_env);
	*p = m_env;
	return S_OK;
}

HRESULT CAction::ApplyEnvelope(struct ACTION_EVENT *pEvent, FWULONG timeIn, FWULONG timeRef, FWFLOAT *pfOut)
{
	if (!pfOut) return S_OK;

	enum ACTION_ENVELOPE envtype = m_env.type;

	if (envtype == ACTION_ENV_REWRITE_FROM_SENDER)
		if (pEvent->pSubs && pEvent->pSubs->pOriginator)
			return pEvent->pSubs->pOriginator->ApplyEnvelope(pEvent->pPrev, timeIn, timeRef, pfOut);
		else if (pEvent->pPrev && pEvent->pPrev->pSubs && pEvent->pPrev->pSubs->pSubscriber)
			return pEvent->pPrev->pSubs->pSubscriber->ApplyEnvelope(pEvent->pPrev, timeIn, timeRef, pfOut);
		else
			envtype = ACTION_ENV_NONE;

	if (envtype == ACTION_ENV_PHYSICAL)
	{
//		LiftKinematicsCalculator calc(0, 0, m_env.fS, m_env.fV, m_env.fA, m_env.fJ);
//		*pfOut = (FWFLOAT)calc.getPosition(calc.getStopTime() * timeIn / Period()) / m_env.fS;
//		return S_OK;
	}

	double t = timeIn;
	double T = timeRef;
	double f = 0;
	double T1, T2;

	switch (envtype)
	{
	case ACTION_ENV_PARA:
	case ACTION_ENV_SIN:
		T1 = m_env.fEaseIn * T;
		T2 = m_env.fEaseOut * T;
		break;
	case ACTION_ENV_PARA_TIME:
	case ACTION_ENV_SIN_TIME:
		T1 = m_env.timeEaseIn;
		T2 = T - m_env.timeEaseIn;
		break;
	default:
		T1 = m_env.timeEaseIn;
		T2 = T - m_env.timeEaseIn;
		break;
	}
	if (T1 < 0) T1 = 0;
	if (T2 > T) T2 = T;
	if (T1 > T2) T1 = T2 = (T1 + T2) / 2;
			
	double q, s, v0;
	switch (envtype)
	{
	case ACTION_ENV_PARA:
	case ACTION_ENV_PARA_TIME:
		v0 = 2.0 / (T2 - T1 + T);
		if (t < 0)			f = 0;
		else if (t < T1)	f = v0 * t * t / 2.0 / T1;
		else if (t <= T2)	f = v0 * T1 / 2.0 + v0 * (t - T1);
		else if (t < T)		f = v0 * T1 / 2.0 + v0 * (T2 - T1) + (v0 - v0 * (t - T2) / (T - T2) / 2.0) * (t - T2);
		else				f = 1;
		break;
	case ACTION_ENV_SIN:
	case ACTION_ENV_SIN_TIME:
		q = T1 * 2.0 / M_PI + T2 - T1 + (T - T2) * 2.0 / M_PI;
		if (t < 0)			s = 0;
		else if (t < T1)	s = T1 * 2.0 / M_PI * (sin((t / T1) * M_PI / 2.0 - M_PI / 2.0) + 1.0);
		else if (t < T2)	s = 2.0 * T1 / M_PI + t - T1;
		else if (t < T)		s = 2.0 * T1 / M_PI + T2 - T1 + ((T - T2) * (2.0 / M_PI)) * sin(((t - T2) / (T - T2)) * M_PI / 2.0);
		else				s = q;
		f = s / q;
		break;
	default:
		f = t / T;
		break;
	}

	if (f < 0) f = 0; if (f > 1) f = 1;
	*pfOut = (FWFLOAT)f;
	return S_OK;
}

//////////////////////////////////////////////
// Events

HRESULT CAction::SendEvent(FWLONG nTimeStamp, FWULONG nEvent, FWLONG nSubCode, FWLONG nReserved)
{
	ACTION_EVENT ev = { nTimeStamp, nEvent, nSubCode, nReserved, NULL, NULL };
	return SendEventEx(&ev);
}

HRESULT CAction::SendEventEx(struct ACTION_EVENT *pEvent)
{
	HRESULT h = S_OK;

	// store the current event info
	ACTION_EVENT *pPrevEvent = m_pCurEvent;
	m_pCurEvent = pEvent;

	if (m_pfHook)
		h = m_pfHook(pEvent, this, m_nParam, m_pParam);
	if (h == S_OK)
		h = HandleEvent(pEvent);	// the only direct call to HandleEvent!
	m_pCurEvent = pPrevEvent;
	return h;
}

HRESULT CAction::RaiseEvent(FWLONG nTimeStamp, FWULONG nEvent, FWLONG nSubCode, FWLONG nReserved)
{
	ACTION_EVENT ev = { nTimeStamp, nEvent, nSubCode, nReserved, NULL, NULL };
	return RaiseEventEx(&ev);
}

		// helper function
		FWLONG _GetTimeSetting(ACTION_SUBS *pSubs, ACTION_EVENT *pEvent)
		{
			if (pSubs->nFlags & ACTION_NOTIME) return pEvent->nTimeStamp;
			if ((pSubs->nFlags & ACTION_MASK_LTE_GTE) && (pSubs->nFlags & ACTION_ONCE))
				return pSubs->nTrigger;
			else
				return pEvent->nSubCode;
		}

HRESULT CAction::RaiseEventEx(struct ACTION_EVENT *pEvent)
{
	// return immediately if suspended
	if (IsSuspended() == S_OK)
		return S_OK;

	// Raise EVENT_BEGIN - when event handled for the first time
	if (pEvent->pSubs && (pEvent->pSubs->nFlags & ACTION_MASK_MODE) != ACTION_ONCE && (pEvent->pSubs->nFlags & ACTION_RESERVED_1) == 0)
	{
		pEvent->pSubs->nFlags |= ACTION_RESERVED_1;
		RaiseEvent(pEvent->nTimeStamp, EVENT_BEGIN, StartTime(), (FWLONG)pEvent);
	}

	// process the event
	HRESULT h = SendEventEx(pEvent);

	// raise subscribed events if SendEvent returned S_OK and there are any subscribers
	MAP_OF_COLL::iterator pos = m_Subsribers.find(pEvent->nEvent);	// locate the COLLection of collections of subscribed actions
	if (h == S_OK && pos != m_Subsribers.end())
	{
		// create lists of subscribers

		COLL &coll = (*pos).second;

		// all the subscriptions will be scanned and classified as ACTION_RESUME, ACTION_SUSPEND, ACTION_UNSUBSCRIBE and regular (HandleEvent)
		list<ACTION_SUBS_EX*> lstSuspend;
		list<ACTION_SUBS_EX*> lstResume;
		list<ACTION_SUBS_EX*> lstCalls;

		int nAux;

		// scan ACTION_ANY subscriptions
		if (coll.lstAny.size())
			for (LIST_OF_SUBS::iterator i = coll.lstAny.begin(); i != coll.lstAny.end(); i++ )
				switch ((*i).nFlags & ACTION_MASK_USE)
				{
					case ACTION_CALL: lstCalls.push_back(&(*i)); break;
					case ACTION_RESUME: lstResume.push_back(&(*i)); break;
					case ACTION_SUSPEND: lstSuspend.push_back(&(*i)); break;
				}

		// scan ACTION_EQ subscriptions
		if (coll.mapEq.size())
		{
			MAP_INT_LIST_OF_SUBS::iterator pos = coll.mapEq.find(pEvent->nSubCode);
			if (pos != coll.mapEq.end())
				for (LIST_OF_SUBS::iterator i = (*pos).second.begin(); i != (*pos).second.end(); i++ )
					switch ((*i).nFlags & ACTION_MASK_USE)
					{
						case ACTION_CALL: lstCalls.push_back(&(*i)); break;
						case ACTION_RESUME: lstResume.push_back(&(*i)); break;
						case ACTION_SUSPEND: lstSuspend.push_back(&(*i)); break;
					}
		}

		// scan ACTION_LTE subscriptions
		if (coll.lstLte.size())
			for (LIST_OF_SUBS::iterator i = coll.lstLte.begin(); i != coll.lstLte.end(); i++ )
				if (pEvent->nSubCode <= (*i).nTrigger)
					switch ((*i).nFlags & ACTION_MASK_USE)
					{
						case ACTION_CALL: lstCalls.push_back(&(*i)); break;
						case ACTION_RESUME: lstResume.push_back(&(*i)); break;
						case ACTION_SUSPEND: lstSuspend.push_back(&(*i)); break;
					}

		// scan ACTION_GTE subscriptions
		if (coll.lstGte.size())
			for (LIST_OF_SUBS::iterator i = coll.lstGte.begin(); i != coll.lstGte.end(); i++ )
			{
				nAux = (*i).nTrigger;
				if (pEvent->nSubCode >= (*i).nTrigger)
					switch ((*i).nFlags & ACTION_MASK_USE)
					{
						case ACTION_CALL: lstCalls.push_back(&(*i)); break;
						case ACTION_RESUME: lstResume.push_back(&(*i)); break;
						case ACTION_SUSPEND: lstSuspend.push_back(&(*i)); break;
					}
				else
					break;
			}

		// store the current event info
		ACTION_EVENT *pPrevEvent = m_pCurEvent;
		m_pCurEvent = pEvent;

		// Execute all ACTION_RESUME subscribed operations
		for (list<ACTION_SUBS_EX*>::iterator i = lstResume.begin(); i != lstResume.end(); i++)
		{
			ACTION_SUBS_EX *pSubs = *i;
			pSubs->pSubscriber->Resume(_GetTimeSetting(pSubs, pEvent));
			UnSubscribe(pEvent->nTimeStamp, pSubs);
		}

		// Recursive Calls: Pass onto subscribers
		for (list<ACTION_SUBS_EX*>::iterator i = lstCalls.begin(); i != lstCalls.end(); i++)
		{
			ACTION_SUBS_EX *pSubs = *i;

			// prepare EVENT's local copy - the subscriber will only access this...
			ACTION_EVENT myEvent = *pEvent;
			myEvent.pSubs = pSubs;
			myEvent.pPrev = pEvent;
			if (!(pSubs->nFlags & ACTION_NOTIME))
				myEvent.nSubCode = _GetTimeSetting(pSubs, pEvent);

			pSubs->pSubscriber->RaiseEventEx(&myEvent);
		}
		
		// Execute all ACTION_SUSPEND subscribed operations
		for (list<ACTION_SUBS_EX*>::iterator i = lstSuspend.begin(); i != lstSuspend.end(); i++)
		{
			ACTION_SUBS_EX *pSubs = *i;
			pSubs->pSubscriber->Suspend(_GetTimeSetting(pSubs, pEvent));
			UnSubscribe(pEvent->nTimeStamp, pSubs);
		}
		
		m_pCurEvent = pPrevEvent;
	}

	// Send EVENT_END and unsubscribe if action IsMorituri
	if (pEvent->pSubs && IsMorituri(pEvent) == S_OK)
	{
		if ((pEvent->pSubs->nFlags & ACTION_MASK_MODE) != ACTION_ONCE)		// no EVENT_END for ACTION_ONCE actions
			RaiseEvent(pEvent->nTimeStamp, EVENT_END, CompleteTime(), (FWLONG)pEvent);
		pEvent->pSubs->pOriginator->UnSubscribe(pEvent->nTimeStamp, pEvent->pSubs);
	}

	return S_OK;
}

HRESULT CAction::SetHandleEventHook(HANDLE_EVENT_HOOK_FUNC pfHook, FWULONG nParam, void *pParam)
{
	m_pfHook = pfHook;
	m_nParam = nParam;
	m_pParam = pParam;
	return S_OK;
}

