// Convert.h : main header file for the CONVERT application
//

#if !defined(AFX_CONVERT_H__2572FEA5_CA76_11D1_B3AB_0040051E90B2__INCLUDED_)
#define AFX_CONVERT_H__2572FEA5_CA76_11D1_B3AB_0040051E90B2__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CConvertApp:
// See Convert.cpp for the implementation of this class
//

class CConvertApp : public CWinApp
{
private:
public:
	CConvertApp();
	~CConvertApp();

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConvertApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CConvertApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONVERT_H__2572FEA5_CA76_11D1_B3AB_0040051E90B2__INCLUDED_)
