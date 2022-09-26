/*
 * CollPoint.h
 */

#ifndef _COLLPOINT_H
#define _COLLPOINT_H

#include "HelpBase.h"
#include "resource.h"

#define COLLPOINT_CLASS_ID_A 0x22f42ee9
#define COLLPOINT_CLASS_ID_B 0x267c699f
#define COLLPOINT_CLASS_ID	Class_ID(COLLPOINT_CLASS_ID_A, COLLPOINT_CLASS_ID_B)

#define PB_IGN_PARENT		0
#define PB_COLL_RADIUS		1

extern ParamUIDesc CPdescParam[];
#define CP_PARAMDESC_LENGTH 2

extern ParamBlockDescID CPdescVer1[];
extern ParamBlockDescID CPdescVer2[];
#define CP_PBLOCK_LENGTH	2
#define CP_CURRENT_VERSION	2

#define COLLPOINT HelperBase<CreatorClass, IDD_COLLISION, IDS_COLLISION_CATEGORY, CollParamDlg, COLLPOINT_CLASS_ID_A, COLLPOINT_CLASS_ID_B, CPdescParam, CP_PARAMDESC_LENGTH>

class CreatorClass; class CollPoint; class CollParamDlg;

class CollParamDlg : public ParamMapUserDlgProc
{
private:
	CollPoint	*ref;

public:
	CollParamDlg (void *r);// { ref = (CollPoint *)r; }

	void DeleteThis() { delete this; }
	BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
};

/* Creation of a collision object */
class CreatorClass : public CreateMouseCallBack
{
private:
	CollPoint		*ref;
public:
	CreatorClass() {}
	void setRef (COLLPOINT *newRef) 
	{ ref = (CollPoint *)newRef; }

	int proc (ViewExp *vpt, int msg, int point, 
		int flags, IPoint2 m, Matrix3& mat);
};

class CollPoint : public COLLPOINT
{
private:
	friend class	CreatorClass;
	static bool		cRotates;
	static float	cRadius;
public:
	CollPoint ();
	virtual void WindowFinal (IObjParam *ip);
	virtual RefTargetHandle Clone (RemapDir& remap = NoRemap())
	{ 
		CollPoint *ret = new CollPoint();
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
		pblock->GetValue (PB_COLL_RADIUS, TimeValue(0), foo, iv); 
		return foo;
	}
	IOResult Load(ILoad *iload);
	virtual void DrawMarker (GraphicsWindow *gw, bool Selected = FALSE, bool Frozen = FALSE);
	virtual void GetLocalBoundBox (TimeValue t, INode *mat, ViewExp *vpt, Box3& box);
};


extern ClassDesc *GetCollDesc();

struct CollPt
{
	Point3	pt;
	bool	rotates;
	float	radius;
	CollPt (const Point3 &p, bool r, float rad) : pt(p), rotates(r),radius(rad) {}
	CollPt () {rotates=false;radius = 0;}
};

#endif
