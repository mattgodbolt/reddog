/*
 * COMpoint.h
 */

#ifndef _COMpoint_H
#define _COMpoint_H

#include "HelpBase.h"
#include "resource.h"

#define COMPOINT_CLASS_ID_A 0x22f42c88
#define COMPOINT_CLASS_ID_B 0x267c699f
#define COMPOINT_CLASS_ID	Class_ID(COMPOINT_CLASS_ID_A, COMPOINT_CLASS_ID_B)

extern ParamUIDesc CPdescParam[];
//#define CP_PARAMDESC_LENGTH 0

extern ParamBlockDescID CPdescVer1[];
//#define CP_PBLOCK_LENGTH	0
//#define CP_CURRENT_VERSION	1

#define COMPOINT HelperBase<COMCreatorClass, IDD_COM, IDS_COM_CAT, COMParamDlg, COMPOINT_CLASS_ID_A, COMPOINT_CLASS_ID_B, CPdescParam, 0>

class COMCreatorClass; class COMpoint; class COMParamDlg;

class COMParamDlg : public ParamMapUserDlgProc
{
private:
	COMpoint	*ref;

public:
	COMParamDlg (COMPOINT *r);// { ref = (COMpoint *)r; }

	void DeleteThis() { delete this; }
	BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
};

/* Creation of a collision object */
class COMCreatorClass : public CreateMouseCallBack
{
private:
	COMpoint		*ref;
public:
	COMCreatorClass() {}
	void setRef (COMPOINT *newRef) 
	{ ref = (COMpoint *)newRef; }

	int proc (ViewExp *vpt, int msg, int point, 
		int flags, IPoint2 m, Matrix3& mat);
};

class COMpoint : public COMPOINT
{
private:
	friend class	COMCreatorClass;
	static bool		cRotates;
public:
	COMpoint ();
	virtual void WindowFinal (IObjParam *ip);
	virtual RefTargetHandle Clone (RemapDir& remap = NoRemap())
	{ 
		COMpoint *ret = new COMpoint();
		ret->ReplaceReference (0, pblock->Clone (remap));
		return (ret);
	}
};

extern ClassDesc *GetCOMDesc();

#endif
