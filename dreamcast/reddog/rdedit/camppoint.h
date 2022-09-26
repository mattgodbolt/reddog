/*
 * camppoint.h
 */

#ifndef _camppoint_H
#define _camppoint_H

#include "HelpBase.h"
#include "resource.h"

#define CAMPPOINT_CLASS_ID_A 0x28516850
#define CAMPPOINT_CLASS_ID_B 0xcc495118

#define CAMPPOINT_CLASS_ID	Class_ID(CAMPPOINT_CLASS_ID_A, CAMPPOINT_CLASS_ID_B)

#define PB_IGN_PARENT		0
#define PB_CAMP_RADIUS		1

extern ParamUIDesc CAMPPOINTdesParam[];
#define CAMPPOINT_PARAMDESC_LENGTH 2

extern ParamBlockDescID CAMPPOINTdescVer1[];
extern ParamBlockDescID CAMPPOINTdescVer2[];
#define CAMPPOINT_PBLOCK_LENGTH	2
#define CAMPPOINT_CURRENT_VERSION	2

#define CAMPPOINT HelperBase<CampCreatorClass, IDD_CAMPPOINT, IDS_CAMPPOINT, CampParamDlg, CAMPPOINT_CLASS_ID_A, CAMPPOINT_CLASS_ID_B, CAMPPOINTdesParam, CAMPPOINT_PARAMDESC_LENGTH>

class CampCreatorClass; class CampPoint; class CampParamDlg;

class CampParamDlg : public ParamMapUserDlgProc
{
private:
	CampPoint	*ref;

public:
	CampParamDlg (void *r);// { ref = (CampPoint *)r; }

	void DeleteThis() { delete this; }
	BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
};

/* Creation of a collision object */
class CampCreatorClass : public CreateMouseCallBack
{
private:
	CampPoint		*ref;
public:
	CampCreatorClass() {}
	void setRef (CAMPPOINT *newRef) 
	{ ref = (CampPoint *)newRef; }

	int proc (ViewExp *vpt, int msg, int point, 
		int flags, IPoint2 m, Matrix3& mat);
};

class CampPoint : public CAMPPOINT
{
private:
	friend class	CampCreatorClass;
	static bool		cRotates;
	static float	cRadius;
public:
	CampPoint ();
	virtual void WindowFinal (IObjParam *ip);
	virtual RefTargetHandle Clone (RemapDir& remap = NoRemap())
	{ 
		CampPoint *ret = new CampPoint();
		ret->ReplaceReference (0, pblock->Clone (remap));
		return (ret);
	}
	bool Rotates() const 
	{ 
		int foo;
		Interval iv = FOREVER;
		pblock->GetValue (PB_IGN_PARENT, TimeValue(0), foo, iv); 
		return foo==1?TRUE:FALSE;
	}
	float GetRadius() const
	{ 
		float foo;
		Interval iv = FOREVER;
		pblock->GetValue (PB_CAMP_RADIUS, TimeValue(0), foo, iv); 
		return foo;
	}
   //	IOResult Load(ILoad *iload);
	virtual void DrawMarker (GraphicsWindow *gw, bool Selected = FALSE, bool Frozen = FALSE);
   //	virtual void GetLocalBoundBox (TimeValue t, INode *mat, ViewExp *vpt, Box3& box);
};


extern ClassDesc *GetCampDesc();

struct CampPt
{
	Point3	pt;
	bool	rotates;
	float	radius;
	CampPt (const Point3 &p, bool r, float rad) : pt(p), rotates(r),radius(rad) {}
	CampPt () {rotates=false;radius = 0;}
};

#endif
