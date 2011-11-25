#ifndef __Registry_H__
#define __Registry_H__

//
// Registry.h
//   - Helper functions registering and unregistering a component.
//

// This function will register a component in the Registry.
// The component calls this function from its DllRegisterServer function.
HRESULT RegisterServer(HMODULE hModule, 
                       const CLSID& clsid, 
                       const wchar_t *szFriendlyName,
                       const wchar_t *szVerIndProgID,
                       const wchar_t *szProgID) ;

// This function will unregister a component.  Components
// call this function from their DllUnregisterServer function.
HRESULT UnregisterServer(const CLSID& clsid,
                         const wchar_t *szVerIndProgID,
                         const wchar_t *szProgID) ;

// This function will register a FreeWill+ class in the Registry.
// The component calls this function from its DllRegisterServer function.
HRESULT RegisterFreeWillClass(const wchar_t *szNoun, const wchar_t *szDefaultVerb, const wchar_t *szVerb, const CLSID& clsid);

// This function will register a FreeWill+ argument in the Registry.
// The component calls this function from its DllRegisterServer function.
HRESULT RegisterFreeWillArg(const wchar_t *szNoun, const wchar_t *szDefaultVerb, const wchar_t *szVerb, const wchar_t *szArg, unsigned nArgIndex, unsigned nArgType);

#endif