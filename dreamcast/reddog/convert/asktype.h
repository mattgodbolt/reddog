#if !defined(AFX_ASKTYPE_H__AA427DE0_C01B_11D2_BE00_00104B47455E__INCLUDED_)
#define AFX_ASKTYPE_H__AA427DE0_C01B_11D2_BE00_00104B47455E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AskType.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAskType dialog

class CAskType : public CDialog
{
// Construction
public:
	CAskType(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAskType)
	enum { IDD = IDD_ASKTYPE };
	BOOL	m_RegenerateFirst;
	CString	m_Greeting;
	int		m_WhatToDo;
	CString	m_StartDir;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAskType)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAskType)
	afx_msg void OnBrowse();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ASKTYPE_H__AA427DE0_C01B_11D2_BE00_00104B47455E__INCLUDED_)
