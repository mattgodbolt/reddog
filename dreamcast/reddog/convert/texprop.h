#if !defined(AFX_TEXPROP_H__2572FEB0_CA76_11D1_B3AB_0040051E90B2__INCLUDED_)
#define AFX_TEXPROP_H__2572FEB0_CA76_11D1_B3AB_0040051E90B2__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// TexProp.h : header file
//

#include "tset.h"

/////////////////////////////////////////////////////////////////////////////
// CTexProp dialog

class CTexProp : public CDialog
{
// Construction
public:
	CTexProp(CString tgaFile, CWnd* pParent = NULL);   // standard constructor
	BOOL OnInitDialog();

// Dialog Data
	//{{AFX_DATA(CTexProp)
	enum { IDD = IDD_TEXPROP };
	CEdit	m_AnimNameCtrl;
	BOOL	m_BumpMap;
	BOOL	m_NoADither;
	BOOL	m_NoDither;
	BOOL	m_NoMipmap;
	CString	m_AnimName;
	UINT	m_AnimSpeed;
	BOOL	m_VQCompress;
	BOOL	m_IsAnimated;
	BOOL	m_PunchThru;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTexProp)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTexProp)
	afx_msg void OnOk();
	afx_msg void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void DoDisabling();

	TextureSettings set;
	CString tgaFile;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXPROP_H__2572FEB0_CA76_11D1_B3AB_0040051E90B2__INCLUDED_)
