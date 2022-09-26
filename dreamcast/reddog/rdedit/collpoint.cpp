#include "StdAfx.h"
#include "CollPoint.h"

ParamUIDesc CPdescParam[] = {
	ParamUIDesc (PB_IGN_PARENT,		TYPE_SINGLECHEKBOX,	IDC_IGNPARENT),
	ParamUIDesc (PB_COLL_RADIUS,	EDITTYPE_FLOAT, IDC_RADIUS, IDC_RADIUS_SPIN, 0.f, 50.f, 0.05f)
};

ParamBlockDescID CPdescVer1[] = {
	{ TYPE_BOOL,	NULL, FALSE, PB_IGN_PARENT },
};
ParamBlockDescID CPdescVer2[] = {
	{ TYPE_BOOL,	NULL, FALSE, PB_IGN_PARENT },
	{ TYPE_FLOAT,	NULL, FALSE, PB_COLL_RADIUS },
};

static ParamVersionDesc versions[] = {
	ParamVersionDesc(CPdescVer1, 1, 1)
};
static ParamVersionDesc currentDesc(CPdescVer2, CP_PBLOCK_LENGTH, CP_CURRENT_VERSION);
#define NUM_OLDVERSIONS 1

IParamMap	*CollPoint::pmapParam;						// the default parameter map
IObjParam	*CollPoint::objParams;
bool		CollPoint::cRotates = TRUE;
float		CollPoint::cRadius	= 0.f;

CollParamDlg::CollParamDlg (void *r) { ref = (CollPoint *)r; }

int CreatorClass::proc (ViewExp *vpt, int msg, int point, 
						int flags, IPoint2 m, Matrix3& mat)
{
	switch (msg) {
	case MOUSE_FREEMOVE:
		vpt->SnapPreview(m,m,NULL, SNAP_IN_3D);
		break;
	case MOUSE_POINT:
	case MOUSE_MOVE:
		if (point==0)
			ref->suspendSnap = 1;
		/* Place the helper */
		mat.SetTrans(vpt->SnapPoint(m,m,NULL,SNAP_IN_3D));
		/* Have we done? */
		if (point==1 && msg==MOUSE_POINT) {
			ref->suspendSnap = 0;
			return CREATE_STOP;
		}
		break;
	case MOUSE_ABORT:
		ref->suspendSnap = 0;
		return CREATE_ABORT;
	}
	return CREATE_CONTINUE;
}


CollPoint::CollPoint (void)
{
	MakeRefByID(FOREVER, 0, CreateParameterBlock(CPdescVer2, CP_PBLOCK_LENGTH, CP_CURRENT_VERSION));
	assert(pblock);

	pblock->SetValue (PB_IGN_PARENT, TimeValue(0), cRotates);
	pblock->SetValue (PB_COLL_RADIUS, TimeValue(0), cRadius);
}
void CollPoint::WindowFinal (IObjParam *ip)
{
	pblock->GetValue (PB_IGN_PARENT,	ip->GetTime(), *((int *)&cRotates), FOREVER);
	pblock->GetValue (PB_COLL_RADIUS,	ip->GetTime(), cRadius, FOREVER);
}

BOOL CollParamDlg::DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		return FALSE;
		// Fall thru
	case WM_DESTROY:
		return TRUE;
	case WM_COMMAND:
		break;
	}
	return FALSE;
}

/*
 * Red Dog helper class description
 */
class CollPointDesc : public ClassDesc
{
public:
	int 			IsPublic() {return 1;}
	void *			Create (BOOL loading = FALSE) {return new CollPoint();}
	const TCHAR *	ClassName() {return GetString(IDS_COLLISION_CLASS_NAME);}
	SClass_ID		SuperClassID() {return HELPER_CLASS_ID;}
	Class_ID		ClassID() {return COLLPOINT_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_HELPER_CATEGORY);}
	void			ResetClassParams (BOOL fileReset);
};

static CollPointDesc CollisionPointDesc;
ClassDesc* GetCollDesc() {return &CollisionPointDesc;}

//TODO: Should implement this method to reset the plugin params when Max is reset
void CollPointDesc::ResetClassParams (BOOL fileReset) 
{

}

IOResult CollPoint::Load(ILoad *iload)
{
	iload->RegisterPostLoadCallback (new ParamBlockPLCB(versions, NUM_OLDVERSIONS, &currentDesc, this, 0));
	return IO_OK;
}

void CollPoint::DrawMarker (GraphicsWindow *gw, bool Selected , bool Frozen )
{
	float radius;
	pblock->GetValue (PB_COLL_RADIUS, 0, radius, FOREVER);

	Point3 pt(0,0,0);
	if(!Frozen)
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_POINT_OBJ));
	gw->marker(&pt,X_MRKR);
	if (radius != 0.f) {
		Point3 seg[2];
		gw->startSegments();
		seg[0] = Point3(-radius, 0.f, 0.f);
		seg[1] = Point3(radius, 0.f, 0.f);
		gw->segment(seg, 1);
		seg[0] = Point3(0.f, 0.f, -radius);
		seg[1] = Point3(0.f, 0.f, radius);
		gw->segment(seg, 1);
		seg[0] = Point3(0.f, -radius, 0.f);
		seg[1] = Point3(0.f, radius, 0.f);
		gw->segment(seg, 1);
	}
}
void CollPoint::GetLocalBoundBox (TimeValue t, INode *mat, ViewExp *vpt, Box3& box)
{
	float radius;
	pblock->GetValue (PB_COLL_RADIUS, 0, radius, FOREVER);
	box = Box3(Point3(-radius,-radius,-radius), Point3(radius,radius,radius));
}
