//
// Registry.cpp
//

#include "stdafx.h"
#include "registry.h"
#include <unknwn.h>
#include <assert.h>
#include <stdio.h>

////////////////////////////////////////////////////////
//
// Internal helper functions prototypes
//

// Set the given key and its value.
LONG setKeyAndValue(HKEY hKeyRoot, const wchar_t *pwszPath, const wchar_t *wszSubkey, const wchar_t *wszValue);

// Delete wszKeyChild and all of its descendents.
LONG recursiveDeleteKey(HKEY hKeyParent, const wchar_t *wszKeyChild);

/////////////////////////////////////////////////////////
//
// Public function implementation
//

//
// Register the component in the registry.
//
HRESULT RegisterServer(HMODULE hModule,            // DLL module handle
                       const CLSID& clsid,         // Class ID
                       const wchar_t *wszFriendlyName, // Friendly Name
                       const wchar_t *wszVerIndProgID, // Programmatic
                       const wchar_t *wszProgID)       //   IDs
{
	// Get server location.
	wchar_t wszModule[512];
	DWORD dwResult = ::GetModuleFileName(hModule, wszModule, sizeof(wszModule)/sizeof(wchar_t));
	assert(dwResult != 0);

	// Convert the CLSID into a wchar_t.
	wchar_t *wszCLSID;
	HRESULT hr = StringFromCLSID(clsid, &wszCLSID);
	assert(SUCCEEDED(hr));

	// Build the key CLSID\\{...}
	wchar_t wszKey[64];
	wcscpy(wszKey, L"CLSID\\");
	wcscat(wszKey, wszCLSID);

	LONG lRes;
  
	// Add the CLSID to the registry.
	lRes = setKeyAndValue(HKEY_CLASSES_ROOT, wszKey, NULL, wszFriendlyName); if (lRes != ERROR_SUCCESS) return lRes;
	// Add the server filename subkey under the CLSID key.
	lRes = setKeyAndValue(HKEY_CLASSES_ROOT, wszKey, L"InprocServer32", wszModule); if (lRes != ERROR_SUCCESS) return lRes;
	// Add the ProgID subkey under the CLSID key.
	lRes = setKeyAndValue(HKEY_CLASSES_ROOT, wszKey, L"ProgID", wszProgID); if (lRes != ERROR_SUCCESS) return lRes;
	// Add the version-independent ProgID subkey under CLSID key.
	lRes = setKeyAndValue(HKEY_CLASSES_ROOT, wszKey, L"VersionIndependentProgID", wszVerIndProgID); if (lRes != ERROR_SUCCESS) return lRes;

	// Add the version-independent ProgID subkey under HKEY_CLASSES_ROOT.
	lRes = setKeyAndValue(HKEY_CLASSES_ROOT, wszVerIndProgID, NULL, wszFriendlyName); if (lRes != ERROR_SUCCESS) return lRes;
	lRes = setKeyAndValue(HKEY_CLASSES_ROOT, wszVerIndProgID, L"CLSID", wszCLSID); if (lRes != ERROR_SUCCESS) return lRes;
	lRes = setKeyAndValue(HKEY_CLASSES_ROOT, wszVerIndProgID, L"CurVer", wszProgID); if (lRes != ERROR_SUCCESS) return lRes;

	// Add the versioned ProgID subkey under HKEY_CLASSES_ROOT.
	lRes = setKeyAndValue(HKEY_CLASSES_ROOT, wszProgID, NULL, wszFriendlyName); if (lRes != ERROR_SUCCESS) return lRes;
	lRes = setKeyAndValue(HKEY_CLASSES_ROOT, wszProgID, L"CLSID", wszCLSID); if (lRes != ERROR_SUCCESS) return lRes;

	CoTaskMemFree(wszCLSID);

	return S_OK;
}

//
// Remove the component from the registry.
//
LONG UnregisterServer(const CLSID& clsid,         // Class ID
                      const wchar_t *wszVerIndProgID, // Programmatic
                      const wchar_t *wszProgID)       //   IDs
{
	// Convert the CLSID into a wchar_t.
	wchar_t *wszCLSID;
	HRESULT hr = StringFromCLSID(clsid, &wszCLSID);
	assert(SUCCEEDED(hr));

	// Build the key CLSID\\{...}
	wchar_t wszKey[64];
	wcscpy(wszKey, L"CLSID\\");
	wcscat(wszKey, wszCLSID);

	LONG lRes;

	// Delete the CLSID Key - CLSID\{...}
	lRes = recursiveDeleteKey(HKEY_CLASSES_ROOT, wszKey); 
	if (lRes != ERROR_SUCCESS && lRes != ERROR_FILE_NOT_FOUND) return lRes;

	// Delete the version-independent ProgID Key.
	lRes = recursiveDeleteKey(HKEY_CLASSES_ROOT, wszVerIndProgID); 
	if (lRes != ERROR_SUCCESS && lRes != ERROR_FILE_NOT_FOUND) return lRes;

	// Delete the ProgID key.
	lRes = recursiveDeleteKey(HKEY_CLASSES_ROOT, wszProgID);
	if (lRes != ERROR_SUCCESS && lRes != ERROR_FILE_NOT_FOUND) return lRes;

	CoTaskMemFree(wszCLSID);

	return S_OK;
}

//
// Register the FreeWill class in the registry.
//
HRESULT RegisterFreeWillClass(const wchar_t *wszNoun, const wchar_t *wszDefaultVerb, const wchar_t *wszVerb, const CLSID& clsid)
{
	// Convert the CLSID into a wchar_t.
	wchar_t *wszCLSID;
	HRESULT hr = StringFromCLSID(clsid, &wszCLSID);
	assert(SUCCEEDED(hr));

	// Build the key FreeWill\\Nouns\\{...}
	wchar_t wszKey[512];
	wcscpy(wszKey, L"Software\\FreeWill\\Nouns\\");

	LONG lRes;

	lRes = setKeyAndValue(HKEY_LOCAL_MACHINE, wszKey, wszNoun, wszDefaultVerb); if (lRes != ERROR_SUCCESS) return lRes;

	wcscat(wszKey, wszNoun);
	wcscat(wszKey, L"\\");
	wcscat(wszKey, wszVerb);
	lRes = setKeyAndValue(HKEY_LOCAL_MACHINE, wszKey, wszCLSID, L""); if (lRes != ERROR_SUCCESS) return lRes;
	
	CoTaskMemFree(wszCLSID);

	return S_OK;
}

//
// Register the FreeWill arg in the registry.
//
HRESULT RegisterFreeWillArg(const wchar_t *wszNoun, const wchar_t *wszDefaultVerb, const wchar_t *wszVerb, const wchar_t *wszArg, unsigned nArgIndex, unsigned nArgType)
{
	wchar_t wszIndex[6];
	_itow(nArgIndex, wszIndex, 10);
	wchar_t wszArgType[6];
	_itow(nArgType, wszArgType, 10);

	// Build the key FreeWill\\Nouns\\{...}
	wchar_t wszKey[512];
	wcscpy(wszKey, L"Software\\FreeWill\\Nouns\\");

	LONG lRes;

	lRes = setKeyAndValue(HKEY_LOCAL_MACHINE, wszKey, wszNoun, wszDefaultVerb); if (lRes != ERROR_SUCCESS) return lRes;

	wcscat(wszKey, wszNoun);
	wcscat(wszKey, L"\\");
	wcscat(wszKey, wszVerb);
	lRes = setKeyAndValue(HKEY_LOCAL_MACHINE, wszKey, wszArg, wszIndex); if (lRes != ERROR_SUCCESS) return lRes;

	wcscat(wszKey, L"\\");
	wcscat(wszKey, wszArg);


	HKEY hKeyArg;
	lRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, wszKey, 0, KEY_WRITE, &hKeyArg); if (lRes != ERROR_SUCCESS) return lRes;
	lRes = RegSetValueEx(hKeyArg, L"type", 0, REG_SZ, (BYTE*)wszArgType, (DWORD)(wcslen(wszArgType)+1) * sizeof(*wszArgType)); if (lRes != ERROR_SUCCESS) return lRes;
	RegCloseKey(hKeyArg);
	
	return S_OK;
}

///////////////////////////////////////////////////////////
//
// Internal helper functions
//

//
// Delete a key and all of its descendents.
//
LONG recursiveDeleteKey(HKEY hKeyParent,           // Parent of key to delete
                        const wchar_t *lpwszKeyChild)  // Key to delete
{
	// Open the child.
	HKEY hKeyChild;
	LONG lRes = RegOpenKeyEx(hKeyParent, lpwszKeyChild, 0, KEY_ALL_ACCESS, &hKeyChild);
	if (lRes != ERROR_SUCCESS) return lRes;

	// Enumerate all of the decendents of this child.
	FILETIME time;
	wchar_t wszBuffer[256];
	DWORD dwSize = 256;
	while (RegEnumKeyEx(hKeyChild, 0, wszBuffer, &dwSize, NULL, NULL, NULL, &time) == S_OK)
	{
		// Delete the decendents of this child.
		lRes = recursiveDeleteKey(hKeyChild, wszBuffer);
		if (lRes != ERROR_SUCCESS)
		{
			// Cleanup before exiting.
			RegCloseKey(hKeyChild);
			return lRes;
		}
		dwSize = 256;
	}

	// Close the child.
	RegCloseKey(hKeyChild);

	// Delete this child.
	return RegDeleteKey(hKeyParent, lpwszKeyChild);
}

//
// Create a key and set its value.
//   - This helper function was borrowed and modifed from
//     Kraig Brockschmidt's book Inside OLE.
//
LONG setKeyAndValue(HKEY hKeyRoot,
					const wchar_t *wszKey,
                    const wchar_t *wszSubkey,
                    const wchar_t *wszValue)
{
	HKEY hKey;
	wchar_t wszKeyBuf[1024];
	wcscpy(wszKeyBuf, wszKey);
	if (wszSubkey != NULL)
	{
		wcscat(wszKeyBuf, L"\\");
		wcscat(wszKeyBuf, wszSubkey );
	}

	// Create and open key and subkey.
	long lResult = RegCreateKeyEx(hKeyRoot,
	                              wszKeyBuf, 
	                              0, NULL, REG_OPTION_NON_VOLATILE,
	                              KEY_WRITE, NULL, 
	                              &hKey, NULL);
	if (lResult != ERROR_SUCCESS) return lResult;

	// Set the Value.
	if (wszValue != NULL)
	{
		lResult = RegSetValueEx(hKey, NULL, 0, REG_SZ, 
		              (BYTE*)wszValue, 
		              (DWORD)(wcslen(wszValue)+1) * sizeof(*wszValue));
		if (lResult != ERROR_SUCCESS) return lResult;
	}

	RegCloseKey(hKey);
	return S_OK;
}
