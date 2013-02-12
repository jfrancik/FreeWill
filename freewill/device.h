// device.h: CFWDevice coclass
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(_DEVICE__)
#define _DEVICE__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../fwlib/factory.h"
#include "../fwlib/fwunknown.h"
#include "freewill.h"
#include <map>
#include <vector>
#include <string>

using namespace std;

class CFWDevice : public FWUNKNOWN<IFWDevice, IID_IFWDevice, IFWDevice >
{
public:

	////////////////////////////////////////////////////////////////////
	// Object Repository
	virtual HRESULT __stdcall CreateObject(FWSTRING theNoun, REFIID iid, /*[out, retval, iid_is(iid)]*/ IFWUnknown**);
	virtual HRESULT __stdcall CreateObjectEx(FWSTRING theNoun, FWSTRING theVerb, 
						  FWULONG nParams, FWPARAM *pParams, 
						  REFIID iid, /*[out, retval, iid_is]*/ IFWUnknown**);
	virtual HRESULT __stdcall CreateUnknown(FWSTRING theNoun, /*[out, retval]*/ IFWUnknown**);
	virtual HRESULT __stdcall CreateUnknownEx(FWSTRING theNoun, FWSTRING theVerb, 
						  FWULONG nParams, FWPARAM *pParams, 
						  /*[out, retval]*/ IFWUnknown**);
	virtual HRESULT __stdcall CreateContext(FWSTRING theNoun, FWULONG idNoun, FWSTRING theVerb, FWULONG idVerb, 
						   FWULONG nParams, FWPARAM *pParams, 
						   /*[out, retval]*/ IFWCreateContext**);

	////////////////////////////////////////////////////////////////////
	// FreeWill+ Device Configuration

	virtual HRESULT __stdcall GetNounCount(/*[out, retval]*/ FWULONG*);
	virtual HRESULT __stdcall NounToId(FWSTRING theNoun, /*[out, retval]*/ FWULONG*);
	virtual HRESULT __stdcall IdToNoun(FWULONG idNoun, /*[out, retval]*/ FWSTRING*);
	virtual HRESULT __stdcall AddNoun(FWSTRING theNoun, FWSTRING theGenericVerb, /*[out, retval]*/ FWULONG*);

	virtual HRESULT __stdcall GetVerbCount(FWULONG idNoun, /*[out, retval]*/ FWULONG*);
	virtual HRESULT __stdcall VerbToId(FWULONG idNoun, FWSTRING theVerb, /*[out, retval]*/ FWULONG*);
	virtual HRESULT __stdcall IdToVerb(FWULONG idNoun, FWULONG idVerb, /*[out, retval]*/ FWSTRING*);
	virtual HRESULT __stdcall AddVerb(FWULONG idNoun, FWSTRING theVerb, /*[out, retval]*/ FWULONG*);

	virtual HRESULT __stdcall GetClassCount(FWULONG idNoun, FWULONG idVerb, /*[out, retval]*/ FWULONG*);
	virtual HRESULT __stdcall ClassToId(FWULONG idNoun, FWULONG idVerb, FWSTRING theClass, /*[out, retval]*/ FWULONG*);
	virtual HRESULT __stdcall IdToClass(FWULONG idNoun, FWULONG idVerb, FWULONG idClass, /*[out, retval]*/ FWSTRING*);
	virtual HRESULT __stdcall AddClass(FWULONG idNoun, FWULONG idVerb, FWSTRING theClass, /*[out, retval]*/ FWULONG*);

	virtual HRESULT __stdcall AddInstantClass(FWULONG idNoun, FWULONG idVerb, IFWUnknown *pInstant, /*[out, retval]*/ FWULONG*);

	// OBSOLETE
	//virtual HRESULT __stdcall GetArgCount(FWULONG idNoun, FWULONG idVerb, /*[out, retval]*/ FWULONG*);
	//virtual HRESULT __stdcall ArgToId(FWULONG idNoun, FWULONG idVerb, FWSTRING theArg, /*[out, retval]*/ FWULONG*);
	//virtual HRESULT __stdcall IdToArg(FWULONG idNoun, FWULONG idVerb, FWULONG idArg, /*[out, retval]*/ FWSTRING*);
	//virtual HRESULT __stdcall AddArg(FWULONG idNoun, FWULONG idVerb, 
	//			FWSTRING theArg, FWULONG nIndex, enum ARGTYPES type, /*[out, retval]*/ FWULONG*);

	virtual HRESULT __stdcall Load();
	virtual HRESULT __stdcall Store();

	////////////////////////////////////////////////////////////////////
	// Object Registration and Tracking
	virtual HRESULT __stdcall RegisterObject(IFWUnknown*);
	virtual HRESULT __stdcall UnregisterObject(IFWUnknown*);
	virtual HRESULT __stdcall GetRegisteredObjectsCount(FWULONG*);

	////////////////////////////////////////////////////////////////////
	// Error Handling
	virtual HRESULT __stdcall GetStatus();
	virtual HRESULT __stdcall GetLastError(/*[out, retval]*/ FWERROR**);
	virtual HRESULT __stdcall GetErrorNum(BOOL bSevereErrorsOnly, BOOL bRecentErrorsOnly, FWULONG*);
	virtual HRESULT __stdcall EnableErrorException(BOOL b);
	virtual HRESULT __stdcall IsErrorExceptionEnabled();
	virtual HRESULT __stdcall SetUserErrorHandler(HRESULT(__stdcall *)(struct FWERROR*, BOOL bRaised));
	virtual HRESULT __stdcall GetUserErrorHandler(HRESULT(__stdcall **)(struct FWERROR*, BOOL bRaised));
	virtual HRESULT __stdcall RaiseError(IFWUnknown *pSender, FWULONG nCode, FWULONG nParams, /*[size_is(nParams)]*/ FWULONG *pParams,
								FWSTRING pMessage, FWULONG nSeverity);
	virtual HRESULT __stdcall RaiseErrorSrc(FWSTRING pSrcFile, FWULONG nSrcLine);
	virtual HRESULT __stdcall Recover(FWULONG nSeverity);
	virtual HRESULT __stdcall GetErrorSeverityLevel(FWULONG *nSeverity);
	virtual HRESULT __stdcall PutSevereErrorLevel(FWULONG nSeverity);


	////////////////////////////////////////////////////////////////////
	// Creation function - keeps a single instance whenever factory class wants a pointer
	static IFWDevice *c_pSingleInstance;
	friend IUnknown *CreateFWDevice() 
	{
		if (c_pSingleInstance) c_pSingleInstance->AddRef();
		else c_pSingleInstance = (IFWDevice*)new CFWDevice;
		return c_pSingleInstance;
	}

	FW_RTTI(FWDevice)
	FW_ERROR_BEGIN
		FW_ERROR_ENTRY(FW_E_FREEWILL,			L"Invalid FreeWill+ Object - created outside the FWDevice", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(FW_E_BADINTERFACE,		L"Interface not inherited from IFWUnknown", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(FW_E_NOINTERFACE,		L"Object does not implement required interface", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(FW_E_CLASSSPECIFIC,		L"Class-specific error (ambigous code)", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(FW_E_CLASS,				L"The System could not create an object because class implementation is unknown for the noun \"%s\" and the verb \"%s\"", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(FW_E_UNIDENTIFIED,		L"Unidentified error", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(FW_E_OUTOFMEMORY,		L"Ran out of memory", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(FW_E_POINTER,			L"Invalid pointer", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(FW_E_NOTFOUND,			L"Object not found", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(FW_E_NOTREADY,			L"Object not ready to perform the operation", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(FW_E_FORMAT,				L"Function not supported for this format", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(FW_E_BADINDEX,			L"Index out of range", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(FW_E_INVALIDARG,			L"One or more arguments are invalid", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(FW_E_MEMOVERRUN,			L"Memory buffer overrun", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(FW_E_STACKOVERRUN,		L"Memory overrun during a stack POP operation", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(FW_E_UNKNOWN_NOUN,		L"Unknown noun: \"%s\"", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(FW_E_UNKNOWN_NOUN_ID,	L"Unknown noun ID: %d", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(FW_E_UNKNOWN_VERB,		L"Unknown verb: \"%s\"", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(FW_E_UNKNOWN_VERB_ID,	L"Unknown verb ID: %d", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(FW_E_UNKNOWN_CLASS,		L"Unknown class: \"%s\"", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(FW_E_UNKNOWN_CLASS_ID,	L"Unknown class ID: %d", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(FW_E_PARAM_TYPE_MISMATCH,L"Cannot create an object of class: %s. Parameter type mismatch: cannot convert parameter #%d from %s to %s.", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(FW_E_PARAM_BAD_INTERFACE,L"Cannot create an object of class: %s. Parameter #%d does not implement a required interface.", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(FW_E_PARAM_IS_NULL,		L"Cannot create an object of class: %s. Parameter #%d cannot be NULL.", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(FW_E_PARAM_BAD_BONE,		L"Cannot create an object of class: %s. Parameter #%d does not represent any existing bone.", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(FW_E_PARAM_NO_BONE_REF,	L"Cannot create an object of class: %s. Parameter #%d seems to be a bone label but there is no node reference.", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(FW_E_PARAM_NO_BODY_REF,	L"Cannot create an object of class: %s. Parameter #%d seems to be a bone index but there is no body reference.", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(FW_E_NAMED_PARAM_NOT_FOUND,L"Named parameter \"%s\" not found when trying to create an object of class: %s", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(FW_E_TOO_MANY_PARAMS,	L"Cannot create an object of class: %s. Too many parameters.", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(FW_E_MISSING_PARAMS,		L"Cannot create an object of class: %s. Missing or too few parameters.", FW_SEV_CRITICAL)
		FW_ERROR_ENTRY(FW_E_PARAMS_INCONSISTENT,L"Cannot create an object of class: %s. Inconsistency or conflict detected at parameter #%d.", FW_SEV_CRITICAL)

		FW_ERROR_END

protected:

	// Error Handling
	FWERROR *m_pLastError;			// the last error
	FWERROR *m_pFirstError;			// the first error
	FWSTRING m_pSrcFile;					// error source file
	FWULONG m_nSrcLine;					// error source line
	FWULONG m_nErrSevLevel;				// error severity level (minimal FW_SEVERITY flag for error to be severe)
	FWULONG m_nExceptionEnabled;			// Error Exception Enable flag (cumulative flag)
	HRESULT(__stdcall *m_pUserErrorHandleF)(struct FWERROR*, BOOL bRaised);	// User Error Handler Function
	FWULONG m_nErrCntSevRec, m_nErrCntSevAll, m_nErrCntAllRec, m_nErrCntAllAll;
										// error counters

	// Counter of the registered objects
	FWULONG nRegObjects;

	// Noun & Verb Object Factory

	struct _NOUN;
	struct _VERB;
	struct _CLASS;

	typedef map<std::wstring, FWULONG> MAP;

	struct _NOUN
	{
		FWULONG idNoun;				// the id
		FWSTRING theNoun;			// the noun
		FWSTRING theGeneric;		// the generic verb name
		_VERB *pGeneric;			// pointer to the generic Verb (initially NULL, then settled in accordnace to theGeneric)
		vector<_VERB*> vecVerbs;	// vector of verbs
		MAP mapVerbs;				// maps verb names into id's (vector indices)
	};
	struct _VERB
	{
		FWULONG idVerb;				// the id
		FWSTRING theVerb;			// the verb
		vector<_CLASS*> vecClasses;	// vector of classes
		MAP mapClasses;				// maps class names into id's (vector indices)
		//vector<_ARG*> vecArgs;	// vector of arguments
		//MAP mapArgs;				// maps arg names into id's (vector indices)
	};
	struct _CLASS
	{
		FWULONG idClass;		// the id
		FWSTRING theClass;		// clsid
		IFWUnknown *pFactory;	// factory
	};
	// OBSOLETE
	//struct _ARG
	//{
	//	FWULONG idArg;				// the id (not the same as nIndex)
	//	FWSTRING theArg;				// arg name
	//	FWULONG nIndex;				// arg index
	//	enum ARGTYPES type;			// arg type
	//};
	vector<_NOUN*> m_vecNouns;		// vector of nouns
	MAP m_mapNouns;					// maps noun names into id's (vector indices)

public:
	CFWDevice();
	~CFWDevice();

	friend class CFWCreateContext;
};

class CFWCreateContext : public FWUNKNOWN<IFWCreateContext, IID_IFWCreateContext, IFWCreateContext>
{
public:
	CFWCreateContext();
	CFWCreateContext(IFWDevice *pIFWDevice, CFWDevice::_NOUN*, CFWDevice::_VERB*, FWULONG nParams, FWPARAM *pParams);
	~CFWCreateContext();

	FW_RTTI(FWCreateContext)

	virtual HRESULT __stdcall GetContext(/*[out]*/ FWSTRING *pNoun, /*[out]*/ FWSTRING *pVerb);
	virtual HRESULT __stdcall GetContextId(/*[out]*/ FWULONG *pIdNoun, /*[out]*/ FWULONG *pIdVerb);

	virtual HRESULT __stdcall Create(REFIID iid, /*[out, retval, iid_is(iid)]*/ IFWUnknown**);
	virtual HRESULT __stdcall CreateUnknown(/*[out, retval]*/ IFWUnknown**);

	virtual HRESULT __stdcall EnumParams(/*[out, retval]*/ IFWEnumParams **p);
	virtual HRESULT __stdcall EnumParamsEx(FWSTRING pName, /*[out, retval]*/ IFWEnumParams **p);

	virtual HRESULT __stdcall SubmitULONG(FWULONG val);
	virtual HRESULT __stdcall SubmitFLOAT(FWFLOAT val);
	virtual HRESULT __stdcall SubmitSTRING(FWSTRING val);
	virtual HRESULT __stdcall SubmitVECTOR(FWVECTOR val);
	virtual HRESULT __stdcall SubmitPUNKNOWN(FWPUNKNOWN val);
	virtual HRESULT __stdcall SubmitPBONE(FWPUNKNOWN val);
	virtual HRESULT __stdcall SubmitPBODY(FWPUNKNOWN val);

	virtual HRESULT __stdcall Reject();

private:
	// Creation Context
	IFWDevice *m_pIFWDevice;
	CFWDevice::_NOUN *m_pNoun;
	CFWDevice::_VERB *m_pVerb;

	// The Created Object
	IFWUnknown *m_pUnknown;

	// The Params
	FWULONG m_nParams;
	FWPARAM *m_pParams;
	vector<FWPARAM> m_vecParams;

	// Map of the named Params
	struct STRLESS
	{
		bool operator()(LPOLESTR p, LPOLESTR q) const 
		{	if (!p) return false; if (!q) return true;
			while(*p == *q && *q) ++p, ++q;
			return *p - *q < 0;
		} 
	};
	struct NAME { ULONG nStart, nLen; };
	typedef map<std::wstring, NAME> MAP;
	MAP m_mapNames;
	bool m_bNamesMapped;

	// Reference objects - to be used by the FWEnumParams
	IKineNode *m_pRefNode;
	IBody *m_pRefBody;

	friend class CFWEnumParams;

private:
	void MapNamedParams();

	HRESULT ErrorHelper(FWSTRING pSrcFile, FWULONG nSrcLine, HRESULT e);
	HRESULT ErrorHelper(FWSTRING pSrcFile, FWULONG nSrcLine, HRESULT e, FWSTRING pParamName);
	HRESULT ErrorHelper(FWSTRING pSrcFile, FWULONG nSrcLine, HRESULT e, FWULONG num, FWULONG nFrom, FWULONG nTo);
};

///////////////////////////////////////////////////////////
//
// CFWEnumParams Inline Class Definition

class CFWEnumParams : public FWUNKNOWN<IFWEnumParams, IID_IFWEnumParams, IFWEnumParams >
{
protected:
	CFWCreateContext *m_pContext;
	ULONG m_nStart;
	ULONG m_nLen;
	ULONG m_nCur;

	IKineNode *m_pRefNode;
	IBody *m_pRefBody;
	FW_PARAM m_nTypeExpected;	// needed to generate error info
	FW_PARAM m_nTypeFound;		// needed to generate error info

public:
	CFWEnumParams();
	CFWEnumParams(CFWCreateContext *pContext, ULONG nStart, ULONG nLen, ULONG nCur = 0);
	~CFWEnumParams();

	virtual FWULONG __stdcall GetNum()				{ return m_nLen - m_nCur; }
	virtual FWULONG __stdcall GetIndex()			{ return m_nCur; }
	virtual void __stdcall PutIndex(FWULONG nCur)	{ m_nCur = nCur; }

	virtual HRESULT __stdcall QueryParam(/*[out]*/ FWPARAM **pParam);
	virtual HRESULT __stdcall QueryLONG(/*[out]*/ FWLONG *val);
	virtual HRESULT __stdcall QueryULONG(/*[out]*/ FWULONG *val);
	virtual HRESULT __stdcall QueryFLOAT(/*[out]*/ FWFLOAT *val);
	virtual HRESULT __stdcall QuerySTRING(/*[out]*/ FWSTRING *val);
	virtual HRESULT __stdcall QueryVECTOR(/*[out]*/ FWVECTOR *val);
	virtual HRESULT __stdcall QueryPUNKNOWN(REFIID iid, /*[out, iid_is(iid)]*/ FWPUNKNOWN *val);
	virtual HRESULT __stdcall QueryPBONE(REFIID iid, /*[out, iid_is(iid)]*/ FWPUNKNOWN *val);
	virtual HRESULT __stdcall QueryPBODY(REFIID iid, /*[out, iid_is(iid)]*/ FWPUNKNOWN *val);

	virtual HRESULT __stdcall Skip(FWLONG nNum);
	virtual HRESULT __stdcall Reset();
	virtual HRESULT __stdcall Clone(/*[out]*/ IFWEnumParams **ppEnum);

	virtual HRESULT __stdcall ErrorEx(FWSTRING pSrcFile, FWULONG nSrcLine, HRESULT h);

	FW_RTTI(FWEnumParams)
};

#endif