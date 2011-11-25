// body.cpp : Defines the Body class
//

#include "stdafx.h"
#include "body.h"
#include <string>
#include <map>

using namespace std;

///////////////////////////////////////////////////////////
// Internal body model

// The ruler for the tables below:
//  NULL OBJT ROOT PELV SPINE NECK  HEAD  JAW   BEAK  HORN  PONYT CLAVL ARM   HAND  FINGR TAIL  HIP   LEG   FOOT  SPUR  TOE   LAST

static FWULONG BODYNUM[] =	// number of body parts
{	 1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    2,    2,    2,   10,    1,    2,    2,    2,    2,   10 };
static FWULONG BODYSEG[] =	// capacity for segments per body part
{    1,    1,    1,    1,    8,    8,    1,    1,    1,    8,    8,    1,    3,    1,    3,    8,    1,    3,    1,    4,    3 };
static FWULONG BODYIND[] =	// indexes for body
{    0,    1,    2,    3,    4,   12,   20,   21,   22,   23,   31,   39,   41,   47,   49,   79,   87,   89,   95,   97,  105,  135 };

static const FWULONG BODYSIZE = BODYIND[BODY_LAST >> 16];

///////////////////////////////////////////////////////////
// CBody

CBody::CBody() : m_ppParts(NULL)
{
	m_ppParts = new IKineChild*[BODYSIZE];
	memset(m_ppParts, 0, sizeof(IKineChild*) * BODYSIZE);
}

CBody::~CBody()
{
	if (m_ppParts)
	{
		RemoveAll();
		delete [] m_ppParts;
	}
}

int CBody::CalcIndex(FWULONG nIndex)
{
	FWULONG nId = nIndex >> 16;
	FWULONG nInd = (nIndex >> 8) & 0xFF;
	FWULONG nSeg = nIndex & 0xFF;

	if (nId >= BODY_LAST || nInd >= BODYNUM[nId] || nSeg >= BODYSEG[nId]) return -1;
	
	return BODYIND[nId] + BODYSEG[nId] * nInd + nSeg;
}

///////////////////////////////////////////////////////////
// Loading body parts from an external model

HRESULT CBody::LoadBody(IKineNode *pBody, FWULONG nScheme)
{
	// Temporarily LEFT swapped with RIGHT (to cover a problem with DXRenderer)
	static FWSTRING DISCREET[] = 
	{
		L"Null", 
		L"Null", 
		L"Null", 
		L"Pelvis", 
		L"Spine", L"Spine1", L"Spine2", L"Spine3", L"Spine4", 0, 0, 0,
		L"Neck", L"Neck1", L"Neck2", L"Neck3", L"Neck4", 0, 0, 0,
		L"Head", 0, 0,
		L"PonyTail1", L"PonyTail11", L"PonyTail12", L"PonyTail13", L"PonyTail14", 0, 0, 0,
		L"PonyTail2", L"PonyTail21", L"PonyTail22", L"PonyTail23", L"PonyTail24", 0, 0, 0,
		L"L Clavicle", L"R Clavicle", L"L UpperArm", L"L Forearm", 0, L"R UpperArm", L"R Forearm", 0, 
		L"L Hand", L"R Hand",
		L"L Finger0", L"L Finger01", L"L Finger02", L"R Finger0", L"R Finger01", L"R Finger02", 
		L"L Finger1", L"L Finger11", L"L Finger12", L"R Finger1", L"R Finger11", L"R Finger12", 
		L"L Finger2", L"L Finger21", L"L Finger22", L"R Finger2", L"R Finger21", L"R Finger22", 
		L"L Finger3", L"L Finger31", L"L Finger32", L"R Finger3", L"R Finger31", L"R Finger32", 
		L"L Finger4", L"L Finger41", L"L Finger42", L"R Finger4", L"R Finger41", L"R Finger42",
		L"Tail", L"Tail1", L"Tail2", L"Tail3", L"Tail4", 0, 0, 0,
		0, 0, L"L Thigh", L"L Calf", L"L HorseLink", L"R Thigh", L"R Calf", L"R HorseLink", 
		L"L Foot", L"R Foot",
		0, 0, 0, 0, 0, 0, 0, 0, 
		L"L Toe0", L"L Toe01", L"L Toe02", L"R Toe0", L"R Toe01", L"R Toe02", 
		L"L Toe1", L"L Toe11", L"L Toe12", L"R Toe1", L"R Toe11", L"R Toe12", 
		L"L Toe2", L"L Toe21", L"L Toe22", L"R Toe2", L"R Toe21", L"R Toe22", 
		L"L Toe3", L"L Toe31", L"L Toe32", L"R Toe3", L"R Toe31", L"R Toe32", 
		L"L Toe4", L"L Toe41", L"L Toe42", L"R Toe4", L"R Toe41", L"R Toe42"
	};

	RemoveAll();

	FWSTRING *pStrings = NULL;
	FWULONG nSize = 0;
	switch (nScheme)
	{
	case BODY_SCHEMA_DISCREET:
		pStrings = DISCREET;
		nSize = sizeof(DISCREET) / sizeof(FWSTRING);
		break;
	}
	if (!pStrings)
		return ERROR(E_NOTIMPL);
	if (!pBody)
		return S_OK;

	typedef map<wstring, FWULONG> MAP;
	MAP map;
	for (FWULONG i = 0; i < nSize; i++)
		if (pStrings[i])
			map[pStrings[i]] = i;

	IKineChild *pChild = NULL;
	IKineEnumChildren *pEnum = NULL;
	pBody->EnumAllDescendants(&pEnum);
	while (pEnum->Next(&pChild) == S_OK)
	{
		LPOLESTR pLabel;
		pChild->GetLabel(&pLabel);
		MAP::iterator i = map.find(pLabel);
		if (i != map.end())
			m_ppParts[i->second] = pChild;
		else
			pChild->Release();
	}
	pEnum->Release();

	IKineNode *pRoot = NULL, *pObj = NULL;
	if (m_ppParts[CalcIndex(BODY_PELVIS)])
		m_ppParts[CalcIndex(BODY_PELVIS)]->GetParent(&pRoot);
	if (pRoot) 
	{
		pRoot->QueryInterface(&m_ppParts[CalcIndex(BODY_ROOT)]);
		pRoot->GetParent(&pObj);
		if (pObj)
		{
			pObj->QueryInterface(&m_ppParts[CalcIndex(BODY_OBJECT)]);
			pObj->Release();
		}
		pRoot->Release();
	}

	return S_OK;
}

HRESULT CBody::LoadBodyPart(IKineNode *pNode, FWSTRING pLabel, FWULONG nIndex)
{
	int i = CalcIndex(nIndex);
	if (i < 0) return ERROR(FW_E_BADINDEX);

	if (m_ppParts)
	{
		if (m_ppParts[i]) m_ppParts[i]->Release();
		m_ppParts[i] = NULL;
		if (pLabel)
			pNode->CheckChild(pLabel, &m_ppParts[i]);	// quiet version of GetChild
		else
		{
			m_ppParts[i] = pNode;
			if (pNode) pNode->AddRef();
		}
	}
	return S_OK;
}

HRESULT CBody::RemoveAll()
{
	if (m_ppParts)
	{
		for (FWULONG i = 0; i < BODYSIZE; i++)
			if (m_ppParts[i]) m_ppParts[i]->Release();
		memset(m_ppParts, 0, sizeof(IKineChild*) * BODYSIZE);
	}
	return S_OK;
}

///////////////////////////////////////////////////////////
// Getting body parts

HRESULT CBody::GetBodyPart(FWULONG nIndex, REFIID iid, /*[out, retval, iid_is(iid)]*/ IFWUnknown **p)
{
	int i = CalcIndex(nIndex);
	if (i < 0) return ERROR(FW_E_BADINDEX);
	if (!p) return ERROR(FW_E_POINTER);

	*p = NULL;
	if (m_ppParts[i])
		return m_ppParts[i]->QueryInterface(iid, (void**)p);
	else
		return S_OK;
}

IFWUnknown *CBody::BodyPart(FWULONG nIndex, REFIID iid)
{
	if (CalcIndex(nIndex) < 0) return NULL;
	IFWUnknown *pUnknown = NULL;
	GetBodyPart(nIndex, iid, (IFWUnknown**)&pUnknown);
	return pUnknown;
}

IKineChild *CBody::BodyChild(FWULONG nIndex)
{
	if (CalcIndex(nIndex) < 0) return NULL;
	IKineChild *pChild = NULL;
	GetBodyPart(nIndex, IID_IKineChild, (IFWUnknown**)&pChild);
	return pChild;
}

IKineNode *CBody::BodyNode(FWULONG nIndex)
{
	if (CalcIndex(nIndex) < 0) return NULL;
	IKineNode *pNode = NULL;
	GetBodyPart(nIndex, IID_IKineNode, (IFWUnknown**)&pNode);
	return pNode;
}

IBounding  *CBody::BodyBounding(FWULONG nIndex)
{
	if (CalcIndex(nIndex) < 0) return NULL;
	IBounding *pBounding = NULL;
	IKineNode *pNode = BodyNode(nIndex);
	if (pNode)
	{
		IKineChild *pChild = NULL;
		if (pNode->CheckChild(L"bound", &pChild) == S_OK)
		{
			pChild->QueryInterface(&pBounding);
			pChild->Release();
		}
		pNode->Release();
	}
	return pBounding;
}

///////////////////////////////////////////////////////////
// Queries

HRESULT CBody::GetInfo(FWULONG nQuery, FWULONG nIndex, /*[out, retval]*/ FWULONG*)
{
	return ERROR(E_NOTIMPL);
}

HRESULT CBody::GetFInfo(FWULONG nQuery, FWULONG nIndex, /*[out, retval]*/ FWFLOAT*)
{
	return ERROR(E_NOTIMPL);
}

HRESULT CBody::GetXYZ(FWULONG nIndex, /*[out, retval]*/ FWVECTOR *pv)
{
	memset(pv, 0, sizeof(FWVECTOR));
	IKineChild *p = BodyChild(nIndex);
	if (p == NULL) return ERROR(FW_E_BADINDEX);
	HRESULT h = p->LtoG(pv);
	p->Release();
	return h;
}

HRESULT CBody::CreateCompatibleTransform(/*[out, retval]*/ ITransform**p)
{
	IKineChild *pPelvis = BodyChild(BODY_PELVIS);
	if (!pPelvis) return Error(FW_E_NOTREADY);
	HRESULT h = pPelvis->CreateCompatibleTransform(p);
	pPelvis->Release();
	return h;
}
