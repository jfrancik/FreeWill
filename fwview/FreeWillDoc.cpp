// FreeWillDoc.cpp : implementation of the CFreeWillDoc class
//

#include "stdafx.h"
#include "fwview.h"
#include <afxpriv.h>	// for WM_INITIALUPDATE

#include "FreeWillDoc.h"
#include ".\freewilldoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFreeWillDoc

IMPLEMENT_DYNCREATE(CFreeWillDoc, CDocument)

BEGIN_MESSAGE_MAP(CFreeWillDoc, CDocument)
	ON_COMMAND(ID_PLAY_PAUSE, OnPlayPause)
	ON_UPDATE_COMMAND_UI(ID_PLAY_PAUSE, OnUpdatePlayPause)
	ON_COMMAND(ID_PLAY_PLAY, OnPlayPlay)
	ON_UPDATE_COMMAND_UI(ID_PLAY_PLAY, OnUpdatePlayPlay)
	ON_COMMAND(ID_PLAY_STOP, OnPlayStop)
	ON_UPDATE_COMMAND_UI(ID_PLAY_STOP, OnUpdatePlayStop)
	ON_COMMAND(ID_PLAY_REWIND, OnPlayRewind)
	ON_UPDATE_COMMAND_UI(ID_PLAY_REWIND, OnUpdatePlayRewind)
	ON_COMMAND(ID_PLAY_FORWARD, OnPlayForward)
	ON_UPDATE_COMMAND_UI(ID_PLAY_FORWARD, OnUpdatePlayForward)
	ON_COMMAND(ID_MANIPULATE_MOVE, OnManipulateMove)
	ON_UPDATE_COMMAND_UI(ID_MANIPULATE_MOVE, OnUpdateManipulateMove)
	ON_COMMAND(ID_MANIPULATE_ROTATE, OnManipulateRotate)
	ON_UPDATE_COMMAND_UI(ID_MANIPULATE_ROTATE, OnUpdateManipulateRotate)
	ON_COMMAND(ID_MANIPULATE_RESET, OnManipulateReset)
	ON_UPDATE_COMMAND_UI(ID_MANIPULATE_RESET, OnUpdateManipulateReset)
	ON_COMMAND(ID_MANIPULATE_RESETALL, OnManipulateResetAll)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_PLAYTIME, OnUpdateIndicatorPlaytime)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_NAME, OnUpdateIndicatorName)
	ON_COMMAND(ID_MANIPULATE_POINT, OnManipulatePoint)
	ON_UPDATE_COMMAND_UI(ID_MANIPULATE_POINT, OnUpdateManipulatePoint)
	ON_COMMAND(ID_MANIPULATE_LOCAL, OnManipulateLocal)
	ON_UPDATE_COMMAND_UI(ID_MANIPULATE_LOCAL, OnUpdateManipulateLocal)
	ON_COMMAND(ID_MANIPULATE_VIEW, OnManipulateView)
	ON_UPDATE_COMMAND_UI(ID_MANIPULATE_VIEW, OnUpdateManipulateView)
	ON_COMMAND(ID_MANIPULATE_ARBITRARY, OnManipulateArbitrary)
	ON_UPDATE_COMMAND_UI(ID_MANIPULATE_ARBITRARY, OnUpdateManipulateArbitrary)
	ON_COMMAND(ID_MANIPULATE_SET_ARBITRARY, OnManipulateSetArbitrary)
	ON_UPDATE_COMMAND_UI(ID_MANIPULATE_SET_ARBITRARY, OnUpdateManipulateSetArbitrary)
	ON_COMMAND(ID_ACCELARATION_2XFASTER, OnAccelaration2xfaster)
	ON_UPDATE_COMMAND_UI(ID_ACCELARATION_2XFASTER, OnUpdateAccelaration2xfaster)
	ON_COMMAND(ID_ACCELARATION_2XSLOWER, OnAccelaration2xslower)
	ON_UPDATE_COMMAND_UI(ID_ACCELARATION_2XSLOWER, OnUpdateAccelaration2xslower)
	ON_COMMAND(ID_ACCELARATION_4XFASTER, OnAccelaration4xfaster)
	ON_UPDATE_COMMAND_UI(ID_ACCELARATION_4XFASTER, OnUpdateAccelaration4xfaster)
	ON_COMMAND(ID_ACCELARATION_4XSLOWER, OnAccelaration4xslower)
	ON_UPDATE_COMMAND_UI(ID_ACCELARATION_4XSLOWER, OnUpdateAccelaration4xslower)
	ON_COMMAND(ID_ACCELARATION_NORMAL, OnAccelarationNormal)
	ON_UPDATE_COMMAND_UI(ID_ACCELARATION_NORMAL, OnUpdateAccelarationNormal)
END_MESSAGE_MAP()


// CFreeWillDoc construction/destruction

CFreeWillDoc::CFreeWillDoc()
{
	m_pFWDevice = NULL;
	m_pRenderer = NULL;
	m_pScene = NULL;
	m_pBody = NULL;
	m_pSceneBuf = NULL;
	m_nSceneBufCount = 0;
	m_pCurrent = NULL;
	m_poleLabel = m_poleManipAxis = NULL;
	SetManipulationMode(ID_MANIPULATE_POINT);
	SetManipAxisMode(ID_MANIPULATE_VIEW);
	m_pManipAxis = NULL;
}

CFreeWillDoc::~CFreeWillDoc()
{
	if (m_pRenderer) m_pRenderer->Release();
	if (m_pScene) m_pScene->Release();
	if (m_pBody) m_pBody->Release();
	if (m_pSceneBuf) delete [] m_pSceneBuf;
	if (m_pCurrent) m_pCurrent->Release();
	if (m_pManipAxis) m_pManipAxis->Release();
	if (m_pFWDevice) while (m_pFWDevice->Release());
}

void CFreeWillDoc::GetFreeWillObjects(IFWDevice **ppFWDevice, IRenderer **ppRenderer, IScene **ppScene, IBody **ppBody)
{
	if (ppFWDevice) { *ppFWDevice = m_pFWDevice; if (m_pFWDevice) m_pFWDevice->AddRef(); }
	if (ppRenderer) { *ppRenderer = m_pRenderer; if (m_pRenderer) m_pRenderer->AddRef(); }
	if (ppScene) { *ppScene = m_pScene; if (m_pScene) m_pScene->AddRef(); }
	if (ppBody) { *ppBody = m_pBody; if (m_pBody) m_pBody->AddRef(); }
}

void CFreeWillDoc::SetFreeWillObjects(IFWDevice *pFWDevice, IRenderer *pRenderer, IScene *pScene, IBody *pBody)
{
	if (m_pFWDevice) m_pFWDevice->Release();
	m_pFWDevice = pFWDevice;
	if (m_pFWDevice) m_pFWDevice->AddRef();
	if (m_pRenderer) m_pRenderer->Release();
	m_pRenderer = pRenderer;
	if (m_pRenderer) m_pRenderer->AddRef();
	if (m_pScene) m_pScene->Release();
	m_pScene = pScene;
	if (m_pScene) m_pScene->AddRef();
	if (m_pBody) m_pBody->Release();
	m_pBody = pBody;
	if (m_pBody) m_pBody->AddRef();
}

IKineNode *CFreeWillDoc::GetCurrentNode()
{ 
	if (m_pCurrent) m_pCurrent->AddRef(); 
	return m_pCurrent; 
}

void CFreeWillDoc::SetCurrentNode(IKineNode *p)
{ 
	FWULONG n;
	if (m_pCurrent) n = m_pCurrent->Release();
	m_pCurrent = p;
	if (m_pCurrent)
	{
		m_pCurrent->AddRef();
		m_pCurrent->GetLabel(&m_poleLabel); 
	}
	else
	{
		m_poleLabel = L""; 
	}
}

void CFreeWillDoc::GetManipArbitraryAxis(IKineChild **p)	
{ 
	if (!p) return; 
	*p = m_pManipAxis; 
	if (m_pManipAxis) m_pManipAxis->AddRef(); 
}

void CFreeWillDoc::SetManipArbitraryAxis(IKineChild *p)	
{ 
	if (m_pManipAxis) m_pManipAxis->Release(); 
	m_pManipAxis = NULL;
	if (p) 
	{
		m_pManipAxis = p;
		m_pManipAxis->AddRef();
		m_pManipAxis->GetLabel(&m_poleManipAxis);
	}
}

void CFreeWillDoc::LoadFromFile(LPCTSTR fname)
{
	//USES_CONVERSION;

	m_pScene->Turnoff(m_pRenderer);
	m_pScene->DelAll();

	// load the 3D model
	IFileLoader *pLoader;
	m_pFWDevice->CreateObject(L"FileLoader", IID_IFileLoader, (IFWUnknown**)&pLoader);

	pLoader->LoadScene((LPTSTR)fname, m_pScene);
	pLoader->Release();


	IKineNode *pBody = NULL;
	if (SUCCEEDED(m_pScene->GetChild(L"Bip01", (IKineChild**)&pBody)) && pBody)
	{
		m_pBody->LoadBody(pBody, BODY_SCHEMA_DISCREET);
		pBody->Release();
	}

	// store scene state
	StoreSceneState();
}

void CFreeWillDoc::StoreSceneState()
{
	if (!m_pScene) return;
	m_nSceneBufCount = 0;
	m_pScene->StoreState(0, NULL, &m_nSceneBufCount);
	if (m_pSceneBuf) delete [] m_pSceneBuf;
	m_pSceneBuf = new BYTE[m_nSceneBufCount];
	m_pScene->StoreState(m_nSceneBufCount, m_pSceneBuf, NULL);
}

void CFreeWillDoc::ResetSceneState()
{
	if (!m_pScene || !m_pSceneBuf) return;
	m_pScene->RetrieveState(m_nSceneBufCount, m_pSceneBuf, NULL);
	m_pScene->Invalidate();
}


BOOL CFreeWillDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	if (m_pScene) m_pScene->Turnoff(m_pRenderer);
	if (m_pScene) m_pScene->DelAll();

	// store scene state
	StoreSceneState();

	return TRUE;
}

// CFreeWillDoc serialization

void CFreeWillDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ASSERT(FALSE);
	}
	else
	{
		if (!m_pFWDevice) 
			SendInitialUpdate();

		ASSERT(m_pFWDevice && m_pRenderer && m_pScene);
		if (!m_pFWDevice || !m_pRenderer || !m_pScene)
			return;

		LoadFromFile(ar.GetFile()->GetFilePath());
	}
}

// CFreeWillDoc diagnostics

#ifdef _DEBUG
void CFreeWillDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CFreeWillDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CFreeWillDoc commands

void CFreeWillDoc::OnPlayPlay()
{
//	if (m_pRenderer) m_pRenderer->PutTotalPlayingTime(4000);
	if (m_pRenderer) m_pRenderer->Play();
}

void CFreeWillDoc::OnUpdatePlayPlay(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_pRenderer != NULL);
}

void CFreeWillDoc::OnPlayPause()
{
	if (m_pRenderer) m_pRenderer->Pause();
}

void CFreeWillDoc::OnUpdatePlayPause(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_pRenderer && m_pRenderer->IsPlaying() == S_OK);
	pCmdUI->SetCheck(m_pRenderer && m_pRenderer->IsPaused() == S_OK);
}

void CFreeWillDoc::OnPlayStop()
{
	if (m_pRenderer) m_pRenderer->Stop();
}

void CFreeWillDoc::OnUpdatePlayStop(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_pRenderer && m_pRenderer->IsPlaying() == S_OK);
}

void CFreeWillDoc::OnPlayRewind()
{
}

void CFreeWillDoc::OnUpdatePlayRewind(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(FALSE);
}

void CFreeWillDoc::OnPlayForward()
{
}

void CFreeWillDoc::OnUpdatePlayForward(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(FALSE);
}

void CFreeWillDoc::OnManipulatePoint()
{
	SetManipulationMode(ID_MANIPULATE_POINT);
}

void CFreeWillDoc::OnUpdateManipulatePoint(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(GetManipulationMode() == ID_MANIPULATE_POINT);
}

void CFreeWillDoc::OnManipulateMove()
{
	SetManipulationMode(ID_MANIPULATE_MOVE);
}

void CFreeWillDoc::OnUpdateManipulateMove(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_pCurrent != NULL);
	pCmdUI->SetRadio(GetManipulationMode() == ID_MANIPULATE_MOVE);
}

void CFreeWillDoc::OnManipulateRotate()
{
	SetManipulationMode(ID_MANIPULATE_ROTATE);
}

void CFreeWillDoc::OnUpdateManipulateRotate(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_pCurrent != NULL);
	pCmdUI->SetRadio(GetManipulationMode() == ID_MANIPULATE_ROTATE);
}

void CFreeWillDoc::OnManipulateLocal()
{
	SetManipAxisMode(ID_MANIPULATE_LOCAL);
}

void CFreeWillDoc::OnUpdateManipulateLocal(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_pCurrent != NULL);
	pCmdUI->SetRadio(GetManipAxisMode() == ID_MANIPULATE_LOCAL);
}

void CFreeWillDoc::OnManipulateView()
{
	SetManipAxisMode(ID_MANIPULATE_VIEW);
}

void CFreeWillDoc::OnUpdateManipulateView(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_pCurrent != NULL);
	pCmdUI->SetRadio(GetManipAxisMode() == ID_MANIPULATE_VIEW);
}

void CFreeWillDoc::OnManipulateArbitrary()
{
	SetManipAxisMode(ID_MANIPULATE_ARBITRARY);
}

void CFreeWillDoc::OnUpdateManipulateArbitrary(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_pManipAxis != NULL && m_pCurrent != NULL);
	pCmdUI->SetRadio(GetManipAxisMode() == ID_MANIPULATE_ARBITRARY);
	CString str;
	str.Format(ID_MANIPULATE_ARBITRARY_MENUTXT, m_poleManipAxis ? m_poleManipAxis : L"<NULL>");
	pCmdUI->SetText(str);
}

void CFreeWillDoc::OnManipulateSetArbitrary()
{
	if (m_pCurrent) this->SetManipArbitraryAxis(m_pCurrent);
}

void CFreeWillDoc::OnUpdateManipulateSetArbitrary(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_pCurrent != NULL);
	CString str;
	str.Format(ID_MANIPULATE_SET_ARBITRARY_MENUTXT, m_poleLabel ? m_poleLabel : L"<NULL>");
	pCmdUI->SetText(str);
}

void CFreeWillDoc::OnManipulateReset()
{
	m_pCurrent->Reset();
}

void CFreeWillDoc::OnManipulateResetAll()
{
	ResetSceneState();	
}


void CFreeWillDoc::OnUpdateManipulateReset(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_pCurrent != NULL);
}

void CFreeWillDoc::OnUpdateIndicatorPlaytime(CCmdUI *pCmdUI)
{
	if (!m_pRenderer)
	{
		pCmdUI->SetText(L"");
		return;
	}
	
	FWLONG nMsec;
	m_pRenderer->GetPlayTime(&nMsec);
	CString str;
	if (m_pRenderer->IsPaused() == S_OK)
		str.Format(ID_INDICATOR_PLAYTIME_3, ((FWFLOAT)nMsec) / 1000.0f);
	else
	if (m_pRenderer->IsPlaying() == S_OK)
		str.Format(ID_INDICATOR_PLAYTIME, ((FWFLOAT)nMsec) / 1000.0f);
	else
	if (nMsec)
		str.Format(ID_INDICATOR_PLAYTIME_4, ((FWFLOAT)nMsec) / 1000.0f);
	else
		str.Format(ID_INDICATOR_PLAYTIME_2, ((FWFLOAT)nMsec) / 1000.0f);
	pCmdUI->SetText(str);
}

void CFreeWillDoc::OnUpdateIndicatorName(CCmdUI *pCmdUI)
{
	USES_CONVERSION;
	if (m_poleLabel)
		pCmdUI->SetText(m_poleLabel);
}


void CFreeWillDoc::OnAccelaration2xfaster()
{
	m_pRenderer->PutAccel(2.0f);
}

void CFreeWillDoc::OnUpdateAccelaration2xfaster(CCmdUI *pCmdUI)
{
	FWFLOAT fAccel;
	m_pRenderer->GetAccel(&fAccel);
	pCmdUI->SetRadio(fAccel == 2.0f);
}

void CFreeWillDoc::OnAccelaration2xslower()
{
	m_pRenderer->PutAccel(0.5f);
}

void CFreeWillDoc::OnUpdateAccelaration2xslower(CCmdUI *pCmdUI)
{
	FWFLOAT fAccel;
	m_pRenderer->GetAccel(&fAccel);
	pCmdUI->SetRadio(fAccel == 0.5f);
}

void CFreeWillDoc::OnAccelaration4xfaster()
{
	m_pRenderer->PutAccel(4.0f);
}

void CFreeWillDoc::OnUpdateAccelaration4xfaster(CCmdUI *pCmdUI)
{
	FWFLOAT fAccel;
	m_pRenderer->GetAccel(&fAccel);
	pCmdUI->SetRadio(fAccel == 4.0f);
}

void CFreeWillDoc::OnAccelaration4xslower()
{
	m_pRenderer->PutAccel(0.25f);
}

void CFreeWillDoc::OnUpdateAccelaration4xslower(CCmdUI *pCmdUI)
{
	FWFLOAT fAccel;
	m_pRenderer->GetAccel(&fAccel);
	pCmdUI->SetRadio(fAccel == 0.25f);
}

void CFreeWillDoc::OnAccelarationNormal()
{
	m_pRenderer->PutAccel(1.0f);
}

void CFreeWillDoc::OnUpdateAccelarationNormal(CCmdUI *pCmdUI)
{
	FWFLOAT fAccel;
	m_pRenderer->GetAccel(&fAccel);
	pCmdUI->SetRadio(fAccel == 1.0f);
}

