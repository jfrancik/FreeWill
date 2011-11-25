// rotate.h
//
////////////////////////////////////////////////////////////////////////

#if !defined(__ROTATE_H)
#define __ROTATE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../fwlib/factory.h"
#include "../fwlib/fwunknown.h"
#include <kineplus.h>
#include <boundplus.h>
#include "action.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// CActionRotate

class CActionRotate : public CAction
{
public:

	// Overrides
	virtual HRESULT __stdcall HandleEvent(struct ACTION_EVENT *pEvent);
	virtual HRESULT __stdcall Create(IFWCreateContext*);

    DECLARE_FACTORY_CLASS(ActionRotate, Action) 
	FW_RTTI(ActionRotate)

protected:
	IKineChild *m_pObj;				// the object
	ITransform *m_pTSrc, *m_pTDest;	// source & dest transforms
	ITransform *m_pT;				// working transform...

public:
	CActionRotate();
	~CActionRotate();
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// CActionRotateInv

class CActionRotateInv : public CAction
{
public:

	// Overrides
	virtual HRESULT __stdcall HandleEvent(struct ACTION_EVENT *pEvent);
	virtual HRESULT __stdcall Create(IFWCreateContext*);

    DECLARE_FACTORY_CLASS(ActionRotateInv, Action) 
	FW_RTTI(ActionRotateInv)

protected:
	FWULONG m_nLimbs;					// count of limbs involved

	IKineChild *m_pPelvis;				// the pelvis (root object)
	IKineChild **m_ppLimb;				// limbs
	
	ITransform *m_pTSrc;				// source transform (single)
	ITransform **m_ppTA, **m_ppTB;		// dest transforms
	ITransform **m_ppTS;				// auxiliary transform...
	ITransform *m_pT, *m_pTX;			// working transform...

	bool m_bInitialised;				// flag

	FWMATRIX m_m, *m_pm;				// bone matrix backup

public:
	CActionRotateInv();
	~CActionRotateInv();
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// CActionRotateTo

class CActionRotateTo : public CAction
{
public:

	// Overrides
	virtual HRESULT __stdcall HandleEvent(struct ACTION_EVENT *pEvent);
	virtual HRESULT __stdcall Create(IFWCreateContext*);

    DECLARE_FACTORY_CLASS(ActionRotateTo, Action) 
	FW_RTTI(ActionRotateTo)

protected:
	IKineChild *m_pObj;				// the object
	IKineChild *m_pRefObj;			// the reference object for the Dest Transformm
	ITransform *m_pTDest;			// dest transforms
	ITransform *m_pT;				// working transform...

public:
	CActionRotateTo();
	~CActionRotateTo();
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// CActionMultiRotate

class CActionMultiRotate : public CAction
{
public:

	// Overrides
	virtual HRESULT __stdcall HandleEvent(struct ACTION_EVENT *pEvent);
	virtual HRESULT __stdcall Create(IFWCreateContext*);

    DECLARE_FACTORY_CLASS(ActionMultiRotate, Action) 
	FW_RTTI(ActionMultiRotate)

protected:
	FWULONG m_nCount;					// object counter
	IKineChild **m_ppObj;			// the objects
	ITransform **m_ppTSrc;			// src  transforms
	ITransform **m_ppTDest;			// dest transforms
	ITransform **m_ppT;				// working transform...

public:
	CActionMultiRotate();
	~CActionMultiRotate();
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// CActionMultiRotateTo

class CActionMultiRotateTo : public CAction
{
public:

	// Overrides
	virtual HRESULT __stdcall HandleEvent(struct ACTION_EVENT *pEvent);
	virtual HRESULT __stdcall Create(IFWCreateContext*);

    DECLARE_FACTORY_CLASS(ActionMultiRotateTo, Action) 
	FW_RTTI(ActionMultiRotateTo)

protected:
	FWULONG m_nCount;					// object counter
	IKineChild **m_ppObj;			// the objects
	ITransform **m_ppTDest;			// dest transforms
	ITransform *m_pT;				// working transform...

public:
	CActionMultiRotateTo();
	~CActionMultiRotateTo();
};

#endif
