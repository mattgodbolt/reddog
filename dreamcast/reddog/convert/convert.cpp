// Convert.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Convert.h"
#include "ConvertDlg.h"
#include "TGAload.h"
#include "AskType.h"
	
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConvertApp

BEGIN_MESSAGE_MAP(CConvertApp, CWinApp)
	//{{AFX_MSG_MAP(CConvertApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConvertApp construction

CConvertApp::CConvertApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

CConvertApp::~CConvertApp()
{
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CConvertApp object

CConvertApp theApp;

static BOOL CALLBACK winCallBack (HWND wnd, LPARAM param)
{
	char buffer[1024];
	if (GetWindowText (wnd, buffer, 1024)) {
		if (!stricmp(buffer, "Red Dog Texture Manager")) {
			//SendMessage (wnd, WM_SYSCOMMAND, SC_RESTORE, 0);
			SetForegroundWindow (wnd);
			return FALSE;
		}
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CConvertApp initialization

BOOL CConvertApp::InitInstance()
{
	HANDLE Handle;
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Is another instance running?
	Handle = CreateMutex (NULL, TRUE, "Red Dog Texture Manager Mutex");
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		CloseHandle (Handle);
		EnumWindows (winCallBack, NULL);
		return FALSE;
	}

	CAskType ask;
	CConvertDlg dlg(ask);
	m_pMainWnd = &dlg;
	(void)dlg.DoModal();
	CloseHandle (Handle);

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
