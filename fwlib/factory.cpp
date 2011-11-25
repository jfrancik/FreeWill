// factory.cpp : Defines the universal class factory
//

#include "stdafx.h"
#include "factory.h"
#include "registry.h"
#include <assert.h>

///////////////////////////////////////////////////////////
//
// Global variables

HMODULE g_hModule = NULL;	// DLL handle
long g_cServerLocks = 0;	// LockServer counter

//////////////////////////////////////////////////////////////////////////
//
// CClassFactory

extern "C" class CClassFactory : public IClassFactory
{
public:
	// IUnknown implementation
	virtual HRESULT _stdcall QueryInterface(REFIID iid, void **ppv);
	virtual ULONG _stdcall AddRef();
	virtual ULONG _stdcall Release();

	// IClassFactory implementation
	virtual HRESULT __stdcall CreateInstance(IUnknown *pUnknownOuter,
	                                         const IID& iid,
	                                         void **ppv) ;
	virtual HRESULT __stdcall LockServer(BOOL bLock);

	// Implementation of DLL export functions
	friend HRESULT __stdcall DllCanUnloadNow();
	friend HRESULT __stdcall DllGetClassObject(const CLSID& clsid, const IID& iid, void **ppv);
	friend HRESULT __stdcall DllRegisterServer();
	friend HRESULT __stdcall DllUnregisterServer();

	// Internal Implementation
protected:
	CClassFactory(CLSID clsid);				// to be constructed by CClassFactory::DllGetClassObject only
	FACTORYCLASSENTRY *m_pClassEntry;		// factory class info

private:
	long m_nRef ;
};

///////////////////////////////////////////////////////////
//
// CClassFactory Construction 
// Called exclusively by DllGetClassObject

CClassFactory::CClassFactory(CLSID clsid) : m_nRef(1)
{
	m_pClassEntry = NULL;
	FACTORYCLASSENTRY *p = g_pFactoryMap;
	while (p->pCLSID && *p->pCLSID != clsid)
		p++;
	if (p->pCLSID)
		m_pClassEntry = p;
	assert(m_pClassEntry);
}

///////////////////////////////////////////////////////////
//
// IUnknown implementation

HRESULT __stdcall CClassFactory::QueryInterface(const IID& iid, void **ppv)
{    
	if ((iid == IID_IUnknown) || (iid == IID_IClassFactory))
		*ppv = (IClassFactory*)this;
	else
	{	
		ppv = NULL;
		return E_NOINTERFACE;
	}
	((IUnknown*)(*ppv))->AddRef();
	return S_OK;
}

ULONG __stdcall CClassFactory::AddRef()
{
	return InterlockedIncrement(&m_nRef) ;
}

ULONG __stdcall CClassFactory::Release() 
{
	if (InterlockedDecrement(&m_nRef) == 0)
	{
		delete this ;
		return 0 ;
	}
	return m_nRef ;
}

///////////////////////////////////////////////////////////
//
// IClassFactory implementation

HRESULT __stdcall CClassFactory::CreateInstance(IUnknown *pUnknownOuter, const IID& iid, void **ppv) 
{
	if (pUnknownOuter != NULL)
		return CLASS_E_NOAGGREGATION;

	if (!m_pClassEntry)
		return CLASS_E_CLASSNOTAVAILABLE;

	IUnknown *pUnknown = NULL;
	pUnknown = m_pClassEntry->pCreateFun();

	if (!pUnknown)
		return E_OUTOFMEMORY;

	// Utworzenie ¿¹danego interfejsu
	HRESULT hr = pUnknown->QueryInterface(iid, ppv);

	pUnknown->Release();

	return hr;
}

HRESULT __stdcall CClassFactory::LockServer(BOOL bLock) 
{
	if (bLock)
		InterlockedIncrement(&g_cServerLocks);
	else
		InterlockedDecrement(&g_cServerLocks);
	return S_OK ;
}

///////////////////////////////////////////////////////////
//
// Exported DLL Functions

HRESULT __stdcall DllCanUnloadNow()
{
	if ((g_cComponents == 0) && (g_cServerLocks == 0))
		return S_OK;
	else
		return S_FALSE;
}

HRESULT __stdcall DllGetClassObject(const CLSID& clsid, const IID& iid, void **ppv)
{
	CClassFactory *pFactory = new CClassFactory(clsid);
	if (pFactory == NULL)
		return E_OUTOFMEMORY;
	if (!pFactory->m_pClassEntry)
	{
		delete pFactory;
		return CLASS_E_CLASSNOTAVAILABLE;
	}

	// Zwróæ ¿¹dany interfejs
	HRESULT hr = pFactory->QueryInterface(iid, ppv);
	pFactory->Release();

	return hr;
}

HRESULT __stdcall DllRegisterServer()
{
	if (!g_hModule)
	{
		MessageBox(NULL, L"Server cannot be registered. Call DllMainHelper function on the DLL entry.", L"KINE+", MB_ICONSTOP);
		return E_HANDLE;
	}
	
	HRESULT hRes;
	for (FACTORYCLASSENTRY *p = g_pFactoryMap; p->pCLSID; p++)
	{
		hRes = RegisterServer(g_hModule, *p->pCLSID, p->m_szFriendlyName, p->m_szVerIndProgId, p->m_szProgId);
		if (hRes != ERROR_SUCCESS) return E_ACCESSDENIED;
	}

	for (FREEWILLCLASSENTRY *p = g_pFreeWillMap; p->m_szNoun; p++)
	{
		if (p->m_pCLSID)
			hRes = RegisterFreeWillClass(p->m_szNoun, L"Generic", p->m_szVerb, *p->m_pCLSID);
		else
			hRes = RegisterFreeWillArg(p->m_szNoun, L"Generic", p->m_szVerb, p->m_szArg, p->m_nArgIndex, p->m_nArgType);
		if (hRes != ERROR_SUCCESS) return E_ACCESSDENIED;
	}

	return S_OK;
}

HRESULT __stdcall DllUnregisterServer()
{
	HRESULT hRes;
	for (FACTORYCLASSENTRY *p = g_pFactoryMap; p->pCLSID; p++)
	{
		hRes = UnregisterServer(*p->pCLSID, p->m_szVerIndProgId, p->m_szProgId);
		if (hRes != ERROR_SUCCESS) return E_ACCESSDENIED;
	}
	return S_OK;
}

///////////////////////////////////////////////////////////
//
// DLL Initialization

BOOL DllMainHelper(HANDLE hModule, DWORD ul_reason_for_call)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
		g_hModule = (HMODULE)hModule;
    return TRUE;
}
