
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include "FWStarter.h"
#include "ChildView.h"

#include "freewill.c"
#include "freewilltools.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DEG2RAD(d)	( (d) * (FWFLOAT)M_PI / 180.0f )
#define RAD2DEG(r)	( 180.0f * (r) / (FWFLOAT)M_PI )

// CChildView

CChildView::CChildView()
{
	m_bFWDone = false;
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_COMMAND(ID_ACTIONS_ACTION1, &CChildView::OnActionsAction1)
	ON_WM_TIMER()
	ON_WM_CREATE()
	ON_COMMAND(ID_ACTIONS_ACTION2, &CChildView::OnActionsAction2)
	ON_COMMAND(ID_ACTIONS_ACTION3, &CChildView::OnActionsAction3)
	ON_COMMAND(ID_ACTIONS_ACTION4, &CChildView::OnActionsAction4)
	ON_COMMAND(ID_ACTIONS_ACTION5, &CChildView::OnActionsAction5)
	ON_WM_KEYDOWN()
	ON_COMMAND(ID_ACTIONS_ACTION6, &CChildView::OnActionsAction6)
END_MESSAGE_MAP()



// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

	return TRUE;
}

void CChildView::OnTimer(UINT_PTR nIDEvent)
{
	FWULONG nMSec;
	m_pRenderer->GetPlayTime(&nMSec);
	m_pActionTick->RaiseEvent(nMSec, EVENT_TICK, nMSec, 0);
	if (!m_pActionTick->AnySubscriptionsLeft())
		m_pRenderer->Stop();

	InvalidateRect(NULL, FALSE);
	CWnd::OnTimer(nIDEvent);
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this);
	if SUCCEEDED(m_pRenderer->BeginFrame())
	{
		m_pRenderer->Clear();
		m_pScene->Render(m_pRenderer);
		m_pRenderer->EndFrame();
	}
}

	#define MB_CANCELTRYCONTINUE        0x00000006L
	#define IDTRYAGAIN      10
	#define IDCONTINUE      11
	HRESULT __stdcall HandleErrors(struct FWERROR *p, BOOL bRaised)
	{
		if (!bRaised)
		{
			TRACE("Last error recovered\n");
			return S_OK;
		}

		FWSTRING pLabel = NULL;
		if (p->pSender)
		{
			IKineChild *pChild;
			if (SUCCEEDED(p->pSender->QueryInterface(&pChild)) && pChild)
			{
				pChild->GetLabel(&pLabel);
				pChild->Release();
			}
		}

		CString str;
		if (pLabel)
			str.Format(L"%ls(%d)\nError 0x%x (class %ls, object %ls)\n%ls\n", p->pSrcFile, p->nSrcLine, p->nCode & 0xffff, p->pClassName, pLabel, p->pMessage);
		else
			str.Format(L"%ls(%d)\nError 0x%x (class %ls)\n%ls\n", p->pSrcFile, p->nSrcLine, p->nCode & 0xffff, p->pClassName, p->pMessage);
		TRACE(str);
		switch (AfxMessageBox(str, MB_CANCELTRYCONTINUE | MB_DEFBUTTON3))
		{
		case IDCANCEL: FatalAppExit(0, L"Application stopped"); break;
		case IDTRYAGAIN: DebugBreak(); break;
		case IDCONTINUE: break;
		}
		return p->nCode;
	}

int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	enum eError { ERROR_FREEWILL, ERROR_DIRECTX, ERROR_INTERNAL };
	try
	{
		// FreeWill: create the FreeWill device
		HRESULT h;
		h = CoCreateInstance(CLSID_FWDevice, NULL, CLSCTX_INPROC_SERVER, IID_IFWDevice, (void**)&m_pFWDevice);
		if (FAILED(h)) throw ERROR_FREEWILL;
		TRACE(L"FreeWill+ system initialised successfully.\n");

		// Set up the error handler
		m_pFWDevice->SetUserErrorHandler(HandleErrors);

		// FreeWill: create & initialise the renderer
		h = m_pFWDevice->CreateObject(L"Renderer", IID_IRenderer, (IFWUnknown**)&m_pRenderer);
		if (FAILED(h)) throw ERROR_DIRECTX;
		h = m_pRenderer->InitDisplay(m_hWnd, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
		if (FAILED(h)) throw ERROR_INTERNAL;
		FWCOLOR back = { 0.33f, 0.33f, 0.33f };		// gray
		m_pRenderer->PutBackColor(back);
		TRACE(L"Renderer started successfully.\n");

		// #FreeWill: create & initialise the buffers - determine hardware factors
		IMeshVertexBuffer *pVertexBuffer;
		IMeshFaceBuffer *pFaceBuffer;
		h = m_pRenderer->GetBuffers(&pVertexBuffer, &pFaceBuffer); 
		if (FAILED(h)) throw ERROR_INTERNAL;
		h = pVertexBuffer->Create(300000, MESH_VERTEX_XYZ | MESH_VERTEX_NORMAL | MESH_VERTEX_BONEWEIGHT | MESH_VERTEX_TEXTURE, 4, 1);
		if (FAILED(h)) throw ERROR_INTERNAL;
		h = pFaceBuffer->Create(300000); if (FAILED(h)) throw ERROR_INTERNAL;
		pVertexBuffer->Release();
		pFaceBuffer->Release();

		// #FreeWill: create & initialise the animation scene
		h = m_pFWDevice->CreateObject(L"Scene", IID_IScene, (IFWUnknown**)&m_pScene);
		if (FAILED(h)) throw ERROR_INTERNAL;

		// #FreeWill: create & initialise the character body
		h = m_pFWDevice->CreateObject(L"Body", IID_IBody, (IFWUnknown**)&m_pBody);
		if (FAILED(h)) throw ERROR_INTERNAL;

		// #FreeWill: initialise the Tick Actions
		m_pActionTick = (IAction*)FWCreateObject(m_pFWDevice, L"Action", L"Generic", (IUnknown*)NULL);

		m_pScene->PutRenderer(m_pRenderer);

		// #Load the Scene
		IFileLoader *pLoader;
		m_pFWDevice->CreateObject(L"FileLoader", IID_IFileLoader, (IFWUnknown**)&pLoader);
		h = pLoader->LoadScene((LPOLESTR)(LPCOLESTR)(L"scene.3D"), m_pScene);
		if (FAILED(h)) throw ERROR_INTERNAL;
		pLoader->Release();
		TRACE(L"Scene loaded.\n");

		// Load Body Object
		IKineNode *pBody = NULL;
		if (SUCCEEDED(m_pScene->GetChild(L"Bip01", (IKineChild**)&pBody)) && pBody)
		{
			m_pBody->LoadBody(pBody, BODY_SCHEMA_DISCREET);
			pBody->Release();
		}

		// setup lights
		//m_pFWDevice->CreateObject(L"DirLight", IID_ISceneLightDir, (IFWUnknown**)&m_pLight1);
		//m_pScene->AddChild(L"DirLight1", m_pLight1);
		//FWCOLOR cWhite1 = { 0.7f, 0.7f, 0.7f };
		//m_pLight1->PutDiffuseColor(cWhite1);
		//FWVECTOR v1 = {0.1f, -0.3f, -0.4f};
		//m_pLight1->Create(v1);

		//m_pFWDevice->CreateObject(L"DirLight", IID_ISceneLightDir, (IFWUnknown**)&m_pLight2);
		//m_pScene->AddChild(L"DirLight2", m_pLight2);
		//FWCOLOR cWhite2 = { 0.6f, 0.6f, 0.6f };
		//m_pLight2->PutDiffuseColor(cWhite2);
		//FWVECTOR v2 = {0.1f, -0.3f, -0.4f};
		//m_pLight2->Create(v2);

		//FWCOLOR cAmb = { 0.35f, 0.35f, 0.35f };
		//m_pRenderer->SetAmbientLight(cAmb);
	}
	catch (eError e)
	{
		CString str;
		switch (e)
		{
		case ERROR_FREEWILL: str = L"FreeWill Graphics system not found. Please reinstall the application."; break;
		case ERROR_DIRECTX:  str = L"The Direct3D renderer could not be initialised. Please update or re-install DirectX."; break;
		case ERROR_INTERNAL: str = L"FreeWill Graphics System could not be initialised. Contact the Technical Support";  break;
		default:             str = L"Unidentified internal error occured. Contact the Technical Support";  break;
		}
		//str += L" This is a fatal error and the application will now shut down.";
		AfxMessageBox(str, MB_OK | MB_ICONHAND);
	}

	m_bFWDone = true;
	SetTimer(101, 1000 / 50, NULL);

	return 0;
}

void CChildView::OnActionsAction1()
{
	IFWUnknown *p = NULL;
	FWULONG nDur = 150;

	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Walk", m_pActionTick, p, nDur, L"open", m_pBody, -46.0f, -57.0f, 12);
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Walk", m_pActionTick, p, nDur, L"open", m_pBody, 118.0f, 50.0f, 12);
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Walk", m_pActionTick, p, nDur, L"open", m_pBody, -46.0f, 50.0f, 12);
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Walk", m_pActionTick, p, nDur, L"close", m_pBody, 118.0f, -57.0f, 12);

	m_pRenderer->Play();
}



void CChildView::OnActionsAction2()
{
	IFWUnknown *p = NULL;
	FWULONG nDur = 150;

	ITransform *pT;
	m_pBody->CreateCompatibleTransform(&pT);
	pT->FromRotationZ(1);
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Rotate", m_pActionTick, p, 500, m_pBody, BODY_ARM, pT);
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Rotate", m_pActionTick, p, 500, m_pBody, BODY_LEG+BODY_LEFT, pT);
	pT->FromRotationZ(-2);
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Rotate", m_pActionTick, p, 1000, m_pBody, BODY_ARM, pT);
	pT->FromRotationZ(1);
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Rotate", m_pActionTick, p, 500, m_pBody, BODY_ARM, pT);

	m_pRenderer->Play();
}


void CChildView::OnActionsAction3()
{
	IFWUnknown *p = NULL;
	FWULONG nDur = 150;

	IKineChild *pWall = NULL;
	m_pScene->GetChild(L"Box03", &pWall);
	FWVECTOR v = { 40, -40, 30 };
		
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Walk", m_pActionTick, p, nDur, L"close", m_pBody, 80.0f, 20.0f, 12);
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Turn", m_pActionTick, p, nDur, L"close", m_pBody, DEG2RAD(-120), 3);
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Point", m_pActionTick, 2500, 500, L"", m_pBody, pWall, v);
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Wait", m_pActionTick, p, 500, L"", m_pBody, 20000);
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Walk", m_pActionTick, p, nDur, L"open", m_pBody, -46.0f, -57.0f, 12);

	m_pRenderer->Play();
}


void CChildView::OnActionsAction4()
{
	// delete the wall...
	IKineChild *pChild = NULL;
	ISceneObject *pObj = NULL;
	if (SUCCEEDED(m_pScene->GetChild(L"Box03", &pChild)) && pChild && SUCCEEDED(pChild->QueryInterface(&pObj)))
		pObj->PutVisible(FALSE);
	if (pChild) pChild->Release(); 
	if (pObj) pObj->Release();
}


void CChildView::OnActionsAction5()
{
	IFWUnknown *p = NULL;
	FWULONG nDur = 150;

	IKineChild *pWall = NULL;
	m_pScene->GetChild(L"Box03", &pWall);
	FWVECTOR v = { 40, -40, 30 };
		
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Bend", m_pActionTick, p, nDur, m_pBody);

	m_pRenderer->Play();
}


void CChildView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);

	IFWUnknown *p = NULL;
	FWULONG nDur = 150;
	switch (nChar)
	{
	case 'A':
		p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur, L"", m_pBody, 0.5, DEG2RAD(-30), DEG2RAD(5));
		break;
	case 'D':
		p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur, L"", m_pBody, 0.5, DEG2RAD(30), DEG2RAD(5));
		break;
	case 'W':
		p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur, L"", m_pBody, 0.5, DEG2RAD(0), DEG2RAD(5));
		break;
	case 'S':
		p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Turn", m_pActionTick, p, nDur, L"close", m_pBody, DEG2RAD(-120), 3);
		break;
	}
	m_pRenderer->Play();


}


void CChildView::OnActionsAction6()
{
	// generic action to start after 5 seconds from start of the system, for 2 seconds
	IFWUnknown *p = NULL;
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Generic", m_pActionTick, 5000, 2000, L"");

	// another generic action, to start immediately after, for 1 second
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Generic", m_pActionTick, p, 1000, L"");



	FWULONG nDur = 150;
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Turn", m_pActionTick, p, nDur, L"close", m_pBody, DEG2RAD(-120), 3);
	m_pRenderer->Play();
}
