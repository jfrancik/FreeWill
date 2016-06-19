
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include "FWStarter.h"
#include "ChildView.h"

#include "Camera.h"
#include <Winuser.h>

#include "freewill.c"
#include "freewilltools.h"

#include <sstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DEG2RAD(d)	( (d) * (FWFLOAT)M_PI / 180.0f )
#define RAD2DEG(r)	( 180.0f * (r) / (FWFLOAT)M_PI )

// CChildView

CChildView::CChildView()
{
	m_pFWDevice = NULL;
	m_pRenderer = NULL;
	m_pScene = NULL;
	m_pLight1 = NULL;
	m_pLight2 = NULL;
	m_pCamera = NULL;
	m_pActionTick = NULL;
	m_bFWDone = false;

	m_pBody[0] = NULL;
	m_pBody[1] = NULL;
	m_pWalkAction[0] = NULL;
	m_pWalkAction[1] = NULL;
	m_nWalkState[0] = 0;
	m_nWalkState[1] = 0;
	m_nCurBody = 0;
}

CChildView::~CChildView()
{
	if (m_pBody[0]) m_pBody[0]->Release();
	if (m_pBody[1]) m_pBody[1]->Release();
	if (m_pLight1) m_pLight1->Release();
	if (m_pLight2) m_pLight2->Release();
	if (m_pCamera) delete m_pCamera;
	if (m_pActionTick) m_pActionTick->Release();
	if (m_pScene) m_pScene->Release();
	if (m_pRenderer) m_pRenderer->Release();
	if (m_pFWDevice) m_pFWDevice->Release();
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
	ON_WM_KEYUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_SCRL, &CChildView::OnUpdateIndicatorScrl)
END_MESSAGE_MAP()



// CChildView message handlers

void CChildView::OnPaint() 
{
	// Render the Scene
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

		// #FreeWill: create & initialise the character bodies
		h = m_pFWDevice->CreateObject(L"Body", IID_IBody, (IFWUnknown**)&m_pBody[0]);
		if (FAILED(h)) throw ERROR_INTERNAL;
		h = m_pFWDevice->CreateObject(L"Body", IID_IBody, (IFWUnknown**)&m_pBody[1]);
		if (FAILED(h)) throw ERROR_INTERNAL;

		// #FreeWill: initialise the Tick Actions
		m_pActionTick = (IAction*)FWCreateObject(m_pFWDevice, L"Action", L"Generic", (IUnknown*)NULL);

		m_pScene->PutRenderer(m_pRenderer);

		// #Load the Scene
		IFileLoader *pLoader;
		m_pFWDevice->CreateObject(L"FileLoader", IID_IFileLoader, (IFWUnknown**)&pLoader);
		h = pLoader->LoadScene((LPOLESTR)(LPCOLESTR)(L"scene_ball.3d"), m_pScene);
		if (FAILED(h)) throw ERROR_INTERNAL;
		pLoader->Release();
		TRACE(L"Scene loaded.\n");

		// Load and move Body Objects
		IKineNode *pBody1 = NULL, *pBody2 = NULL;
		if (SUCCEEDED(m_pScene->GetChild(L"Bip01", (IKineChild**)&pBody1)) && pBody1)
		{
			// load body 1
			m_pBody[0]->LoadBody(pBody1, BODY_SCHEMA_DISCREET);

			pBody1->Release();
			pBody1 = m_pBody[0]->BodyNode(BODY_OBJECT);

			// reproduce body
			pBody1->ReproduceEx(IID_IKineNode, (IFWUnknown**)&pBody2);
			m_pScene->AddChild(L"Bip02", pBody2);

			// load body 2
			m_pBody[1]->LoadBody(pBody2, BODY_SCHEMA_DISCREET);

			// rotate body 1
			ITransform *pT = NULL;
			pBody1->CreateCompatibleTransform(&pT);
			pT->FromIdentity();
			pT->MulRotationZ(DEG2RAD(180));
			pT->MulTranslationXYZ(-80, 0, 0);
			pBody1->PutLocalTransform(pT);

			// move body 2
			pT->FromIdentity();
			pT->MulTranslationXYZ(-560, 0, 0);
			pBody2->PutLocalTransform(pT);
			
			pT->Release();
			pBody1->Release();
			pBody2->Release();
		}

		// setup camera
		m_pCamera = new CCamera;
		m_pCamera->SetBaseBone(m_pScene);
		m_pCamera->Create();
		m_pCamera->Move(100, -120, 50);
		m_pCamera->Pan(DEG2RAD(70));
		m_pCamera->Tilt(DEG2RAD(-5));

		// move the ball
		IKineChild *pChild = NULL;
		if SUCCEEDED(m_pScene->GetChild(L"Sphere02", &pChild))
		{
			ITransform *pT = NULL;
			pChild->CreateCompatibleTransform(&pT);
			pT->MulTranslationXYZ(-110, -30, 0);
			pChild->PutLocalTransform(pT);
			pChild->Release();
		}

		// make unwanted scene object invisible
		ISceneObject *pObj = NULL;
		if (SUCCEEDED(m_pScene->GetChild(L"Sphere01", &pChild)) && pChild && SUCCEEDED(pChild->QueryInterface(&pObj)))
			pObj->PutVisible(FALSE);
		if (pChild) pChild->Release(); 
		if (pObj) pObj->Release();


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

		// Build the pitch
		ISceneObject *pPitch = NULL;
		IKineNode *pBone = NULL;
		IMesh *pMesh = NULL;
		IMaterial *pMaterial = NULL;
		ITexture *pTexture = NULL;
		ITransform *pT = NULL;
		m_pScene->NewObject(L"Pitch", &pPitch);
		pPitch->CreateChild(L"Pitch", &pBone);
		pPitch->NewMesh(L"Pitch_mesh", &pMesh);
		pMesh->Open(NULL, NULL);
		pMesh->InitAdvNormalSupport(0);
		
		FWULONG nVertex = 0;
		pMesh->SetVertexXYZ(nVertex, 0, 0, 0); pMesh->AddNormal(&nVertex, 0, 0, 1); pMesh->SetVertexTextureUV(nVertex, 0, 0, 0); nVertex++;
		pMesh->SetVertexXYZ(nVertex, 0, 1, 0); pMesh->AddNormal(&nVertex, 0, 0, 1); pMesh->SetVertexTextureUV(nVertex, 0, 0, 1); nVertex++;
		pMesh->SetVertexXYZ(nVertex, 1, 0, 0); pMesh->AddNormal(&nVertex, 0, 0, 1); pMesh->SetVertexTextureUV(nVertex, 0, 1, 0); nVertex++;
		pMesh->SetVertexXYZ(nVertex, 1, 1, 0); pMesh->AddNormal(&nVertex, 0, 0, 1); pMesh->SetVertexTextureUV(nVertex, 0, 1, 1); nVertex++;
		
		FWULONG nFace = 0;
		pMesh->SetFace(nFace, 0, 1, 2); nFace++;
		pMesh->SetFace(nFace, 1, 3, 2); nFace++;

		pMesh->InitAdvVertexBlending(0.01f, 0);
		for (FWULONG i = 0; i < nVertex; i++)
			pMesh->AddBlendWeight(i, 1.0f, L"Pitch");
		pMesh->Close();

		m_pFWDevice->CreateObject(L"Material", IID_IMaterial, (IFWUnknown**)&pMaterial);
		m_pRenderer->CreateTexture(&pTexture);
		pTexture->LoadFromFile(L"pitch.png");
		pTexture->SetUVTile(1, 1);
		pMaterial->SetTexture(0, pTexture);
		pMesh->SetMaterial(pMaterial);

		pBone->CreateCompatibleTransform(&pT);
		pT->FromIdentity();
		pT->MulScale(640, 400, 1);
		pT->MulTranslationXYZ(-640, -200, -2);
		pBone->PutLocalTransform(pT);

		
		pMaterial->Release();
		pTexture->Release();
		pMesh->Release();
		pPitch->Release();
		pBone->Release();
		pT->Release();

		// Start playing!
		m_pRenderer->Play();
		m_pRenderer->PutPlayTime(-3000);
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
	// Proceed with the Animation
	FWLONG nMSec;
	m_pRenderer->GetPlayTime(&nMSec);
	m_pActionTick->RaiseEvent(nMSec, EVENT_TICK, nMSec, 0);

	// release and delete all overdue (finished) walking actions
	for (int i = 0; i < 2; i++)
		if (m_pWalkAction[i])
			if (m_pWalkAction[i]->Release() == 0)
				m_pWalkAction[i] = NULL;
			else
				m_pWalkAction[i]->AddRef();

	// scan the keyboard
	if (GetFocus() == this)
	{
		// control the camera
		if (GetKeyState(VK_LEFT) < 0) m_pCamera->Move(1, 0, 0);
		if (GetKeyState(VK_RIGHT) < 0) m_pCamera->Move(-1, 0, 0);
		if (GetKeyState(VK_UP) < 0 && GetKeyState(VK_SHIFT) >= 0) m_pCamera->Move(0, 1, 0);
		if (GetKeyState(VK_DOWN) < 0 && GetKeyState(VK_SHIFT) >= 0) m_pCamera->Move(0, -1, 0);
		if (GetKeyState(VK_UP) < 0 && GetKeyState(VK_SHIFT) < 0) m_pCamera->Move(0, 0, 1);
		if (GetKeyState(VK_DOWN) < 0 && GetKeyState(VK_SHIFT) < 0) m_pCamera->Move(0, 0, -1);

		// control the characters
		FWULONG nDur = 150;
		if (m_pWalkAction[m_nCurBody] == NULL)	// only continue if the previous action is now finished
		{
			if (GetKeyState('A') < 0)
			{
				m_nWalkState[m_nCurBody] = 'A';
				m_pWalkAction[m_nCurBody] = (IAction*)FWCreateObject(m_pFWDevice, L"Action", L"Step", m_pActionTick, nMSec, nDur, L"", m_pBody[m_nCurBody], 0.5, DEG2RAD(-30), DEG2RAD(5));
			}
			else if (GetKeyState('D') < 0)
			{
				m_nWalkState[m_nCurBody] = 'D';
				m_pWalkAction[m_nCurBody] = (IAction*)FWCreateObject(m_pFWDevice, L"Action", L"Step", m_pActionTick, nMSec, nDur, L"", m_pBody[m_nCurBody], 0.5, DEG2RAD(30), DEG2RAD(5));
			}
			else if (GetKeyState('W') < 0)
			{
				m_nWalkState[m_nCurBody] = 'W';
				m_pWalkAction[m_nCurBody] = (IAction*)FWCreateObject(m_pFWDevice, L"Action", L"Step", m_pActionTick, nMSec, nDur, L"", m_pBody[m_nCurBody], 0.5, DEG2RAD(0), DEG2RAD(5));
			}
			else if (GetKeyState('S') < 0)
			{
				m_nWalkState[m_nCurBody] = 'S';
				m_pWalkAction[m_nCurBody] = (IAction*)FWCreateObject(m_pFWDevice, L"Action", L"Step", m_pActionTick, nMSec, nDur, L"", m_pBody[m_nCurBody], -0.5, DEG2RAD(0), DEG2RAD(5));
			}
			else if (GetKeyState('X') < 0)
			{
				m_nWalkState[m_nCurBody] = 'X';
				m_pWalkAction[m_nCurBody] = (IAction*)FWCreateObject(m_pFWDevice, L"Action", L"Turn", m_pActionTick, nMSec, nDur, L"", m_pBody[m_nCurBody]);
			}
		}
	}

	// Add final step if just finished all the steps...
	for (int i = 0; i < 2; i++)
		if (m_pWalkAction[m_nCurBody] == NULL && m_nWalkState[m_nCurBody] != 0)
		{
			m_nWalkState[i] = 0;
			m_pWalkAction[i] = (IAction*)FWCreateObject(m_pFWDevice, L"Action", L"Step", m_pActionTick, nMSec, 150, L"", m_pBody[i], 0, DEG2RAD(0), DEG2RAD(0));
		}

	InvalidateRect(NULL, FALSE);
	CWnd::OnTimer(nIDEvent);
}

void CChildView::OnActionsAction1()
{
	IFWUnknown *p = NULL;
	FWLONG nStart;
	m_pRenderer->GetPlayTime(&nStart);
	FWULONG nDur = 500;
		
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Point",    m_pActionTick, nStart, nDur, L"right", m_pBody[0], m_pScene, L"Sphere02.Sphere02");
}

void CChildView::OnActionsAction2()
{
	IFWUnknown *p = NULL;
	FWLONG nStart;
	m_pRenderer->GetPlayTime(&nStart);
	FWULONG nDur = 500;
		
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Take",    m_pActionTick, nStart, nDur, L"right", m_pBody[0], m_pScene, L"Sphere02.Sphere02");
}

void CChildView::OnActionsAction3()
{
	IFWUnknown *p = NULL;
	FWLONG nStart;
	m_pRenderer->GetPlayTime(&nStart);
	FWULONG nDur = 500;

	ITransform *pT = NULL;
	m_pScene->CreateCompatibleTransform(&pT);
	pT->MulRotationZ(DEG2RAD(90));
	pT->MulRotationY(DEG2RAD(180));
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"RotateTo", m_pActionTick, nStart, nDur, m_pBody[0], BODY_HAND + BODY_RIGHT, pT, BODY_PELVIS);
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Reach",    m_pActionTick, nStart, nDur, L"right", m_pBody[0], BODY_FINGER + BODY_MIDDLE, m_pScene, L"Sphere02.Sphere02");
}

void CChildView::OnActionsAction4()
{
	IFWUnknown *p = NULL;
	FWLONG nStart;
	m_pRenderer->GetPlayTime(&nStart);
	FWULONG nDur = 500;

	ITransform *pT;
	m_pBody[0]->CreateCompatibleTransform(&pT);
	pT->FromRotationZ(DEG2RAD(60));
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Rotate", m_pActionTick, nStart, nDur, m_pBody[0], BODY_ARM, pT);
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Rotate", m_pActionTick, nStart, nDur, m_pBody[0], BODY_LEG+BODY_LEFT, pT);
	pT->FromRotationZ(DEG2RAD(-60));
	    FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Rotate", m_pActionTick, p, nDur, m_pBody[0], BODY_LEG+BODY_LEFT, pT);
	pT->FromRotationZ(DEG2RAD(-120));
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Rotate", m_pActionTick, p, nDur, m_pBody[0], BODY_ARM, pT);
	pT->FromRotationZ(DEG2RAD(60));
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Rotate", m_pActionTick, p, nDur, m_pBody[0], BODY_ARM, pT);

	pT->Release();
}


void CChildView::OnActionsAction5()
{
	IFWUnknown *p = NULL;
	FWLONG nStart;
	m_pRenderer->GetPlayTime(&nStart);
	FWLONG nDur = 1500;

	float h = 46;
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Move", m_pActionTick, nStart, nDur, m_pScene, L"Sphere02.Sphere02", 0.0f, 0.0f, 46.0f);
	((IAction*)p)->SetEnvelope(ACTION_ENV_PARA, 1, 1);
	for (int i = 0; i < 20; i++)
	{
		h *= 0.75;
		nDur *= 0.75;
		p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Move", m_pActionTick, p, nDur, m_pScene, L"Sphere02.Sphere02", 0.0f, 0.0f, -h);
		((IAction*)p)->SetEnvelope(ACTION_ENV_PARA, 0, 0);
		p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Move", m_pActionTick, p, nDur, m_pScene, L"Sphere02.Sphere02", 0.0f, 0.0f, h);
		((IAction*)p)->SetEnvelope(ACTION_ENV_PARA, 1, 1);
	}
}


void CChildView::OnActionsAction6()
{
	IFWUnknown *p = NULL;
	FWLONG nStart;
	m_pRenderer->GetPlayTime(&nStart);
	FWULONG nDur = 1000;

	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Move", m_pActionTick, nStart, nDur, m_pScene, L"Sphere02.Sphere02", -400.0f, 0.0f, 0.0f);
	((IAction*)p)->SetEnvelope(ACTION_ENV_PARA, 0, 1);
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Move", m_pActionTick, nStart, nDur, m_pScene, L"Sphere02.Sphere02", 0.0f, 0.0f, 46.0f);
	((IAction*)p)->SetEnvelope(ACTION_ENV_PARA, 1, 1);
}

void CChildView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
	switch (nChar)
	{
	case VK_SPACE:
		m_nCurBody = 1 - m_nCurBody;
		break;
	}
}

void CChildView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}


void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetCapture();
	m_ptDrag = point;
	CWnd::OnLButtonDown(nFlags, point);
}


void CChildView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (GetCapture() == this)
	{
		ReleaseCapture();
	}

	CWnd::OnLButtonUp(nFlags, point);
}


void CChildView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (GetCapture() == this)
	{
		m_pCamera->Pan((point.x - m_ptDrag.x) / 500.0f);
		m_pCamera->Tilt(-(point.y - m_ptDrag.y) / 500.0f);
		m_ptDrag = point;
	}

	CWnd::OnMouseMove(nFlags, point);
}


void CChildView::OnUpdateIndicatorScrl(CCmdUI *pCmdUI)
{
	FWLONG nTime;
	m_pRenderer->GetPlayTime(&nTime);

	std::wstringstream str;
	if (nTime < 0) { str << "-"; nTime = -nTime; }
	str << nTime / 3600000 << ":" << (nTime % 3600000) / 60000 << ":" << (nTime % 60000) / 1000 << "." << (nTime % 1000) / 100;

	pCmdUI->SetText(str.str().c_str());
}
