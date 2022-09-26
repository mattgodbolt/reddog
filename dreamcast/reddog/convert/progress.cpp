// Progress.cpp : implementation file
//

#include "stdafx.h"
#include "Convert.h"
#include "Progress.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProgress dialog


CProgress::CProgress(CWnd* pParent /*=NULL*/)
	: CDialog(CProgress::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProgress)
	//}}AFX_DATA_INIT
	Create (IDD, pParent);
}


void CProgress::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProgress)
	DDX_Control(pDX, IDC_EXTRA, m_ExtraText);
	DDX_Control(pDX, IDC_EXPLAIN, m_ExplainText);
	DDX_Control(pDX, IDC_PROGRESS1, m_ProgressCtrl);
	//}}AFX_DATA_MAP
}

void CProgress::SetText (CString text)
{
	m_ExplainText.SetWindowText (text);
}

void CProgress::SetExtra (CString text)
{
	m_ExtraText.SetWindowText (text);
}

void CProgress::SetProgress (int amount)
{
	m_ProgressCtrl.SetPos (amount);
}

BEGIN_MESSAGE_MAP(CProgress, CDialog)
	//{{AFX_MSG_MAP(CProgress)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProgress message handlers
