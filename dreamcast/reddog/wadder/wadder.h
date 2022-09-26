// Wadder.h : main header file for the WADDER application
//

#if !defined(AFX_WADDER_H__5F4D2E6B_2864_11D2_BDFF_00104B47455E__INCLUDED_)
#define AFX_WADDER_H__5F4D2E6B_2864_11D2_BDFF_00104B47455E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CWadderApp:
// See Wadder.cpp for the implementation of this class
//

class CWadderApp : public CWinApp
{
public:
	CWadderApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWadderApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CWadderApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WADDER_H__5F4D2E6B_2864_11D2_BDFF_00104B47455E__INCLUDED_)
