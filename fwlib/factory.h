// FACTORY: a Mini Library for DLL COM Servers
// Facilitates creating COM DLL server modules without use of ATL
//
// Provides:
//	- Server DLL export functions and entry-point function
//	- easy-to-use Class Factory implementation (macrodefinition-based)
//
// Requires linking with factory.cpp or appropriate LIB file
//
// Copyright (C) 2003-2005 by Jarek Francik
//
// Extended for exclusive need of FreeWill+ by Jarek Francik in Feb 2006
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(_FACTORY__)
#define _FACTORY__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <objbase.h>

/////////////////////////////////////////////////////////
// Global Variable (defined in UNKNOWN.CPP)
extern long g_cComponents;		// Component counter: global component counter,
								// InterlockedIncrement(&g_cComponents) called in UNKNOWN constructor
								// InterlockedDecrement(&g_cComponents) called in UNKNOWN destructor

/////////////////////////////////////////////////////////
// DLL Export Functions
extern "C" HRESULT __stdcall DllCanUnloadNow();
extern "C" HRESULT __stdcall DllGetClassObject(const CLSID& clsid, const IID& iid, void **ppv);
extern "C" HRESULT __stdcall DllRegisterServer();
extern "C" HRESULT __stdcall DllUnregisterServer();

/////////////////////////////////////////////////////////
// DLL Entry Point Helper Function
BOOL DllMainHelper(HANDLE hModule, DWORD ul_reason_for_call);

/////////////////////////////////////////////////////////
// ClassFactory Map
// Usage:
// - in the header file class definition:
//	 DECLARE_FACTORY_CLASS(Mesh, Mesh)
// - in the cpp file:
//	 BEGIN_FACTORY_MAP
//		FACTORY_CLASS(Mesh, "Mesh Plus Controller", "MeshPlus.Mesh", "MeshPlus.Mesh.1")
//		// TO DO: Use separate entry for every component supported
//	 END_FACTORY_MAP

#define DECLARE_FACTORY_CLASS(class_name, interface_name)	\
	friend IUnknown *Create##class_name() { return (I##interface_name*)new C##class_name; }

#define BEGIN_FACTORY_MAP			FACTORYCLASSENTRY g_pFactoryMap[] = {
#define FACTORY_CLASS(class_name, friendly_name, version_independent_prog_id, prog_id) \
	{ &CLSID_##class_name, friendly_name, version_independent_prog_id, prog_id, Create##class_name},
#define FACTORY_CLASS_EX(class_name, clsid_name, friendly_name, version_independent_prog_id, prog_id) \
	{ &CLSID_##clsid_name, friendly_name, version_independent_prog_id, prog_id, Create##class_name},
#define END_FACTORY_MAP				{ NULL } };

struct FACTORYCLASSENTRY
{
	const CLSID *pCLSID;
	const wchar_t *m_szFriendlyName;
	const wchar_t *m_szVerIndProgId;
	const wchar_t *m_szProgId;
	IUnknown *(*pCreateFun)();
};
extern FACTORYCLASSENTRY g_pFactoryMap[];

/////////////////////////////////////////////////////////
// FreeWill+ Object Map
// Usage:
// - in the cpp file:
//	 BEGIN_FREEWILL_MAP
//		FREEWILL_CLASS("KineBone", "Generic", KineBone)
//		FREEWILL_CLASS("Action", "Rotate", ActionRotate)
//		FREEWILL_ARG("Action", "Rotate", "Rotation", 1)
//		// TO DO: Use separate entry for every component supported
//	 END_FREEWILL_MAP

#define BEGIN_FREEWILL_MAP			FREEWILLCLASSENTRY g_pFreeWillMap[] = {
#define FREEWILL_CLASS(noun, verb, class_name) \
	{ noun, verb, &CLSID_##class_name, NULL, 0, 0 },
#define FREEWILL_ARG(noun, verb, arg_name, arg_index, arg_type) \
	{ noun, verb, NULL, arg_name, arg_index, (FWULONG)(arg_type) },
#define END_FREEWILL_MAP				{ NULL } };

struct FREEWILLCLASSENTRY
{
	const wchar_t *m_szNoun;
	const wchar_t *m_szVerb;
	const CLSID *m_pCLSID;
	const wchar_t *m_szArg;
	unsigned m_nArgIndex;
	unsigned m_nArgType;
};
extern FREEWILLCLASSENTRY g_pFreeWillMap[];

#endif
