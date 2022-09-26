// AskType.cpp : implementation file
//

#include "stdafx.h"
#include "convert.h"
#include "AskType.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAskType dialog


CAskType::CAskType(CWnd* pParent /*=NULL*/)
	: CDialog(CAskType::IDD, pParent)
{
	char buffer[1024];
	DWORD size;
	//{{AFX_DATA_INIT(CAskType)
	m_RegenerateFirst = FALSE;
	m_Greeting = _T("");
	m_WhatToDo = -1;
	m_StartDir = _T("");
	//}}AFX_DATA_INIT
	size = sizeof (buffer);
	GetUserName (buffer, &size);
	buffer[0] = toupper(buffer[0]);
	m_Greeting = CString("Yo ") + CString(buffer) + CString("!  What do you want to do?");
	m_WhatToDo = 0;
	CRegKey key;
	key.Open (HKEY_CURRENT_USER, "SOFTWARE\\Argonaut Software Ltd\\Red Dog\\Convert");
	char keyValue[1024];
	DWORD keyLength = 1024;
	strcpy (keyValue, "P:\\TEXTURES\\");
	key.QueryValue (keyValue, "Start Directory", &keyLength);
	m_StartDir = keyValue;
}


void CAskType::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAskType)
	DDX_Check(pDX, IDC_REGENERATE, m_RegenerateFirst);
	DDX_Text(pDX, IDC_GREETS, m_Greeting);
	DDX_Radio(pDX, IDC_DEFAULT, m_WhatToDo);
	DDX_Text(pDX, IDC_START_DIR, m_StartDir);
	DDV_MaxChars(pDX, m_StartDir, 256);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAskType, CDialog)
	//{{AFX_MSG_MAP(CAskType)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CAskType::OnBrowse() 
{
	UpdateData();
	CString file = m_StartDir;
	if (file.Right(1) != "\\")
		file += "\\";
	CFileDialog dialog (TRUE, "tga", file + "ChooseAnyTexture.tga", 
		OFN_EXPLORER | OFN_ENABLESIZING | OFN_READONLY | OFN_PATHMUSTEXIST | OFN_ENABLESIZING | OFN_EXPLORER,
		NULL, this);
	if (dialog.DoModal() == IDOK) {
		m_StartDir = dialog.GetPathName();
		// Strip off the filename
		m_StartDir = m_StartDir.Mid (0, m_StartDir.GetLength() - dialog.GetFileName().GetLength());
		if (m_StartDir.Left (11).CompareNoCase("P:\\TEXTURES"))
			m_StartDir = "P:\\Textures";
		CRegKey key;
		key.Open (HKEY_CURRENT_USER, "SOFTWARE\\Argonaut Software Ltd\\Red Dog\\Convert");
		key.SetValue (m_StartDir, "Start Directory");
		UpdateData (FALSE);
	}
}

void CAskType::OnOK() 
{
	CDialog::OnOK();

	CRegKey key;
	key.Open (HKEY_CURRENT_USER, "SOFTWARE\\Argonaut Software Ltd\\Red Dog\\Convert");
	key.SetValue (m_StartDir, "Start Directory");
}
