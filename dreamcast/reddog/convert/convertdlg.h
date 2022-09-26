// ConvertDlg.h : header file
//

#if !defined(AFX_CONVERTDLG_H__2572FEA7_CA76_11D1_B3AB_0040051E90B2__INCLUDED_)
#define AFX_CONVERTDLG_H__2572FEA7_CA76_11D1_B3AB_0040051E90B2__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "Progress.h"
#include "TexProp.h"
#include "TexRec.h"
#include "AskType.h"

/////////////////////////////////////////////////////////////////////////////
// CConvertDlg dialog

class CConvertDlg : public CDialog
{
// Construction
public:
	CConvertDlg(CAskType &ask, CWnd* pParent = NULL);	// standard constructor
	virtual ~CConvertDlg(); // Destructor
	void RegenerateDatabase (void);
	

// Dialog Data
	//{{AFX_DATA(CConvertDlg)
	enum { IDD = IDD_CONVERT_DIALOG };
	CListCtrl	m_TextureList;
	BOOL	m_FilterAnimated;
	BOOL	m_FilterCompressed;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConvertDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON		m_hIcon;
	CImageList	*m_ImageList;

	int			EnumList (bool onlySelected, CStringArray &array);
	int			FillListBox (BOOL all = 100);
	BOOL		isUpToDate (CString fileName);
	CString		setName;
	void		RecurseRegenerate (CTexRec &record, CString startDirectory, CProgress &prog);
	void		DoFunkyStuff (void);
	CAskType	&parms;
	
	// Generated message map functions
	//{{AFX_MSG(CConvertDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnOk();
	afx_msg void OnRefresh();
	afx_msg void OnRclickTexturelist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkTexturelist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnShowall();
	afx_msg void OnConvert();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	static	CString		getPVRFile (CString fileName);
	static	CString		getSETFile (CString fileName);
	static	CString		getTGAFile (CString fileName);

};

#include "tset.h"
//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONVERTDLG_H__2572FEA7_CA76_11D1_B3AB_0040051E90B2__INCLUDED_)
