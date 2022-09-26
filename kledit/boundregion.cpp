#include "stdafx.h"
#include "boundregion.h"

ParamUIDesc BOUNDREGIONdesParam[] = {
  	ParamUIDesc (PB_BOUND_PARENT,		TYPE_SINGLECHEKBOX,	IDC_BOUNDPARENT),
  	ParamUIDesc (PB_BOUND_RADIUS,	EDITTYPE_FLOAT, IDC_BOUNDRADIUS, IDC_BOUNDRADIUS_SPIN, 0.f, 50.f, 0.05f)
};

ParamBlockDescID BOUNDREGIONdescVer1[] = {
	{ TYPE_BOOL,	NULL, FALSE, PB_BOUND_PARENT },
};

ParamBlockDescID BOUNDREGIONdescVer2[] = {
  	{ TYPE_BOOL,	NULL, FALSE, PB_BOUND_PARENT },
  	{ TYPE_FLOAT,	NULL, FALSE, PB_BOUND_RADIUS },
};

static ParamVersionDesc versions[] = {
	ParamVersionDesc(BOUNDREGIONdescVer1, 1, 1)
};
static ParamVersionDesc currentDesc(BOUNDREGIONdescVer2, BOUNDREGION_PBLOCK_LENGTH, BOUNDREGION_CURRENT_VERSION);
#define NUM_OLDVERSIONS 1
IParamMap	*BoundRegion::pmapParam;						// the default parameter map
IObjParam	*BoundRegion::objParams;

BoundParamDlg::BoundParamDlg (void *r) { ref = (BoundRegion *)r; }

int BoundCreatorClass::proc (ViewExp *vpt, int msg, int point, 
					int flags, IPoint2 m, Matrix3& mat)
{
	static Point3 scale, oPoint;
	static IPoint2 sp1;
	switch (msg) {
	case MOUSE_FREEMOVE:
		vpt->SnapPreview(m,m,NULL, SNAP_IN_3D);
		break;
	case MOUSE_POINT:
		if (point==0) {
			ref->suspendSnap = 1;
			/* Place the helper */
			mat.SetTrans(vpt->SnapPoint(m,m,NULL,SNAP_IN_3D));
			oPoint = vpt->SnapPoint(m,m,NULL,SNAP_IN_3D);
		}
		if (point==1) {
			sp1 = m;
		}
		/* Have we done? */
		if (point==2) {
			ref->suspendSnap = 0;
			return CREATE_STOP;
		}
		break;
	case MOUSE_MOVE:
		if (point==1) {
			Point3 trans = mat.GetTrans();
			scale = vpt->SnapPoint(m,m,NULL,SNAP_IN_3D) - trans;
			scale = Point3 (scale.x, scale.y, 0.f);
			mat = ScaleMatrix (scale);
			mat.SetTrans (trans);
		} else if (point==2) {
			Point3 trans = mat.GetTrans();
			scale.z = vpt->SnapLength(vpt->GetCPDisp(oPoint,Point3(0,0,1),sp1,m,TRUE));
			mat = ScaleMatrix (scale);
			mat.SetTrans (trans);
		}
		break;
	case MOUSE_ABORT:
		ref->suspendSnap = 0;
		return CREATE_ABORT;
	}
	return CREATE_CONTINUE;
#if 0
	switch (msg) {
	case MOUSE_FREEMOVE:
		vpt->SnapPreview(m,m,NULL, SNAP_IN_3D);
		break;
	case MOUSE_POINT:
	case MOUSE_MOVE:
	   //	if (point==0)
	   //		ref->suspendSnap = 1;
		/* Place the helper */
		mat.SetTrans(vpt->SnapPoint(m,m,NULL,SNAP_IN_3D));
		/* Have we done? */
	   	if (point==1 && msg==MOUSE_POINT) {
	   //		ref->suspendSnap = 0;
			return CREATE_STOP;
		}
		break;
	case MOUSE_ABORT:
	 //	ref->suspendSnap = 0;
		return CREATE_ABORT;
	}
	return CREATE_CONTINUE;
#endif
}
BoundRegion::BoundRegion (void)
{
	MakeRefByID(FOREVER, 0, CreateParameterBlock(BOUNDREGIONdescVer2, BOUNDREGION_PBLOCK_LENGTH, BOUNDREGION_CURRENT_VERSION));
	assert(pblock);

 //	pblock->SetValue (PB_IGN_PARENT, TimeValue(0), cRotates);
 //	pblock->SetValue (PB_COLL_RADIUS, TimeValue(0), cRadius);
}
void BoundRegion::WindowFinal (IObjParam *ip)
{
 //	pblock->GetValue (PB_IGN_PARENT,	ip->GetTime(), *((int *)&cRotates), FOREVER);
 //	pblock->GetValue (PB_COLL_RADIUS,	ip->GetTime(), cRadius, FOREVER);
}

BOOL BoundParamDlg::DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		return FALSE;
		// Fall thru
	case WM_DESTROY:
		return TRUE;
	case WM_COMMAND:
	  switch (LOWORD(wParam))
	  {
		case IDC_BOUNDPARENT:
	 Animatable anim;
	 ISplineOps *iso = (ISplineOps*)GetCOREInterface()->GetInterface(I_SPLINEOPS);
   	 
	 if (iso)
	 iso->StartCommandMode(ScmCreateLine);

  return TRUE;
	 break;
	  }
	break;
	}
	return FALSE;
}

/*
 * Red Dog helper class description
 */
class boundregionDesc : public ClassDesc
{
public:
	int 			IsPublic() {return 1;}
   	void *			Create (BOOL loading = FALSE) {return new BoundRegion();}
	const TCHAR *	ClassName() {return GetString(IDS_BOUNDREGION);}
	SClass_ID		SuperClassID() {return HELPER_CLASS_ID;}
	Class_ID		ClassID() {return BOUNDREGION_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_HELPER_CATEGORY);}
	void			ResetClassParams (BOOL fileReset);
};

static boundregionDesc BoundRegionDesc;
ClassDesc* GetBoundDesc() {return &BoundRegionDesc;}

//TODO: Should implement this method to reset the plugin params when Max is reset
void boundregionDesc::ResetClassParams (BOOL fileReset) 
{

}

//IOResult camppoint::Load(ILoad *iload)
//{
//	iload->RegisterPostLoadCallback (new ParamBlockPLCB(versions, NUM_OLDVERSIONS, &currentDesc, this, 0));
//	return IO_OK;
//}

void BoundRegion::DrawMarker (GraphicsWindow *gw, bool Selected , bool Frozen )
{
	return;
	float radius;
	pblock->GetValue (PB_BOUND_RADIUS, 0, radius, FOREVER);

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
//void camppoint::GetLocalBoundBox (TimeValue t, INode *mat, ViewExp *vpt, Box3& box)
//{
   //	float radius;
   //	pblock->GetValue (PB_COLL_RADIUS, 0, radius, FOREVER);
   //	box = Box3(Point3(-radius,-radius,-radius), Point3(radius,radius,radius));
//}
