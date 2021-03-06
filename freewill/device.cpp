// device.cpp : Defines IFWDevice Main System Device
//

#include "stdafx.h"
#include "device.h"

///////////////////////////////////////////////////////////
// Construction / Destruction

IFWDevice *CFWDevice::c_pSingleInstance = NULL;

CFWDevice::CFWDevice()
{
	PutFWDevice(this); Release();
	c_pSingleInstance = this;
	m_pLastError = m_pFirstError = NULL;
	m_nExceptionEnabled = 0;
	m_pUserErrorHandleF = NULL;
	m_nErrCntSevRec = m_nErrCntSevAll = m_nErrCntAllRec = m_nErrCntAllAll = 0;
	PutSevereErrorLevel(FW_SEV_SERIOUS);
	nRegObjects = 0;
	Load();
}

CFWDevice::~CFWDevice()
{
	c_pSingleInstance = NULL;
	m_nRef = 0x7fffffff;	// factories contain weak pointers to this and will try to release them!

	TRACE(L"Deleting object factories:\n");
	for each (_NOUN *pNoun in m_vecNouns)
	{
		std::wstring strNoun = pNoun->theNoun;
		if (pNoun->theNoun)		
			free(pNoun->theNoun);
		if (pNoun->theGeneric)	
			free(pNoun->theGeneric);
		for each (_VERB *pVerb in pNoun->vecVerbs)
		{
			std::wstring strVerb = pVerb->theVerb;
			if (pVerb->theVerb)
				free(pVerb->theVerb);
			for each (_CLASS *pClass in pVerb->vecClasses)
			{
				std::wstring strClass = pClass->theClass;
				if (pClass->theClass) 
					free(pClass->theClass);
				if (pClass->pFactory)
				{
					TRACE(L"Deleting factory: %ws: %ws (%ws)\n", strNoun.c_str(), strVerb.c_str(), strClass.c_str());
					pClass->pFactory->Release();
				}
				delete pClass;
			}
			//for each (_ARG *pArg in pVerb->vecArgs)
			//{
			//	if (p->theArg) free(p->theArg);
			//	delete p;
			//}
			delete pVerb;
		}
		delete pNoun;
	}

	TRACE(L"Destructing FreeWill+ Device. Count of registered objects: %d\n", nRegObjects);
	TRACE(L"Count of the logged errors: %d in which severe errors: %d\n", m_nErrCntAllAll, m_nErrCntSevAll);
	for (FWERROR *p = m_pFirstError; p; p = p->pNext)
		TRACE(L"%ls(%d): Error 0x%x (class %ls), %ls\n", p->pSrcFile, p->nSrcLine, p->nCode & 0xffff, p->pClassName, p->pMessage);

	while (m_pLastError)
	{
		FWERROR *p = m_pLastError;
		m_pLastError = p->pPrev;
		if (p->pMessage) free(p->pMessage);
		delete p;
	}
	TRACE(L"FreeWill Device deleted and unloaded\n");
}

////////////////////////////////////////////////////////////////////
// Object Repository

HRESULT __stdcall CFWDevice::CreateObject(FWSTRING theNoun, REFIID iid, /*[out, retval, iid_is]*/ IFWUnknown **p)
{
	assert(p);
	FWULONG idNoun = 0;

	// find idNoun
	MAP::iterator i = m_mapNouns.find(theNoun);
	if (i == m_mapNouns.end())
		return ERROR(FW_E_UNKNOWN_NOUN, 1, (FWULONG*)&theNoun);
	idNoun = i->second;

	// locate the Noun
	if (idNoun > m_vecNouns.size())
		return ERROR(FW_E_UNKNOWN_NOUN_ID, 1, &idNoun);
	_NOUN *pNoun = m_vecNouns[idNoun];

	// Settle pGeneric in accordance to theGeneric - if not done yet...
	if (!pNoun->pGeneric)
	{
		MAP::iterator i = pNoun->mapVerbs.find(pNoun->theGeneric);
		if (i == pNoun->mapVerbs.end())
			return ERROR(FW_E_UNKNOWN_VERB, 1, (FWULONG*)&pNoun->theGeneric);
		FWULONG idGeneric = i->second;
		if (idGeneric > pNoun->vecVerbs.size())
			return ERROR(FW_E_UNKNOWN_VERB_ID, 1, &idGeneric);
		pNoun->pGeneric = pNoun->vecVerbs[idGeneric];
	}
	
	// go on with a IFWCreateContext instance
	CFWCreateContext ctx(this, pNoun, pNoun->pGeneric, 0, NULL);
	RegisterObject(&ctx);
	return ctx.Create(iid, p);
}

HRESULT __stdcall CFWDevice::CreateObjectEx(FWSTRING theNoun, FWSTRING theVerb, 
						FWULONG nParams, FWPARAM *pParams, 
						REFIID iid, /*[out, retval, iid_is]*/ IFWUnknown **p)
{
	assert(p);
	FWULONG idNoun = 0;
	FWULONG idVerb = 0;

	// find idNoun
	MAP::iterator i = m_mapNouns.find(theNoun);
	if (i == m_mapNouns.end())
		return ERROR(FW_E_UNKNOWN_NOUN, 1, (FWULONG*)&theNoun);
	idNoun = i->second;

	// locate the Noun
	if (idNoun > m_vecNouns.size())
		return ERROR(FW_E_UNKNOWN_NOUN_ID, 1, &idNoun);
	_NOUN *pNoun = m_vecNouns[idNoun];
	
	// find idVerb
	MAP::iterator j = pNoun->mapVerbs.find(theVerb);
	if (j == pNoun->mapVerbs.end())
		return ERROR(FW_E_UNKNOWN_VERB, 1, (FWULONG*)&theVerb);
	idVerb = j->second;

	// locate the Verb
	if (idVerb > pNoun->vecVerbs.size())
		return ERROR(FW_E_UNKNOWN_VERB_ID, 1, &idVerb);
	_VERB *pVerb = pNoun->vecVerbs[idVerb];
	
	// go on with a IFWCreateContext instance
	CFWCreateContext ctx(this, pNoun, pVerb, nParams, pParams);
	RegisterObject(&ctx);
	return ctx.Create(iid, p);
}

HRESULT __stdcall CFWDevice::CreateUnknown(FWSTRING theNoun, /*[out, retval]*/ IFWUnknown **p)
{
	return CreateObject(theNoun, IID_IFWUnknown, p);
}

HRESULT __stdcall CFWDevice::CreateUnknownEx(FWSTRING theNoun, FWSTRING theVerb, 
						FWULONG nParams, FWPARAM *pParams, 
						/*[out, retval]*/ IFWUnknown **p)
{
	return CreateObjectEx(theNoun, theVerb, nParams, pParams, IID_IFWUnknown, p);
}

HRESULT __stdcall CFWDevice::CreateContext(FWSTRING theNoun, FWULONG _idNoun, FWSTRING theVerb, FWULONG _idVerb, 
						FWULONG nParams, FWPARAM *pParams, 
						/*[out, retval]*/ IFWCreateContext **ppContext)
{
	assert(ppContext);
	FWULONG idNoun = _idNoun;
	FWULONG idVerb = _idVerb;

	// find idNoun
	if (theNoun)
	{
		MAP::iterator i = m_mapNouns.find(theNoun);
		if (i == m_mapNouns.end())
			return ERROR(FW_E_UNKNOWN_NOUN, 1, (FWULONG*)&theNoun);
		idNoun = i->second;
	}

	// locate the Noun
	if (idNoun > m_vecNouns.size())
		return ERROR(FW_E_UNKNOWN_NOUN_ID, 1, &idNoun);
	_NOUN *pNoun = m_vecNouns[idNoun];
	
	// find idVerb
	if (theVerb)
	{
		MAP::iterator j = pNoun->mapVerbs.find(theVerb);
		if (j == pNoun->mapVerbs.end())
			return ERROR(FW_E_UNKNOWN_VERB, 1, (FWULONG*)&theVerb);
		idVerb = j->second;
	}

	// locate the Verb
	if (idVerb > pNoun->vecVerbs.size())
		return ERROR(FW_E_UNKNOWN_VERB_ID, 1, &idVerb);
	_VERB *pVerb = pNoun->vecVerbs[idVerb];
	
	// Make IFWCreateContext instance
	*ppContext = new CFWCreateContext(this, pNoun, pVerb, nParams, pParams);
	RegisterObject(*ppContext);
	return S_OK;
}

////////////////////////////////////////////////////////////////////
// FreeWill+ Device Configuration

HRESULT __stdcall CFWDevice::GetNounCount(/*[out, retval]*/ FWULONG *pnCnt)
{
	assert(pnCnt);
	*pnCnt = (FWULONG)m_vecNouns.size();
	return S_OK;
}

HRESULT __stdcall CFWDevice::NounToId(FWSTRING theNoun, /*[out, retval]*/ FWULONG *pnId)
{
	assert(pnId);
	MAP::iterator i = m_mapNouns.find(theNoun);
	if (i == m_mapNouns.end())
		return ERROR(FW_E_UNKNOWN_NOUN, 1, (FWULONG*)&theNoun);
	else
		*pnId = i->second;
	return S_OK;
}

HRESULT __stdcall CFWDevice::IdToNoun(FWULONG idNoun, /*[out, retval]*/ FWSTRING *pNoun)
{
	assert(pNoun);
	if (idNoun > m_vecNouns.size())
		return ERROR(FW_E_UNKNOWN_NOUN_ID, 1, &idNoun);
	*pNoun = m_vecNouns[idNoun]->theNoun;
	return S_OK;
}

HRESULT __stdcall CFWDevice::AddNoun(FWSTRING theNoun, FWSTRING theGenericVerb, /*[out, retval]*/ FWULONG *pnId)
{
	MAP::iterator i = m_mapNouns.find(theNoun);
	if (i != m_mapNouns.end())
	{
		// name already exists
		if (pnId) *pnId = i->second;
		return S_FALSE;
	}
	_NOUN *pNoun = new _NOUN;
	pNoun->theNoun = _wcsdup(theNoun);
	pNoun->theGeneric = _wcsdup(theGenericVerb);
	pNoun->pGeneric = NULL;

	m_vecNouns.push_back(pNoun);
	pNoun->idNoun = (FWULONG)m_vecNouns.size() - 1;
	if (pnId) *pnId = pNoun->idNoun;
	m_mapNouns[pNoun->theNoun] = pNoun->idNoun;
	return S_OK;
}

HRESULT __stdcall CFWDevice::GetVerbCount(FWULONG idNoun, /*[out, retval]*/ FWULONG *pnCnt)
{
	assert(pnCnt);
	if (idNoun > m_vecNouns.size())
		return ERROR(FW_E_UNKNOWN_NOUN_ID, 1, &idNoun);
	*pnCnt = (FWULONG)m_vecNouns[idNoun]->vecVerbs.size();
	return S_OK;
}

HRESULT __stdcall CFWDevice::VerbToId(FWULONG idNoun, FWSTRING theVerb, /*[out, retval]*/ FWULONG *pnId)
{
	assert(pnId);
	if (idNoun > m_vecNouns.size())
		return ERROR(FW_E_UNKNOWN_NOUN_ID, 1, &idNoun);
	_NOUN *pNoun = m_vecNouns[idNoun];
	MAP::iterator i = pNoun->mapVerbs.find(theVerb);
	if (i == pNoun->mapVerbs.end())
		return ERROR(FW_E_UNKNOWN_VERB, 1, (FWULONG*)&theVerb);
	else
		*pnId = i->second;
	return S_OK;
}

HRESULT __stdcall CFWDevice::IdToVerb(FWULONG idNoun, FWULONG idVerb, /*[out, retval]*/ FWSTRING *pVerb)
{
	assert(pVerb);
	if (idNoun > m_vecNouns.size())
		return ERROR(FW_E_UNKNOWN_NOUN_ID, 1, &idNoun);
	if (idVerb > m_vecNouns[idNoun]->vecVerbs.size())
		return ERROR(FW_E_UNKNOWN_VERB_ID, 1, &idVerb);
	*pVerb = m_vecNouns[idNoun]->vecVerbs[idVerb]->theVerb;
	return S_OK;
}

HRESULT __stdcall CFWDevice::AddVerb(FWULONG idNoun, FWSTRING theVerb, /*[out, retval]*/ FWULONG *pnId)
{
	if (idNoun > m_vecNouns.size())
		return ERROR(FW_E_UNKNOWN_NOUN_ID, 1, &idNoun);
	_NOUN *pNoun = m_vecNouns[idNoun];
	MAP::iterator i = pNoun->mapVerbs.find(theVerb);
	if (i != pNoun->mapVerbs.end())
	{
		// name already exists
		if (pnId) *pnId = i->second;
		return S_FALSE;
	}
	_VERB *pVerb = new _VERB;
	pVerb->theVerb = _wcsdup(theVerb);

	pNoun->vecVerbs.push_back(pVerb);
	pVerb->idVerb = (FWULONG)pNoun->vecVerbs.size() - 1;
	if (pnId) *pnId = pVerb->idVerb;
	pNoun->mapVerbs[pVerb->theVerb] = pVerb->idVerb;
	return S_OK;
}

HRESULT __stdcall CFWDevice::GetClassCount(FWULONG idNoun, FWULONG idVerb, /*[out, retval]*/ FWULONG *pnCnt)
{
	assert(pnCnt);
	if (idNoun > m_vecNouns.size())
		return ERROR(FW_E_UNKNOWN_NOUN_ID, 1, &idNoun);
	if (idVerb > m_vecNouns[idNoun]->vecVerbs.size())
		return ERROR(FW_E_UNKNOWN_VERB_ID, 1, &idVerb);
	*pnCnt = (FWULONG)m_vecNouns[idNoun]->vecVerbs[idVerb]->vecClasses.size();
	return S_OK;
}

HRESULT __stdcall CFWDevice::ClassToId(FWULONG idNoun, FWULONG idVerb, FWSTRING theClass, /*[out, retval]*/ FWULONG *pnId)
{
	assert(pnId);
	if (idNoun > m_vecNouns.size())
		return ERROR(FW_E_UNKNOWN_NOUN_ID, 1, &idNoun);
	if (idVerb > m_vecNouns[idNoun]->vecVerbs.size())
		return ERROR(FW_E_UNKNOWN_VERB_ID, 1, &idVerb);
	_VERB *pVerb = m_vecNouns[idNoun]->vecVerbs[idVerb];
	MAP::iterator i = pVerb->mapClasses.find(theClass);
	if (i == pVerb->mapClasses.end())
		return ERROR(FW_E_UNKNOWN_CLASS, 1, (FWULONG*)&theClass);
	else
		*pnId = i->second;
	return S_OK;
}

HRESULT __stdcall CFWDevice::IdToClass(FWULONG idNoun, FWULONG idVerb, FWULONG idClass, /*[out, retval]*/ FWSTRING *pClass)
{
	assert(pClass);
	if (idNoun > m_vecNouns.size())
		return ERROR(FW_E_UNKNOWN_NOUN_ID, 1, &idNoun);
	if (idVerb > m_vecNouns[idNoun]->vecVerbs.size())
		return ERROR(FW_E_UNKNOWN_VERB_ID, 1, &idVerb);
	if (idClass > m_vecNouns[idNoun]->vecVerbs[idVerb]->vecClasses.size())
		return ERROR(FW_E_UNKNOWN_CLASS_ID, 1, &idClass);
	*pClass = m_vecNouns[idNoun]->vecVerbs[idVerb]->vecClasses[idClass]->theClass;
	return S_OK;
}

HRESULT __stdcall CFWDevice::AddClass(FWULONG idNoun, FWULONG idVerb, FWSTRING theClass, /*[out, retval]*/ FWULONG *pnId)
{
	if (idNoun > m_vecNouns.size())
		return ERROR(FW_E_UNKNOWN_NOUN_ID, 1, &idNoun);
	if (idVerb > m_vecNouns[idNoun]->vecVerbs.size())
		return ERROR(FW_E_UNKNOWN_VERB_ID, 1, &idVerb);
	_VERB *pVerb = m_vecNouns[idNoun]->vecVerbs[idVerb];
	MAP::iterator i = pVerb->mapClasses.find(theClass);
	if (i != pVerb->mapClasses.end())
	{
		// name already exists
		if (pnId) *pnId = i->second;
		return S_FALSE;
	}
	_CLASS *pClass = new _CLASS;
	pClass->theClass = _wcsdup(theClass);
	pClass->pFactory = NULL;

	pVerb->vecClasses.push_back(pClass);
	pClass->idClass = (FWULONG)pVerb->vecClasses.size() - 1;
	if (pnId) *pnId = pClass->idClass;
	pVerb->mapClasses[pClass->theClass] = pClass->idClass;
	return S_OK;
}

HRESULT __stdcall CFWDevice::AddInstantClass(FWULONG idNoun, FWULONG idVerb, IFWUnknown *pInstant, /*[out, retval]*/ FWULONG *pnId)
{
	if (idNoun > m_vecNouns.size())
		return ERROR(FW_E_UNKNOWN_NOUN_ID, 1, &idNoun);
	if (idVerb > m_vecNouns[idNoun]->vecVerbs.size())
		return ERROR(FW_E_UNKNOWN_VERB_ID, 1, &idVerb);
	_VERB *pVerb = m_vecNouns[idNoun]->vecVerbs[idVerb];

	_CLASS *pClass = new _CLASS;
	pClass->theClass = NULL;
	pClass->pFactory = pInstant;
	if (pClass->pFactory) pClass->pFactory->AddRef();

	pVerb->vecClasses.push_back(pClass);
	pClass->idClass = (FWULONG)pVerb->vecClasses.size() - 1;
	if (pnId) *pnId = pClass->idClass;
	//pVerb->mapClasses[pClass->theClass] = pClass->idClass;	--- instant classes are not mapped
	return S_OK;
}

/*
OBSOLETE FUNCTIONS
HRESULT __stdcall CFWDevice::GetArgCount(FWULONG idNoun, FWULONG idVerb, /*[out, retval]* / FWULONG *pnCnt)
{
	assert(pnCnt);
	if (idNoun > m_vecNouns.size())
		return ERROR(FW_E_UNKNOWN_NOUN_ID, 1, &idNoun);
	if (idVerb > m_vecNouns[idNoun]->vecVerbs.size())
		return ERROR(FW_E_UNKNOWN_VERB_ID, 1, &idVerb);
	*pnCnt = (FWULONG)m_vecNouns[idNoun]->vecVerbs[idVerb]->vecArgs.size();
	return S_OK;
}

HRESULT __stdcall CFWDevice::ArgToId(FWULONG idNoun, FWULONG idVerb, FWSTRING theArg, /*[out, retval]* / FWULONG *pnId)
{
	assert(pnId);
	if (idNoun > m_vecNouns.size())
		return ERROR(FW_E_UNKNOWN_NOUN_ID, 1, &idNoun);
	if (idVerb > m_vecNouns[idNoun]->vecVerbs.size())
		return ERROR(FW_E_UNKNOWN_VERB_ID, 1, &idVerb);
	_VERB *pVerb = m_vecNouns[idNoun]->vecVerbs[idVerb];
	MAP::iterator i = pVerb->mapArgs.find(theArg);
	if (i == pVerb->mapArgs.end())
		return ERROR(FW_E_UNKNOWN_ARG, 1, (FWULONG*)&theArg);
	else
		*pnId = i->second;
	return S_OK;
}

HRESULT __stdcall CFWDevice::IdToArg(FWULONG idNoun, FWULONG idVerb, FWULONG idArg, /*[out, retval]* / FWSTRING *pArg)
{
	assert(pArg);
	if (idNoun > m_vecNouns.size())
		return ERROR(FW_E_UNKNOWN_NOUN_ID, 1, &idNoun);
	if (idVerb > m_vecNouns[idNoun]->vecVerbs.size())
		return ERROR(FW_E_UNKNOWN_VERB_ID, 1, &idVerb);
	if (idArg > m_vecNouns[idNoun]->vecVerbs[idVerb]->vecArgs.size())
		return ERROR(FW_E_UNKNOWN_ARG_ID, 1, &idArg);
	*pArg = m_vecNouns[idNoun]->vecVerbs[idVerb]->vecArgs[idArg]->theArg;
	return S_OK;
}

HRESULT __stdcall CFWDevice::AddArg(FWULONG idNoun, FWULONG idVerb, 
			FWSTRING theArg, FWULONG nIndex, enum ARGTYPES type, /*[out, retval]* / FWULONG *pnId)
{
	if (idNoun > m_vecNouns.size())
		return ERROR(FW_E_UNKNOWN_NOUN_ID, 1, &idNoun);
	if (idVerb > m_vecNouns[idNoun]->vecVerbs.size())
		return ERROR(FW_E_UNKNOWN_VERB_ID, 1, &idVerb);
	_VERB *pVerb = m_vecNouns[idNoun]->vecVerbs[idVerb];
	MAP::iterator i = pVerb->mapArgs.find(theArg);
	if (i != pVerb->mapArgs.end())
	{
		// name already exists
		if (pnId) *pnId = i->second;
		return S_FALSE;
	}
	_ARG *pArg = new _ARG;
	pArg->theArg = _wcsdup(theArg);
	pArg->nIndex = nIndex;
	pArg->type = type;

	pVerb->vecArgs.push_back(pArg);
	pArg->idArg = (FWULONG)pVerb->vecArgs.size() - 1;
	if (pnId) *pnId = pArg->idArg;
	pVerb->mapArgs[pArg->theArg] = pArg->idArg;
	return S_OK;
}
*/

HRESULT __stdcall CFWDevice::Load()
{
	// Open the FreeWill Classes root HKEY
//	USES_CONVERSION;	// not needed in UNICODE compile version
	HKEY hKeyMain;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\FreeWill\\Nouns", 0, KEY_READ, &hKeyMain) == ERROR_SUCCESS)
	{
		// Enumerate Nouns
		FILETIME time;
		wchar_t szNoun[MAX_PATH];
		DWORD dwNoun = MAX_PATH;
		DWORD iNoun = 0;
		while (RegEnumKeyEx(hKeyMain, iNoun++, szNoun, &dwNoun, NULL, NULL, NULL, &time) == ERROR_SUCCESS)
		{
			// Open the Noun HKEY
			HKEY hKeyNoun;
			wchar_t szNounKey[MAX_PATH];
			wcscpy(szNounKey, L"Software\\FreeWill\\Nouns\\");
			wcscat(szNounKey, szNoun);
			if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szNounKey, 0, KEY_READ, &hKeyNoun) == ERROR_SUCCESS)
			{
				// Get the Noun Value (the Defz
				wchar_t szNounVal[MAX_PATH];
				DWORD dwNounVal = MAX_PATH;
				if (!RegQueryValueEx(hKeyNoun, NULL, NULL, NULL, (LPBYTE)szNounVal, &dwNounVal) == ERROR_SUCCESS)
					wcscpy(szNounVal, L"Generic");

				// Add the Noun
				FWULONG idNoun;
				AddNoun(/*A2OLE*/(szNoun), /*A2OLE*/(szNounVal), &idNoun);

				// Enumerate Verbs
				wchar_t szVerb[MAX_PATH];
				DWORD dwVerb = MAX_PATH;
				DWORD iVerb = 0;
				while (RegEnumKeyEx(hKeyNoun, iVerb++, szVerb, &dwVerb, NULL, NULL, NULL, &time) == ERROR_SUCCESS)
				{
					// Add the Verb
					FWULONG idVerb;
					AddVerb(idNoun, /*A2OLE*/(szVerb), &idVerb);

					// Open the Verb HKEY
					HKEY hKeyVerb;
					wchar_t szVerbKey[MAX_PATH];
					wcscpy(szVerbKey, L"Software\\FreeWill\\Nouns\\");
					wcscat(szVerbKey, szNoun);
					wcscat(szVerbKey, L"\\");
					wcscat(szVerbKey, szVerb);
					if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szVerbKey, 0, KEY_READ, &hKeyVerb) == ERROR_SUCCESS)
					{
						// Enumarate CLSID's
						wchar_t szClsid[MAX_PATH];
						DWORD dwClsid = MAX_PATH;
						DWORD iClsid = 0;
						while (RegEnumKeyEx(hKeyVerb, iClsid++, szClsid, &dwClsid, NULL, NULL, NULL, &time) == ERROR_SUCCESS)
						{
							// Register Verb & Clsid
							if (szClsid[0] == '{')
								// register a class
								AddClass(idNoun, idVerb, /*A2OLE*/(szClsid), NULL);
							// OBSOLETE - registering the arg
							//else
							//{
							//	// register the arg
							//	HKEY hKeyArg;
							//	wchar_t szArgKey[MAX_PATH];
							//	wcscpy(szArgKey, L"Software\\FreeWill\\Nouns\\");
							//	wcscat(szArgKey, szNoun);
							//	wcscat(szArgKey, L"\\");
							//	wcscat(szArgKey, szVerb);
							//	wcscat(szArgKey, L"\\");
							//	wcscat(szArgKey, szClsid);
							//	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szArgKey, 0, KEY_READ, &hKeyArg) == ERROR_SUCCESS)
							//	{
							//		FWULONG nIndex;
							//		enum ARGTYPES type;
							//		wchar_t szArgVal[MAX_PATH];
							//		DWORD dwArgVal = MAX_PATH;
							//		if (!RegQueryValueEx(hKeyArg, NULL, NULL, NULL, (LPBYTE)szArgVal, &dwArgVal) == ERROR_SUCCESS)
							//			wcscpy(szArgVal, L"0");
							//		nIndex = atoi(szArgVal);
							//		dwArgVal = MAX_PATH;
							//		if (!RegQueryValueEx(hKeyArg, L"type", NULL, NULL, (LPBYTE)szArgVal, &dwArgVal) == ERROR_SUCCESS)
							//			wcscpy(szArgVal, L"0");
							//		type = (enum ARGTYPES)atoi(szArgVal);
							//		AddArg(idNoun, idVerb, /*A2OLE*/(szClsid), nIndex, type, NULL);
							//		// Close the ARB HKEY
							//		RegCloseKey(hKeyArg);
							//	}
							//}
							dwClsid = MAX_PATH;
						}
						// Close the VERB HKEY
						RegCloseKey(hKeyVerb);
					}
					dwVerb = MAX_PATH;
				}
				// Close the Noun HKEY
				RegCloseKey(hKeyNoun);
			}
			dwNoun = MAX_PATH;
		}

		// Close the Root HKEY.
		RegCloseKey(hKeyMain);
	}
	return S_OK;
}

HRESULT __stdcall CFWDevice::Store()
{
	return ERROR(E_NOTIMPL);
}

////////////////////////////////////////////////////////////////////
// Object Registration and Tracking

HRESULT __stdcall CFWDevice::RegisterObject(IFWUnknown *p)
{
	p->PutFWDevice(this);
	nRegObjects++;
	return S_OK;
}

HRESULT __stdcall CFWDevice::UnregisterObject(IFWUnknown *p)
{
	nRegObjects--;
	return S_OK;
}

HRESULT __stdcall CFWDevice::GetRegisteredObjectsCount(FWULONG *p)
{
	if (p) *p = nRegObjects;
	return S_OK;
}

///////////////////////////////////////////////////////////
// Error Handling
	
HRESULT __stdcall CFWDevice::GetStatus()
{
	return m_nErrCntSevRec ? E_FAIL : S_OK;
}

HRESULT __stdcall CFWDevice::GetLastError(/*[out, retval]*/ FWERROR **p)
{
	m_nErrCntSevRec = m_nErrCntAllRec = 0;
	if (!m_pLastError)
		return S_OK;
    if (p) *p = m_pLastError;
	return m_pLastError->nCode;
}

HRESULT __stdcall CFWDevice::GetErrorNum(BOOL bSevereErrorsOnly, BOOL bRecentErrorsOnly, FWULONG *p)
{
	if (p)
		*p = bSevereErrorsOnly ?
		(bRecentErrorsOnly ? m_nErrCntSevRec : m_nErrCntSevAll) :
		(bRecentErrorsOnly ? m_nErrCntAllRec : m_nErrCntAllAll);
	return S_OK;
}

HRESULT __stdcall CFWDevice::EnableErrorException(BOOL b)
{
	if (b) m_nExceptionEnabled++;
	else if (m_nExceptionEnabled) m_nExceptionEnabled--;
	return S_OK;
}

HRESULT __stdcall CFWDevice::IsErrorExceptionEnabled()
{
	return m_nExceptionEnabled ? S_OK : S_FALSE;
}

HRESULT __stdcall CFWDevice::SetUserErrorHandler(HRESULT(__stdcall *p)(struct FWERROR*, BOOL bRaised))
{
	m_pUserErrorHandleF = p;
	return S_OK;
}

HRESULT __stdcall CFWDevice::GetUserErrorHandler(HRESULT(__stdcall **p)(struct FWERROR*, BOOL bRaised))
{
	if (p) *p = m_pUserErrorHandleF;
	return S_OK;
}

HRESULT __stdcall CFWDevice::RaiseError(IFWUnknown *pSender, FWULONG nCode, FWULONG nParams, FWULONG *pParams, FWSTRING pMessage, FWULONG nSeverity)
{
	if (HRESULT_FACILITY(nCode) != FACILITY_ITF && SUCCEEDED(nCode)) return S_OK;	// no support for system-warnings

	FWERROR *p = new FWERROR;
	memset(p, 0, sizeof(FWERROR));
	p->pPrev = m_pLastError;
	if (m_pLastError) m_pLastError->pNext = p;
	if (!m_pFirstError) m_pFirstError = p;
	m_pLastError = p;
	
	if (HRESULT_FACILITY(nCode) != FACILITY_ITF)
	{
		// Win COM system error
		FWSTRING pBuffer;
		if (::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, nCode, 
							MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT), (LPWSTR)&pBuffer, 0, NULL) == 0)
		{
			m_pLastError->pMessage = _wcsdup(L"Unidentified system error");
			m_pLastError->nSeverity = FW_SEV_CRITICAL;
		}
		else
		{
			m_pLastError->pMessage = _wcsdup(pBuffer);
			m_pLastError->nSeverity = FW_SEV_CRITICAL;
			LocalFree(pBuffer);
		}
	}
	else
	{
		// FreeWill+ error code (common or class-specific)
		FWSTRING pMsg;
		if (FAILED(GetClassError(nCode, nParams, pParams, &pMsg, &m_pLastError->nSeverity))
		&& FAILED(pSender->GetClassError(nCode, nParams, pParams, &pMsg, &m_pLastError->nSeverity)))
			GetClassError(FW_E_UNIDENTIFIED, 0, NULL, &pMsg, &m_pLastError->nSeverity);
	
		if (pMessage) pMsg = pMessage;
		m_pLastError->pMessage = _wcsdup(pMsg);

		if (nSeverity & FW_SEV_SET_ATLEAST) m_pLastError->nSeverity = max(m_pLastError->nSeverity, nSeverity & FW_SEV_MASK);
		else if (nSeverity & FW_SEV_SET_ATMOST) m_pLastError->nSeverity = min(m_pLastError->nSeverity, nSeverity & FW_SEV_MASK);
		else if (nSeverity != FW_SEV_DEFAULT) m_pLastError->nSeverity = nSeverity & FW_SEV_MASK;
	}

	bool bSevere = (m_pLastError->nSeverity >= m_nErrSevLevel);
	m_pLastError->nCode = MAKE_HRESULT(bSevere ? 1 : 0, HRESULT_FACILITY(nCode), HRESULT_CODE(nCode));

	// update counters
	if (bSevere) m_nErrCntSevRec++, m_nErrCntSevAll++;
	m_nErrCntAllRec++, m_nErrCntAllAll++;

	if (pSender)
	{
		pSender->GetClassId(&m_pLastError->pClassName);
		pSender->GetRTCId(&m_pLastError->nRTCId);
	}

	m_pLastError->pSender = pSender;

	// rewrite source info from RaiseErrorScr call
	m_pLastError->pSrcFile = m_pSrcFile;
	m_pSrcFile = NULL;
	m_pLastError->nSrcLine = m_nSrcLine;
	m_nSrcLine = 0;

	if (bSevere && IsErrorExceptionEnabled() == S_OK)
		throw m_pLastError;
	if (m_pUserErrorHandleF)
		m_pUserErrorHandleF(m_pLastError, TRUE);

	return m_pLastError->nCode;
}

HRESULT __stdcall CFWDevice::RaiseErrorSrc(FWSTRING pSrcFile, FWULONG nSrcLine)
{
	m_pSrcFile = pSrcFile;
	m_nSrcLine = nSrcLine;
	return S_OK;
}

HRESULT __stdcall CFWDevice::Recover(FWULONG nSeverity)
{
	if (!m_pLastError) return S_OK;

	if (nSeverity == FW_SEV_NOTHING)
	{
		FWERROR *p = m_pLastError;
		m_pLastError = m_pLastError->pPrev;
		if (p->pMessage) free(p->pMessage);
		delete p;
		if (m_pLastError) m_pLastError->pNext = NULL;
		m_nErrCntSevRec--; m_nErrCntSevAll--; m_nErrCntAllRec--; m_nErrCntAllAll--;
		if (m_pLastError == NULL) m_pFirstError = NULL;
		if (IsErrorExceptionEnabled() == S_FALSE && m_pUserErrorHandleF)
			m_pUserErrorHandleF(NULL, FALSE);
	}
	else
	{
		bool bSevereBefore = (m_pLastError->nSeverity >= m_nErrSevLevel);
		if (nSeverity & FW_SEV_SET_ATLEAST) m_pLastError->nSeverity = max(m_pLastError->nSeverity, nSeverity & FW_SEV_MASK);
		else if (nSeverity & FW_SEV_SET_ATMOST) m_pLastError->nSeverity = min(m_pLastError->nSeverity, nSeverity & FW_SEV_MASK);
		else m_pLastError->nSeverity = nSeverity & FW_SEV_MASK;
		bool bSevereAfter = (m_pLastError->nSeverity >= m_nErrSevLevel);
		if (bSevereBefore && !bSevereAfter) m_nErrCntSevRec--, m_nErrCntSevAll--;
		if (!bSevereBefore && bSevereAfter) m_nErrCntSevRec++, m_nErrCntSevAll++;
		if (IsErrorExceptionEnabled() == S_FALSE && m_pUserErrorHandleF)
			m_pUserErrorHandleF(m_pLastError, FALSE);
	}

	return S_OK;
}

HRESULT __stdcall CFWDevice::GetErrorSeverityLevel(FWULONG *nSeverity)
{
	if (nSeverity) *nSeverity = m_nErrSevLevel;
	return S_OK;
}

HRESULT __stdcall CFWDevice::PutSevereErrorLevel(FWULONG nSeverity)
{
	if (nSeverity < FW_SEV_WARNING || nSeverity > FW_SEV_CRITICAL)
		return ERROR(FW_E_INVALIDARG);
	m_nErrSevLevel = nSeverity;
	return S_OK;
}

///////////////////////////////////////////////////////////
// class CFWCreateContext

CFWCreateContext::CFWCreateContext()
	: m_pIFWDevice(NULL), m_pNoun(NULL), m_pVerb(NULL), m_nParams(0), m_pParams(NULL), m_pRefNode(NULL), m_pRefBody(NULL), m_bNamesMapped(false)

{
}

CFWCreateContext::CFWCreateContext(IFWDevice *pIFWDevice, CFWDevice::_NOUN *pNoun, CFWDevice::_VERB *pVerb, FWULONG nParams, FWPARAM *pParams)
	: m_pIFWDevice(pIFWDevice), m_pNoun(pNoun), m_pVerb(pVerb), m_nParams(nParams), m_pParams(pParams), m_pRefNode(NULL), m_pRefBody(NULL), m_bNamesMapped(false)
{
	if (m_pIFWDevice) m_pIFWDevice->AddRef();
}

CFWCreateContext::~CFWCreateContext()
{
	if (m_pIFWDevice) m_pIFWDevice->Release();
	if (m_pRefNode) m_pRefNode->Release();
	if (m_pRefBody) m_pRefBody->Release();
}

HRESULT __stdcall CFWCreateContext::GetContext(/*[out]*/ FWSTRING *pNoun, /*[out]*/ FWSTRING *pVerb)
{
	if (pNoun) *pNoun = m_pNoun->theNoun;
	if (pVerb) *pVerb = m_pVerb->theVerb;
	return S_OK;
}

HRESULT __stdcall CFWCreateContext::GetContextId(/*[out]*/ FWULONG *pIdNoun, /*[out]*/ FWULONG *pIdVerb)
{
	if (pIdNoun) *pIdNoun = m_pNoun->idNoun;
	if (pIdVerb) *pIdVerb = m_pVerb->idVerb;
	return S_OK;
}

HRESULT __stdcall CFWCreateContext::Create(REFIID iid, /*[out, retval, iid_is(iid)]*/ IFWUnknown **p)
{
	if (!m_pNoun || !m_pVerb)
		return ERROR(FW_E_NOTREADY);

	CFWDevice::_CLASS *pClass = m_pVerb->vecClasses[0];
	if (!pClass->pFactory)
	{
		CLSID clsid;
		HRESULT h = CLSIDFromString(pClass->theClass, &clsid);
		assert(SUCCEEDED(h));
		h = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IFWUnknown, (void**)&pClass->pFactory);
		if (FAILED(h)) 
		{
			FWSTRING pParam[] = { m_pNoun->theNoun, m_pVerb->theVerb };
			m_pIFWDevice->RaiseErrorSrc(__WFILE__, __LINE__); 
			return m_pIFWDevice->RaiseError(m_pIFWDevice, FW_E_CLASS, 2, (FWULONG*)pParam, NULL, FW_SEV_DEFAULT);
		}
		m_pIFWDevice->RegisterObject(pClass->pFactory);
		m_pIFWDevice->Release();	// make FreeWill+ reference from a factory a weak pointer...
	}

	HRESULT h = pClass->pFactory->GetClone(iid, &m_pUnknown);
	if (!FAILED(h))
		h = m_pUnknown->Create(this);
	*p = m_pUnknown;
	return h;
}

HRESULT __stdcall CFWCreateContext::CreateUnknown(/*[out, retval]*/ IFWUnknown **p)
{
	return Create(IID_IFWUnknown, p);
}

HRESULT __stdcall CFWCreateContext::Reject()
{
	if (!m_pNoun || !m_pVerb)
		return ERROR(FW_E_NOTREADY);
	return E_NOTIMPL;
}

HRESULT CFWCreateContext::ErrorHelper(FWSTRING pSrcFile, FWULONG nSrcLine, HRESULT e)
{
	FWSTRING clsid = L"unititialised";
	if (m_pUnknown) m_pUnknown->GetClassId(&clsid);
	ErrorSrc(pSrcFile, nSrcLine);
	return Error(e, 1, (FWULONG*)&clsid);
}

HRESULT CFWCreateContext::ErrorHelper(FWSTRING pSrcFile, FWULONG nSrcLine, HRESULT e, FWSTRING pParamName)
{
	FWSTRING clsid = L"unititialised";
	if (m_pUnknown) m_pUnknown->GetClassId(&clsid);
	FWULONG params[] = { *(FWULONG*)&pParamName, *(FWULONG*)&clsid };
	ErrorSrc(pSrcFile, nSrcLine);
	return Error(e, 2, params);
}

HRESULT CFWCreateContext::ErrorHelper(FWSTRING pSrcFile, FWULONG nSrcLine, HRESULT e, FWULONG num, FWULONG nFrom, FWULONG nTo)
{
	FWSTRING clsid = L"unititialised";
	FWSTRING types[] = { L"NO_TYPE", L"END_OF_LIST", L"FWULONG", L"FWFLOAT", L"FWSTRING", L"FWVECTOR", L"FWPUNKNOWN", L"FWPBONE", L"FWPBODY" };
	if (m_pUnknown) m_pUnknown->GetClassId(&clsid);
	FWULONG params[] = { *(FWULONG*)&clsid, num, *(FWULONG*)&types[nFrom], *(FWULONG*)&types[nTo] };
	ErrorSrc(pSrcFile, nSrcLine);
	return Error(e, 4, params);
}

void CFWCreateContext::MapNamedParams()
{
	ULONG nParams = m_nParams + m_vecParams.size();
	FWPARAM *pParam = m_pParams;
	vector<FWPARAM>::iterator iter = m_vecParams.begin();
	NAME data = { 0, 0 };
	FWSTRING pName = NULL;
	for (ULONG i = 0; i < nParams; i++)
	{
		if (i > m_nParams) pParam = &(*iter++);
		if (pParam->m_pName != NULL)
			if (pName == NULL || wcscmp(pParam->m_pName, pName) != 0)
			{
				m_mapNames[pName ? pName : L""] = data;
				data.nStart = i;
				data.nLen = 0;
				pName = pParam->m_pName;
			}
		data.nLen++;
		pParam++;
	}
	m_mapNames[pName ? pName : L""] = data;
}

HRESULT __stdcall CFWCreateContext::EnumParams(/*[out, retval]*/ IFWEnumParams **p)
{
	assert(p);
	*p = new CFWEnumParams(this, 0, m_nParams + m_vecParams.size() - 0);
	FWDevice()->RegisterObject(*p);
	return S_OK;
}

HRESULT __stdcall CFWCreateContext::EnumParamsEx(FWSTRING pName, /*[out, retval]*/ IFWEnumParams **p)
{
	if (!m_bNamesMapped)
		MapNamedParams();
	m_bNamesMapped = true;

	assert(p);
	MAP::iterator i = m_mapNames.find(pName ? pName : L"");
	if (i == m_mapNames.end())
		*p = new CFWEnumParams(this, 0, 0);
	else
		*p = new CFWEnumParams(this, i->second.nStart, i->second.nLen);
	FWDevice()->RegisterObject(*p);
	return S_OK;
}

HRESULT __stdcall CFWCreateContext::SubmitULONG(FWULONG val)
{
	FWPARAM param = { FW_PARAM_ULONG, 0, 0, NULL };
	param.m_ULONG = val;
	m_vecParams.push_back(param);
	return S_OK;
}

HRESULT __stdcall CFWCreateContext::SubmitFLOAT(FWFLOAT val)
{
	FWPARAM param = { FW_PARAM_FLOAT, 0, 0, NULL };
	param.m_FLOAT = val;
	m_vecParams.push_back(param);
	return S_OK;
}

HRESULT __stdcall CFWCreateContext::SubmitSTRING(FWSTRING val)
{
	FWPARAM param = { FW_PARAM_STRING, 0, 0, NULL };
	param.m_STRING = val;
	m_vecParams.push_back(param);
	return S_OK;
}

HRESULT __stdcall CFWCreateContext::SubmitVECTOR(FWVECTOR val)
{
	FWPARAM param = { FW_PARAM_VECTOR, 0, 0, NULL };
	param.m_VECTOR = val;
	m_vecParams.push_back(param);
	return S_OK;
}

HRESULT __stdcall CFWCreateContext::SubmitPUNKNOWN(FWPUNKNOWN val)
{
	FWPARAM param = { FW_PARAM_PUNKNOWN, 0, 0, NULL };
	param.m_PUNKNOWN = val;
	m_vecParams.push_back(param);
	return S_OK;
}

HRESULT __stdcall CFWCreateContext::SubmitPBONE(FWPUNKNOWN val)
{
	FWPARAM param = { FW_PARAM_PBONE, 0, 0, NULL };
	param.m_PBONE = val;
	m_vecParams.push_back(param);
	return S_OK;
}

HRESULT __stdcall CFWCreateContext::SubmitPBODY(FWPUNKNOWN val)
{
	FWPARAM param = { FW_PARAM_PBODY, 0, 0, NULL };
	param.m_PBODY = val;
	m_vecParams.push_back(param);
	return S_OK;
}

///////////////////////////////////////////////////////////
//
// CFWEnumParams Inline Class Definition

CFWEnumParams::CFWEnumParams(): m_pContext(NULL), m_nStart(0), m_nCur(0), m_nLen(0),
	m_pRefNode(NULL), m_pRefBody(NULL), m_nTypeExpected(FW_PARAM_NONE), m_nTypeFound(FW_PARAM_NONE)
{
	if (m_pContext) m_pContext->AddRef();
}

CFWEnumParams::CFWEnumParams(CFWCreateContext *pContext, ULONG nStart, ULONG nLen, ULONG nCur) : m_pContext(pContext), m_nStart(nStart), m_nCur(nCur), m_nLen(nLen),
	m_pRefNode(NULL), m_pRefBody(NULL), m_nTypeExpected(FW_PARAM_NONE), m_nTypeFound(FW_PARAM_NONE)
{
	if (m_pContext) m_pContext->AddRef();
}

CFWEnumParams::~CFWEnumParams()
{
	if (m_pRefNode) m_pRefNode->Release();
	if (m_pRefBody) m_pRefBody->Release();
	if (m_pContext) m_pContext->Release();
}

HRESULT __stdcall CFWEnumParams::QueryParam(/*[out]*/ FWPARAM **pParam)
{
	if (pParam) *pParam = NULL;
	if (m_nCur >= m_nLen)
		return FW_E_MISSING_PARAMS;
	if (pParam)
	{
		if (m_nStart + m_nCur <= m_pContext->m_nParams) *pParam = m_pContext->m_pParams + m_nStart + m_nCur;
		else *pParam = &(m_pContext->m_vecParams[m_nStart + m_nCur - m_pContext->m_nParams]);
	}
	m_nCur++;
	m_nTypeFound = (pParam && *pParam) ? (*pParam)->m_type : FW_PARAM_NONE;
	return S_OK;
}

HRESULT __stdcall CFWEnumParams::QueryLONG(FWLONG *val)
{
	m_nTypeExpected = FW_PARAM_LONG;
	ULONG myIndex = GetIndex();
	FWPARAM *pParam = NULL;
	HRESULT h = QueryParam(&pParam);
	if (FAILED(h)) return h;
	switch (pParam->m_type)
	{
		case FW_PARAM_LONG:		if (val) *val = pParam->m_LONG; break;
		case FW_PARAM_ULONG:	if (val) *val = pParam->m_LONG; break;
		case FW_PARAM_FLOAT:	if (val) *val = (FWULONG)(pParam->m_FLOAT); break;
		default:				PutIndex(myIndex); return FW_E_PARAM_TYPE_MISMATCH;
	}
	return S_OK;
}

HRESULT __stdcall CFWEnumParams::QueryULONG(FWULONG *val)
{
	m_nTypeExpected = FW_PARAM_ULONG;
	ULONG myIndex = GetIndex();
	FWPARAM *pParam = NULL;
	HRESULT h = QueryParam(&pParam);
	if (FAILED(h)) return h;
	switch (pParam->m_type)
	{
		case FW_PARAM_LONG:		if (val) *val = pParam->m_ULONG; break;
		case FW_PARAM_ULONG:	if (val) *val = pParam->m_ULONG; break;
		case FW_PARAM_FLOAT:	if (val) *val = (FWULONG)(pParam->m_FLOAT); break;
		default:				PutIndex(myIndex); return FW_E_PARAM_TYPE_MISMATCH;
	}
	return S_OK;
}

HRESULT __stdcall CFWEnumParams::QueryFLOAT(FWFLOAT *val)
{
	m_nTypeExpected = FW_PARAM_FLOAT;
	ULONG myIndex = GetIndex();
	FWPARAM *pParam = NULL;
	HRESULT h = QueryParam(&pParam);
	if (FAILED(h)) return h;
	switch (pParam->m_type)
	{
		case FW_PARAM_LONG:		if (val) *val = (FWFLOAT)pParam->m_LONG; break;
		case FW_PARAM_ULONG:	if (val) *val = (FWFLOAT)pParam->m_ULONG; break;
		case FW_PARAM_FLOAT:	if (val) *val = pParam->m_FLOAT; break;
		default:				PutIndex(myIndex); return FW_E_PARAM_TYPE_MISMATCH;
	}
	return S_OK;
}

HRESULT __stdcall CFWEnumParams::QuerySTRING(FWSTRING *val)
{
	m_nTypeExpected = FW_PARAM_STRING;
	ULONG myIndex = GetIndex();
	FWPARAM *pParam = NULL;
	HRESULT h = QueryParam(&pParam);
	if (FAILED(h)) return h;
	switch (pParam->m_type)
	{
		case FW_PARAM_STRING:	if (val) *val = pParam->m_STRING; break;
		default:				PutIndex(myIndex); return FW_E_PARAM_TYPE_MISMATCH;
	}
	return S_OK;
}

HRESULT __stdcall CFWEnumParams::QueryVECTOR(FWVECTOR *val)
{
	m_nTypeExpected = FW_PARAM_VECTOR;
	ULONG myIndex = GetIndex();
	FWPARAM *pParam = NULL;
	HRESULT h = QueryParam(&pParam);
	if (FAILED(h)) return h;
	FWFLOAT x, y, z;
	switch (pParam->m_type)
	{
		case FW_PARAM_VECTOR:	
			if (val) *val = pParam->m_VECTOR; 
			break;
		case FW_PARAM_LONG:
			x = (FWFLOAT)pParam->m_LONG;
			h = QueryFLOAT(&y); if FAILED(h) { PutIndex(myIndex); return h; }
			h = QueryFLOAT(&z); if FAILED(h) { PutIndex(myIndex); return h; }
			if (val) { val->x = x; val->y = y; val->z = z; }
			break;
		case FW_PARAM_ULONG:
			x = (FWFLOAT)pParam->m_ULONG;
			h = QueryFLOAT(&y); if FAILED(h) { PutIndex(myIndex); return h; }
			h = QueryFLOAT(&z); if FAILED(h) { PutIndex(myIndex); return h; }
			if (val) { val->x = x; val->y = y; val->z = z; }
			break;
		case FW_PARAM_FLOAT:
			x = pParam->m_FLOAT;
			h = QueryFLOAT(&y); if FAILED(h) { PutIndex(myIndex); return h; }
			h = QueryFLOAT(&z); if FAILED(h) { PutIndex(myIndex); return h; }
			if (val) { val->x = x; val->y = y; val->z = z; }
			break;
		default:				PutIndex(myIndex); return FW_E_PARAM_TYPE_MISMATCH;
	}
	return S_OK;
}

HRESULT __stdcall CFWEnumParams::QueryPUNKNOWN(REFIID iid, FWPUNKNOWN *ppVal)
{
	m_nTypeExpected = FW_PARAM_PUNKNOWN;
	ULONG myIndex = GetIndex();
	FWPARAM *pParam = NULL;
	FWPUNKNOWN p = NULL;
	HRESULT h = QueryParam(&pParam);
	if (FAILED(h)) return h;
	switch (pParam->m_type)
	{
		case FW_PARAM_PUNKNOWN:
			if (pParam->m_PUNKNOWN == NULL) p = NULL;
			else h = pParam->m_PUNKNOWN->QueryInterface(iid, (void**)&p);
			if (FAILED(h)) { PutIndex(myIndex); return FW_E_PARAM_BAD_INTERFACE; }
			if (ppVal) *ppVal = p; else if (p) p->Release();
			break;
		default:
			PutIndex(myIndex); 
			return FW_E_PARAM_TYPE_MISMATCH;
	}
	return S_OK;
}

HRESULT __stdcall CFWEnumParams::QueryPBODY(REFIID iid, FWPUNKNOWN *ppVal)
{
	m_nTypeExpected = FW_PARAM_PBODY;
	ULONG myIndex = GetIndex();
	FWPARAM *pParam = NULL;
	FWPUNKNOWN p = NULL;
	HRESULT h = QueryParam(&pParam);
	if (FAILED(h)) return h;
	switch (pParam->m_type)
	{
		case FW_PARAM_PBODY:
			if (pParam->m_PUNKNOWN == NULL) p = NULL;
			else
			{
				h = pParam->m_PBODY->QueryInterface(IID_IBody, (void**)&p);
				if (FAILED(h)) { PutIndex(myIndex); return FW_E_PARAM_BAD_INTERFACE; }
				if (m_pRefBody) m_pRefBody->Release();
				m_pRefBody = (IBody*)(p);
				m_pRefBody->AddRef();
			}
			if (ppVal) *ppVal = p; else if (p) p->Release();
			break;

		default:
			PutIndex(myIndex); 
			return FW_E_PARAM_TYPE_MISMATCH;
	}
	return S_OK;
}

HRESULT __stdcall CFWEnumParams::QueryPBONE(REFIID iid, FWPUNKNOWN *ppVal)
{
	m_nTypeExpected = FW_PARAM_PBONE;
	ULONG myIndex = GetIndex();
	FWPARAM *pParam = NULL;
	FWPUNKNOWN p = NULL;
	HRESULT h = QueryParam(&pParam);
	if (FAILED(h)) return h;
	FWSTRING label;
	FWULONG id;
	IKineChild *pChild;

	if (ppVal)
		switch (pParam->m_type)
		{
			case FW_PARAM_PBONE:
				if (pParam->m_PBONE == NULL) p = NULL;
				else
				{
					// store the pointer provided by the user as reference node m_pRefNode (must be node, not child)
					if (m_pRefNode) m_pRefNode->Release();
					m_pRefNode = NULL;
					h = pParam->m_PBONE->QueryInterface(IID_IKineNode, (void**)&m_pRefNode);
					if (FAILED(h)) { PutIndex(myIndex); return FW_E_PARAM_BAD_INTERFACE; }

					// get a label if provided
					if (SUCCEEDED(QuerySTRING(&label)))
					{
						// string-labeled bone
						h = m_pRefNode->CheckChild(label, &pChild);
						if (pChild == NULL || FAILED(h)) { PutIndex(myIndex); return FW_E_PARAM_BAD_BONE; }
						h = pChild->QueryInterface(iid, (void**)&p);
						pChild->Release();
						if (FAILED(h)) { PutIndex(myIndex); return FW_E_PARAM_BAD_INTERFACE; }
					}
					else
					{
						h = m_pRefNode->QueryInterface(iid, (void**)&p);
						if (FAILED(h)) { PutIndex(myIndex); return FW_E_PARAM_BAD_INTERFACE; }
					}
				}
				if (ppVal) *ppVal = p; else if (p) p->Release();
				break;

			case FW_PARAM_PBODY:
				if (!pParam->m_PBODY) { PutIndex(myIndex); return FW_E_PARAM_IS_NULL; }
				if (m_pRefBody) m_pRefBody->Release();
				m_pRefBody = NULL;
				h = pParam->m_PBODY->QueryInterface(IID_IBody, (void**)&m_pRefBody);
				if (FAILED(h)) { PutIndex(myIndex); return FW_E_PARAM_BAD_INTERFACE; }

				//index must be provided
				if (SUCCEEDED(QueryULONG(&id)))
				{
					// ulong-labeled bone
					h = m_pRefBody->GetBodyPart(id, iid, (IFWUnknown**)&p);
					if (!p || FAILED(h)) { PutIndex(myIndex); return FW_E_PARAM_BAD_BONE; }
				}
				else
				{
					PutIndex(myIndex); 
					return FW_E_PARAM_BAD_BONE;
				}
				if (ppVal) *ppVal = p; else if (p) p->Release();
				break;

			case FW_PARAM_STRING:	
				label = pParam->m_STRING;
				if (!m_pRefNode) { PutIndex(myIndex); return FW_E_PARAM_NO_BONE_REF; }
				h = m_pRefNode->CheckChild(label, &pChild);
				if (pChild == NULL || FAILED(h)) { PutIndex(myIndex); return FW_E_PARAM_BAD_BONE; }
				h = pChild->QueryInterface(iid, (void**)&p);
				pChild->Release();
				if (FAILED(h)) { PutIndex(myIndex); return FW_E_PARAM_BAD_INTERFACE; }
				if (ppVal) *ppVal = p; else if (p) p->Release();
				break;

			case FW_PARAM_LONG:	
				id = pParam->m_LONG;
				if (!m_pRefBody) { PutIndex(myIndex); return FW_E_PARAM_NO_BODY_REF; }
				h = m_pRefBody->GetBodyPart(id, iid, (IFWUnknown**)&p);
				if ((id && !p) || FAILED(h)) { PutIndex(myIndex); return FW_E_PARAM_BAD_BONE; }
				if (ppVal) *ppVal = p; else if (p) p->Release();
				break;

			case FW_PARAM_ULONG:	
				id = pParam->m_ULONG;
				if (!m_pRefBody) { PutIndex(myIndex); return FW_E_PARAM_NO_BODY_REF; }
				h = m_pRefBody->GetBodyPart(id, iid, (IFWUnknown**)&p);
				if ((id && !p) || FAILED(h)) { PutIndex(myIndex); return FW_E_PARAM_BAD_BONE; }
				if (ppVal) *ppVal = p; else if (p) p->Release();
				break;

			case FW_PARAM_FLOAT:	
				id = (FWULONG)(pParam->m_FLOAT);
				if (!m_pRefBody) { PutIndex(myIndex); return FW_E_PARAM_NO_BODY_REF; }
				h = m_pRefBody->GetBodyPart(id, iid, (IFWUnknown**)&p);
				if ((id && !p) || FAILED(h)) { PutIndex(myIndex); return FW_E_PARAM_BAD_BONE; }
				if (ppVal) *ppVal = p; else if (p) p->Release();
				break;

			default:
				PutIndex(myIndex); 
				return FW_E_PARAM_TYPE_MISMATCH;
		}
	else
		// a short version for verification only
		switch (pParam->m_type)
		{
			case FW_PARAM_PBONE:
				if (!pParam->m_PBONE) break;
				QuerySTRING(NULL);	//consume a label if any
				break;

			case FW_PARAM_PBODY:
				if (!pParam->m_PBODY) { PutIndex(myIndex); return FW_E_PARAM_IS_NULL; }
				if FAILED(QueryULONG(NULL))		// index is obligatory
				{ 
					PutIndex(myIndex); 
					return FW_E_PARAM_BAD_BONE; 
				}
				break;

			case FW_PARAM_STRING:
			case FW_PARAM_LONG:
			case FW_PARAM_ULONG:
			case FW_PARAM_FLOAT:
				break;

			default:
				PutIndex(myIndex); 
				return FW_E_PARAM_TYPE_MISMATCH;
		}
	return S_OK;
}

HRESULT __stdcall CFWEnumParams::Skip(FWLONG nNum)
{
	if (nNum < 0 && -nNum > (FWLONG)m_nCur) nNum = -(FWLONG)m_nCur;
	if (nNum > 0 &&  nNum > (FWLONG)(m_nLen - m_nCur)) nNum = m_nLen - m_nCur;
	m_nCur += nNum; 
	return S_OK;
}

HRESULT __stdcall CFWEnumParams::Reset()
{
	m_nStart = 0;
	return S_OK;
}

HRESULT __stdcall CFWEnumParams::Clone(/*[out]*/ IFWEnumParams **ppEnum)
{
	assert(ppEnum);
	*ppEnum = new CFWEnumParams(m_pContext, m_nStart, m_nLen, m_nCur);
	FWDevice()->RegisterObject(*ppEnum);
	return S_OK;
}

HRESULT __stdcall CFWEnumParams::ErrorEx(FWSTRING pSrcFile, FWULONG nSrcLine, HRESULT hErr)
{
	ErrorSrc(pSrcFile, nSrcLine);
	if (hErr < FW_E_PARAM_FIRST || hErr > FW_E_PARAM_LAST)
		return Error(hErr);

	FWSTRING clsid = L"unititialised";
	FWSTRING types[] = { L"NO_TYPE", L"END_OF_LIST", L"FWULONG", L"FWFLOAT", L"FWSTRING", L"FWVECTOR", L"FWPUNKNOWN", L"FWPBONE", L"FWPBODY" };

	if (m_pContext->m_pUnknown) m_pContext->m_pUnknown->GetClassId(&clsid);
	
	FWSTRING from = (m_nTypeFound    >= 0 && m_nTypeFound    < (sizeof(types)/sizeof(FWSTRING))) ? types[m_nTypeFound   ] : L"???";
	FWSTRING to =   (m_nTypeExpected >= 0 && m_nTypeExpected < (sizeof(types)/sizeof(FWSTRING))) ? types[m_nTypeExpected] : L"???";

	FWULONG params[] = { *(FWULONG*)&clsid, m_nStart+m_nCur+1, *(FWULONG*)&from, *(FWULONG*)&to };
	return Error(hErr, 4, params);
}

