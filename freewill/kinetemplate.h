// kinetemplate.h
//
// Everything You Would Ever Need to Implement Kine+ Based Objects:
// template class CTKineObj3D - implements IKineObj3D
// template class CTKineChild - implements IKineChild
// template class CTKineNode  - implements IKineChild + IKineNode
// template class CTKineTargetedObject - implements IKineTargetedObject
// class CKineEnumChildren - a fully inline class implementation
/////////////////////////////////////////////////////////////////////////////

#if !defined(_KINETEMPL__)
#define _KINETEMPL_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "kineplus.h"
#include <map>
#include <vector>
#include "math.h"

#define ERRORLABEL ErrorSrc(__WFILE__, __LINE__), ErrorLabel

///////////////////////////////////////////////////////////
//
// IKineObj3D implementation template

template <class T>
class CTKineObj3D : public T
{
protected:
	// Transforms...
	ITransform *m_pIBaseTransform;
	ITransform *m_pILocalTransform;
	ITransform *m_pIGlobalTransform;
	ITransform *m_pIParentTransform;
	ITransform *m_pIRefTransform;
	bool m_bIsValid;

	struct STACKINFO { FWMATRIX data; STACKINFO *pLink; } *m_pStackInfo;

public:

	CTKineObj3D() 
		: m_pIBaseTransform(NULL), m_pILocalTransform(NULL), 
		m_pIGlobalTransform(NULL), m_pIParentTransform(NULL), m_pIRefTransform(NULL), 
		m_bIsValid(false), m_pStackInfo(NULL)  { }

	~CTKineObj3D()
	{
		if (m_pIBaseTransform) m_pIBaseTransform->Release();
		if (m_pILocalTransform) m_pILocalTransform->Release();
		if (m_pIGlobalTransform) m_pIGlobalTransform->Release();
		if (m_pIParentTransform) m_pIParentTransform->Release();
		if (m_pIRefTransform) m_pIRefTransform->Release();
		m_bIsValid = false;
		ClearStack();
	}

	virtual HRESULT __stdcall GetBaseTransform(ITransform *pVal)
	{
		if (!pVal) return ERROR(FW_E_POINTER);
		if (m_pIBaseTransform)
			return pVal->FromTransform(m_pIBaseTransform);
		else
			return pVal->FromIdentity();
	}

	virtual HRESULT __stdcall GetBaseTransformRef(ITransform **pVal)
	{
		if (!pVal) return ERROR(FW_E_POINTER);
		*pVal = m_pIBaseTransform;
		if (*pVal) (*pVal)->AddRef();
		return S_OK;
	}

	virtual HRESULT __stdcall GetBaseVector(FWVECTOR *pVec)
	{
		if (!pVec) return ERROR(FW_E_POINTER);
		if (m_pIBaseTransform)
			m_pIBaseTransform->AsVector(pVec);
		else
			memset(pVec, 0, sizeof(FWVECTOR));
		return S_OK;
	}

	virtual HRESULT __stdcall PutBaseTransform(ITransform *p)
	{
		if (!p) return ERROR(FW_E_POINTER);
		Invalidate();
		if (!m_pIBaseTransform)
            m_pIBaseTransform = (ITransform*)p->Clone(IID_ITransform);
		return p->AsTransform(m_pIBaseTransform);
	}

	virtual HRESULT __stdcall GetLocalTransform(ITransform *pVal)
	{
		if (!pVal) return ERROR(FW_E_POINTER);
		if (m_pILocalTransform)
			return pVal->FromTransform(m_pILocalTransform);
		else
			return pVal->FromIdentity();
	}

	virtual HRESULT __stdcall GetLocalTransformRef(ITransform **pVal)
	{
		if (!pVal) return ERROR(FW_E_POINTER);
		if (!m_pILocalTransform && m_pIBaseTransform)
			m_pILocalTransform = (ITransform*)m_pIBaseTransform->Clone(IID_ITransform);
		*pVal = m_pILocalTransform;
		if (*pVal) (*pVal)->AddRef();
		return S_OK;
	}

	virtual HRESULT __stdcall PutLocalTransform(ITransform *p)
	{
		if (!p) return ERROR(FW_E_POINTER);
		Invalidate();
        if (!m_pILocalTransform) 
			m_pILocalTransform = (ITransform*)p->Clone(IID_ITransform);
		return p->AsTransform(m_pILocalTransform);
	}

	virtual HRESULT __stdcall GetRefTransform(ITransform *pVal)
	{
		if (!pVal) return ERROR(FW_E_POINTER);
		if (m_pIRefTransform)
			return pVal->FromTransform(m_pIRefTransform);
		else
			return pVal->FromIdentity();
	}

	virtual HRESULT __stdcall GetRefTransformRef(ITransform **pVal)
	{
		if (!pVal) return ERROR(FW_E_POINTER);
		*pVal = m_pIRefTransform;
		if (*pVal) (*pVal)->AddRef();
		return S_OK;
	}

	virtual HRESULT __stdcall PutRefTransform(ITransform *p)
	{
		if (!p) return ERROR(FW_E_POINTER);
        if (!m_pIRefTransform) 
			m_pIRefTransform = (ITransform*)p->Clone(IID_ITransform);
		return p->AsTransform(m_pIRefTransform);
	}

	virtual HRESULT __stdcall GetGlobalTransform(ITransform *pVal)
	{
		if (!pVal) return ERROR(FW_E_POINTER);
		HRESULT h = Validate();
		if (FAILED(h)) return h;
		pVal->FromTransform(m_pIGlobalTransform);
		return S_OK;
	}

	virtual HRESULT __stdcall GetGlobalMatrix(FWMATRIX M)
	{
		if (M == NULL) return ERROR(FW_E_POINTER);
		HRESULT h = Validate();
		if (FAILED(h)) return h;
		return m_pIGlobalTransform->AsMatrix(M);
	}

	virtual HRESULT __stdcall GetParentTransform(ITransform *pVal)
	{
		if (!pVal) return ERROR(FW_E_POINTER);
		HRESULT h = Validate();
		if (FAILED(h)) return h;
		pVal->FromTransform(m_pIParentTransform);
		return S_OK;
	}

	virtual HRESULT __stdcall GetParentMatrix(FWMATRIX M)
	{
		if (M == NULL) return ERROR(FW_E_POINTER);
		HRESULT h = Validate();
		if (FAILED(h)) return h;
		return m_pIParentTransform->AsMatrix(M);
	}

	virtual HRESULT __stdcall CreateCompatibleTransform(ITransform **pVal)
	{
		if (!pVal) return ERROR(FW_E_POINTER);
		*pVal = NULL;
		if (!m_pILocalTransform && !m_pIBaseTransform)
			Validate();	// ATTENTION! Possibility of recursive call...
		if (!m_pILocalTransform && !m_pIBaseTransform)
			return ERROR(KINE_E_NOTRANSFORM);
		if (!m_pILocalTransform)
			m_pILocalTransform = (ITransform*)m_pIBaseTransform->Clone(IID_ITransform);
		*pVal = (ITransform*)m_pILocalTransform->Clone(IID_ITransform);
		return S_OK;
	}

	virtual HRESULT __stdcall Reset()
	{
		Invalidate();
		if (m_pILocalTransform)	return m_pILocalTransform->FromIdentity();
		else return S_OK;
	}

	virtual HRESULT __stdcall FreezeRef()
	{
		HRESULT h = Validate();
		if (FAILED(h)) return h;
		return PutRefTransform(m_pIGlobalTransform);
	}

	virtual HRESULT __stdcall Adopt()
	{
		if (!m_pILocalTransform) return S_OK;
		if (m_pIBaseTransform)
			m_pILocalTransform->Multiply(m_pIBaseTransform);
		PutBaseTransform(m_pILocalTransform);
		m_pILocalTransform->FromIdentity();
		Invalidate();
		return S_OK;
	}

	virtual HRESULT __stdcall Orthonormalize()
	{
		ITransform *pT = NULL;
		HRESULT h = GetLocalTransformRef(&pT);
		if (FAILED(h)) return h;
		h = pT->Orthonormalize();
		pT->Release();
		return h;
	}

	virtual HRESULT __stdcall TransformLocal(ITransform *pNewVal)
	{
		if (!pNewVal) return ERROR(FW_E_POINTER);
		Invalidate();
		if (m_pILocalTransform) return m_pILocalTransform->Multiply(pNewVal);
		else return PutLocalTransform(pNewVal);
	}

	virtual HRESULT __stdcall Transform(ITransform *pNewVal, FWULONG mode)
	{
		if (!pNewVal) return ERROR(FW_E_POINTER);
		Invalidate();

		switch (mode & KINE_MASK_TRANSFORM)
		{
		case KINE_LOCAL:
			if (m_pILocalTransform == NULL)
				m_pILocalTransform = (ITransform*)pNewVal->Clone(IID_ITransform);
			return m_pILocalTransform->MultiplyEx(pNewVal, mode & KINE_MASK_MULTIPLY_MODE);
		case KINE_BASE:
			if (m_pIBaseTransform == NULL)
				m_pIBaseTransform = (ITransform*)pNewVal->Clone(IID_ITransform);
			return m_pIBaseTransform->MultiplyEx(pNewVal, mode & KINE_MASK_MULTIPLY_MODE);
		default:
			return ERROR(KINE_E_NOTRANSFORM);
		}
	}

	virtual HRESULT __stdcall TransformInv(ITransform *pT, IKineObj3D *pTerm)
	{
		ITransform *pTT = (ITransform*)pT->Clone(IID_ITransform);;

		pTT->MulRotationY(3.14159265358979323846);
		pTT->Multiply(pT);
		pTT->MulRotationY(3.14159265358979323846);

		// S = Gf' * Gp
		ITransform *pTS = (ITransform*)pT->Clone(IID_ITransform);
		pTerm->GetTransform(pTS, KINE_GLOBAL);
		this->Multiply(pTS, KINE_GLOBAL | KINE_INVERTED);

		// T = M * S * T * S'
		pTT->MultiplyEx(pTS, KINE_BOTH_SIDES);
		pTT->Reset(FALSE, TRUE);

		pTT->Orthonormalize();
		this->Transform(pTT, KINE_RIGHT_SIDE);

		// T = S' T' S
		pTT->Inverse();
		pTT->MultiplyEx(pTS, KINE_BOTH_SIDES_INVERTED);

		pTT->Orthonormalize();
		pTerm->Transform(pTT, KINE_LOCAL | KINE_RIGHT_SIDE);

		pTT->Release();
		pTS->Release();

		return S_OK;


		//ITransform *pTEnd = (ITransform*)pT->Clone(IID_ITransform);		// take identity
		//pTerm->Multiply(pTEnd, KINE_GLOBAL);							// mul global T of pTerm
		//Multiply(pTEnd, KINE_GLOBAL | KINE_INVERTED);					// mul inverted global T of me 
		//pTEnd->MultiplyEx(pT, KINE_INVERTED);							// mul pT
		//Multiply(pTEnd, KINE_GLOBAL);									// mul global T of me
		//pTerm->Multiply(pTEnd, KINE_GLOBAL | KINE_INVERTED);			// mul inverted global T of pTerm
		//pTEnd->Orthonormalize();

		//HRESULT h = S_OK;
		////h = Transform(pT);
		//if (SUCCEEDED(h)) h = pTerm->Transform(pTEnd, KINE_RIGHT_SIDE);
		//pTEnd->Release();
		//return h;

	}

	virtual HRESULT __stdcall GetTransform(ITransform *pVal, FWULONG mode)
	{
		if (!pVal) return ERROR(FW_E_POINTER);

		HRESULT h;
		switch (mode & KINE_MASK_TRANSFORM)
		{
		case KINE_LOCAL:
			pVal->FromIdentity();
			if (m_pILocalTransform)
				return pVal->MultiplyEx(m_pILocalTransform, mode & KINE_MASK_INVERTED);
			return S_OK;
		case KINE_BASE:
			pVal->FromIdentity();
			if (m_pIBaseTransform)
				return pVal->MultiplyEx(m_pIBaseTransform, mode & KINE_MASK_INVERTED);
			return S_OK;
		case KINE_GLOBAL:
			h = Validate(); if (FAILED(h)) return h;
			pVal->FromIdentity();
			return pVal->MultiplyEx(m_pIGlobalTransform, mode & KINE_MASK_INVERTED);
		case KINE_PARENT:
			h = Validate(); if (FAILED(h)) return h;
			pVal->FromIdentity();
			return pVal->MultiplyEx(m_pIParentTransform, mode & KINE_MASK_INVERTED);
		case KINE_BASE_LOCAL:
			pVal->FromIdentity();
			if (m_pILocalTransform)
				pVal->Multiply(m_pILocalTransform);
			if (m_pIBaseTransform)
				pVal->Multiply(m_pIBaseTransform);
			if (mode & KINE_MASK_INVERTED)
				return pVal->Inverse();
			return S_OK;
		case KINE_PARENT_BASE:
			h = Validate(); if (FAILED(h)) return h;
			pVal->FromIdentity();
			if (m_pILocalTransform)
				pVal->MultiplyEx(m_pILocalTransform, KINE_INVERTED);
			if (m_pIGlobalTransform)
				pVal->Multiply(m_pIGlobalTransform);
			if (mode & KINE_MASK_INVERTED)
				return pVal->Inverse();
			return S_OK;
		default:
			return ERROR(KINE_E_NOTRANSFORM);
		}
	}

	virtual HRESULT __stdcall Multiply(ITransform *pVal, FWULONG mode)
	{
		if (!pVal) return ERROR(FW_E_POINTER);

		HRESULT h;
		switch (mode & KINE_MASK_TRANSFORM)
		{
		case KINE_LOCAL:
			if (m_pILocalTransform)
				return pVal->MultiplyEx(m_pILocalTransform, mode & KINE_MASK_MULTIPLY_MODE);
			return S_OK;
		case KINE_BASE:
			if (m_pIBaseTransform)
				return pVal->MultiplyEx(m_pIBaseTransform, mode & KINE_MASK_MULTIPLY_MODE);
			return S_OK;
		case KINE_GLOBAL:
			h = Validate(); if (FAILED(h)) return h;
			return pVal->MultiplyEx(m_pIGlobalTransform, mode & KINE_MASK_MULTIPLY_MODE);
		case KINE_PARENT:
			h = Validate(); if (FAILED(h)) return h;
			return pVal->MultiplyEx(m_pIParentTransform, mode & KINE_MASK_MULTIPLY_MODE);
		case KINE_BASE_LOCAL:
			switch (mode & KINE_MASK_MULTIPLY_MODE)
			{
			case KINE_REGULAR:
			case KINE_LEFT_SIDE:
			case KINE_RIGHT_SIDE_INVERTED:
			case KINE_BOTH_SIDES:
				if (m_pILocalTransform)
					pVal->Multiply(m_pILocalTransform);
				if (m_pIBaseTransform)
					pVal->Multiply(m_pIBaseTransform);
				return S_OK;
			case KINE_INVERTED:
			case KINE_LEFT_SIDE | KINE_INVERTED:
			case KINE_RIGHT_SIDE:
			case KINE_BOTH_SIDES_INVERTED:
				if (m_pIBaseTransform)
					pVal->MultiplyEx(m_pIBaseTransform, mode & KINE_MASK_MULTIPLY_MODE);
				if (m_pILocalTransform)
					pVal->MultiplyEx(m_pILocalTransform, mode & KINE_MASK_MULTIPLY_MODE);
				return S_OK;
			default:
				return ERROR(KINE_E_NOTRANSFORM);
			}
			return S_OK;
		case KINE_PARENT_BASE:
			h = Validate(); if (FAILED(h)) return h;
			switch (mode & KINE_MASK_MULTIPLY_MODE)
			{
			case KINE_REGULAR:
			case KINE_LEFT_SIDE:
			case KINE_RIGHT_SIDE_INVERTED:
			case KINE_BOTH_SIDES:
				if (m_pILocalTransform)
					pVal->MultiplyEx(m_pILocalTransform, KINE_INVERTED);
				if (m_pIGlobalTransform)
					pVal->Multiply(m_pIGlobalTransform);
				return S_OK;
			case KINE_INVERTED:
			case KINE_LEFT_SIDE | KINE_INVERTED:
			case KINE_RIGHT_SIDE:
			case KINE_BOTH_SIDES_INVERTED:
				if (m_pIGlobalTransform)
					pVal->MultiplyEx(m_pIGlobalTransform, KINE_INVERTED);
				if (m_pILocalTransform)
					pVal->MultiplyEx(m_pILocalTransform, KINE_REGULAR);
				return S_OK;
			default:
				return ERROR(KINE_E_NOTRANSFORM);
			}
			return S_OK;
		default:
			return ERROR(KINE_E_NOTRANSFORM);
		}
	}

	virtual HRESULT __stdcall LtoG(FWVECTOR *pVector)
	{
		if (!pVector) return ERROR(FW_E_POINTER);
		HRESULT h = Validate();
		if (FAILED(h)) return h;
		return m_pIGlobalTransform->ApplyTo(pVector);
	}

	virtual HRESULT __stdcall GtoL(FWVECTOR *pVector)
	{
		if (!pVector) return ERROR(FW_E_POINTER);
		HRESULT h = Validate();
		if (FAILED(h)) return h;

		ITransform *pInv;
		pInv = (ITransform*)m_pIGlobalTransform->Clone(IID_ITransform);
		m_pIGlobalTransform->AsTransform(pInv);
		h = pInv->Inverse();
		if (SUCCEEDED(h)) h = pInv->ApplyTo(pVector);
		pInv->Release();
		return h;
	}

	virtual HRESULT __stdcall Validate()
	{
		if (m_bIsValid)
			return S_OK;

		HRESULT h = S_OK;

		// initialise or create global transform
		if (m_pIGlobalTransform)
			m_pIGlobalTransform->FromIdentity();
		else
			if (m_pILocalTransform) m_pIGlobalTransform = (ITransform*)m_pILocalTransform->Clone(IID_ITransform);
			else if (m_pIBaseTransform) m_pIGlobalTransform = (ITransform*)m_pIBaseTransform->Clone(IID_ITransform);
			else if (m_pIParentTransform) m_pIGlobalTransform = (ITransform*)m_pIParentTransform->Clone(IID_ITransform);
			else
			{	// no global, base, local transform: create a default one!
				h = FWDevice()->CreateObject(L"Transform", IID_ITransform, (IFWUnknown**)&m_pIGlobalTransform);
				if (FAILED(h)) return h;
			}
		if (!m_pIGlobalTransform) return ERROR(KINE_E_NOTRANSFORM);
		// create all missing transforms
		if (!m_pILocalTransform)  m_pILocalTransform  = (ITransform*)m_pIGlobalTransform->Clone(IID_ITransform);
		if (!m_pIBaseTransform)   m_pIBaseTransform   = (ITransform*)m_pIGlobalTransform->Clone(IID_ITransform);
		if (!m_pIParentTransform) m_pIParentTransform = (ITransform*)m_pIGlobalTransform->Clone(IID_ITransform);
		if (!m_pILocalTransform || !m_pIBaseTransform || !m_pIParentTransform) return ERROR(KINE_E_NOTRANSFORM);

		// combine with local
		h = m_pIGlobalTransform->Multiply(m_pILocalTransform);
		if (FAILED(h)) return FW_E_CLASSSPECIFIC;

		// combine with base
		h = m_pIGlobalTransform->Multiply(m_pIBaseTransform);
		if (FAILED(h)) return FW_E_CLASSSPECIFIC;

		// combine with parent
		h = m_pIGlobalTransform->Multiply(m_pIParentTransform);
		if (FAILED(h)) return FW_E_CLASSSPECIFIC;

		m_bIsValid = true;

		return S_OK;
	}

	virtual HRESULT __stdcall Invalidate()
	{
		m_bIsValid = false;
		return S_OK;
	}

	virtual HRESULT __stdcall IsValid()
	{
		return m_bIsValid ? S_OK : S_FALSE;
	}

	virtual HRESULT __stdcall Reproduce(/*[out, retval]*/ IKineObj3D **p)
	{
		if (!p) return ERROR(FW_E_POINTER);

		IKineObj3D *pRep = (IKineObj3D*)Clone(IID_IKineObj3D);
		if (!pRep) return ERROR(FW_E_OUTOFMEMORY);

		if (m_pIRefTransform) pRep->PutRefTransform(m_pIRefTransform);
		if (m_pILocalTransform) pRep->PutLocalTransform(m_pILocalTransform);
		if (m_pIBaseTransform) pRep->PutBaseTransform(m_pIBaseTransform);

		*p = pRep;
		return S_OK;
	}

	virtual HRESULT __stdcall ReproduceEx(REFIID iid, /*[out, retval, iid_is(iid)]*/ IFWUnknown **p)
	{
		HRESULT h;
		IKineObj3D *pObj = NULL;
		h = Reproduce(&pObj);
		if (FAILED(h)) return h;
		h = pObj->QueryInterface(iid, (void**)p);
		pObj->Release();
		if (FAILED(h)) return ERROR(h);
		return S_OK;
	}

	virtual HRESULT __stdcall CanReproduce()
	{
		return S_OK;
	}

	virtual HRESULT __stdcall PushState()
	{
		const static FWMATRIX IDENTITY = { {1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1} };
		STACKINFO *p = new STACKINFO;
		if (m_pILocalTransform) m_pILocalTransform->AsMatrix(p->data);
		else memcpy(p->data, IDENTITY, sizeof(p->data));
		p->pLink = m_pStackInfo;
		m_pStackInfo = p;
		return S_OK;
	}

	virtual HRESULT __stdcall PopState()
	{
		STACKINFO *p = m_pStackInfo;
		if (p)
		{
			m_pStackInfo = p->pLink;
			if (m_pILocalTransform) m_pILocalTransform->FromMatrix(p->data);
			delete p;
			return S_OK;
		}
		else
			return ERROR(FW_E_STACKOVERRUN);
	}

	virtual HRESULT __stdcall ClearStack()
	{
		while (m_pStackInfo)
		{
			STACKINFO *p = m_pStackInfo;
			m_pStackInfo = m_pStackInfo->pLink;
			delete p;
		}
		return S_OK;
	}

	virtual HRESULT __stdcall StoreState(FWULONG nBufSize, BYTE *pBuf, FWULONG *pnCount)
	{
		if (pnCount) *pnCount = sizeof(FWMATRIX);
		if (!pBuf)
			return S_OK;	// test run...
		if (nBufSize < *pnCount)
			return ERROR(FW_E_MEMOVERRUN);
		const static FWMATRIX IDENTITY = { {1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1} };
		if (m_pILocalTransform) m_pILocalTransform->AsMatrix(*(FWMATRIX*)pBuf);
		else memcpy(pBuf, IDENTITY, sizeof(FWMATRIX));
		return S_OK;
	}

	virtual HRESULT __stdcall RetrieveState(FWULONG nBufSize, BYTE *pBuf, FWULONG *pnCount)
	{
		if (pnCount) *pnCount = sizeof(FWMATRIX);
		if (nBufSize < *pnCount)
			return ERROR(FW_E_MEMOVERRUN);
		if (m_pILocalTransform) m_pILocalTransform->FromMatrix(*(FWMATRIX*)pBuf);
		return S_OK;
	}


// Error Handling...
protected:
	HRESULT ErrorLabel(HRESULT hCode)
	{
		FWULONG N[1];
		IKineChild *pChild = NULL;
		if (SUCCEEDED(QueryInterface(&pChild)))
			{ pChild->GetLabel((FWSTRING*)&N[0]); pChild->Release(); }
		else
			N[0] = (FWULONG)(__int64)L"(unknown)";
		return Error(hCode, 1, N, NULL, 0);
	}
};

///////////////////////////////////////////////////////////
//
// IKineChild implementation template

template <class T>
class CTKineChild : public CTKineObj3D<T>
{
protected:

	IKineNode *m_pIParent;	// The Primary Parent (secondary parents are unknown)
	FWULONG m_nId;			// id at the Primary Parent (a copy)
	LPOLESTR m_pLabel;		// label at the Primary Parent (a copy)

public:

	CTKineChild()  : m_pIParent(NULL), m_nId(-1), m_pLabel(NULL) { }

	~CTKineChild()
	{
		// m_pIParent is a weak pointer, no Release call here...
	}

	// Error Definitions
	FW_ERROR_BEGIN
		FW_ERROR_ENTRY(KINE_E_ROOTOBJERR,		L"The hierarchy root object cannot be assigned with labels or ids", FW_SEV_CRITICAL)
	FW_ERROR_END

	virtual HRESULT __stdcall GetParent(/*[out, retval]*/ IKineNode **p)
	{
		if (p) *p = m_pIParent;
		if (p && *p) (*p)->AddRef();
		return m_pIParent ? S_OK : S_FALSE;
	}

	virtual HRESULT __stdcall IsParent(IKineChild *p)
	{
		return (p == m_pIParent) ? S_OK : S_FALSE;
	}

	virtual HRESULT __stdcall Delete()
	{
		if (!m_pIParent) return S_OK;
		else return m_pIParent->DelChildPtr(this);
	}

	virtual HRESULT __stdcall GetId(FWULONG *pId)
	{
		assert(pId);
		HRESULT h = S_OK;
		if (m_nId < 0 && m_pIParent)
			h = GetIdEx(m_pIParent, &m_nId);
		*pId = m_nId;
		return h;
	}

	virtual HRESULT __stdcall GetLabel(LPOLESTR *pVal)
	{
		if (!pVal) return ERROR(FW_E_POINTER);
		HRESULT h = S_OK;
		if (!m_pLabel && m_pIParent)
			h = GetLabelEx(m_pIParent, &m_pLabel);
		*pVal = m_pLabel ? m_pLabel : L"<root>";
		return h;
	}

	virtual HRESULT __stdcall PutLabel(LPOLESTR pVal)
	{
		if (m_pIParent)
			return PutLabelEx(m_pIParent, pVal);
		else
			return ERROR(KINE_E_ROOTOBJERR, 0, NULL, NULL, 0);
	}

	virtual HRESULT __stdcall GetIdEx(IKineNode *pParentEx, FWULONG *pId)
	{
		assert(pParentEx && pId);
		*pId = -1;
		return pParentEx->GetChildInformation(this, NULL, pId);
	}

	virtual HRESULT __stdcall GetLabelEx(IKineNode *pParentEx, LPOLESTR *pVal)
	{
		assert(pParentEx && pVal);
		*pVal = L"";
		return pParentEx->GetChildInformation(this, pVal, NULL);
	}

	virtual HRESULT __stdcall PutLabelEx(IKineNode *pParentEx, LPOLESTR pVal)
	{
		assert(pParentEx && pVal);
		FWULONG nId;
		HRESULT h = pParentEx->GetChildInformation(this, NULL, &nId);
		if (FAILED(h)) return h;
		return pParentEx->RenameChildAt(nId, pVal);
	}

	virtual HRESULT __stdcall OnParentNotify(IKineNode *pParent, LPOLESTR pLabel, FWULONG nId)
	{
		assert(pParent);
		m_pIParent = pParent;	// this is weak pointer --- no AddRef call!
		m_nId = nId;
		m_pLabel = pLabel;
		return S_OK;
	}

	virtual HRESULT __stdcall OnParentUnnotify(IKineNode *pParent)
	{
		assert(pParent);
		if (pParent != m_pIParent)
			return S_OK;	// it was not my notification parent
		m_pIParent = NULL;	//weak pointer --- no Release call!
		m_nId = -1;
		m_pLabel = NULL;
		return S_OK;
	}

// Overwritten Versions of IKineObj3D

	// combines with the Parent's Transforms
	virtual HRESULT __stdcall Validate()
	{
		if (m_bIsValid)
			return S_OK;

		// Prepare the Parent Transform
		if (m_pIParent)
		{
			if (!m_pIParentTransform)
				m_pIParent->CreateCompatibleTransform(&m_pIParentTransform);
			HRESULT h = m_pIParent->GetGlobalTransform(m_pIParentTransform);
			if (FAILED(h)) return FW_E_CLASSSPECIFIC;
		}

		return CTKineObj3D<T>::Validate();
	}


// Error Handling...
protected:

	HRESULT ErrorLabel(HRESULT hCode)
	{
		FWULONG N[1];
		GetLabel((FWSTRING*)&N[0]);
		return Error(hCode, 1, N, NULL, 0);
	}

	HRESULT ErrorLabel(HRESULT hCode, FWSTRING pArg)
	{
		FWULONG N[2];
		N[0] = (FWULONG)(__int64)pArg;
		GetLabel((FWSTRING*)&N[1]);
		return Error(hCode, 2, N, NULL, 0);
	}

	HRESULT ErrorLabel(HRESULT hCode, FWULONG nArg)
	{
		FWULONG N[2];
		N[0] = nArg;
		GetLabel((FWSTRING*)&N[1]);
		return Error(hCode, 2, N, NULL, 0);
	}
};

///////////////////////////////////////////////////////////
//
// IKineNode implementation template


template <class T>
class CTKineNode : public CTKineChild<T>
{
protected:

	// predicate functor for the map of LPOLESTR
	// compares p and q (like wsccmp), strings are zero-or-dot-terminated
	struct STRLESS
	{
		bool operator()(LPOLESTR p, LPOLESTR q) const 
		{	while(*p == *q && *q && *q != '.') ++p, ++q;
			return (*p == '.' ? 0 : *p) - (*q == '.' ? 0 : *q) < 0;
		} 
	};

	// child record...
	struct IPTR
	{
		IKineChild *pChild;		// the child itself...
		LPOLESTR pLabel;		// the label
		BOOL bParentalDep;
	};
	
	typedef std::map<LPOLESTR, FWULONG, STRLESS> STRMAP;	// map: labels to indices
	typedef std::map<IKineChild*, FWULONG> PTRMAP;		// map: pointers to indices
	typedef std::vector<IPTR> PTRVEC;					// vec: indices to IPTR
	
	STRMAP m_strmap;	// maps labels to indices
	PTRMAP m_ptrmap;	// maps pointers to indices
	PTRVEC m_vector;	// maps indices to IPTR

public:

	CTKineNode() { }

	~CTKineNode()
	{
		for (unsigned i = 0; i < m_vector.size(); i++)
		{
			IKineChild *p;
			GetChildAt(i, &p);
			if (p) 
			{
				p->Release();
				DelChildAt(i);
			}
		}
	}

	// Error Definitions
	FW_ERROR_BEGIN
		FW_ERROR_ENTRY(KINE_E_NOTRANSFORM,		L"Operation not completed due to lack of an essential transforms", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(KINE_E_ROOTOBJERR,		L"The hierarchy root object cannot be assigned with labels or ids", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(KINE_E_BADCHILDINDEX,		L"There is no child object with index value %d in node \"%s\"", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(KINE_E_LABELNOTFOUND,		L"Label %s not found in node \"%s\"", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(KINE_E_EMPTYLABEL,		L"Label is empty in node \"%s\"", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(KINE_E_LABELEXISTS,		L"Label %s already exists in node \"%s\"", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(KINE_E_OBJECTNOTFOUND,	L"Object or object pointer not found in node \"%s\"", FW_SEV_CRITICAL)
	FW_ERROR_END

	virtual HRESULT __stdcall CreateUniqueLabel(LPOLESTR pLabel, FWULONG nSize, /*[in, out, size_is(nSize)]*/ LPOLESTR pUniqueLabel)
	{
		// first --- try pLabel
		size_t nLen = wcslen(pLabel);
		if (nLen > nSize-1) return ERROR(FW_E_MEMOVERRUN);
		wcscpy_s(pUniqueLabel, nSize, pLabel);
		if (HasChild(pUniqueLabel) == S_FALSE) 
			return S_OK; 

		// scan through digits at the end of pLabel (if any)
		LPOLESTR p = _wcsdup(pLabel);
		FWULONG nWidth = 0;	// width of the index (a number attached to the end of pLabel)
		for (FWULONG i = nLen - 1; i >= 0 && iswdigit(p[i]); i--)
			if (++nWidth >= 3) break;

		// retrieve pure label and the nIndex, create the formatter
		unsigned nIndex = 0;
		OLECHAR fmt[8];
		if (nWidth)
		{
			nIndex = _wtoi(p + nLen - nWidth);
			p[nLen - nWidth] = '\0';
			wcscpy_s(fmt, 8, L"%ls%00d");
			fmt[5] = '0' + (OLECHAR)nWidth;
		}
		else
			wcscpy_s(fmt, 6, L"%ls%d");

		if (wcslen(p)+3 > nSize-1) return ERROR(FW_E_MEMOVERRUN);
		
		int nCount = 990;
		do
		{
			swprintf(pUniqueLabel, nSize, fmt, p, nIndex++);
			if (--nCount <= 0) return ERROR(KINE_E_LABELEXISTS);
		} while (HasChild(pUniqueLabel) == S_OK);

		free(p);
		
		return S_OK;
	}

	virtual HRESULT __stdcall AddChild(LPOLESTR pLabel, IKineChild *p)
	{
		return AddChildEx(pLabel, p, TRUE, NULL, NULL);
	}

	virtual HRESULT __stdcall GetChild(LPOLESTR pLabel, IKineChild **p)
	{
		assert(pLabel);
		return GetChildEx(pLabel, FALSE, NULL, NULL, NULL, p);
	}

	virtual HRESULT __stdcall CheckChild(LPOLESTR pLabel, IKineChild **p)
	{
		assert(pLabel);
		return GetChildEx(pLabel, TRUE, NULL, NULL, NULL, p);
	}

	virtual HRESULT __stdcall HasChild(LPOLESTR pLabel)
	{
		IKineChild *p = NULL;
		HRESULT h = CheckChild(pLabel, &p);
		if (p) p->Release();
		return h;
	}

	virtual HRESULT __stdcall DelChild(LPOLESTR pLabel)
	{
		assert(pLabel);
		IKineNode *pParent = NULL;
		FWULONG nId;
		HRESULT h = GetChildEx(pLabel, FALSE, &pParent, &nId, NULL, NULL);
		if (FAILED(h)) return h;
		assert(pParent);
		h = pParent->DelChildAt(nId);
		pParent->Release();
		return h;
	}

	virtual HRESULT __stdcall DelAll()
	{
		for (unsigned i = 0; i < m_vector.size(); i++)
		{
			IKineChild *p;
			GetChildAt(i, &p);
			if (p) 
			{
				p->Release();
				DelChildAt(i);
			}
		}
		return S_OK;
	}

	virtual HRESULT __stdcall RenameChild(LPOLESTR pLabel, LPOLESTR pNewLabel)
	{
		assert(pLabel);
		IKineNode *pParent = NULL;
		FWULONG nId;
		HRESULT h = GetChildEx(pLabel, FALSE, &pParent, &nId, NULL, NULL);
		if (FAILED(h)) return h;
		assert(pParent);
		h = pParent->RenameChildAt(nId, pNewLabel);
		pParent->Release();
		return h;
	}

	virtual HRESULT __stdcall AddChildEx(LPOLESTR pLabel, IKineChild *p, BOOL bSetParentalDep, IKineNode **pParentNode, FWULONG *pnId)
	{
		assert(pLabel && p);	// pnId may be NULL
		if (pParentNode) *pParentNode = NULL;
		if (wcschr(pLabel, '.'))
		{
			// label is qualified
			// resolve label, then pass the action to the appropriate node
			IKineNode *pParent = NULL;
			FWULONG nId;
			LPOLESTR pRemaining;

			// try to resolve name, intercept any error
			HRESULT h;
			try { h = GetChildEx(pLabel, FALSE, &pParent, &nId, &pRemaining, NULL); }
			catch(FWERROR *e) { h = e->nCode; }

			// after resolving pRemaining should contain one and just one
			// simple label that specifies a new child to be added
			
			if (SUCCEEDED(h))
			{
				// if resolve succeeded - this label already exists, cannot add a new child
				if (pParent) pParent->Release();
				return ERRORLABEL(KINE_E_LABELEXISTS, pLabel);
			}
			if (wcschr(pRemaining, '.'))
			{
				// remaining is still qualified:
				// cannot create more than one level in a qualified label
				FWDevice()->Recover(FW_SEV_NOTHING);
				if (pParent) pParent->Release();
				return ERRORLABEL(KINE_E_LABELNOTFOUND, pLabel);
			}
			if (h == KINE_E_LABELNOTFOUND)
			{
				// that's all right
				FWDevice()->Recover(FW_SEV_NOTHING);
				assert(pParent);
				h = pParent->AddChildEx(pRemaining, p, bSetParentalDep, pParentNode, pnId);
				pParent->Release();
				return h;
			}
			else
			{
				// unidentified error
				if (pParent) pParent->Release();
				return h;
			}
		}
		else
		{
			// simple label: 
			// no label resolving needed, do it yourself
			if (pParentNode) 
			{
				*pParentNode = this;
				AddRef();	// added 25-11-2003
			}

			// check for existing
			if (m_strmap.find(pLabel) != m_strmap.end())
				return ERRORLABEL(KINE_E_LABELEXISTS, pLabel);

			// add pointers to m_vector
			IPTR iptr = { p, _wcsdup(pLabel), bSetParentalDep };
			iptr.pChild->AddRef();
			m_vector.push_back(iptr);
			FWULONG nId = (FWULONG)m_vector.size() - 1;
			if (pnId) *pnId = nId;
			// add mapping
			if (iptr.pLabel) m_strmap[iptr.pLabel] = nId;
			m_ptrmap[p] = nId;
			
			// notify child
			if (bSetParentalDep)
				p->OnParentNotify(this, iptr.pLabel, nId);

			// invalidate child
			if (bSetParentalDep && iptr.pChild)
				iptr.pChild->Invalidate();

			return S_OK;
		}
	}

	virtual HRESULT __stdcall GetChildEx(LPOLESTR pLabel, BOOL bQuiet, IKineNode **pParentNode, FWULONG *pnId, LPOLESTR *pRemaining, IKineChild **pChild)
	{
		assert(pLabel);	// all [out] params may be NULL

		if (pParentNode)
		{
			*pParentNode = this;
			AddRef();	// added 25/11/2003
		}
		if (pnId) *pnId = -1;
		if (pRemaining) *pRemaining = pLabel;
		if (pChild) *pChild = NULL;

		if (!pLabel || !*pLabel)
			return bQuiet ? S_FALSE : (ERRORLABEL(KINE_E_EMPTYLABEL));

		// find index among my children: m_strmap predicate accepts zero-or-dot-terminated strings
		STRMAP::iterator i = m_strmap.find(pLabel);
		if (i == m_strmap.end())
			// error: not found
 			return bQuiet ? S_FALSE : (ERRORLABEL(KINE_E_LABELNOTFOUND, pLabel));

		LPOLESTR p = wcschr(pLabel, '.');
		if (p == NULL)
		{
			// not-qualified label: it's my child
			FWULONG nId = i->second;
			if (pnId) *pnId = nId;
			if (pRemaining) *pRemaining = NULL;
			if (pChild) 
			{
				*pChild = m_vector[nId].pChild;
				(*pChild)->AddRef();	// added 25/11/2003
			}
			return S_OK;
		}

		// qualified label: still need looking
		IKineNode *pNode = NULL;
		m_vector[i->second].pChild->QueryInterface(&pNode);
		if (pNode)
		{
			if (pParentNode)
			{
				//it's not me...
				*pParentNode = NULL;
				Release();
			}
			HRESULT h = pNode->GetChildEx(p+1, bQuiet, pParentNode, pnId, pRemaining, pChild);
			pNode->Release();
			return h;
		}
		else
			// the child addressed may be not a parent node...
			return bQuiet ? S_FALSE : (ERRORLABEL(KINE_E_LABELNOTFOUND, pLabel));
	}

	virtual HRESULT __stdcall GetChildInformation(IKineChild *pKineChild, LPOLESTR *p, FWULONG *pnId)
	{
		assert(pKineChild);	// all [out] params may be NULL
		PTRMAP::iterator i = m_ptrmap.find(pKineChild);
		if (i == m_ptrmap.end())
			return ERRORLABEL(KINE_E_OBJECTNOTFOUND);
		if (pnId) *pnId = i->second;
		if (p) *p = m_vector[i->second].pLabel;
		return S_OK;
	}

	virtual HRESULT __stdcall DelChildPtr(IKineChild *pChild)
	{
		assert(pChild);
		PTRMAP::iterator i = m_ptrmap.find(pChild);
		if (i == m_ptrmap.end())
			return ERRORLABEL(KINE_E_OBJECTNOTFOUND);
		DelChildAt(i->second);
		return S_OK;
	}

	virtual HRESULT __stdcall RenameChildPtr(IKineChild *pChild, LPOLESTR pNewLabel)
	{
		assert(pChild);
		PTRMAP::iterator i = m_ptrmap.find(pChild);
		if (i == m_ptrmap.end())
			return ERRORLABEL(KINE_E_OBJECTNOTFOUND);
		RenameChildAt(i->second, pNewLabel);
		return S_OK;
	}

	virtual HRESULT __stdcall GetChildCount(FWULONG *nCount)
	{
		assert(nCount);
		*nCount = (FWULONG)m_vector.size();
		return S_OK;
	}

	virtual HRESULT __stdcall GetChildAt(FWULONG nId, IKineChild **p)
	{
		assert(p);
		if (nId < 0 || nId >= (FWULONG)m_vector.size())
			return ERRORLABEL(KINE_E_BADCHILDINDEX, nId);
		*p = m_vector[nId].pChild;
		if (*p) (*p)->AddRef();
		return S_OK;
	}

	virtual HRESULT __stdcall GetLabelAt(FWULONG nId, LPOLESTR *p)
	{
		assert(p);
		if (nId > (FWULONG)m_vector.size())
			return ERRORLABEL(KINE_E_BADCHILDINDEX, nId);
		*p = m_vector[nId].pLabel;
		return S_OK;
	}

	virtual HRESULT __stdcall DelChildAt(FWULONG nId)
	{
		if (nId > (FWULONG)m_vector.size())
			return ERRORLABEL(KINE_E_BADCHILDINDEX, nId);
		IPTR *p = &m_vector[nId];
		if (!p->pChild)
			return ERRORLABEL(KINE_E_BADCHILDINDEX, nId);
		// delete mappings
		if (p->pLabel)
		{
			STRMAP::iterator i1 = m_strmap.find(p->pLabel);
			if (i1 != m_strmap.end())
				m_strmap.erase(i1);
		}
		if (p->pChild)
		{
			PTRMAP::iterator i2 = m_ptrmap.find(p->pChild);
			if (i2 != m_ptrmap.end())
				m_ptrmap.erase(i2);
		}
		// unnotify child
		p->pChild->OnParentUnnotify(this);

		// release
		if (p->pChild) p->pChild->Release();
		if (p->pLabel) free(p->pLabel);
		memset(p, 0, sizeof(IPTR));
		return S_OK;
	}

	virtual HRESULT __stdcall RenameChildAt(FWULONG nId, LPOLESTR pNewLabel)
	{
		assert(pNewLabel);
		if (nId > (FWULONG)m_vector.size())
			return ERRORLABEL(KINE_E_BADCHILDINDEX, nId);
		if (m_strmap.find(pNewLabel) != m_strmap.end())
			return ERRORLABEL(KINE_E_LABELEXISTS, pNewLabel);
		IPTR *p = &m_vector[nId];
		if (!p->pChild)
			return ERRORLABEL(KINE_E_BADCHILDINDEX, nId);
		// delete old string & string mapping
		if (p->pLabel)
		{
			STRMAP::iterator i1 = m_strmap.find(p->pLabel);
			if (i1 != m_strmap.end())
				m_strmap.erase(i1);
			free(p->pLabel);
		}
		// create new label
		p->pLabel = _wcsdup(pNewLabel);
		// create new mapping
		if (p->pLabel) m_strmap[p->pLabel] = nId;
		// notify child
		p->pChild->OnParentNotify(this, p->pLabel, nId);
		return S_OK;
	}

	virtual HRESULT __stdcall CreateFlatNamespace()
	{
		IKineChild *pDesc = NULL;
		IKineEnumChildren *pEnum = NULL;
		EnumAllDescendants(&pEnum);
		while (pEnum->Next(&pDesc) == S_OK)
		{
			assert(pDesc);
			LPOLESTR pLabel = NULL;
			pDesc->GetLabel(&pLabel);
			if (pDesc->IsParent(this) == S_FALSE)
			{
				IKineChild *pCheck = NULL;
				CheckChild(pLabel, &pCheck);
				if (pCheck)
					pCheck->Release();
				else
					AddChildEx(pLabel, pDesc, FALSE, NULL, NULL); 
			}
			pDesc->Release();
		}
		pEnum->Release();
		return S_OK;
	}

	virtual HRESULT __stdcall EnumChildren(IKineEnumChildren **pp)
	{
		assert(pp);
		CKineEnumChildren *p = new CKineEnumChildren(this);
		if (!p) return ERROR(FW_E_OUTOFMEMORY, 0, NULL, NULL, 0);
		FWDevice()->RegisterObject(p);
		*pp = p;
		return S_OK;
	}

	virtual HRESULT __stdcall EnumAllDirectDescendants(REFIID iidExclude, IKineEnumChildren **pp)
	{
		assert(pp);
		CKineEnumChildren *p = new CKineEnumChildren(this, true, iidExclude);
		if (!p) return ERROR(FW_E_OUTOFMEMORY, 0, NULL, NULL, 0);
		FWDevice()->RegisterObject(p);
		*pp = p;
		return S_OK;
	}

	virtual HRESULT __stdcall EnumAllDescendants(IKineEnumChildren **pp)
	{
		assert(pp);
		CKineEnumChildren *p = new CKineEnumChildren(this, true);
		if (!p) return ERROR(FW_E_OUTOFMEMORY, 0, NULL, NULL, 0);
		FWDevice()->RegisterObject(p);
		*pp = p;
		return S_OK;
	}

// Overwritten Versions of IKineObj3D

	// Overriden from CTKineObj3D - extends the functionality to all the child nodes
	virtual HRESULT __stdcall Invalidate()
	{
		HRESULT h = CTKineObj3D<T>::Invalidate();
		if (FAILED(h)) return h;
		for (unsigned i = 0; i < m_vector.size(); i++)
		{
			// invalidate children
			if (!m_vector[i].bParentalDep)
				continue;
			if (m_vector[i].pChild)
			{
				h = m_vector[i].pChild->Invalidate();
				if (FAILED(h)) return h;
			}
		}
		return S_OK;
	}

	// Overriden from CTKineObj3D - extends the functionality to all the child nodes
	virtual HRESULT __stdcall Reproduce(/*[out, retval]*/ IKineObj3D **p)
	{
		// reproduce myself (as IKineObj3D)
		IKineObj3D *pRepObj = NULL;
		HRESULT h = CTKineObj3D<T>::Reproduce(&pRepObj);
		if (FAILED(h)) return h;

		// query for IKineNode
		IKineNode *pRepNode = NULL;
		h = pRepObj->QueryInterface(&pRepNode);
		if (FAILED(h)) { pRepObj->Release(); return ERROR(FW_E_NOINTERFACE); }

		// enumerate my children
		IKineChild *pChild = NULL;
		IKineEnumChildren *pEnum = NULL;
		EnumChildren(&pEnum);
		while (pEnum->Next(&pChild) == S_OK)
		{
			pChild->Release();	// will not be deleted, but Release now is handy...

			// get label
			LPOLESTR pLabel = NULL;
			pChild->GetLabel(&pLabel);

			// omit if I am not the primary parent
			if (pChild->IsParent(this) == S_FALSE)
				continue;

			// omit if a mesh
			if (pChild->CanReproduce() == S_FALSE)
				continue;

			// reproduce (as IKineObj3D)
			IKineNode *pRepDesc = NULL;
			h = pChild->ReproduceEx(IID_IKineNode, (IFWUnknown**)&pRepDesc);
			if (FAILED(h)) continue;

			// add the descendant as a child
			pRepNode->AddChild(pLabel, pRepDesc);
			pRepDesc->Release();
		}
		pRepNode->Release();
		pEnum->Release();
		*p = pRepObj;
		return S_OK;
	}

	// Overriden from CTKineObj3D - extends the functionality to all the child nodes
	virtual HRESULT __stdcall PushState()
	{
		CTKineObj3D<T>::PushState();

		IKineChild *p;
		IKineEnumChildren *pEnum = NULL;
		EnumChildren(&pEnum);
		while (pEnum->Next(&p) == S_OK)
		{
			// omit if I am not the primary parent
			if (p->IsParent(this) == S_FALSE)
			{
				p->Release();
				continue;
			}
			p->PushState();
			p->Release();
		}
		pEnum->Release();
		return S_OK;
	}

	// Overriden from CTKineObj3D - extends the functionality to all the child nodes
	virtual HRESULT __stdcall PopState()
	{
		IKineChild *p;
		IKineEnumChildren *pEnum = NULL;
		EnumChildren(&pEnum);
		HRESULT h = S_OK;
		while (pEnum->Next(&p) == S_OK)
		{
			// omit if I am not the primary parent
			if (p->IsParent(this) == S_FALSE)
			{
				p->Release();
				continue;
			}

			p->PopState();
			p->Release();
		}
		pEnum->Release();

		FWDevice()->EnableErrorException(TRUE);
		try { CTKineObj3D<T>::PopState(); }
		catch (FWERROR *p) { h = p->nCode; FWDevice()->Recover(FW_SEV_NOTHING); }
		FWDevice()->EnableErrorException(FALSE);

		return SUCCEEDED(h) ? h : ERROR(h);
	}

	// Overriden from CTKineObj3D - extends the functionality to all the child nodes
	virtual HRESULT __stdcall StoreState(FWULONG nBufSize, BYTE *pBuf, FWULONG *pnCount)
	{
		FWULONG nCount = 0;	// counter of positions used
		HRESULT h = S_OK;	// hresult

		FWDevice()->EnableErrorException(TRUE);
		try { CTKineObj3D<T>::StoreState(nBufSize, pBuf, &nCount); }
		catch (FWERROR *p) { h = p->nCode; FWDevice()->Recover(FW_SEV_NOTHING); }
		FWDevice()->EnableErrorException(FALSE);

		IKineChild *p;
		IKineEnumChildren *pEnum = NULL;
		EnumChildren(&pEnum);
		while (pEnum->Next(&p) == S_OK)
		{
			// omit if I am not the primary parent
			if (p->IsParent(this) == S_FALSE)
			{
				p->Release();
				continue;
			}

			FWULONG nCount1 = 0;
			FWDevice()->EnableErrorException(TRUE);
			try { p->StoreState(nBufSize - nCount, pBuf ? pBuf + nCount : NULL, &nCount1); }
			catch (FWERROR *p) { h = p->nCode; FWDevice()->Recover(FW_SEV_NOTHING); }
			FWDevice()->EnableErrorException(FALSE);
			nCount += nCount1;
			p->Release();
		}
		pEnum->Release();

		if (pnCount) *pnCount = nCount;

		return SUCCEEDED(h) ? h : ERROR(h);
	}

	// Overriden from CTKineObj3D - extends the functionality to all the child nodes
	virtual HRESULT __stdcall RetrieveState(FWULONG nBufSize, BYTE *pBuf, FWULONG *pnCount)
	{
		FWULONG nCount = 0;	// counter of positions used
		HRESULT h = S_OK;	// hresult

		FWDevice()->EnableErrorException(TRUE);
		try { CTKineObj3D<T>::RetrieveState(nBufSize, pBuf, &nCount); }
		catch (FWERROR *p) { h = p->nCode; FWDevice()->Recover(FW_SEV_NOTHING); }
		FWDevice()->EnableErrorException(FALSE);

		IKineChild *p;
		IKineEnumChildren *pEnum = NULL;
		EnumChildren(&pEnum);
		while (pEnum->Next(&p) == S_OK)
		{
			// omit if I am not the primary parent
			if (p->IsParent(this) == S_FALSE)
			{
				p->Release();
				continue;
			}

			FWULONG nCount1 = 0;
			FWDevice()->EnableErrorException(TRUE);
			try { p->RetrieveState(nBufSize - nCount, pBuf ? pBuf + nCount : NULL, &nCount1); }
			catch (FWERROR *p) { h = p->nCode; FWDevice()->Recover(FW_SEV_NOTHING); }
			FWDevice()->EnableErrorException(FALSE);
			nCount += nCount1;
			p->Release();
		}
		pEnum->Release();

		if (pnCount) *pnCount = nCount;

		return SUCCEEDED(h) ? h : ERROR(h);
	}
};

///////////////////////////////////////////////////////////
//
// IKineTargetedObject implementation template


template <class T>
class CTKineTargetedObject : public CTKineNode<T>
{
protected:
	KINE_TARGET m_mode;			// mode - see KINE_TARGET
	IKineNode *m_pNodePair;		// external paired node (target or object depending on mode)
	IKineNode *m_pNodeUp;		// up-vector node
	BOOL m_bTiltWithTarget;		// flag set with PutTiltWithTarget

public:

	CTKineTargetedObject()
	{
		m_mode = KINE_TARGET_INCENTER;
		m_pNodePair = NULL;
		m_pNodeUp = NULL;
		m_bTiltWithTarget = FALSE;
	}

	~CTKineTargetedObject()
	{
		Destruct();
	}

	virtual HRESULT __stdcall PutPosition(FWVECTOR vPosition)
	{
		if (!m_pNodePair) Construct();
		FWVECTOR tmp;
		switch (m_mode)
		{
		case KINE_TARGET_INCENTER:
			PutNodePos(m_pNodePair, &vPosition);
			break;
		case KINE_TARGET_ORBITING:
			GetNodePos(m_pNodePair, &tmp);
			PutNodePos(this, &vPosition);
			PutNodePos(m_pNodePair, &tmp);
			break;
		case KINE_TARGET_PEER:
		case KINE_TARGET_REMOTE:
			PutNodePos(this, &vPosition);
			break;
		}
		return S_OK;
	}

	virtual HRESULT __stdcall GetPosition(/*[out, retval]*/ FWVECTOR *p)
	{
		GetNodePos(m_mode == KINE_TARGET_INCENTER ? m_pNodePair : this, p);
		return S_OK;
	}

	virtual HRESULT __stdcall PutTarget(FWVECTOR vTarget)
	{
		if (!m_pNodePair) Construct();
		FWVECTOR tmp;
		switch (m_mode)
		{
		case KINE_TARGET_INCENTER:
			GetNodePos(m_pNodePair, &tmp);
			PutNodePos(this, &vTarget);
			PutNodePos(m_pNodePair, &tmp);
			break;
		case KINE_TARGET_ORBITING:
		case KINE_TARGET_PEER:
		case KINE_TARGET_REMOTE:
			PutNodePos(m_pNodePair, &vTarget);
			break;
		}
		return S_OK;
	}

	virtual HRESULT __stdcall GetTarget(/*[out, retval]*/ FWVECTOR *p)
	{
		GetNodePos(m_mode == KINE_TARGET_INCENTER ? this : m_pNodePair, p);
		return S_OK;
	}

	virtual HRESULT __stdcall PutDirection(FWVECTOR vDir)
	{
		if (!m_pNodePair) Construct();
		FWVECTOR tmp;
		GetPosition(&tmp);
		vDir.x += tmp.x; vDir.y += tmp.y; vDir.z += tmp.z; 
		PutTarget(vDir);
		return S_OK;
	}

	virtual HRESULT __stdcall GetDirection(/*[out, retval]*/ FWVECTOR *p)
	{
		if (!p) return S_OK;
		FWVECTOR v1, v2;
		GetPosition(&v1);
		GetTarget(&v2);
		p->x = v2.x - v1.x; p->y = v2.y - v1.y; p->z = v2.z - v1.z; 
		return S_OK;
	}

	virtual HRESULT __stdcall PutUpVector(FWVECTOR vUp)
	{
		HRESULT h = S_OK;
		if (!m_pNodePair)
			Construct(NULL);
		if (!m_pNodeUp)
		{
			// create up-vector node
			IKineNode *pNode = NULL;
			if ((m_bTiltWithTarget && m_mode != KINE_TARGET_INCENTER) || (!m_bTiltWithTarget && m_mode == KINE_TARGET_INCENTER))
				pNode = m_pNodePair;
			else
				pNode = this;
				//h = QueryInterface(IID_IKineNode, (void**)&pNode);
			if (FAILED(h)) return ERROR(FW_E_NOINTERFACE);

			h = AddUpNode(pNode);	
			if (FAILED(h)) return h;
		}
		FWVECTOR v;
		if (m_bTiltWithTarget)
			GetTarget(&v);
		else
			GetPosition(&v);
		v.x += vUp.x; v.y += vUp.y; v.z += vUp.z;
		PutNodePos(m_pNodeUp, &v);
		return S_OK;
	}

	virtual HRESULT __stdcall GetUpVector(/*[out, retval]*/ FWVECTOR *pV)
	{
		if (!pV) return S_OK;
		pV->x = pV->y = pV->z = 0.0f;
		if (!m_pNodeUp) return S_OK;

		FWVECTOR v1, v2;
		if (m_bTiltWithTarget)
			GetTarget(&v1);
		else
			GetPosition(&v1);
		GetNodePos(m_pNodeUp, &v2);
		pV->x = v2.x - v1.x; pV->y = v2.y - v1.y; pV->z = v2.z - v1.z;
		return S_OK;
	}

	virtual HRESULT __stdcall GetLookAtLHTransform(/*[out, retval]*/ ITransform **p)
	{
		if (!p) return S_OK;
		HRESULT h;
		FWVECTOR vEye, vAt, vUp;
		h = GetPosition(&vEye); if (FAILED(h)) return h;
		h = GetTarget(&vAt); if (FAILED(h)) return h;
		h = GetUpVector(&vUp); if (FAILED(h)) return h;
		h = FWDevice()->CreateObject(L"Transform", IID_ITransform, (IFWUnknown**)p); if (FAILED(h)) return h;
		return (*p)->FromLookAtLH(&vEye, &vAt, &vUp);
	}
	
	virtual HRESULT __stdcall GetLookAtRHTransform(/*[out, retval]*/ ITransform **p)
	{
		if (!p) return S_OK;
		HRESULT h;
		FWVECTOR vEye, vAt, vUp;
		h = GetPosition(&vEye); if (FAILED(h)) return h;
		h = GetTarget(&vAt); if (FAILED(h)) return h;
		h = GetUpVector(&vUp); if (FAILED(h)) return h;
		h = FWDevice()->CreateObject(L"Transform", IID_ITransform, (IFWUnknown**)p); if (FAILED(h)) return h;
		return (*p)->FromLookAtRH(&vEye, &vAt, &vUp);
	}

	// Kinematical Configuration
	virtual HRESULT __stdcall PutConfig(enum KINE_TARGET mode, IKineNode *pRemoteTarget)
	{
		FWVECTOR v1, v2, v3;
		GetPosition(&v1);
		GetTarget(&v2);
		GetUpVector(&v3);
		Destruct();
		m_mode = mode;
		Construct(pRemoteTarget);
		PutPosition(v1);
		PutTarget(v2);
		PutUpVector(v3);
		return S_OK;
	}

	virtual HRESULT __stdcall GetConfig(/*[out]*/ enum KINE_TARGET *pMode, /*[out]*/ IKineNode **pRemoteTarget)
	{
		if (pMode) *pMode = m_mode;
		if (pRemoteTarget) *pRemoteTarget = (m_mode == KINE_TARGET_REMOTE) ? m_pNodePair : NULL;
		return S_OK;
	}

	virtual HRESULT __stdcall GetPairedNode(/*[out, retval]*/ IKineNode **p)
	{
		if (p) *p = m_pNodePair;
		return S_OK;
	}

	virtual HRESULT __stdcall PutTiltWithTarget(BOOL b)
	{
		if (!m_pNodePair)
		{
			m_bTiltWithTarget = b;
			return S_OK;
		}
		else
		{
			FWVECTOR v;
			GetUpVector(&v);
			//if (m_pNodeUp)	m_pNodeUp->Delete();
			if (m_pNodeUp)  m_pNodeUp->Release();   m_pNodeUp = NULL;
			m_bTiltWithTarget = b;
			PutUpVector(v);
			return S_OK;
		}
		return S_OK;
	}

	virtual HRESULT __stdcall GetTiltWithTarget()
	{
		return m_bTiltWithTarget ? S_OK : S_FALSE;
	}

// Overridables

	virtual HRESULT CreatePairNode(IKineNode **p)	{ return FWDevice()->CreateObject(L"KineBone", IID_IKineNode, (IFWUnknown**)p); }
	virtual HRESULT CreateUpNode(IKineNode **p)		{ return FWDevice()->CreateObject(L"KineBone", IID_IKineNode, (IFWUnknown**)p); }

// Tools
protected:

	HRESULT AddPairNode(IKineNode *pParentNode, BOOL bTarget)
	{
		FWSTRING pLabel = NULL;
		GetLabel(&pLabel);
		FWSTRING pPostfix = bTarget ? L"_target" : L"_obj";
		FWSTRING pNewLabel = new OLECHAR[wcslen(pLabel) + wcslen(pPostfix) + 1];
		wcscpy(pNewLabel, pLabel);
		wcscat(pNewLabel, pPostfix);
		HRESULT h = CreatePairNode(&m_pNodePair);	
		if (FAILED(h)) return h;
		h = pParentNode->AddChild(pNewLabel, m_pNodePair);
		delete [] pNewLabel;
		if (FAILED(h)) 	{ m_pNodePair->Delete(); m_pNodePair->Release(); m_pNodePair = NULL; return h; }
		return h;
	}

	HRESULT AddUpNode(IKineNode *pParentNode)
	{
		FWSTRING pLabel = NULL;
		GetLabel(&pLabel);
		FWSTRING pPostfix = L"_upvec";
		FWSTRING pNewLabel = new OLECHAR[wcslen(pLabel) + wcslen(pPostfix) + 1];
		wcscpy(pNewLabel, pLabel);
		wcscat(pNewLabel, pPostfix);
		HRESULT h = CreateUpNode(&m_pNodeUp);	
		if (FAILED(h)) return h;
		h = pParentNode->AddChild(pNewLabel, m_pNodeUp);
		delete [] pNewLabel;
		if (FAILED(h)) 	{ m_pNodeUp->Delete(); m_pNodeUp->Release(); m_pNodeUp = NULL; return h; }
		return h;
	}

	HRESULT Construct(IKineNode *pRemoteTarget = NULL)
	{
		HRESULT h = S_OK;
		IKineNode *pParent = NULL;

		switch (m_mode)
		{
		case KINE_TARGET_INCENTER:
			h = AddPairNode(this, FALSE);	
			if (FAILED(h)) return h;
			break;

		case KINE_TARGET_ORBITING:
			h = AddPairNode(this, TRUE);	
			if (FAILED(h)) return h;
			break;

		case KINE_TARGET_PEER:
			h = GetParent(&pParent);
			if (FAILED(h)) return h;
			h = AddPairNode(pParent, TRUE);	
			if (FAILED(h)) return h;
			pParent->Release();
			break;

		case KINE_TARGET_REMOTE:
			h = AddPairNode(pRemoteTarget, TRUE);	
			if (FAILED(h)) return h;
			break;
		}
		return S_OK;
	}

	HRESULT Destruct()
	{
		if (m_pNodePair) { m_pNodePair->Delete(); m_pNodePair->Release(); m_pNodePair = NULL; }
		if (m_pNodeUp)   { m_pNodeUp->Delete(); m_pNodeUp->Release();   m_pNodeUp = NULL; }
		return S_OK;
	}

	HRESULT PutNodePos(IKineObj3D *p, FWVECTOR *pV)
	{
		FWVECTOR v = { 0, 0, 0 };
		p->LtoG(&v);
		v.x = pV->x - v.x; v.y = pV->y - v.y; v.z = pV->z - v.z;
		ITransform *pT = NULL;
		HRESULT h = FWDevice()->CreateObject(L"Transform", IID_ITransform, (IFWUnknown**)&pT); if (FAILED(h)) return h;
		pT->FromTranslationVector(&v);
		p->Transform(pT, KINE_BASE);
		pT->Release();
		return S_OK;
	}

	HRESULT GetNodePos(IKineObj3D *p, FWVECTOR *pV)
	{
		if (!pV) return S_OK;
		pV->x = pV->y = pV->z = 0.0f;
		return p->LtoG(pV);
	}
};

///////////////////////////////////////////////////////////
//
// CKineEnumChildren Inline Class Definition

class CKineEnumChildren : public FWUNKNOWN<IKineEnumChildren, IID_IKineEnumChildren, IKineEnumChildren >
{
protected:
	bool m_bAllDesc;
	IKineNode *m_pNode;
	FWULONG m_i;
	IKineEnumChildren *m_pRecEnum;	// enumerator for recursive search
	IID m_iidExcl;

public:
	CKineEnumChildren(IKineNode *p = NULL, bool bAllDesc = FALSE) : m_pNode(p), m_bAllDesc(bAllDesc)
	{
		memset(&m_iidExcl, 0, sizeof(m_iidExcl));
		m_pRecEnum = NULL;
		if (m_pNode) m_pNode->AddRef();
		Reset();
	}

	CKineEnumChildren(IKineNode *p, bool bAllDesc, REFIID iidExcl) : m_pNode(p), m_bAllDesc(true), m_iidExcl(iidExcl)
	{
		m_pRecEnum = NULL;
		if (m_pNode) m_pNode->AddRef();
		Reset();
	}

	~CKineEnumChildren()
	{
		if (m_pNode) m_pNode->Release();
		if (m_pRecEnum) m_pRecEnum->Release();
	}

	virtual HRESULT __stdcall Next(IKineChild **pResult)
	{
		if (!m_pNode) return ERROR(FW_E_POINTER, 0, NULL, NULL, 0);

		HRESULT h = S_OK;
		if (m_pRecEnum)
		{
			h = m_pRecEnum->Next(pResult);
			if (h == S_OK) 
				return h;
			m_pRecEnum->Release();
			m_pRecEnum = NULL;
		}
		
		IKineChild *pChild = NULL;
		FWULONG nCount;
		m_pNode->GetChildCount(&nCount);

		if (m_i < nCount)
			h = m_pNode->GetChildAt(m_i, &pChild);

		IUnknown *pExcl = NULL;
		if (pChild && SUCCEEDED(pChild->QueryInterface(m_iidExcl, (void**)&pExcl)))
		{
			m_i++;
			pExcl->Release();
			pChild->Release();
			return Next(pResult);
		}

		if (pResult) *pResult = pChild;
		else if (pChild) pChild->Release();

		IKineNode *pChildNode = NULL;
		if (m_bAllDesc && pChild && pChild->IsParent(m_pNode) == S_OK && SUCCEEDED(pChild->QueryInterface(&pChildNode)))
		{
			pChildNode->EnumAllDirectDescendants(m_iidExcl, &m_pRecEnum);
			pChildNode->Release();
		}
		
		if (pChild)
			m_i++;

		if (FAILED(h)) return h;
		return (pChild) ? S_OK : S_FALSE;
	}

	virtual HRESULT __stdcall Skip(FWULONG cConnections)
	{
		HRESULT h = S_OK;
		for (FWULONG i = 0; i < cConnections; i++)
			h = Next(NULL);
		return h;
	}

	virtual HRESULT __stdcall Reset()
	{
		m_i = 0;
		return S_OK;
	}

	virtual HRESULT __stdcall Clone(IKineEnumChildren **ppEnum)
	{
		CKineEnumChildren *p = new CKineEnumChildren(m_pNode);
		if (!p) return ERROR(FW_E_OUTOFMEMORY, 0, NULL, NULL, 0);
		FWDevice()->RegisterObject(p);
		p->m_i = m_i;
		*ppEnum = p;
		return S_OK;
	}

	FW_RTTI(KineEnumChildren)
};

#endif  // _KINETEMPL_
