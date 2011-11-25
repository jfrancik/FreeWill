// fwview.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "fwview.h"
#include "MainFrm.h"

#include "FreeWillDoc.h"
#include "LeftView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFreeWillApp

BEGIN_MESSAGE_MAP(CFreeWillApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};



// CFreeWillApp construction

CFreeWillApp::CFreeWillApp()
{
	m_pSplashWnd = NULL;
}


// The one and only CFreeWillApp object

CFreeWillApp theApp;

// CFreeWillApp initialization

bool _InstallLib(REFCLSID clsid, LPCTSTR path)
{
	HRESULT h;
	IUnknown *pUnknown;

	// check if already installed
	h = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IUnknown, (void**)&pUnknown);
	if (SUCCEEDED(h) && pUnknown)
	{
		pUnknown->Release();
		return true;
	}

	// try to register the server
	HINSTANCE hDll = LoadLibrary(path);
	if (!hDll) return false;
	HRESULT (*p)() = (HRESULT(*)())GetProcAddress(hDll, "DllRegisterServer");
	if (!p) return false;
	h = p();
	if (FAILED(h)) return false;

	// final verification...
	h = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IUnknown, (void**)&pUnknown);
	if (FAILED(h) || !pUnknown)
		return false;
	pUnknown->Release();
	return true;
}

void CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
	((CFreeWillApp*)AfxGetApp())->HideSplashWindow();
}

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}
MIDL_DEFINE_GUID(CLSID, CLSID_DX9Renderer,0xB98AF6DD,0x184F,0x4cb7,0xA3,0xDE,0xDD,0xE6,0x7D,0x83,0x11,0x85);
MIDL_DEFINE_GUID(CLSID, CLSID_Action,0x212122F6,0x0554,0x48e8,0x89,0x03,0x69,0xA1,0x46,0xEF,0xFF,0x40);

BOOL CFreeWillApp::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("FreeWill+"));
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)
	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate *pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CFreeWillDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CLeftView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);
	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);
	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.


	// splash window
//	ShowSplashWindow();
	
	// Fast installation procedure...
	if (!_InstallLib(CLSID_FWDevice, L"freewill")
		|| !_InstallLib(CLSID_DX9Renderer, L"fwrender")
		|| !_InstallLib(CLSID_Action, L"fwaction"))
	{
		AfxMessageBox(L"This application cannot be run because one of its essential modules "
			L"could not be found.\nPlease re-install the application to fix the problem.");
		return TRUE;
	}

	// Checking for last opened file...
	CString filename = GetProfileString(L"Recent File List", L"File1", L"");
	CFile f;
	if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileNew && !filename.IsEmpty() && f.Open(filename, CFile::modeRead))
	{
		cmdInfo.m_nShellCommand = CCommandLineInfo::FileOpen;
		cmdInfo.m_strFileName = filename;
	}
	f.Abort();

	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW/*MAXIMIZED*/);

	m_pMainWnd->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	SetTimer(NULL, 99, 1500, TimerProc);

	return TRUE;
}

void CFreeWillApp::ShowSplashWindow()
{
	m_pSplashWnd = new CAboutDlg;
	((CAboutDlg*)m_pSplashWnd)->Create(IDD_ABOUTBOX);
	((CAboutDlg*)m_pSplashWnd)->CenterWindow();
	((CAboutDlg*)m_pSplashWnd)->SetWindowPos(&CWnd::wndTopMost, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
	((CAboutDlg*)m_pSplashWnd)->ShowWindow(SW_SHOW);
	((CAboutDlg*)m_pSplashWnd)->UpdateWindow();
}

void CFreeWillApp::HideSplashWindow()
{
	if (m_pSplashWnd) delete m_pSplashWnd; m_pSplashWnd = NULL;
}

// CAboutDlg dialog used for App About

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void CFreeWillApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// CFreeWillApp message handlers


BOOL CFreeWillApp::OnIdle(LONG lCount)
{
	CWinApp::OnIdle(lCount);
	((CFrameWnd*)AfxGetMainWnd())->GetActiveDocument()->UpdateAllViews(NULL, WM_PAINT);
	return 0;
}
