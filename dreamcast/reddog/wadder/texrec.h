#if !defined(AFX_TEXREC_H__6D6A3023_EEFC_11D1_B3AC_0040051E90B2__INCLUDED_)
#define AFX_TEXREC_H__6D6A3023_EEFC_11D1_B3AC_0040051E90B2__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// TexRec.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTexRec DAO recordset

class CTexRec : public CDaoRecordset
{
public:
	CTexRec(CDaoDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CTexRec)

// Field/Param Data
	//{{AFX_FIELD(CTexRec, CDaoRecordset)
	long	m_ID;
	CString	m_Texture_filename;
	BOOL	m_Don_t_mipmap;
	BOOL	m_Don_t_dither;
	BOOL	m_Don_t_dither_alpha;
	BOOL	m_VQ_compress;
	BOOL	m_First_frame_of_animation;
	BOOL	m_GlobalAnim;
	long	m_GlobalSpeed;
	CString	m_StratName;
	long	m_GBIX;
	BOOL	m_BumpMap;
	BOOL	m_Punchthru;
	long	m_wadGBIX;
	//}}AFX_FIELD

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTexRec)
	public:
	virtual CString GetDefaultDBName();		// Default database name
	virtual CString GetDefaultSQL();		// Default SQL for Recordset
	virtual void DoFieldExchange(CDaoFieldExchange* pFX);  // RFX support
	//}}AFX_VIRTUAL

// Implementation
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXREC_H__6D6A3023_EEFC_11D1_B3AC_0040051E90B2__INCLUDED_)
