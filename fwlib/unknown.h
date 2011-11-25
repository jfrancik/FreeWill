// UNKNOWN: a Mini Library for DLL COM Servers
// Facilitates creating COM DLL server modules without use of ATL
//
// Provides:
//	- template-based UNKNOWN base class with full implementation for IUnknown interface
//
// Does not require linking to CPP or importing LIB files
// long g_cComponents variable must be defined in the module unless LIB file imported (see unknown.cpp)
//
// Copyright (C) 2003-2005 by Jarek Francik
/////////////////////////////////////////////////////////////////////////////

#if !defined(_UNKNOWN__)
#define _UNKNOWN__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <objbase.h>

/////////////////////////////////////////////////////////
// Global Variable
extern long g_cComponents;		// Component counter: global component counter,
								// InterlockedIncrement(&g_cComponents) called in UNKNOWN constructor
								// InterlockedDecrement(&g_cComponents) called in UNKNOWN destructor

/////////////////////////////////////////////////////////
// Template-based UNKNOWN base class
// Just derive your own class from UNKNOWN to gain from
// ready-to-use implementation of QueryInterface, AddRef and Release.
// Provide all the interfaces from which you derive the class,
// along with appropriate IIDs, as UNKNOWN template parameters.
//
// See fwunknown.h file for specialised templates for use with FreeWill+ Interfaces.
//
// Usage:
//
//  // for just one interface:
//  class YourClass : public UNKNOWN <IYourInterface, IID_IYourInterface, IYourInterface> { ... };
//
//  // for up to 8 interfaces:
//  class YourClass : public UNKNOWN<
//							// here is the list of your base interfaces:
//							BASECLASS<IYourInterface1, IYourInterface2, IYourInterface3, .....>
//							// here is the list of all the implemented interfaces:
//							IID_IYourInterface1, IYourInterface1,		// always top-level interface here
//							IID_IYourInterface2, IYourInterface2,
//							IID_IYourInterface3, IYourSuperInterface,	
									// IYourSuperInterfaceis may be a top-level interface derived from IYourInterface3
//							......   >
// { ... };
//
// Remarks:
// In the list of the base interfaces:
// - All and only the top-level interfaces should be listed.
//   Top-level interfaces are those that have no derived interface 
//   among the interfaces implemented by the class.
// In the list of implemented interfaces:
// - All implemented IID's besides IID_IUnknown should be listed.
// - IID's may be coupled with their associated interfaces or any derived interfaces.
// - Ambigous interface names must be replaced with their unambigous derivatives.
//   An ambigous interface is an interface that has more than one top-level interface
//   among the interfaces implemented by the class (the hierarchy of their derivatives
//   is branched).
//
// Example:
//	There are given following interfaces:
//	interface IA;
//	interface IB;
//	interface IBA : IB;
//	interface IBB : IB;
//  class CImpl : public UNKNOWN<
//							BASECLASS<IA, IBA, IBB>,
//							IID_IA,  IA,
//							IID_IB,  IBA,
//							IID_IBA, IBA,
//							IID_IBB, IBB>
//	{ .... };

template<int> class __DUMMY { };	// not to be used directly

template <class BASE1 = __DUMMY<101>,
		  class BASE2 = __DUMMY<102>,
		  class BASE3 = __DUMMY<103>,
		  class BASE4 = __DUMMY<104>,
		  class BASE5 = __DUMMY<105>,
		  class BASE6 = __DUMMY<106>,
		  class BASE7 = __DUMMY<107>,
		  class BASE8 = __DUMMY<108> >
class BASECLASS : public BASE1, public BASE2, public BASE3, public BASE4, 
				  public BASE5, public BASE6, public BASE7, public BASE8
{
};

template <class BASECLASS,
          REFIID IID1 = IID_IUnknown, class INTERFACE1 = __DUMMY<1>,
		  REFIID IID2 = IID_IUnknown, class INTERFACE2 = __DUMMY<2>,
		  REFIID IID3 = IID_IUnknown, class INTERFACE3 = __DUMMY<3>,
		  REFIID IID4 = IID_IUnknown, class INTERFACE4 = __DUMMY<4>,
		  REFIID IID5 = IID_IUnknown, class INTERFACE5 = __DUMMY<5>,
		  REFIID IID6 = IID_IUnknown, class INTERFACE6 = __DUMMY<6>,
		  REFIID IID7 = IID_IUnknown, class INTERFACE7 = __DUMMY<7>,
		  REFIID IID8 = IID_IUnknown, class INTERFACE8 = __DUMMY<8> >
class UNKNOWN : public BASECLASS,
					public __DUMMY< 1>, public __DUMMY< 2>, public __DUMMY< 3>, public __DUMMY< 4>, 
					public __DUMMY< 5>, public __DUMMY< 6>, public __DUMMY< 7>, public __DUMMY< 8>
{
protected:
	long m_nRef;

	typedef INTERFACE1 ITFirst;

public:
	UNKNOWN() : m_nRef(1)
	{
		m_nRef = 1;
		InterlockedIncrement(&g_cComponents);
	}

	virtual ~UNKNOWN()
	{
		InterlockedDecrement(&g_cComponents);
	}

	HRESULT _stdcall QueryInterface(REFIID iid, void **ppv)
	{
		if (iid == IID_IUnknown)
			*ppv = static_cast<INTERFACE1*>(this);
		else if (iid == IID1)
			*ppv = static_cast<INTERFACE1*>(this);
		else if (iid == IID2)
			*ppv = static_cast<INTERFACE2*>(this);
		else if (iid == IID3)
			*ppv = static_cast<INTERFACE3*>(this);
		else if (iid == IID4)
			*ppv = static_cast<INTERFACE4*>(this);
		else if (iid == IID5)
			*ppv = static_cast<INTERFACE5*>(this);
		else if (iid == IID6)
			*ppv = static_cast<INTERFACE6*>(this);
		else if (iid == IID7)
			*ppv = static_cast<INTERFACE7*>(this);
		else if (iid == IID8)
			*ppv = static_cast<INTERFACE8*>(this);
		else
			return E_NOINTERFACE;
		((IUnknown*)(*ppv))->AddRef();
		return S_OK;
	}

	ULONG _stdcall AddRef()
	{
		return InterlockedIncrement(&m_nRef);
	}

	ULONG _stdcall Release()
	{
		if (InterlockedDecrement(&m_nRef) == 0)
		{
			delete this;
			return 0;
		}
		return m_nRef;
	}
};

#endif
