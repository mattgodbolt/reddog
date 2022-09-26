// TexRec.cpp : implementation file
//

#include "stdafx.h"
#include "TexRec.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTexRec

IMPLEMENT_DYNAMIC(CTexRec, CDaoRecordset)

CTexRec::CTexRec(CDaoDatabase* pdb)
	: CDaoRecordset(pdb)
{
	//{{AFX_FIELD_INIT(CTexRec)
	m_ID = 0;
	m_Texture_filename = _T("");
	m_Don_t_mipmap = FALSE;
	m_Don_t_dither = FALSE;
	m_Don_t_dither_alpha = FALSE;
	m_VQ_compress = FALSE;
	m_First_frame_of_animation = FALSE;
	m_GlobalAnim = FALSE;
	m_GlobalSpeed = 0;
	m_StratName = _T("");
	m_GBIX		= 0xffffffff;
	m_BumpMap = FALSE;
	m_Punchthru = FALSE;
	m_wadGBIX = 0;
	m_nFields = 14;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dbOpenDynaset;
}


CString CTexRec::GetDefaultDBName()
{
	return _T("P:\\Textures\\Textures.mdb");
}

CString CTexRec::GetDefaultSQL()
{
	return _T("[Textures]");
}

void CTexRec::DoFieldExchange(CDaoFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(CTexRec)
	pFX->SetFieldType(CDaoFieldExchange::outputColumn);
	DFX_Long(pFX, _T("[ID]"), m_ID);
	DFX_Text(pFX, _T("[TexFilename]"), m_Texture_filename);
	DFX_Bool(pFX, _T("[Don't mipmap]"), m_Don_t_mipmap);
	DFX_Bool(pFX, _T("[Don't dither]"), m_Don_t_dither);
	DFX_Bool(pFX, _T("[Don't dither alpha]"), m_Don_t_dither_alpha);
	DFX_Bool(pFX, _T("[VQ compress]"), m_VQ_compress);
	DFX_Bool(pFX, _T("[First frame of animation]"), m_First_frame_of_animation);
	DFX_Bool(pFX, _T("[GlobalAnim]"), m_GlobalAnim);
	DFX_Long(pFX, _T("[GlobalSpeed]"), m_GlobalSpeed);
	DFX_Text(pFX, _T("[StratName]"), m_StratName);
	DFX_Long(pFX, _T("[GBIX]"), m_GBIX);
	DFX_Bool(pFX, _T("[BumpMap]"), m_BumpMap);
	DFX_Bool(pFX, _T("[Punchthru]"), m_Punchthru);
	DFX_Long(pFX, _T("[wadGBIX]"), m_wadGBIX);
	//}}AFX_FIELD_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CTexRec diagnostics

#ifdef _DEBUG
void CTexRec::AssertValid() const
{
	CDaoRecordset::AssertValid();
}

void CTexRec::Dump(CDumpContext& dc) const
{
	CDaoRecordset::Dump(dc);
}
#endif //_DEBUG
