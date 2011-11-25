// FW UNKNOWN: a Mini Library for DLL COM Servers
// Facilitates creating COM DLL server modules without use of ATL
// A special extension for the FreeWill+ system
//
// Provides:
//	- ERROR macrodefinition for reporting run-time errors the easy way
//	- template-based FWUNKNOWN base class with full implementation for IFWUnknown interface
//	- macrodefinitions for automatic implementation of IFWUnknown RTTI functions
//	- macrodefinitions for automatic implementation of IFWUnknown error handling functions
//
// Does not require linking to CPP/LIB files
//
// Copyright (C) 2003-2005 by Jarek Francik
/////////////////////////////////////////////////////////////////////////////

#ifndef __FWUNKNOWN_H
#define __FWUNKNOWN_H

#include "unknown.h"
#include "common.h"
#include <map>
#include <assert.h>

#define DEBUG_SIGNATURE		// allocates FWSTRING ClassId for life-cycle of each object

static const CLSID __CLSID_FWDevice = {0x4DAC894C,0x75C0,0x4538,{0xB8,0x3C,0xE0,0xC9,0x9D,0x70,0x99,0x7B}};

////////////////////////////////////////////////////////////////////////////////////
// The ERROR Macrodefinition
// Use the ERROR macro to report run-time errors
// Automatically provides the information on the source file name & line number

#include <stdio.h>
#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WFILE__ WIDEN(__FILE__)

#undef ERROR
#define ERROR ErrorSrc(__WFILE__, __LINE__), Error


////////////////////////////////////////////////////////////////////////////////////
// Template Base Class: IFWUnknown + IUnknown implementation

template <class BASECLASS,
		  REFIID IID1 = IID_IUnknown, class INTERFACE1 = __DUMMY<1>,
		  REFIID IID3 = IID_IUnknown, class INTERFACE3 = __DUMMY<3>,
		  REFIID IID4 = IID_IUnknown, class INTERFACE4 = __DUMMY<4>,
		  REFIID IID5 = IID_IUnknown, class INTERFACE5 = __DUMMY<5>,
		  REFIID IID6 = IID_IUnknown, class INTERFACE6 = __DUMMY<6>,
		  REFIID IID7 = IID_IUnknown, class INTERFACE7 = __DUMMY<7>,
		  REFIID IID8 = IID_IUnknown, class INTERFACE8 = __DUMMY<8> >
class FWUNKNOWN : public UNKNOWN<BASECLASS, 
									  IID1, INTERFACE1, IID_IFWUnknown, INTERFACE1, IID3, INTERFACE3, IID4, INTERFACE4, 
									  IID5, INTERFACE5, IID6, INTERFACE6, IID7, INTERFACE7, IID8, INTERFACE8>
{
private:
	IFWDevice *m_pFWDevice;
#ifdef DEBUG_SIGNATURE
public:
	FWSTRING m_pDebugSignature;
#endif

public:

	FWUNKNOWN() : m_pFWDevice(NULL) 
	{ 
#ifdef DEBUG_SIGNATURE
		m_pDebugSignature = NULL;
#endif
	}
	virtual ~FWUNKNOWN()	
	{ 
#ifdef DEBUG_SIGNATURE
		delete m_pDebugSignature;
#endif

		// unregister
		if (m_pFWDevice && m_pFWDevice != (IFWUnknown*)(ITFirst*)this) 
		{ 
			m_pFWDevice->UnregisterObject((ITFirst*)this); 
			m_pFWDevice->Release(); 
		} 
	}

	/////////////////////////////////////////
	// Error Handling

	HRESULT _stdcall Error(HRESULT nErrorCode, FWULONG nParams = 0, FWULONG *pParams = NULL, FWSTRING pMessage = NULL, FWULONG nSeverity = FW_SEV_DEFAULT)
	{
		return FWDevice()->RaiseError((ITFirst*)this, nErrorCode, nParams, pParams, pMessage, nSeverity);
	}

	HRESULT _stdcall ErrorSrc(FWSTRING pSrcFile, FWULONG nSrcLine)
	{
		return FWDevice()->RaiseErrorSrc(pSrcFile, nSrcLine);
	}

	HRESULT _stdcall GetClassError(FWULONG nCode, FWULONG nParams, FWULONG *pParams,	FWSTRING *pMsg, FWULONG *pSeverity)
	{
		return FW_E_UNIDENTIFIED;
	}

	/////////////////////////////////////////
	// Cloning

	HRESULT _stdcall GetClone(REFIID iid, IFWUnknown **p)
	{
		return ERROR(E_NOTIMPL);
	}

	IFWUnknown *_stdcall Clone(REFIID iid)
	{
		return NULL;
	}

	///////////////////////////////////////////////
	// Object Initialisation

	HRESULT _stdcall QueryFitness(IFWCreateContext*, /*[out, retval]*/ FWFLOAT *pfFitness)
	{
		if (pfFitness) *pfFitness = 1.0f;
		return S_OK;
	}

	HRESULT _stdcall Create(IFWCreateContext*)
	{
		return S_OK;
	}

	/////////////////////////////////////////
	// Class RT Identification

	HRESULT _stdcall GetClassId(FWSTRING *p)
	{	if (p) *p = L"uninitialised";
		return S_OK;
	}

	HRESULT _stdcall GetRTCId(FWULONG *p)
	{	
		static FWULONG N = (FWULONG)(__int64)&N;
		if (p) *p = N;
		return S_OK;
	}

	/////////////////////////////////////////
	// FWDevice Access

	HRESULT _stdcall GetFWDevice(IFWDevice **pp)
	{
		if (!m_pFWDevice)
			CoCreateInstance(__CLSID_FWDevice, NULL, CLSCTX_INPROC_SERVER, IID_IFWDevice, (void**)&m_pFWDevice);

		if (pp) 
		{
			*pp = m_pFWDevice; 
			if (*pp) (*pp)->AddRef();
		}
		return S_OK;
	}

	IFWDevice *_stdcall FWDevice()
	{
		if (!m_pFWDevice)
			CoCreateInstance(__CLSID_FWDevice, NULL, CLSCTX_INPROC_SERVER, IID_IFWDevice, (void**)&m_pFWDevice);

		return m_pFWDevice;
	}

	HRESULT _stdcall PutFWDevice(IFWDevice *p)
	{
		if (m_pFWDevice) m_pFWDevice->Release();
        m_pFWDevice = p; 
		if (m_pFWDevice) m_pFWDevice->AddRef();
#ifdef DEBUG_SIGNATURE
		FWSTRING pId;
		GetClassId(&pId);
		m_pDebugSignature = wcsdup(pId);
#endif
		return S_OK;
	}

	///////////////////////////////////////////////
	// The Context Object 

	HRESULT _stdcall GetContextObject(FWULONG index, IID *pIID, void **pUnknown)	{ return ERROR(E_NOTIMPL); }
	HRESULT _stdcall PutContextObject(FWULONG index, REFIID iid, void *pUnknown)	{ return ERROR(E_NOTIMPL); }
};

////////////////////////////////////////////////////////////////////////////////////
// Macrodefinitions for IFWUnknown RTTI functions
// Provides implementation for folowing member functions:
// - Clone
// - GetClassId
// - GetRTCId

#define FW_RTTI(classname)										\
	HRESULT _stdcall GetClone(REFIID iid, IFWUnknown **p)		\
	{															\
		if (!p) return ERROR(FW_E_POINTER);						\
		C##classname *pUnknown = new C##classname;				\
		if (!pUnknown) return ERROR(FW_E_OUTOFMEMORY);			\
		FWDevice()->RegisterObject((ITFirst*)pUnknown);			\
		HRESULT h = pUnknown->QueryInterface(iid, (void**)p);	\
		pUnknown->Release();									\
		if (FAILED(h)) return ERROR(FW_E_NOINTERFACE);			\
		return S_OK;											\
	}															\
	IFWUnknown *_stdcall Clone(REFIID iid)						\
	{															\
		C##classname *pUnknown = new C##classname;				\
		if (!pUnknown) return NULL;								\
		FWDevice()->RegisterObject((ITFirst*)pUnknown);			\
		IFWUnknown *p;											\
		HRESULT h = pUnknown->QueryInterface(iid, (void**)&p);	\
		pUnknown->Release();									\
		if (FAILED(h)) return NULL;								\
		return p;												\
	}															\
	HRESULT _stdcall GetClassId(FWSTRING *p)					\
	{	if (p) *p = L#classname;								\
		return S_OK;											\
	}															\
	HRESULT _stdcall GetRTCId(FWULONG *p)						\
	{	static FWULONG N = (FWULONG)(__int64)&N;				\
		if (p) *p = N;											\
		return S_OK;											\
	}

////////////////////////////////////////////////////////////////////////////////////
// Macrodefinitions for IFWUnknown error handling functions
// Provides implementation for folowing member functions:
// - GetClassError

#define FW_ERROR_BEGIN	\
public:																														\
	HRESULT _stdcall GetClassError(FWULONG nnCodeCode, FWULONG nParams, FWULONG *pParams,	FWSTRING *pMsg, FWULONG *pSeverity)	\
	{	FWSTRING p = NULL; FWULONG nSeverity = 0;																				\
		switch(nnCodeCode) {

#define FW_ERROR_ENTRY(code, msg, severity)	\
		case code: p = msg; nSeverity = severity; break;

#define FW_ERROR_DEFAULT	\
		default:

#define FW_ERROR_END	\
		}																					\
		if (p == NULL) return FW_E_UNIDENTIFIED;											\
		FWULONG P[16];																		\
		memset(P, 0, sizeof(P));															\
		if (nParams && pParams) memcpy(P, pParams, sizeof(FWULONG) * max(16, nParams));		\
		static wchar_t pBuf[256];															\
		_snwprintf(pBuf, 256, p, P[0], P[1], P[2], P[3], P[4], P[5], P[6], P[7],			\
								 P[8], P[9], P[10], P[11], P[12], P[13], P[14], P[15]);		\
		if (pMsg) *pMsg = pBuf;																\
		if (pSeverity) *pSeverity = nSeverity;												\
		return S_OK;																		\
	}

#endif