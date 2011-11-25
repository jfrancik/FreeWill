#ifndef __COMMON_FREEWILLTOOLS_H_
#define __COMMON_FREEWILLTOOLS_H_

typedef IKineChild *PBONE;
typedef IBody *PBODY;

struct CParam : public FWPARAM
{
	CParam()								{ m_nIndex = 0; m_pName = NULL; m_type = FW_PARAM_NONE; }

	CParam(FW_PARAM t, FWULONG nIndex = 0)	{ m_nIndex = nIndex; m_pName = NULL; m_type = t; memset(&m_VECTOR, 0, sizeof(m_VECTOR)); }
	CParam(int n, FWULONG nIndex = 0)		{ m_nIndex = nIndex; m_pName = NULL; m_type = FW_PARAM_ULONG; m_ULONG = (FWULONG)n; }
	CParam(FWULONG n, FWULONG nIndex = 0)	{ m_nIndex = nIndex; m_pName = NULL; m_type = FW_PARAM_ULONG; m_ULONG = n; }
	CParam(FWFLOAT f, FWULONG nIndex = 0)	{ m_nIndex = nIndex; m_pName = NULL; m_type = FW_PARAM_FLOAT; m_FLOAT = f; }
	CParam(double f, FWULONG nIndex = 0)	{ m_nIndex = nIndex; m_pName = NULL; m_type = FW_PARAM_FLOAT; m_FLOAT = (FWFLOAT)f; }
	CParam(FWSTRING s, FWULONG nIndex = 0)	{ m_nIndex = nIndex; m_pName = NULL; m_type = FW_PARAM_STRING; m_STRING = s; }
	CParam(FWVECTOR v, FWULONG nIndex = 0)	{ m_nIndex = nIndex; m_pName = NULL; m_type = FW_PARAM_VECTOR; m_VECTOR = v; }
	CParam(FWPUNKNOWN p, FWULONG nIndex = 0){ m_nIndex = nIndex; m_pName = NULL; m_type = FW_PARAM_PUNKNOWN; m_PUNKNOWN = p; }
	CParam(PBONE p, FWULONG nIndex = 0)		{ m_nIndex = nIndex; m_pName = NULL; m_type = FW_PARAM_PBONE; m_PBONE = p; }
	CParam(PBODY p, FWULONG nIndex = 0)		{ m_nIndex = nIndex; m_pName = NULL; m_type = FW_PARAM_PBODY; m_PBODY = p; }

	CParam(FW_PARAM t, FWSTRING pName)		{ m_nIndex = 0; m_pName = pName; m_type = t; memset(&m_VECTOR, 0, sizeof(m_VECTOR)); }
	CParam(int n, FWSTRING pName)			{ m_nIndex = 0; m_pName = pName; m_type = FW_PARAM_ULONG; m_ULONG = (FWULONG)n; }
	CParam(FWULONG n, FWSTRING pName)		{ m_nIndex = 0; m_pName = pName; m_type = FW_PARAM_ULONG; m_ULONG = n; }
	CParam(FWFLOAT f, FWSTRING pName)		{ m_nIndex = 0; m_pName = pName; m_type = FW_PARAM_FLOAT; m_FLOAT = f; }
	CParam(double f, FWSTRING pName)		{ m_nIndex = 0; m_pName = pName; m_type = FW_PARAM_FLOAT; m_FLOAT = (FWFLOAT)f; }
	CParam(FWSTRING s, FWSTRING pName)		{ m_nIndex = 0; m_pName = pName; m_type = FW_PARAM_STRING; m_STRING = s; }
	CParam(FWVECTOR v, FWSTRING pName)		{ m_nIndex = 0; m_pName = pName; m_type = FW_PARAM_VECTOR; m_VECTOR = v; }
	CParam(FWPUNKNOWN p, FWSTRING pName)	{ m_nIndex = 0; m_pName = pName; m_type = FW_PARAM_PUNKNOWN; m_PUNKNOWN = p; }
	CParam(PBONE p, FWSTRING pName)			{ m_nIndex = 0; m_pName = pName; m_type = FW_PARAM_PBONE; m_PBONE = p; }
	CParam(PBODY p, FWSTRING pName)			{ m_nIndex = 0; m_pName = pName; m_type = FW_PARAM_PBODY; m_PBODY = p; }
};

struct IFWDevice;
//inline HRESULT FWCreateDevice(IFWDevice **p)
//{
//	return CoCreateInstance(CLSID_FWDevice, NULL, CLSCTX_INPROC_SERVER, IID_IFWDevice, (void**)p);
//}

inline IFWUnknown *FWCreateObject(IFWDevice *pDevice, FWSTRING pNoun, FWSTRING pVerb,
			   CParam p1 = CParam(), CParam p2 = CParam(), CParam p3 = CParam(), CParam p4 = CParam(), 
			   CParam p5 = CParam(), CParam p6 = CParam(), CParam p7 = CParam(), CParam p8 = CParam())
{
	IFWUnknown *p;
	CParam params[] = { p1, p2, p3, p4, p5, p6, p7, p8 };
	int n;
	for (n = sizeof(params)/sizeof(FWPARAM) - 1; n >= 0 && params[n].m_type == FW_PARAM_NONE; n--)
		;

	HRESULT h;
	if (n >= 0) h = pDevice->CreateObjectEx(pNoun, pVerb, n+1, params, IID_IFWUnknown, &p);
	else h = pDevice->CreateObjectEx(pNoun, pVerb, 0, NULL, IID_IFWUnknown, &p);
	
	if SUCCEEDED(h) return p;
	else return NULL;
}

inline IFWUnknown *FWCreateObject(IFWDevice *pDevice, FWSTRING pNoun, FWSTRING pVerb,
			   CParam p1, CParam p2, CParam p3, CParam p4, 
			   CParam p5, CParam p6, CParam p7, CParam p8,
			   CParam p9, CParam p10 = CParam(), CParam p11 = CParam(), CParam p12 = CParam(), 
			   CParam p13 = CParam(), CParam p14 = CParam(), CParam p15 = CParam(), CParam p16 = CParam())
{
	IFWUnknown *p;
	CParam params[] = { p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16 };
	int n;
	for (n = sizeof(params)/sizeof(FWPARAM) - 1; n >= 0 && params[n].m_type == FW_PARAM_NONE; n--)
		;

	HRESULT h;
	if (n >= 0) h = pDevice->CreateObjectEx(pNoun, pVerb, n+1, params, IID_IFWUnknown, &p);
	else h = pDevice->CreateObjectEx(pNoun, pVerb, 0, NULL, IID_IFWUnknown, &p);
	
	if SUCCEEDED(h) return p;
	else return NULL;
}

inline IFWUnknown *FWCreateObject(IFWDevice *pDevice, FWSTRING pNoun, FWSTRING pVerb,
			   CParam p1, CParam p2, CParam p3, CParam p4, 
			   CParam p5, CParam p6, CParam p7, CParam p8,
			   CParam p9, CParam p10, CParam p11, CParam p12, 
			   CParam p13, CParam p14, CParam p15, CParam p16,
			   CParam p17, CParam p18 = CParam(), CParam p19 = CParam(), CParam p20 = CParam(), 
			   CParam p21 = CParam(), CParam p22 = CParam(), CParam p23 = CParam(), CParam p24 = CParam(),
			   CParam p25 = CParam(), CParam p26 = CParam(), CParam p27 = CParam(), CParam p28 = CParam(), 
			   CParam p29 = CParam(), CParam p30 = CParam(), CParam p31 = CParam(), CParam p32 = CParam())
{
	IFWUnknown *p;
	CParam params[] = { p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16,
		p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32 };
	int n;
	for (n = sizeof(params)/sizeof(FWPARAM) - 1; n >= 0 && params[n].m_type == FW_PARAM_NONE; n--)
		;
	
	HRESULT h;
	if (n >= 0) h = pDevice->CreateObjectEx(pNoun, pVerb, n+1, params, IID_IFWUnknown, &p);
	else h = pDevice->CreateObjectEx(pNoun, pVerb, 0, NULL, IID_IFWUnknown, &p);
	
	if SUCCEEDED(h) return p;
	else return NULL;
}

inline IFWUnknown *FWCreateObjWeakPtr(IFWDevice *pDevice, FWSTRING pNoun, FWSTRING pVerb,
			   CParam p1 = CParam(), CParam p2 = CParam(), CParam p3 = CParam(), CParam p4 = CParam(), 
			   CParam p5 = CParam(), CParam p6 = CParam(), CParam p7 = CParam(), CParam p8 = CParam())
{
	IFWUnknown *p = FWCreateObject(pDevice, pNoun, pVerb, p1, p2, p3, p4, p5, p6, p7, p8);
	if (p) p->Release();
	return p;
}

inline IFWUnknown *FWCreateObjWeakPtr(IFWDevice *pDevice, FWSTRING pNoun, FWSTRING pVerb,
			   CParam p1, CParam p2, CParam p3, CParam p4, 
			   CParam p5, CParam p6, CParam p7, CParam p8,
			   CParam p9, CParam p10 = CParam(), CParam p11 = CParam(), CParam p12 = CParam(), 
			   CParam p13 = CParam(), CParam p14 = CParam(), CParam p15 = CParam(), CParam p16 = CParam())
{
	IFWUnknown *p = FWCreateObject(pDevice, pNoun, pVerb, 
		p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16);
	if (p) p->Release();
	return p;
}

inline IFWUnknown *FWCreateObjWeakPtr(IFWDevice *pDevice, FWSTRING pNoun, FWSTRING pVerb,
			   CParam p1, CParam p2, CParam p3, CParam p4, 
			   CParam p5, CParam p6, CParam p7, CParam p8,
			   CParam p9, CParam p10, CParam p11, CParam p12, 
			   CParam p13, CParam p14, CParam p15, CParam p16,
			   CParam p17, CParam p18 = CParam(), CParam p19 = CParam(), CParam p20 = CParam(), 
			   CParam p21 = CParam(), CParam p22 = CParam(), CParam p23 = CParam(), CParam p24 = CParam(),
			   CParam p25 = CParam(), CParam p26 = CParam(), CParam p27 = CParam(), CParam p28 = CParam(), 
			   CParam p29 = CParam(), CParam p30 = CParam(), CParam p31 = CParam(), CParam p32 = CParam())
{
	IFWUnknown *p = FWCreateObject(pDevice, pNoun, pVerb, 
		p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, 
		p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32);
	if (p) p->Release();
	return p;
}

#endif