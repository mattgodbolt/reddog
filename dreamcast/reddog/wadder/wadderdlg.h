// WadderDlg.h : header file
//

#if !defined(AFX_WADDERDLG_H__5F4D2E6D_2864_11D2_BDFF_00104B47455E__INCLUDED_)
#define AFX_WADDERDLG_H__5F4D2E6D_2864_11D2_BDFF_00104B47455E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class TexLib; class WadFile;

#include "TexRec.h"

/////////////////////////////////////////////////////////////////////////////
// CWadderDlg dialog

class CWadderDlg : public CDialog
{
// Construction
public:
	CWadderDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CWadderDlg)
	enum { IDD = IDD_WADDER_DIALOG };
	CProgressCtrl	m_TexProgress;
	CEdit	m_LogCtrl;
	CButton	m_OK;
	CButton	m_Cancel;
	CString	m_Log;
	CString	m_WentOK;
	CString	m_TexRamAmt;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWadderDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CWadderDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	afx_msg LRESULT OnLogMessage (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTexRAMUpdate (WPARAM wParam, LPARAM lParam);

public:
	void Log (CString text);
};

class CWorker
{
private:
	HWND window;
	CString Tokenise (CString &ref);
	CTexRec record;
	bool RecurseReadObj (CFile &file, WadFile &wad, TexLib &wadTexList);
public:
	CWorker (HWND);
	void Log (CString text);
	// Get the GBIX of a texture
	unsigned long getGBIX (CString texture, unsigned long &wadGBIX);
	// Output a texture library
	void OutputTexLib (WadFile &file, TexLib &lib);
	// Worker
	UINT Go ();
	static UINT Worker (void *);
	int AddRDO (CString string, WadFile &wad, TexLib &wadTexList);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WADDERDLG_H__5F4D2E6D_2864_11D2_BDFF_00104B47455E__INCLUDED_)
