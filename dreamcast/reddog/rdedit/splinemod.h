/*
 * splinemod.h
 */

#ifndef _splinemod_H
#define _splinemod_H

#include "HelpBase.h"
#include "resource.h"

#define SPLINEMOD_CLASS_ID_A 0x51884a78
#define SPLINEMOD_CLASS_ID_B 0x107b1de7

#define SPLINEMOD_CLASS_ID	Class_ID(SPLINEMOD_CLASS_ID_A, SPLINEMOD_CLASS_ID_B)

#define PB_IGN_PARENT		0
#define PB_SPLINEMOD_RADIUS		1

extern ParamUIDesc SPLINEMODdesParam[];
#define SPLINEMOD_PARAMDESC_LENGTH 1

extern ParamBlockDescID SPLINEMODdescVer1[];
extern ParamBlockDescID SPLINEMODdescVer2[];
#define SPLINEMOD_PBLOCK_LENGTH	1
#define SPLINEMOD_CURRENT_VERSION	2

#define SPLINEMOD HelperBase<SplineModCreatorClass, IDD_SPLINEMOD, IDS_SPLINEMOD, SplineModParamDlg, SPLINEMOD_CLASS_ID_A, SPLINEMOD_CLASS_ID_B, SPLINEMODdesParam, SPLINEMOD_PARAMDESC_LENGTH>

class SplineModCreatorClass; class SplineMod; class SplineModParamDlg;

class SplineModParamDlg : public ParamMapUserDlgProc
{
private:
	SplineMod	*ref;

public:
	SplineModParamDlg (void *r);// { ref = (SplineModPoint *)r; }
	void Update();
	void DeleteThis() { delete this; }
	BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
};

/* Creation of a collision object */
class SplineModCreatorClass : public CreateMouseCallBack
{
private:
	SplineMod		*ref;
public:
	SplineModCreatorClass() {}
	void setRef (SPLINEMOD *newRef) 
	{ ref = (SplineMod *)newRef; }

	int proc (ViewExp *vpt, int msg, int point, 
		int flags, IPoint2 m, Matrix3& mat);
};

class SplineMod : public SPLINEMOD
{
private:
	friend class	CreatorClass;
public:
	SplineMod ();
	virtual void WindowFinal (IObjParam *ip);
	virtual RefTargetHandle Clone (RemapDir& remap = NoRemap())
	{ 
		SplineMod *ret = new SplineMod();
		ret->ReplaceReference (0, pblock->Clone (remap));
		return (ret);
	}
};


extern ClassDesc *GetSplineModDesc();


#endif
