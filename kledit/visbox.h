/*
 * VisBox.h
 */

#ifndef _VisBox_H
#define _VisBox_H

#include "HelpBase.h"
#include "resource.h"
#include "RedDog.h"

#define VISBOX_CLASS_ID_A 0x24f42c88
#define VISBOX_CLASS_ID_B 0x267c699f
#define VISBOX_CLASS_ID	Class_ID(VISBOX_CLASS_ID_A, VISBOX_CLASS_ID_B)

extern ParamUIDesc VBdescParam[];
extern ParamBlockDescID VBdescVer1[];

#define VB_DESCPARAM_SIZE 1
#define VISBOX HelperBase<VISCreatorClass, IDD_VISBOX, IDS_VIS_CAT, VISParamDlg, VISBOX_CLASS_ID_A, VISBOX_CLASS_ID_B, VBdescParam, VB_DESCPARAM_SIZE>

class VISCreatorClass; class VisBox; class VISParamDlg;

class VISParamDlg : public ParamMapUserDlgProc
{
private:
	VisBox	*ref;

public:
	VISParamDlg (VISBOX *r);

	void DeleteThis() { delete this; }
	BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
};

/* Creation of a collision object */
class VISCreatorClass : public CreateMouseCallBack
{
private:
	VisBox		*ref;
public:
	VISCreatorClass() {}
	void setRef (VISBOX *newRef) 
	{ ref = (VisBox *)newRef; }

	int proc (ViewExp *vpt, int msg, int point, 
		int flags, IPoint2 m, Matrix3& mat);
};

class VisBox : public VISBOX
{
private:
	friend class	VISCreatorClass;
public:
	VisBox ();
	virtual RefTargetHandle Clone (RemapDir& remap = NoRemap())
	{ 
		VisBox *ret = new VisBox();
		ret->ReplaceReference (0, pblock->Clone (remap));
		return (ret);
	}
	virtual void DrawMarker (GraphicsWindow *gw, bool Selected = FALSE, bool Frozen = FALSE);

	virtual void GetLocalBoundBox (TimeValue t, INode *mat, ViewExp *vpt, Box3& box)
	{
		box = Box3(Point3(0,0,0), Point3(1,1,1));
	}
	virtual int DoOwnSelectHilite ()	{ return 0; }

	RedDog::VisBox GetBox(Matrix3 &objToWorld);
	int GetExcludeAreas();
	void SetExcludeAreas(int areas);
};

extern ClassDesc *GetVISDesc();

#endif
