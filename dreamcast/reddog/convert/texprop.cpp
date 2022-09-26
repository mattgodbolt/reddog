// TexProp.cpp : implementation file
//

#include "stdafx.h"
#include "Convert.h"
#include "TexProp.h"
#include <fstream.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTexProp dialog


CTexProp::CTexProp(CString tgaFile, CWnd* pParent /*=NULL*/)
	: CDialog(CTexProp::IDD, pParent), set(tgaFile)
{
	//{{AFX_DATA_INIT(CTexProp)
	m_BumpMap = FALSE;
	m_NoADither = FALSE;
	m_NoDither = FALSE;
	m_NoMipmap = FALSE;
	m_AnimName = _T("");
	m_AnimSpeed = 0;
	m_VQCompress = FALSE;
	m_IsAnimated = FALSE;
	m_PunchThru = FALSE;
	//}}AFX_DATA_INIT
		
	this->tgaFile = tgaFile;
	Create (IDD, pParent);
}

void CTexProp::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTexProp)
	DDX_Control(pDX, IDC_ANIM_NAME, m_AnimNameCtrl);
	DDX_Check(pDX, IDC_BUMPMAP, m_BumpMap);
	DDX_Check(pDX, IDC_NOADITHER, m_NoADither);
	DDX_Check(pDX, IDC_NODITHER, m_NoDither);
	DDX_Check(pDX, IDC_NOMIPMAP, m_NoMipmap);
	DDX_Text(pDX, IDC_ANIM_NAME, m_AnimName);
	DDX_Text(pDX, IDC_ANIM_SPEED, m_AnimSpeed);
	DDV_MinMaxUInt(pDX, m_AnimSpeed, 0, 512);
	DDX_Check(pDX, IDC_VQCOMPRESS, m_VQCompress);
	DDX_Check(pDX, IDC_ISANIMATED, m_IsAnimated);
	DDX_Check(pDX, IDC_PUNCHTHRU, m_PunchThru);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTexProp, CDialog)
	//{{AFX_MSG_MAP(CTexProp)
	ON_BN_CLICKED(ID_OK, OnOk)
	ON_BN_CLICKED(ID_CANCEL, OnCancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CTexProp::OnInitDialog()
{
	m_AnimName = set.animName;
	m_AnimSpeed = set.animSpeed;
	m_NoMipmap = set.dontMipmap;
	m_NoDither = set.dontDither;
	m_NoADither = set.dontDitherAlpha;
	m_VQCompress = set.vqCompress;
	m_IsAnimated = set.firstAnimFrame;
	m_BumpMap = set.bumpMap;
	m_PunchThru = set.punchThru;
	CheckRadioButton (IDC_ANIM_GLOBAL, IDC_ANIM_STRAT, set.globalAnim ? IDC_ANIM_GLOBAL : IDC_ANIM_STRAT);

	CDialog::OnInitDialog();

	DoDisabling();
	return TRUE;
}

void CTexProp::DoDisabling()
{
	if (IsDlgButtonChecked (IDC_ANIM_GLOBAL)) {
		m_AnimNameCtrl.EnableWindow (FALSE);
	} else {
		m_AnimNameCtrl.EnableWindow ();
	}
}

void CTexProp::OnOk() 
{
	
	UpdateData();

	set.dontMipmap = m_NoMipmap?true:false;
	set.dontDither = m_NoDither?true:false;
	set.dontDitherAlpha = m_NoADither?true:false;
	set.vqCompress = m_VQCompress?true:false;
	set.firstAnimFrame = m_IsAnimated?true:false;
	set.bumpMap = m_BumpMap?true:false;
	set.globalAnim = IsDlgButtonChecked(IDC_ANIM_GLOBAL) ? true : false;
	set.punchThru = m_PunchThru?true:false;
	
	set.animName = m_AnimName;
	
	set.animSpeed = m_AnimSpeed;

	set.WriteBack(tgaFile);
	
	EndDialog(1);
	delete this; // dodgy
}

void CTexProp::OnCancel() 
{
	EndDialog(0);	
	delete this; // dodgy
}

BOOL CTexProp::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch (wParam) {
	case IDC_ANIM_GLOBAL:
	case IDC_ANIM_STRAT:
		DoDisabling();
		break;
	}
	return CDialog::OnCommand(wParam, lParam);
}
