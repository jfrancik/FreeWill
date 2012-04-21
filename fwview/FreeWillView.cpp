// FreeWillView.cpp : implementation of the CFreeWillView class
//

#include "stdafx.h"
#include "fwview.h"

#include "FreeWillDoc.h"
#include "FreeWillView.h"

#include "freewill.c"
#include "freewilltools.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DEG2RAD(d)	( (d) * M_PI / 180.0 )
#define RAD2DEG(r)	( 180.0 * (r) / M_PI )

#include "../diag.inl"

CDiag *pDefDiag = NULL;
bool bMsgBox = true;
void _defDiag();
void _defDiag(FWSTRING pName, IBody *pBody, FWULONG nId, CDiag::MODE mode)
{
	if (!pDefDiag) pDefDiag = new CDiag(pName, pBody, nId, mode);
	_defDiag();
}

void _defDiag()
{
	if (!pDefDiag) return;
	if (bMsgBox)
	{
		CString z = *pDefDiag;
		AfxMessageBox(z);
	}
}

// CFreeWillView

IMPLEMENT_DYNCREATE(CFreeWillView, CView)

BEGIN_MESSAGE_MAP(CFreeWillView, CView)
	ON_WM_SIZE()
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_TEST, OnUpdateIndicatorTest)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_OP, OnUpdateIndicatorOp)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
//	ON_WM_KEYDOWN()
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_TEST_1, OnTest1)
	ON_COMMAND(ID_TEST_2, OnTest2)
	ON_COMMAND(ID_TEST_3, OnTest3)
	ON_COMMAND(ID_TEST_4, OnTest4)
	ON_COMMAND(ID_TEST_5, OnTest5)
	ON_COMMAND(ID_TEST_6, OnTest6)
	ON_COMMAND(ID_TEST_7, OnTest7)
	ON_COMMAND(ID_TEST_8, OnTest8)
	ON_COMMAND(ID_TEST_SETA, OnTestSetA)
	ON_UPDATE_COMMAND_UI(ID_TEST_SETA, OnUpdateTestSetA)
	ON_COMMAND(ID_TEST_SETB, OnTestSetB)
	ON_UPDATE_COMMAND_UI(ID_TEST_SETB, OnUpdateTestSetB)
	ON_COMMAND(ID_TEST_SETC, OnTestSetC)
	ON_UPDATE_COMMAND_UI(ID_TEST_SETC, OnUpdateTestSetC)
	ON_COMMAND(ID_TEST_SETD, OnTestSetD)
	ON_UPDATE_COMMAND_UI(ID_TEST_SETD, OnUpdateTestSetD)
	ON_COMMAND(ID_TEST_X, OnTestX)
	ON_COMMAND(ID_TEST_Y, OnTestY)
	ON_COMMAND(ID_TEST_Z, OnTestZ)
	ON_COMMAND(ID_TEST_SETA, OnTestSetA)
	ON_UPDATE_COMMAND_UI(ID_TEST_SETA, OnUpdateTestSetA)
	ON_COMMAND(ID_FILE_SAVE_STILL, OnFileSaveStill)
	ON_COMMAND(ID_TEST_STOREMODE, OnTestStoremode)
	ON_UPDATE_COMMAND_UI(ID_TEST_STOREMODE, OnUpdateTestStoremode)
	ON_WM_TIMER()
	ON_COMMAND(ID_VIEW_RESET, &CFreeWillView::OnViewReset)
	ON_COMMAND(ID_VIEW_FULLSCREEN, &CFreeWillView::OnViewFullscreen)
END_MESSAGE_MAP()

// CFreeWillView construction/destruction

CFreeWillView::CFreeWillView()
{
	m_pFWDevice = NULL;
	m_pRenderer = NULL;
	m_pScene = NULL;
	m_pBody = NULL;
	m_pActionTick = NULL;
	m_nManipMode = 0;
	m_pAxisT = m_pAxisInvT = NULL;
	m_nTestSet = AfxGetApp()->GetProfileIntW(L"Settings", L"TestSet", 0);
	m_bStoreAVI = FALSE;
}

CFreeWillView::~CFreeWillView()
{
	if (m_pFWDevice) m_pFWDevice->Release();
	if (m_pRenderer) m_pRenderer->Release();
	if (m_pScene) m_pScene->Release();
	if (m_pBody) m_pBody->Release();
	if (m_pActionTick) m_pActionTick->Release();
}

BOOL CFreeWillView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
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
//		TRACE(str);
		switch (AfxMessageBox(str, MB_CANCELTRYCONTINUE | MB_DEFBUTTON3))
		{
		case IDCANCEL: FatalAppExit(0, L"Application stopped"); break;
		case IDTRYAGAIN: DebugBreak(); break;
		case IDCONTINUE: break;
		}
		return p->nCode;
	}

void CFreeWillView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	if (m_pFWDevice) return;

	// create the FreeWill device
	HRESULT h;
	h = CoCreateInstance(CLSID_FWDevice, NULL, CLSCTX_INPROC_SERVER, IID_IFWDevice, (void**)&m_pFWDevice);
	if (FAILED(h)) return;

	// create the error handler
	m_pFWDevice->SetUserErrorHandler(HandleErrors);

	// create & initialise the renderer
	h = m_pFWDevice->CreateObject(L"Renderer", IID_IRenderer, (IFWUnknown**)&m_pRenderer);
	if (FAILED(h)) return;
	h = m_pRenderer->InitDisplay(m_hWnd, 0, 0);
	if (FAILED(h)) return;
	FWCOLOR back = { 0.6f, 0, 0.7f };
	//FWCOLOR back = { 0, 0, 0 };
	m_pRenderer->PutBackColor(back);

	// create & initialise the buffers - determine hardware factors
	IMeshVertexBuffer *pVertexBuffer;
	IMeshFaceBuffer *pFaceBuffer;
	h = m_pRenderer->GetBuffers(&pVertexBuffer, &pFaceBuffer); if (FAILED(h)) return;
	h = pVertexBuffer->Create(1000000, MESH_VERTEX_XYZ | MESH_VERTEX_NORMAL | MESH_VERTEX_BONEWEIGHT /*| MESH_VERTEX_BONEINDEX*/ /*| MESH_VERTEX_DIFFUSE*/ | MESH_VERTEX_TEXTURE, 4, 1);
	if (FAILED(h)) return;
	h = pFaceBuffer->Create(1000000); if (FAILED(h)) return;
	pVertexBuffer->Release();
	pFaceBuffer->Release();

	m_pRenderer->SetCallback(FW_CB_LOSTDEVICE, NULL, 0, NULL);
	m_pRenderer->SetCallback(FW_CB_RESETDEVICE, NULL, 0, NULL);

	// create & initialise the animation scene
	h = m_pFWDevice->CreateObject(L"Scene", IID_IScene, (IFWUnknown**)&m_pScene);
	if (FAILED(h)) return;
	m_pScene->PutRenderer(m_pRenderer);

	// create & initialise the character body
	h = m_pFWDevice->CreateObject(L"Body", IID_IBody, (IFWUnknown**)&m_pBody);
	if (FAILED(h)) return;

	// initialise the MFC Document
	GetDocument()->SetFreeWillObjects(m_pFWDevice, m_pRenderer, m_pScene, m_pBody);

	// initialise the Tick Action
	m_pActionTick = (IAction*)FWCreateObject(m_pFWDevice, L"Action", L"Generic", (IUnknown*)NULL);
//	m_pActionTick->SetSinusoidalEnvelope(0.4f, 0.6f);
	m_pActionTick->SetSinusoidalEnvelopeT(250, 250);

	KillTimer(1);
	SetTimer(1, 10, NULL);
}

void CFreeWillView::OnUpdate(CView *pSender, LPARAM lHint, CObject *pHint)
{
	switch (lHint)
	{
	case WM_PAINT:
		// idle time refresh...
		InvalidateRect(NULL, FALSE);
		break;
	}
}

void CFreeWillView::OnDraw(CDC *pDC)
{
	if (!m_pRenderer || !m_pActionTick || !m_pScene) return;

	// draw - DX
	if (m_pRenderer->IsPlaying() == S_OK && m_pRenderer->IsDeviceAvailable() == S_OK)
	{
		FWULONG nMSec;
		m_pRenderer->GetPlayTime(&nMSec);
		m_pActionTick->RaiseEvent(nMSec, EVENT_TICK, nMSec, 0);
		if (!m_pActionTick->AnySubscriptionsLeft())
			m_pRenderer->Stop();
	}

	HRESULT h = m_pRenderer->BeginFrame();
	if SUCCEEDED(h)
	{
		m_pRenderer->Clear();
		m_pScene->Render(m_pRenderer);
		m_pRenderer->EndFrame();
	}

	// draw - rotation circle (GDI)
	if (GetDocument()->GetManipulationMode() == ID_MANIPULATE_ROTATE)
	{
		CRect rect;
		GetClientRect(rect);
		int x = rect.Width() / 2;
		int y = rect.Height() / 2;
		int n = min(x, y) * 3 / 4;
		int A = 7;
		rect.DeflateRect(x - n, y - n);
		CPen pen(PS_SOLID, 1, RGB(0, 128, 0));
		CPen *pOldPen = pDC->SelectObject(&pen);
		HGDIOBJ hOldBrush = pDC->SelectObject(GetStockObject(NULL_BRUSH));
		pDC->Ellipse(rect);
		pDC->Rectangle(rect.left, y - A, rect.left + A + A, y + A);
		pDC->Rectangle(rect.right, y - A, rect.right - A - A, y + A);
		pDC->Rectangle(x - A, rect.top, x + A, rect.top + A + A);
		pDC->Rectangle(x - A, rect.bottom, x + A, rect.bottom - A - A);
		pDC->SelectObject(pOldPen);
		pDC->SelectObject(hOldBrush);
	}
}

void CFreeWillView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	if (m_pRenderer && m_pRenderer->IsDeviceAvailable() == S_FALSE) return;

	if (!m_pScene) return;

	ISceneCamera *pCamera = NULL;
	m_pScene->GetCurrentCamera(&pCamera);
	if (pCamera)
	{
		pCamera->Render(m_pRenderer);
		pCamera->Release();
	}
}

void CFreeWillView::OnTimer(UINT nIDEvent)
{
	CView::OnTimer(nIDEvent);
	if (nIDEvent == 5)
	{
		KillTimer(5);
		_defDiag();
	}
}

// CFreeWillView diagnostics

#ifdef _DEBUG
void CFreeWillView::AssertValid() const
{
	CView::AssertValid();
}

void CFreeWillView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CFreeWillDoc *CFreeWillView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFreeWillDoc)));
	return (CFreeWillDoc*)m_pDocument;
}
#endif //_DEBUG

//////////////////////////////////////////////////////////////////////////
// Direct Manipulation (by mouse)

#define ID_MANIPULATE_ROTATE_OUT	(ID_MANIPULATE_ROTATE + 300)

void CFreeWillView::OnUpdateIndicatorTest(CCmdUI *pCmdUI)
{
return;
	pCmdUI->SetText(L"");

	if (!m_pScene) return;

	HRESULT h;
	IBounding *pBox1 = NULL;
	IBounding *pBox2 = NULL;
	IBounding *pSphere = NULL;
	IKineChild *pChild = NULL;

	h = m_pScene->GetChild(L"Bip01.L Hand.bound", &pChild);
	if (pChild) h = pChild->QueryInterface(&pBox1);
	if (pChild) pChild->Release();

	h = m_pScene->GetChild(L"Bip01.R Hand.bound", &pChild);
	if (pChild) h = pChild->QueryInterface(&pBox2);
	if (pChild) pChild->Release();

	if (pBox1 && pBox2)
	{
		// create spherical bounding box...
		m_pFWDevice->CreateObjectEx(L"Bounding", L"Sphere", 0, NULL, IID_IBounding, (IFWUnknown**)&pSphere);
		BOUND_OBB obb;
		pBox2->GetData(BOUND_FORMAT_OBB, sizeof(obb), (BYTE*)&obb);
		pSphere->PutData(BOUND_FORMAT_OBB, sizeof(obb), (BYTE*)&obb);
		BOUND_SPHERE sphere;
		pSphere->GetData(BOUND_FORMAT_SPHERE, sizeof(sphere), (BYTE*)&sphere);
		sphere.fRadius = sphere.fRadius / 1.73205f;
		pSphere->PutData(BOUND_FORMAT_SPHERE, sizeof(sphere), (BYTE*)&sphere);

		// test spherical
		h = pBox1->Detect(pSphere);
		
		// test boxes
		h = pBox1->Detect(pBox2);
		
		bool bColl = (h == S_FALSE);
		static bool bStat = false;
		if (bColl) pCmdUI->SetText(L"COLL");
		if (bColl && !bStat) MessageBeep(MB_OK);
		bStat = bColl;
	}

	if (pBox1) pBox1->Release();
	if (pBox2) pBox2->Release();
	if (pSphere) pSphere->Release();
}

void CFreeWillView::OnUpdateIndicatorOp(CCmdUI *pCmdUI)
{
	CString str;
	int nRotOut = (int)((m_x - m_x0) / (FWFLOAT)M_PI * 180.0f); if (nRotOut < 0) nRotOut += 360;
	switch(m_nManipMode)
	{
	case ID_MANIPULATE_MOVE:
		str.Format(L"Moving (%d, %d, %d)", -(int)(m_x - m_x0), 0, -(int)(m_y - m_y0));
		break;
	case ID_MANIPULATE_ROTATE:
		str.Format(L"Rotating pitch = %d, roll = %d)", (int)(m_y - m_y0), -(int)(m_x - m_x0));
		break;
	case ID_MANIPULATE_ROTATE_OUT:
		str.Format(L"Rotating yaw = %d", nRotOut);
		break;
	case ID_MANIPULATE_POINT:
	default:
		str = L"";
		break;
	}
	pCmdUI->SetText(str);
}

// Hit Test - within and utside the rotation circle
enum CFreeWillView::HITVAL CFreeWillView::HitTest(CPoint point)
{
	CRect rect;
	GetClientRect(rect);
	int x = rect.Width() / 2;
	int y = rect.Height() / 2;
	int n = min(x, y) * 3 / 4;
	int A = 7;
	rect.DeflateRect(x - n, y - n);

	if (point.x >= rect.left && point.x <= rect.left + A + A && point.y >= y - A && point.y <= y + A)
		return HIT_H;
	if (point.x >= rect.right - A - A && point.x <= rect.right && point.y >= y - A && point.y <= y + A)
		return HIT_H;
	if (point.x >= x - A && point.x <= x + A && point.y >= rect.top && point.y <= rect.top + A + A)
		return HIT_V;
	if (point.x >= x - A && point.x <= x + A && point.y >= rect.bottom - A - A && point.y <= rect.bottom)
		return HIT_V;
	if ((point.x - x) * (point.x - x) + (point.y - y) * (point.y - y) < n * n)
		return HIT_IN;
	return HIT_OUT;
}

void CFreeWillView::OnLButtonDown(UINT nFlags, CPoint point)
{
	IKineObj3D *pObj = GetDocument()->GetCurrentNode();
	if (!pObj) return;

	m_nManipMode = GetDocument()->GetManipulationMode();

	m_x = m_x0 = (FWFLOAT)point.x / 10;
	m_y = m_y0 = (FWFLOAT)point.y / 10;
	m_dx = m_dy = 1.0f;
	SetCapture();

	// determine rotation mode - depending on the hit test
	if (m_nManipMode == ID_MANIPULATE_ROTATE)
		switch (HitTest(point))
		{
		case HIT_IN:
			break;
		case HIT_H:
			m_dy = 0.0f;
			break;
		case HIT_V:
			m_dx = 0.0f;
			break;
		case HIT_OUT:
			{
				m_nManipMode = ID_MANIPULATE_ROTATE_OUT; 
				m_dx = m_dy = 1.0f;
				CRect rect;
				GetClientRect(rect);
				m_x = (FWFLOAT)atan(((double)point.y - (double)rect.Height()/2.0) / ((double)point.x - (double)rect.Width()/2.0));
				if (point.x < rect.Width()/2) m_x += (FWFLOAT)M_PI;
				break;
			}
		}

	// determine the Axis Transforms - depending on the Axis Mode
	m_pFWDevice->CreateObject(L"Transform", IID_ITransform, (IFWUnknown**)&m_pAxisT);
	m_pFWDevice->CreateObject(L"Transform", IID_ITransform, (IFWUnknown**)&m_pAxisInvT);
	ITransform *pTObj = NULL, *pTRef = NULL;
	switch (GetDocument()->GetManipAxisMode())
	{
	case ID_MANIPULATE_LOCAL:
		m_pFWDevice->CreateObject(L"Transform", IID_ITransform, (IFWUnknown**)&pTRef);
		break;	// leave both matrices as identity
	case ID_MANIPULATE_VIEW:
		{
			// extract object's global transform T
			pObj->CreateCompatibleTransform(&pTObj);
			pObj->GetGlobalTransform(pTObj);

			// get camera transform
			ISceneCamera *pCamera = NULL;
			m_pScene->GetCurrentCamera(&pCamera);
			IKineTargetedObj *pObjCam = NULL;
			if (pCamera) pCamera->QueryInterface(&pObjCam);
			if (pObjCam) pObjCam->GetLookAtLHTransform(&pTRef);
			pObjCam->Release();
			pCamera->Release();

			// final refinement
			pTRef->MulRotationSinCosZ(-1, 0);
			pTRef->MulRotationSinCosX(1, 0);
			break;
		}
	case ID_MANIPULATE_ARBITRARY:
		{
			// extract object's global transform T
			pObj->CreateCompatibleTransform(&pTObj);
			pObj->GetGlobalTransform(pTObj);

			// get arbitrary object's inversed global matrix 
			IKineChild *pArbObj = NULL;
			GetDocument()->GetManipArbitraryAxis(&pArbObj);
			ASSERT(pArbObj);
			ITransform *pT = NULL;
			pArbObj->CreateCompatibleTransform(&pT);
			pArbObj->GetGlobalTransform(pT);
			m_pFWDevice->CreateObject(L"Transform", IID_ITransform, (IFWUnknown**)&pTRef);
			pTRef->Multiply(pT);
			pT->Release();
			pArbObj->Release();
			pTRef->Inverse();

			// final refinement
			break;
		}
	}

	if (pTObj) m_pAxisT->Multiply(pTObj);
	if (pTRef) m_pAxisT->Multiply(pTRef);

	m_pAxisT->Orthonormalize();
	m_pAxisT->Reset(FALSE, TRUE);

	m_pAxisInvT->FromTransform(m_pAxisT);
	m_pAxisInvT->Inverse();

	if (pTObj) pTObj->Release();
	if (pTRef) pTRef->Release();
	pObj->Release();

	CView::OnLButtonDown(nFlags, point);
}

void CFreeWillView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (GetCapture() == this)
	{
		OnMouseMove(nFlags, point);
		switch(m_nManipMode)
		{
		case ID_MANIPULATE_POINT:
			ReleaseCapture();
			MessageBeep(MB_OK);
			break;
		case ID_MANIPULATE_MOVE:
		case ID_MANIPULATE_ROTATE:
		case ID_MANIPULATE_ROTATE_OUT:
			if (GetCapture() != this) break;
			m_nManipMode = 0;
			ReleaseCapture();
		}
	}

	if (m_pAxisT)
	{
		m_pAxisT->Release(); m_pAxisT = NULL;
		m_pAxisInvT->Release(); m_pAxisInvT = NULL;
	}

	CView::OnLButtonUp(nFlags, point);
}

void CFreeWillView::OnMouseMove(UINT nFlags, CPoint point)
{
	// determine & set cursor shape
	switch(GetDocument()->GetManipulationMode())
	{
	case ID_MANIPULATE_POINT:
		break;
	case ID_MANIPULATE_MOVE:
		SetCursor(AfxGetApp()->LoadCursor(IDC_CURSOR_MOVE));
		break;
	case ID_MANIPULATE_ROTATE:
		if (GetCapture() == this)
		{
			if (m_nManipMode == ID_MANIPULATE_ROTATE)
				if (m_dx == 0.0f) SetCursor(AfxGetApp()->LoadCursor(IDC_CURSOR_ROTATE_V));
				else if (m_dy == 0.0f) SetCursor(AfxGetApp()->LoadCursor(IDC_CURSOR_ROTATE_H));
				else SetCursor(AfxGetApp()->LoadCursor(IDC_CURSOR_ROTATE_IN));
			else SetCursor(AfxGetApp()->LoadCursor(IDC_CURSOR_ROTATE_OUT));
		}
		else
			switch(HitTest(point))
			{
			case HIT_IN:	SetCursor(AfxGetApp()->LoadCursor(IDC_CURSOR_ROTATE_IN)); break;
			case HIT_OUT:	SetCursor(AfxGetApp()->LoadCursor(IDC_CURSOR_ROTATE_OUT)); break;
			case HIT_H:		SetCursor(AfxGetApp()->LoadCursor(IDC_CURSOR_ROTATE_H)); break;
			case HIT_V:		SetCursor(AfxGetApp()->LoadCursor(IDC_CURSOR_ROTATE_V)); break;
			}
		break;
	}

	// do the manipulation
	if (GetCapture() == this)
	{
		bool bCtrl = GetKeyState(VK_CONTROL) < 0;
		IKineObj3D *pObj = GetDocument()->GetCurrentNode();
		ITransform *pT = NULL;

		switch(m_nManipMode)
		{
		case ID_MANIPULATE_POINT:
			break;
		case ID_MANIPULATE_MOVE:
		case ID_MANIPULATE_ROTATE:
			{
				if (!pObj) break;
				FWFLOAT dx = (FWFLOAT)point.x / 10 - m_x;
				FWFLOAT dy = (FWFLOAT)point.y / 10 - m_y;
				dx *= m_dx; dy *= m_dy;	// hor/vert-only modes...

				ITransform *pCur = NULL;
				pObj->CreateCompatibleTransform(&pCur);
                pObj->GetLocalTransform(pCur);
				pObj->GetLocalTransformRef(&pT);
				pT->FromIdentity();

				if (m_pAxisT) pT->Multiply(m_pAxisT);
				if (m_nManipMode == ID_MANIPULATE_MOVE)
					pT->MulTranslationXYZ(-dy, 0, -dx);
				else
					pT->MulRotationYawPitchRoll(0, -dx * 3.14159265f / 180.0f, dy * 3.14159265f / 180.0f);
				if (m_pAxisInvT) pT->Multiply(m_pAxisInvT);
				pT->Multiply(pCur);
				pCur->Release();
				pT->Release();
				pObj->Invalidate();
				m_x += dx; m_y += dy;

				break;
			}
		case ID_MANIPULATE_ROTATE_OUT:
			{
				CRect rect; GetClientRect(rect);
				FWFLOAT dx = (FWFLOAT)atan(((double)point.y - (double)rect.Height()/2.0) / ((double)point.x - (double)rect.Width()/2.0));
				if (point.x <= rect.Width()/2) dx += (FWFLOAT)M_PI; //if (dx < 0.0) dx += 2 * (FWFLOAT)M_PI;
				dx -= m_x;

				ITransform *pCur = NULL;
				pObj->CreateCompatibleTransform(&pCur);
                pObj->GetLocalTransform(pCur);
				pObj->GetLocalTransformRef(&pT);
				pT->FromIdentity();

				if (m_pAxisT) pT->Multiply(m_pAxisT);
				pT->MulRotationYawPitchRoll(dx, 0, 0);
				if (m_pAxisInvT) pT->Multiply(m_pAxisInvT);
				pT->Multiply(pCur);
				pCur->Release();
				pT->Release();
				pObj->Invalidate();
				m_x += dx;
				break;
			}
		}
		if (pObj) pObj->Release();
	}

	CView::OnMouseMove(nFlags, point);
}

BOOL CFreeWillView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	bool bCtrl = GetKeyState(VK_CONTROL) < 0;
	IKineObj3D *pObj = GetDocument()->GetCurrentNode();
	ITransform *pT = NULL;

	switch(GetDocument()->GetManipulationMode())
	{
	case ID_MANIPULATE_POINT:
		break;
	case ID_MANIPULATE_MOVE:
		{
			if (!pObj) break;

			ITransform *pCur = NULL;
			pObj->CreateCompatibleTransform(&pCur);
			pObj->GetLocalTransform(pCur);
			pObj->GetLocalTransformRef(&pT);
			pT->FromIdentity();

			if (m_pAxisT) pT->Multiply(m_pAxisT);
			pT->MulTranslationXYZ(0.0f, -(FWFLOAT)zDelta / 30.0f, 0.0f);
			if (m_pAxisInvT) pT->Multiply(m_pAxisInvT);
			pT->Multiply(pCur);
			pCur->Release();
			pT->Release();
			pObj->Invalidate();
			break;
		}
	case ID_MANIPULATE_ROTATE:
	case ID_MANIPULATE_ROTATE_OUT:
		break;
	}
	if (pObj) pObj->Release();

	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

void CFreeWillView::OnFileSaveStill()
{
	if (!m_pRenderer) return;
	CFileDialog dlg(FALSE, L"bmp", L"*.bmp", OFN_OVERWRITEPROMPT, L"BMP Files (*.bmp)|*.bmp|All Files|*.*||");
	if (dlg.DoModal() == IDOK)
	{
		m_pRenderer->InitOffScreen(0, 0);

		ISceneCamera *pCamera = NULL;
		m_pScene->GetCurrentCamera(&pCamera);
		if (pCamera)
		{
			pCamera->Render(m_pRenderer);
//			pCamera->Release();
		}

		m_pRenderer->OpenStillFile(dlg.GetFileName(), RENDER_JPG);
		m_pRenderer->BeginFrame();
		m_pScene->Render(m_pRenderer);
		m_pRenderer->EndFrame();
		m_pRenderer->DoneOffScreen();

		if (pCamera)
		{
			pCamera->Render(m_pRenderer);
			pCamera->Release();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// TESTS & DEMOS

// TESTS & DEMOS --- COMMON

void CFreeWillView::OnTest1()
{
	m_pActionTick->UnSubscribeAll();
	switch (m_nTestSet)
	{
	case 0: OnTestA1(); break;
	case 1: OnTestB1(); break;
	case 2: OnTestC1(); break;
	case 3: OnTestD1(); break;
	}
	if (m_bStoreAVI) { SaveAVI(L"avi1.avi"); return; }
	m_pRenderer->Play();
}

void CFreeWillView::OnTest2()
{
	m_pActionTick->UnSubscribeAll();
	switch (m_nTestSet)
	{
	case 0: OnTestA2(); break;
	case 1: OnTestB2(); break;
	case 2: OnTestC2(); break;
	case 3: OnTestD2(); break;
	}
	if (m_bStoreAVI) { SaveAVI(L"avi1.avi"); return; }
	m_pRenderer->Play();
}

void CFreeWillView::OnTest3()
{
	m_pActionTick->UnSubscribeAll();
	switch (m_nTestSet)
	{
	case 0: OnTestA3(); break;
	case 1: OnTestB3(); break;
	case 2: OnTestC3(); break;
	case 3: OnTestD3(); break;
	}
	if (m_bStoreAVI) { SaveAVI(L"avi1.avi"); return; }
	m_pRenderer->Play();
}

void CFreeWillView::OnTest4()
{
	m_pActionTick->UnSubscribeAll();
	switch (m_nTestSet)
	{
	case 0: OnTestA4(); break;
	case 1: OnTestB4(); break;
	case 2: OnTestC4(); break;
	case 3: OnTestD4(); break;
	}
	if (m_bStoreAVI) { SaveAVI(L"avi1.avi"); return; }
	m_pRenderer->Play();
}

void CFreeWillView::OnTest5()
{
	m_pActionTick->UnSubscribeAll();
	switch (m_nTestSet)
	{
	case 0: OnTestA5(); break;
	case 1: OnTestB5(); break;
	case 2: OnTestC5(); break;
	case 3: OnTestD5(); break;
	}
	if (m_bStoreAVI) { SaveAVI(L"avi1.avi"); return; }
	m_pRenderer->Play();
}

void CFreeWillView::OnTest6()
{
	m_pActionTick->UnSubscribeAll();
	switch (m_nTestSet)
	{
	case 0: OnTestA6(); break;
	case 1: OnTestB6(); break;
	case 2: OnTestC6(); break;
	case 3: OnTestD6(); break;
	}
	if (m_bStoreAVI) { SaveAVI(L"avi1.avi"); return; }
	m_pRenderer->Play();
}

void CFreeWillView::OnTest7()
{
	m_pActionTick->UnSubscribeAll();
	switch (m_nTestSet)
	{
	case 0: OnTestA7(); break;
	case 1: OnTestB7(); break;
	case 2: OnTestC7(); break;
	case 3: OnTestD7(); break;
	}
	if (m_bStoreAVI) { SaveAVI(L"avi1.avi"); return; }
	m_pRenderer->Play();
}

void CFreeWillView::OnTest8()
{
	m_pActionTick->UnSubscribeAll();
	switch (m_nTestSet)
	{
	case 0: OnTestA8(); break;
	case 1: OnTestB8(); break;
	case 2: OnTestC8(); break;
	case 3: OnTestD8(); break;
	}
	if (m_bStoreAVI) { SaveAVI(L"avi1.avi"); return; }
	m_pRenderer->Play();
}

void CFreeWillView::OnTestSetA()						{ m_nTestSet = 0; AfxGetApp()->WriteProfileInt(L"Settings", L"TestSet", m_nTestSet); }
void CFreeWillView::OnUpdateTestSetA(CCmdUI *pCmdUI)	{ pCmdUI->SetRadio(m_nTestSet == 0); }
void CFreeWillView::OnTestSetB()						{ m_nTestSet = 1; AfxGetApp()->WriteProfileInt(L"Settings", L"TestSet", m_nTestSet); }
void CFreeWillView::OnUpdateTestSetB(CCmdUI *pCmdUI)	{ pCmdUI->SetRadio(m_nTestSet == 1); }
void CFreeWillView::OnTestSetC()						{ m_nTestSet = 2; AfxGetApp()->WriteProfileInt(L"Settings", L"TestSet", m_nTestSet); }
void CFreeWillView::OnUpdateTestSetC(CCmdUI *pCmdUI)	{ pCmdUI->SetRadio(m_nTestSet == 2); }
void CFreeWillView::OnTestSetD()						{ m_nTestSet = 3; AfxGetApp()->WriteProfileInt(L"Settings", L"TestSet", m_nTestSet); }
void CFreeWillView::OnUpdateTestSetD(CCmdUI *pCmdUI)	{ pCmdUI->SetRadio(m_nTestSet == 3); }

void CFreeWillView::OnTestX()
{
	IKineChild *pChild = NULL;
	ITransform *pT;

	m_pScene->GetChild(L"Camera01", &pChild);
	pChild->GetLocalTransformRef(&pT);
	pT->MulRotationZ(DEG2RAD(-22.5));
	pChild->Invalidate();
	pChild->Release();
	pT->Release();
}

void CFreeWillView::OnTestY()
{
	IKineChild *pChild = NULL;
	ITransform *pT;

	m_pScene->GetChild(L"Camera01", &pChild);
	pChild->GetLocalTransformRef(&pT);
	pT->MulRotationZ(DEG2RAD(22.5));
	pChild->Invalidate();
	pChild->Release();
	pT->Release();
}

void CFreeWillView::OnTestZ()
{
	_defDiag(L"pelvis", m_pBody, BODY_PELVIS, CDiag::ALL);
}

void CFreeWillView::OnTestPrepare(LPCTSTR fname, bool bResetBones)
{
	if (bResetBones)
	{
		IKineChild *pCamera = NULL;
		m_pScene->GetChild(L"Camera01", &pCamera);
		if (pCamera)
		{
			pCamera->PushState();
			GetDocument()->ResetSceneState();
			pCamera->PopState();
			pCamera->Release();
		}
	}
	CString str = GetDocument()->GetTitle();
	if (str.CompareNoCase(fname) != 0)
	{
		CFileDialog dlg(TRUE, L"3d", fname, 0, L"3D Files (*.3d)|*.3d|All Files|*.*||");
		if (dlg.DoModal() == IDOK)
		{
			UpdateWindow();
			AfxGetApp()->OpenDocumentFile(dlg.GetFileName());
		}
	}
}

void CFreeWillView::SaveAVI(LPCTSTR fname)
{
	m_pRenderer->InitOffScreen(800, 600);

	ISceneCamera *pCamera = NULL;
	m_pScene->GetCurrentCamera(&pCamera);
	if (pCamera)
		pCamera->Render(m_pRenderer);

	m_pRenderer->OpenMovieFile(fname, 25);
	FWULONG i = 0;
	while (m_pActionTick->AnySubscriptionsLeft())
	{
		m_pActionTick->RaiseEvent(i, EVENT_TICK, i, 0);
		i++;
		m_pRenderer->BeginFrame();
		m_pScene->Render(m_pRenderer);
		m_pRenderer->EndFrame();
	}
	if (fname) m_pRenderer->CloseMovieFile();
	m_pRenderer->DoneOffScreen();

	if (pCamera)
	{
		pCamera->Render(m_pRenderer);
		pCamera->Release();
	}
}

void CFreeWillView::OnTestStoremode()
{
	m_bStoreAVI = !m_bStoreAVI;
}

void CFreeWillView::OnUpdateTestStoremode(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bStoreAVI);
}

/////////////////////////////////////////////////////////////////////////////
// TESTS & DEMOS --- SET B

void CFreeWillView::OnTestB1()
{
	OnTestPrepare(L"SceneBallPose.3d");
	if (!m_pRenderer) return;

	// get the sphere, query for IKineObj3D interface
	IKineChild *pChild = NULL;
	m_pScene->GetChild(L"Sphere02.Sphere02", &pChild); ASSERT(pChild);

	// Translate to the given location
	ITransform *pT = NULL;
	pChild->CreateCompatibleTransform(&pT);
	pT->FromTranslationXYZ(16.0f, 0.0f, -12.0f);
	pChild->PutLocalTransform(pT);
	pChild->Release();

	IAction *pAction = NULL;

	// make the sphere moving
	FWVECTOR vector = { 0, 0, 12 };
	{CParam params[] = { m_pActionTick, 0, 1800, m_pScene, L"Sphere02.Sphere02", vector};
	m_pFWDevice->CreateObjectEx(L"Action", L"Move", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->SetEnvelope(ACTION_ENV_PARA, 1.0f, 1.0f);
	pAction->Release();

	pT->Release();
}

void CFreeWillView::OnTestB2()
{
	OnTestPrepare(L"SceneBallPose.3d");
	if (!m_pRenderer) return;

	// get the sphere, query for IKineObj3D interface
	IKineChild *pChild = NULL;
	m_pScene->GetChild(L"Sphere02.Sphere02", &pChild); ASSERT(pChild);

	// Translate to the given location
	ITransform *pT = NULL;
	pChild->CreateCompatibleTransform(&pT);
	pT->FromTranslationXYZ(16.0f, 0.0f, -12.0f);
	pChild->PutLocalTransform(pT);
	pChild->Release();

	IAction *pAction = NULL;

	// make the sphere moving
	{CParam params[] = { m_pActionTick, 0, 1980, m_pScene, L"Sphere02.Sphere02", 0, 0, 12 };
	m_pFWDevice->CreateObjectEx(L"Action", L"Move", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->SetEnvelope(ACTION_ENV_PARA, 1.0f, 1.0f);
	pAction->Release();

//	// perform the TAKE action
//	{CParam params[] = { m_pActionTick, 0, 2000, L"left", m_pBody, m_pScene, L"Sphere02.Sphere02" };
//	m_pFWDevice->CreateObjectEx(L"Action", L"Take", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
//	pAction->Release();

	// perform the TAKE action
	{CParam params[] = { m_pActionTick, 0, 2000, L"right", m_pBody, m_pScene, L"Sphere02.Sphere02" };
	m_pFWDevice->CreateObjectEx(L"Action", L"Take", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->Release();

	// perform the BEND action (without legs)
	{CParam params[] = { m_pActionTick, 500, 900, m_pBody, DEG2RAD(40) };
	m_pFWDevice->CreateObjectEx(L"Action", L"Bend", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->Release();

	pT->Release();
}

void CFreeWillView::OnTestB3()
{
	OnTestPrepare(L"SceneBallPose.3d");
	if (!m_pRenderer) return;

	// get the sphere, query for IKineObj3D interface
	IKineChild *pChild = NULL;
	m_pScene->GetChild(L"Sphere02.Sphere02", &pChild); ASSERT(pChild);

	// Translate to the given location
	ITransform *pT = NULL;
	pChild->CreateCompatibleTransform(&pT);
	pT->FromTranslationXYZ(16.0f, 0.0f, -12.0f);
	pChild->PutLocalTransform(pT);
	pChild->Release();

	IAction *pAction = NULL;

	// make the sphere moving
	{CParam params[] = { m_pActionTick, 0, 1980, m_pScene, L"Sphere02.Sphere02", 0, 0,  12 };
	m_pFWDevice->CreateObjectEx(L"Action", L"Move", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->SetEnvelope(ACTION_ENV_PARA, 1.0f, 1.0f);
	pAction->Release();

	// perform the TAKE action
	{CParam params[] = { m_pActionTick, 0, 2000, L"down;right", m_pBody, m_pScene, L"Sphere02.Sphere02" };
	m_pFWDevice->CreateObjectEx(L"Action", L"Take", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->Release();

	// perform the BEND action (with legs)
	{CParam params[] = { m_pActionTick, 500, 900, m_pBody, DEG2RAD(50),  DEG2RAD(8)};
	m_pFWDevice->CreateObjectEx(L"Action", L"Bend", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->Release();
	
	pT->Release();
}

void CFreeWillView::OnTestB4()
{
	OnTestPrepare(L"SceneBallPose.3d");
	if (!m_pRenderer) return;

	// get the sphere, query for IKineObj3D interface
	IKineChild *pChild = NULL;
	m_pScene->GetChild(L"Sphere02.Sphere02", &pChild); ASSERT(pChild);

	// Translate to the given location
	ITransform *pT = NULL;
	pChild->CreateCompatibleTransform(&pT);
	pT->FromTranslationXYZ(16.0f, 0.0f, -12.0f);
	pChild->PutLocalTransform(pT);
	pChild->Release();

	IAction *pAction = NULL;

	// make the sphere moving
	{CParam params[] = { m_pActionTick, 0, 1980, m_pScene, L"Sphere02.Sphere02", 0, 0, 12 };
	m_pFWDevice->CreateObjectEx(L"Action", L"Move", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->SetEnvelope(ACTION_ENV_PARA, 1.0f, 1.0f);
	pAction->Release();

	// perform the TAKE action
	{CParam params[] = { m_pActionTick, 0, 2000, L"down", m_pBody, m_pScene, L"Sphere02.Sphere02" };
	m_pFWDevice->CreateObjectEx(L"Action", L"Take", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->Release();

	// perform the SQUAT action
	{CParam params[] = { m_pActionTick, 500, 900, m_pBody, DEG2RAD(50),  DEG2RAD(8), DEG2RAD(30)};
	m_pFWDevice->CreateObjectEx(L"Action", L"Squat", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->Release();
	pT->Release();
}

void CFreeWillView::OnTestB5()
{
	OnTestPrepare(L"SceneBallPoseA.3d");
	if (!m_pRenderer) return;

	// get the sphere, query for IKineObj3D interface
	IKineChild *pChild = NULL;
	m_pScene->GetChild(L"Sphere02.Sphere02", &pChild); ASSERT(pChild);

	// Translate to the given location
	ITransform *pT = NULL;
	pChild->CreateCompatibleTransform(&pT);
	pT->FromTranslationXYZ(16.0f, 0.0f, -12.0f);
	pChild->PutLocalTransform(pT);
	pChild->Release();

	IAction *pAction = NULL;

	// make the sphere moving
	{CParam params[] = { m_pActionTick, 0, 1980, m_pScene, L"Sphere02.Sphere02", 0, 0, 12 };
	m_pFWDevice->CreateObjectEx(L"Action", L"Move", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->SetEnvelope(ACTION_ENV_PARA, 1.0f, 1.0f);
	pAction->Release();

	// perform the TAKE action
	{CParam params[] = { m_pActionTick, 0, 2000, L"down", m_pBody, m_pScene, L"Sphere02.Sphere02" };
	m_pFWDevice->CreateObjectEx(L"Action", L"Take", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->Release();

	// perform the BEND action (without legs)
	{CParam params[] = { m_pActionTick, 700, 500, m_pBody, DEG2RAD(15) };
	m_pFWDevice->CreateObjectEx(L"Action", L"Bend", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->Release();

	// perform the STEP action
	{CParam params[] = { m_pActionTick, 500, 900, m_pBody, DEG2RAD(12.0f)};
	m_pFWDevice->CreateObjectEx(L"Action", L"Step", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->Release();

	pT->Release();
}

void CFreeWillView::OnTestB6()
{
	OnTestPrepare(L"SceneBallPoseB.3d");
	if (!m_pRenderer) return;

	// get the sphere, query for IKineObj3D interface
	IKineChild *pChild = NULL;
	m_pScene->GetChild(L"Sphere02.Sphere02", &pChild); ASSERT(pChild);

	// Translate to the given location
	ITransform *pT = NULL;
	pChild->CreateCompatibleTransform(&pT);
	pT->FromTranslationXYZ(16.0f, 0.0f, -12.0f);
	pChild->PutLocalTransform(pT);
	pChild->Release();

	IAction *pAction = NULL;

	// make the sphere moving
	{CParam params[] = { m_pActionTick, 0, 1980, m_pScene, L"Sphere02.Sphere02", 0, 0, 12 };
	m_pFWDevice->CreateObjectEx(L"Action", L"Move", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->SetEnvelope(ACTION_ENV_PARA, 1.0f, 1.0f);
	pAction->Release();

	// perform the TAKE action
	{CParam params[] = { m_pActionTick, 0, 2000, L"down", m_pBody, m_pScene, L"Sphere02.Sphere02" };
	m_pFWDevice->CreateObjectEx(L"Action", L"Take", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->Release();

	// perform the SWING action
	{CParam params[] = { m_pActionTick, 500, 900, m_pBody, DEG2RAD(40)};
	m_pFWDevice->CreateObjectEx(L"Action", L"Swing", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->Release();

	pT->Release();
}

void CFreeWillView::OnTestB7()
{
}

void CFreeWillView::OnTestB8()
{
}

/////////////////////////////////////////////////////////////////////////////
// TESTS & DEMOS --- SET D

void CFreeWillView::OnTestD1()
{
}

void CFreeWillView::OnTestD2()
{
}

void CFreeWillView::OnTestD3()
{
}

void CFreeWillView::OnTestD4()
{
}

void CFreeWillView::OnTestD5()
{
}

void CFreeWillView::OnTestD6()
{
}

void CFreeWillView::OnTestD7()
{
}

void CFreeWillView::OnTestD8()
{
}


/////////////////////////////////////////////////////////////////////////////
// TESTS & DEMOS --- SET A

void CFreeWillView::OnTestA1()
{
	// Reach Action Test
	OnTestPrepare(L"scene_ball.3d", false);

	ITransform *pT = NULL;
	IFWUnknown *p = NULL;
	m_pScene->CreateCompatibleTransform(&pT);

//	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"RotateTo", m_pActionTick, 0, 1500, m_pBody, BODY_HAND + BODY_RIGHT, pT, m_pBody, BODY_ARM + BODY_RIGHT);
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Reach",    m_pActionTick, 0, 2000, L"right", m_pBody, BODY_FINGER + BODY_MIDDLE, m_pScene, L"Sphere02.Sphere02");

	pT->Release();
}

void CFreeWillView::OnTestA2()
{
	// Reach Action Test (2)
	OnTestPrepare(L"scene_ball.3d");
	OnTestA1();
}

void CFreeWillView::OnTestA3()
{
	// GRASP Action Test
	IAction *pAction = NULL;
	{ CParam params[] = { 
		  m_pActionTick, 0, 2000, L"right", m_pBody };
	m_pFWDevice->CreateObjectEx(L"Action", L"Grasp", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->Release();
}

void CFreeWillView::OnTestA4()
{
	OnTestPrepare(L"lobby.3d", true);

	IAction *pAction = NULL;

	// perform the STEP action
	{CParam params[] = { m_pActionTick, 0, 500, L"left", m_pBody, 11.0f};
	m_pFWDevice->CreateObjectEx(L"Action", L"Step", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->Release();

	// perform the STEP action
	{CParam params[] = { m_pActionTick, 500, 500, L"right", m_pBody, 22.0f};
	m_pFWDevice->CreateObjectEx(L"Action", L"Step", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->Release();

	// perform the STEP action
	{CParam params[] = { m_pActionTick, 1000, 500, L"left", m_pBody, 22.0f};
	m_pFWDevice->CreateObjectEx(L"Action", L"Step", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->Release();

	// perform the STEP action
	{CParam params[] = { m_pActionTick, 1500, 500, L"right", m_pBody, 22.0f};
	m_pFWDevice->CreateObjectEx(L"Action", L"Step", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->Release();

	// perform the STEP action
	{CParam params[] = { m_pActionTick, 2000, 500, L"left", m_pBody, 11.0f};
	m_pFWDevice->CreateObjectEx(L"Action", L"Step", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->Release();

	{CParam params[] = { 
		  m_pActionTick, 2300, 1000, L"right", m_pBody, m_pScene, L"Box03.Box03", 4.7f, 9.3f, -0.3f };
	m_pFWDevice->CreateObjectEx(L"Action", L"Point", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->Release();
}

void CFreeWillView::OnTestA5()
{
	HRESULT h;
	IKineChild *pChild = NULL;
	h = m_pScene->GetChild(L"Bip01", &pChild);
	if (FAILED(h)) return;

	IKineNode *pRep = NULL;
	pChild->ReproduceEx(IID_IKineNode, (IFWUnknown**)&pRep);
	pChild->Release();

	static float vX = 16.0f;
	ITransform *pT = NULL;
	pRep->GetLocalTransformRef(&pT);
	pT->MulTranslationXYZ(0, -vX, 0);
	pT->Release();
	vX += 16.0f;

	OLECHAR buf[11];
	m_pScene->CreateUniqueLabel(L"Bip01", 10, buf);
	m_pScene->AddChild(buf, pRep);
//	pRep->CreateFlatNamespace();

	GetDocument()->UpdateAllViews(this, RELOAD_SCENE_STRUCT);

	pRep->Release();

	GetDocument()->StoreSceneState();
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
// LOBBY SCENE

IKineNode *CFreeWillView::Reproduce(LPOLESTR pLabel)
{
	HRESULT h;
	IKineChild *pChild = NULL;
	h = m_pScene->GetChild(pLabel, &pChild);
	if (FAILED(h)) return NULL;

	IKineNode *pRep = NULL;
	pChild->ReproduceEx(IID_IKineNode, (IFWUnknown**)&pRep);
	pChild->Release();

	OLECHAR buf[11];
	m_pScene->CreateUniqueLabel(pLabel, 10, buf);
 
	m_pScene->AddChild(buf, pRep);
//	pRep->CreateFlatNamespace();
	return pRep;
}

void CFreeWillView::MoveNode(IKineChild *pNode, FWFLOAT x, FWFLOAT y, FWFLOAT z)
{
	ITransform *pT = NULL;
	pNode->GetLocalTransformRef(&pT);
	pT->MulTranslationXYZ(x, y, z);
	pT->Release();
	pNode->Invalidate();
}

void CFreeWillView::RotNodeX(IKineChild *pNode, FWFLOAT rot)
{
	ITransform *pT = NULL;
	pNode->GetLocalTransformRef(&pT);
	pT->MulRotationX(rot);
	pT->Release();
	pNode->Invalidate();
}

void CFreeWillView::RotNodeY(IKineChild *pNode, FWFLOAT rot)
{
	ITransform *pT = NULL;
	pNode->GetLocalTransformRef(&pT);
	pT->MulRotationY(rot);
	pT->Release();
	pNode->Invalidate();
}

void CFreeWillView::RotNodeZ(IKineChild *pNode, FWFLOAT rot)
{
	ITransform *pT = NULL;
	pNode->GetLocalTransformRef(&pT);
	pT->MulRotationZ(rot);
	pT->Release();
	pNode->Invalidate();
}

FWULONG CFreeWillView::Walk(LPOLESTR pLabel, FWULONG nTime, FWFLOAT rt1, FWFLOAT fd, FWFLOAT rt2)
{
	// calculate walk params with fd and rt2
	rt2 -= rt1;
	FWFLOAT fDist = sqrt(fd * fd + rt2 * rt2);
	int nSteps = (int)fDist / 10;
	FWFLOAT fStep = fDist / nSteps;
	FWFLOAT fDir = atan2(rt2, fd);

	// retrieve or reproduce and move to rt1 position
	IKineNode *pNode;
	if (pLabel)
	{
		m_pScene->GetChild(pLabel, (IKineChild**)&pNode);
	}
	else
	{
		pNode = Reproduce(L"Bip01");
		MoveNode(pNode, rt1, 0, 0);
	}

	// rotate
	IKineChild *pChild = NULL;
	pNode->GetChild(L"Bip01", &pChild);
	ITransform *pT = NULL;
	pChild->GetLocalTransformRef(&pT);
	pT->MulRotationZ(fDir);
	pT->Release();
	pChild->Invalidate();
	pChild->Release();

	// load body
	IBody *pBody = NULL;
	m_pFWDevice->CreateObject(L"Body", IID_IBody, (IFWUnknown**)&pBody);
	pBody->LoadBody(pNode, BODY_SCHEMA_DISCREET);

	// perform!
	IAction *pAction;
	int flagLR = 0;

	// first step
	{CParam params[] = { m_pActionTick, nTime, 500, flagLR ? L"left" : L"right", pBody, fStep/2.0};
	m_pFWDevice->CreateObjectEx(L"Action", L"Step", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->Release();
	nTime += 500;
	flagLR = 1 - flagLR;

	// following steps
	for (int i = 0; i < nSteps - 1; i++)
	{
		{CParam params[] = { m_pActionTick, nTime, 500, flagLR ? L"left" : L"right", pBody, fStep};
		m_pFWDevice->CreateObjectEx(L"Action", L"Step", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
		pAction->Release();
		nTime += 500;
		flagLR = 1 - flagLR;
	}

	// last step
	{CParam params[] = { m_pActionTick, nTime, 500, flagLR ? L"left" : L"right", pBody, fStep/2.0};
	m_pFWDevice->CreateObjectEx(L"Action", L"Step", sizeof(params)/sizeof(FWPARAM), params, IID_IAction, (IFWUnknown**)&pAction);}
	pAction->Release();
	nTime += 500;
	flagLR = 1 - flagLR;

	pNode->Release();
	pBody->Release();
	return nTime;
}

void CFreeWillView::OnTestA7()
{
	OnTestPrepare(L"lobby.3d", true);

	// move camera
	IKineChild *pCamera = NULL;
	m_pScene->GetChild(L"Camera01", &pCamera);
	RotNodeX(pCamera, (FWFLOAT)DEG2RAD(30));
	MoveNode(pCamera, -34, 147, 50);
	pCamera->Release();

	// enlarge the lobby
	IKineNode *pNode;
	pNode = Reproduce(L"Box03");
	MoveNode(pNode, -165, 0, 0);
	pNode->Release();
	pNode = Reproduce(L"Box02");
	MoveNode(pNode, -165, 0, 0);
	pNode->Release();
	pNode = Reproduce(L"Box02");
	MoveNode(pNode, 0, 110, 0);
	pNode->Release();
	pNode = Reproduce(L"Box02");
	MoveNode(pNode, -165, 110, 0);
	pNode->Release();

	// move biped
	IKineChild *pBip = NULL;
	m_pScene->GetChild(L"Bip01", &pBip);
	MoveNode(pBip, -240, 200, 0);
	pBip->Release();

	GetDocument()->StoreSceneState();
}

void CFreeWillView::OnTestA8()
{
//	OnTestPrepare(L"lobby.3d", true);

	FWULONG nTime = 0, t;
	
	// lift #1
	t = Walk(NULL, 0,		0,		244,	20);	nTime = max(nTime, t);
	t = Walk(NULL, 3000,	0,		204,	40);	nTime = max(nTime, t);
	t = Walk(NULL, 5200,	140,	224,	50);	nTime = max(nTime, t);
	
	// lift #2
	t = Walk(NULL, 0,		130,	244,	130);	nTime = max(nTime, t);
	t = Walk(NULL, 1000,	0,		224,	100);	nTime = max(nTime, t);
	t = Walk(NULL, 1200,	200,	204,	150);	nTime = max(nTime, t);
	t = Walk(NULL, 4000,	50,		204,	110);	nTime = max(nTime, t);
	t = Walk(NULL, 4300,	190,	180,	115);	nTime = max(nTime, t);
	
	// lift #3
	t = Walk(NULL, 0,		0,		224,	207);	nTime = max(nTime, t);
	t = Walk(NULL, 3500,	290,	204,	205);	nTime = max(nTime, t);
	t = Walk(NULL, 2000,	100,	204,	190);	nTime = max(nTime, t);
	t = Walk(NULL, 7000,	330,	214,	235);	nTime = max(nTime, t);
	
	// lift #4
	t = Walk(NULL, 0   ,	360,	244,	310);	nTime = max(nTime, t);
	t = Walk(NULL, 2000,	320,	204,	300);	nTime = max(nTime, t);
	t = Walk(NULL, 2000,	200,	244,	260);	nTime = max(nTime, t);
	t = Walk(NULL, 4500,	280,	184,	290);	nTime = max(nTime, t);
	t = Walk(NULL, 6200,	280,	204,	270);	nTime = max(nTime, t);
}

void CFreeWillView::OnTestA6()
{
}


/////////////////////////////////////////////////////////////////////////////
// TESTS & DEMOS --- SET C


//void CFreeWillView::OnTestC7()
//{
//	OnTestPrepare(L"lobby.3d", true);
//
//	FWDOUBLE fAngle = 0.0;
//
//	ITransform *pT;
//	IKineChild *p;
//	FWVECTOR v;
//	
//	p = m_pBody->BodyChild(BODY_ROOT);
//	p->GetBaseTransformRef(&pT);
//	pT->AsVector(&v);
//	pT->Reset(TRUE, TRUE);
//	pT->MulRotationZ(DEG2RAD(-90));
//	pT->MulTranslationVector(&v);
//	pT->Orthonormalize();
//	p->Invalidate();
//	pT->Release();
//	p->Release();
//
//	p = m_pBody->BodyChild(BODY_PELVIS);
//	p->GetBaseTransformRef(&pT);
//	pT->AsVector(&v);
//	pT->Reset(TRUE, TRUE);
//	pT->MulRotationX(DEG2RAD(-90));
//	pT->MulRotationY(DEG2RAD(-90));
//	pT->MulTranslationVector(&v);
//	p->Invalidate();
//	pT->Release();
//	p->Release();
//
//	p = m_pBody->BodyChild(BODY_LEG);
//	p->GetBaseTransformRef(&pT);
//	pT->AsVector(&v);
//	pT->Reset(TRUE, TRUE);
//	pT->MulRotationZ(DEG2RAD(fAngle));
//	pT->MulRotationY(DEG2RAD(180));
//	pT->MulTranslationVector(&v);
//	p->Invalidate();
//	pT->Release();
//	p->Release();
//
//	p = m_pBody->BodyChild(BODY_LEG+1);
//	p->GetBaseTransformRef(&pT);
//	pT->AsVector(&v);
//	pT->Reset(TRUE, TRUE);
//	pT->MulRotationZ(DEG2RAD(-fAngle-fAngle));
//	pT->MulTranslationVector(&v);
//	p->Invalidate();
//	pT->Release();
//	p->Release();
//
//	p = m_pBody->BodyChild(BODY_FOOT);
//	p->GetBaseTransformRef(&pT);
//	pT->AsVector(&v);
//	pT->Reset(TRUE, TRUE);
//	pT->MulRotationZ(DEG2RAD(fAngle));
//	pT->MulTranslationVector(&v);
//	p->Invalidate();
//	pT->Release();
//	p->Release();
//
//	p = m_pBody->BodyChild(BODY_LEG + BODY_LEFT);
//	p->GetBaseTransformRef(&pT);
//	pT->AsVector(&v);
//	pT->Reset(TRUE, TRUE);
//	pT->MulRotationZ(DEG2RAD(fAngle));
//	pT->MulRotationY(DEG2RAD(180));
//	pT->MulTranslationVector(&v);
//	p->Invalidate();
//	pT->Release();
//	p->Release();
//
//	p = m_pBody->BodyChild(BODY_LEG+1 + BODY_LEFT);
//	p->GetBaseTransformRef(&pT);
//	pT->AsVector(&v);
//	pT->Reset(TRUE, TRUE);
//	pT->MulRotationZ(DEG2RAD(-fAngle-fAngle));
//	pT->MulTranslationVector(&v);
//	p->Invalidate();
//	pT->Release();
//	p->Release();
//
//	p = m_pBody->BodyChild(BODY_FOOT + BODY_LEFT);
//	p->GetBaseTransformRef(&pT);
//	pT->AsVector(&v);
//	pT->Reset(TRUE, TRUE);
//	pT->MulRotationZ(DEG2RAD(fAngle));
//	pT->MulTranslationVector(&v);
//	p->Invalidate();
//	pT->Release();
//	p->Release();
//}


void CFreeWillView::OnTestC4()
{
	OnTestPrepare(L"lobby.3d", true);
//	OnTestC7();

	FWULONG uDur = 500;
	bool bRight = true;
	IFWUnknown *p = NULL;

	_defDiag();

	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, uDur,  L"left", m_pBody, CParam(10, L"length"));

	int N = 4;
	for (int j = 0; j < N; j++)
	{
		for (int i = 0; i < 8; i++)
		{
			p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, uDur,  m_pBody, CParam(10, L"length"), DEG2RAD(-22.5));
			p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, uDur,  m_pBody, CParam(10, L"length"), DEG2RAD(-22.5));
		}

		for (int i = 0; i < 8; i++)
		{
			p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, uDur,  m_pBody, CParam(10, L"length"), DEG2RAD(22.5));
			if (i < 8-1 || j < N-1)
				p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, uDur,  m_pBody, CParam(10, L"length"), DEG2RAD(22.5));
			else
				p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, uDur,  m_pBody, DEG2RAD(0), DEG2RAD(22.5));
		}
	}
}

void DelObj(IKineNode *p, FWSTRING pLabel)
{
	IKineChild *pChild = NULL;
	ISceneObject *pObj = NULL;
	if (SUCCEEDED(p->GetChild(pLabel, &pChild)) && pChild && SUCCEEDED(pChild->QueryInterface(&pObj)))
		pObj->PutVisible(FALSE);
	if (pChild) pChild->Release(); if (pObj) pObj->Release();
}

void CFreeWillView::OnTestC5()
{
	OnTestPrepare(L"lobby.3d", true);
	DelObj(m_pScene, L"Box03");

	FWULONG t = 0;
	FWULONG nDur = 1000;

	IFWUnknown *p = NULL;

	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur,  m_pBody, DEG2RAD(20.0f));
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur,  m_pBody, DEG2RAD(20.0f));
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur,  m_pBody, DEG2RAD(20.0f));
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur,  m_pBody, DEG2RAD(40.0f));
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur,  m_pBody, DEG2RAD(40.0f));
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur,  m_pBody, DEG2RAD(40.0f));
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur,  m_pBody, DEG2RAD(20.0f));
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur,  m_pBody, DEG2RAD(20.0f));
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur,  m_pBody, DEG2RAD(20.0f));
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur,  m_pBody, DEG2RAD(15.0f));
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur,  m_pBody, DEG2RAD(15.0f));
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur,  m_pBody, DEG2RAD(15.0f));
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur,  m_pBody, DEG2RAD(0));

	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Point", m_pActionTick, p, 1000, L"right", m_pBody, m_pScene, L"Box03.Box03", 4.7f, 9.3f, -0.3f );
}

void CFreeWillView::OnTestC1()
{
	OnTestPrepare(L"lobby.3d", true);
	FWULONG t = 0;
	FWULONG nDur = 500;
	IFWUnknown *p = NULL;
	
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur, L"left;back;down", m_pBody, CParam(12, L"forward"));
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur, L"left ; dupa;;;down", m_pBody, CParam(12, L"forward"), CParam(4, L"left"));
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur, L"   left;back;down", m_pBody, CParam(12, L"forward"));
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur, L" ; ; ; left back; down ", m_pBody, CParam(12, L"forward"), CParam(4, L"right"));
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur, L" left;back;down ; ", m_pBody, CParam(12, L"forward"));
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur, L"left;back;down", m_pBody, DEG2RAD(0.0f), DEG2RAD(0));
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, 0, L"left;half", m_pBody, 0, 0 );
}

void CFreeWillView::OnTestC2()
{
	OnTestPrepare(L"lobby.3d", true);
	FWULONG t = 0;
	FWULONG nDur = 500;
	IFWUnknown *p = NULL;

	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur, m_pBody, CParam(12, L"length"));
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur, m_pBody, CParam(12, L"length"), DEG2RAD(30));
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur, m_pBody, CParam(12, L"length"));
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur, m_pBody, DEG2RAD(0.0f), DEG2RAD(-30));
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, 0, m_pBody, 0, 0 );
}

void CFreeWillView::OnTestC3()
{
	OnTestPrepare(L"lobby.3d", true);
	FWULONG t = 0;
	FWULONG nDur = 1500;
	IFWUnknown *p = NULL;

	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur, m_pBody,	DEG2RAD(20.0f), DEG2RAD(0.0f));
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur, m_pBody,	DEG2RAD(20.0f), DEG2RAD(0));
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur, m_pBody,	DEG2RAD(20.0f), DEG2RAD(0));
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur, m_pBody,	DEG2RAD(0.0f), DEG2RAD(0));
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, 0, L"left;half", m_pBody, 0, 0 );
}

void CFreeWillView::OnTestC6()
{
	OnTestPrepare(L"lobby.3d", true);
	DelObj(m_pScene, L"Box03");

	IFWUnknown *p = NULL;
	
	p = ::FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Walk", m_pActionTick, p, 150.0f, L"open", m_pBody, 30.0f, 0.0f, 15.0f, DEG2RAD(45));
	p = ::FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Walk", m_pActionTick, p, 150.0f, L"close", m_pBody, 40.0f, 40.0f, 15.0f, DEG2RAD(45));

}

void CFreeWillView::OnTestC7()
{
	OnTestPrepare(L"lobby.3d", false);
	DelObj(m_pScene, L"Box03");

	IFWUnknown *p = NULL;
	FWULONG nDur = 1000;

//	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur, L"right", m_pBody, DEG2RAD(20));
//	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur, L"", m_pBody, DEG2RAD(20));
//	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur, L"", m_pBody, DEG2RAD(20));
//	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur, L"", m_pBody, DEG2RAD(0));

//	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Turn", m_pActionTick, p, nDur, L"open", m_pBody, M_PI);
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Turn", m_pActionTick, p, nDur, L"close;force-left", m_pBody, M_PI);

//	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur, L"", m_pBody, DEG2RAD(20));
//	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur, L"", m_pBody, DEG2RAD(20));
//	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur, L"", m_pBody, DEG2RAD(0));
}

void CFreeWillView::OnTestC8()
{
//	OnTestPrepare(L"lobby.3d", true);
	DelObj(m_pScene, L"Box03");

	IFWUnknown *p = NULL;
	FWULONG nDur = 150;

//	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Walk", m_pActionTick, p, nDur, L"", m_pBody, 0.0f, -57.0f, 12);


	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Walk", m_pActionTick, p, nDur, L"open", m_pBody, -46.0f, -57.0f, 12);
	//p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Walk", m_pActionTick, p, nDur, L"open", m_pBody, -46.0f, -57.0f, 12);
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Walk", m_pActionTick, p, nDur, L"open", m_pBody, 118.0f, 50.0f, 12);
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Walk", m_pActionTick, p, nDur, L"open", m_pBody, -46.0f, 50.0f, 12);
	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Walk", m_pActionTick, p, nDur, L"close", m_pBody, 118.0f, -57.0f, 12);


	//FWULONG t = 0;
	//while (m_pActionTick->AnySubscriptionsLeft())
	//{
	//	m_pActionTick->RaiseEvent(t, EVENT_TICK, t, NULL);
	//	t += 500;

	//	m_pRenderer->BeginFrame();
	//	m_pScene->Render(m_pRenderer);
	//	m_pRenderer->EndFrame();

	//	//Sleep(500);
	//}

	
	
	
	
//	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Generic", m_pActionTick, 1000, 1000);


	//p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur, L"right", m_pBody, 0, CParam(DEG2RAD(60), L"bend"));
	//p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur, L"right", m_pBody, 0, CParam(DEG2RAD(60), L"bend"));
	//p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur, L"right", m_pBody, 0, CParam(DEG2RAD(60), L"bend"));
	//p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur, L"right", m_pBody, 0, CParam(DEG2RAD(60), L"bend"));
	
//	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur, L"", m_pBody, CParam(12, L"forward"));
//	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur, L"", m_pBody, 0);
//	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Wait", m_pActionTick, p, nDur, m_pBody, 20000);
//	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur, L"", m_pBody, CParam(12, L"forward"));
//	p = FWCreateObjWeakPtr(m_pFWDevice, L"Action", L"Step", m_pActionTick, p, nDur, L"", m_pBody, 0);
}


void CFreeWillView::OnViewReset()
{
	m_pRenderer->ResetDeviceEx(m_hWnd, 0, 0);
}


void CFreeWillView::OnViewFullscreen()
{
	HWND hWnd;
	m_pRenderer->GetWindow(&hWnd);
	if (hWnd == m_hWnd)
		m_pRenderer->ResetDeviceEx(NULL, 0, 0);
	else
		m_pRenderer->ResetDeviceEx(m_hWnd, 0, 0);
}


