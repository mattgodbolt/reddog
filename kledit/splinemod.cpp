#include "stdafx.h"
#include "SPLINE3D.h"
#include "shape.h"
#include "splinemod.h"
#include "splshape.h"

ParamUIDesc SPLINEMODdesParam[] = {
  	ParamUIDesc (PB_IGN_PARENT,		TYPE_SINGLECHEKBOX,	IDC_SPLINEMODPARENT),
};

ParamBlockDescID SPLINEMODdescVer1[] = {
	{ TYPE_BOOL,	NULL, FALSE, PB_IGN_PARENT },
};

ParamBlockDescID SPLINEMODdescVer2[] = {
	{ TYPE_BOOL,	NULL, FALSE, PB_IGN_PARENT },
};

static ParamVersionDesc versions[] = {
	ParamVersionDesc(SPLINEMODdescVer1, 1, 1)
};
static ParamVersionDesc currentDesc(SPLINEMODdescVer2, SPLINEMOD_PBLOCK_LENGTH, SPLINEMOD_CURRENT_VERSION);
#define NUM_OLDVERSIONS 1
IParamMap	*SplineMod::pmapParam;						// the default parameter map
IObjParam	*SplineMod::objParams;

SplineModParamDlg::SplineModParamDlg (void *r) { ref = (SplineMod *)r; }

int SplineModCreatorClass::proc (ViewExp *vpt, int msg, int point, 
					int flags, IPoint2 m, Matrix3& mat)
{
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
}
SplineMod::SplineMod (void)
{
	MakeRefByID(FOREVER, 0, CreateParameterBlock(SPLINEMODdescVer2, SPLINEMOD_PBLOCK_LENGTH, SPLINEMOD_CURRENT_VERSION));
	assert(pblock);

 //	pblock->SetValue (PB_IGN_PARENT, TimeValue(0), cRotates);
 //	pblock->SetValue (PB_COLL_RADIUS, TimeValue(0), cRadius);
}
void SplineMod::WindowFinal (IObjParam *ip)
{
 //	pblock->GetValue (PB_IGN_PARENT,	ip->GetTime(), *((int *)&cRotates), FOREVER);
 //	pblock->GetValue (PB_COLL_RADIUS,	ip->GetTime(), cRadius, FOREVER);
}

//float test = 0.0;
void SplineModParamDlg::Update (void)
{
	/*go to the selected and delete it*/
	Point3	a, b, c, d, tempv, tempv2, av, n, tangent, v;
	float l1;

	int NUMSELS = GetCOREInterface()->GetSelNodeCount();

	if (NUMSELS)
	{
		INode  *deleteme = GetCOREInterface()->GetSelNode(0);

	 	//SEE IF ITS A SPLINE


	   Object *nodeobj = deleteme->GetObjectRef();
		if (nodeobj->SuperClassID() == SHAPE_CLASS_ID)
		{

			SplineShape *ob = (SplineShape*)nodeobj;
			
			if (ob->CanMakeBezier())
			{
 				int splinenum = ob->shape.splineCount;

				for (int loop=0;loop<splinenum;loop++)
				{

				Spline3D *spline = ob->shape.GetSpline(loop);

			
				   
			//TEST STUFF 
 //		  	spline->SetAux2(0,test) ;
//			spline->DeleteKnot(0);

			//RETURNS KNOT COUNT
			int knot = spline->KnotCount();
#define SPLINE_TURN 0.3333f
  
			for (int i=0; i<knot-1; i++)
			{
				
				if (i == 0)
				{
					a = spline->GetKnotPoint(i);
					d = spline->GetKnotPoint(i+1);
					tempv = d - a;
					tempv2 = tempv;
					tempv *= SPLINE_TURN;
					b = tempv + a;
					
	   				if (i+2 == knot)
						c = d - tempv;
					else
					{
						tempv = d - spline->GetKnotPoint(i+2);
						l1 = Length(tempv2);
						tempv = Normalize(tempv);
						tempv2 = Normalize(tempv2);
						av = tempv + tempv2;
						av *= 0.5f;
						n = CrossProd(tempv, tempv2);
						tangent = CrossProd(n, av);
						tangent = Normalize(tangent);
						v = tangent;
						v *= SPLINE_TURN * l1;
						c = d - v;
					}
				}
				else if (i == knot - 2)
				{
					a = spline->GetKnotPoint(i);
					d = spline->GetKnotPoint(i+1);
					tempv = a - d;
					tempv2 = tempv;
					tempv *= SPLINE_TURN;
					c = tempv + d;
					tempv = a - spline->GetKnotPoint(i-1);
					l1 = Length(tempv2);
					tempv = Normalize(tempv);
					tempv2 = Normalize(tempv2);
					av = tempv + tempv2;
					av *= 0.5f;
					n = CrossProd(tempv, tempv2);
					tangent = CrossProd(n, av);
					tangent = Normalize(tangent);
					v = tangent;
					v *= SPLINE_TURN * l1;
					b = a - v;
				}
				else
				{
					a = spline->GetKnotPoint(i);
					d = spline->GetKnotPoint(i+1);
					tempv2 = d - a;
					tempv = d - spline->GetKnotPoint(i+2);
					l1 = Length(tempv2);
					tempv = Normalize(tempv);
					tempv2 = Normalize(tempv2);
					av = tempv + tempv2;
					av *= 0.5f;
					n = CrossProd(tempv, tempv2);
					tangent = CrossProd(n, av);
					tangent = Normalize(tangent);
					v = tangent;
					v *= SPLINE_TURN * l1;
					c = d - v;
					tempv2 = a - d;
					tempv = a - spline->GetKnotPoint(i-1);
					tempv = Normalize(tempv);
					tempv2 = Normalize(tempv2);
					av = tempv + tempv2;
					av *= 0.5f;
					n = CrossProd(tempv, tempv2);
					tangent = CrossProd(n, av);
					tangent = Normalize(tangent);
					v = tangent;
					v *= SPLINE_TURN * l1;
					b = a - v;
				}
				
				spline->SetOutVec(i,b);
				spline->SetInVec(i+1,c);
			}

			
			
			//TO SEE THE AFFECTS OF YOUR ACTIONS
			ob->shape.UpdateSels();
		  	ob->shape.InvalidateGeomCache();
			ob->NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
			GetCOREInterface()->ForceCompleteRedraw();
			}
			}
		}
	}

}

BOOL SplineModParamDlg::DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		return FALSE;
		// Fall thru
	case WM_DESTROY:
		return TRUE;
	case WM_COMMAND:

	   Update();
		
		
		
		break;
	}
	return FALSE;
}

/*
 * Red Dog helper class description
 */
class splinemodDesc : public ClassDesc
{
public:
	int 			IsPublic() {return 1;}
	void *			Create (BOOL loading = FALSE) {return new SplineMod();}
	const TCHAR *	ClassName() {return GetString(IDS_SPLINEMOD);}
	SClass_ID		SuperClassID() {return HELPER_CLASS_ID;}
	Class_ID		ClassID() {return SPLINEMOD_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_HELPER_CATEGORY);}
	void			ResetClassParams (BOOL fileReset);
};

static splinemodDesc SplineModDesc;
ClassDesc* GetSplineModDesc() {return &SplineModDesc;}

//TODO: Should implement this method to reset the plugin params when Max is reset
void splinemodDesc::ResetClassParams (BOOL fileReset) 
{

}
