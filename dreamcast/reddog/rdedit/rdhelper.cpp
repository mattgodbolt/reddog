/*
 * RDHelper.cpp
 * Implements the helper objects used to splice maps together
 */

#include "RDHelper.h"
#include "RDEdit.h"

/*
 * Helper class variables
 */

HWND			RDHelper::paramWindow;
IObjParam		*RDHelper::objParams;

/*
 * Helper methods
 */

/* Creation of a helper object */
class RDHCreate : public CreateMouseCallBack
{
private:
	RDHelper		*ref;
public:
	RDHCreate() {}
	void setRef (RDHelper *newRef) { ref = newRef; }

	int proc (ViewExp *vpt, int msg, int point, 
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

};

static RDHCreate RDHelperCreator;

RefResult RDHelper::NotifyRefChanged (Interval i, RefTargetHandle target, 
									  PartID &partID, RefMessage message)
{
	return REF_SUCCEED;
}

CreateMouseCallBack *RDHelper::GetCreateMouseCallBack()
{
	RDHelperCreator.setRef (this);
	return &RDHelperCreator;
}

/*
 * Bounding box calculations
 */
void RDHelper::GetLocalBoundBox (TimeValue t, INode* inode, ViewExp* vpt, Box3& box) 
{
	box = Box3(Point3(0,0,0), Point3(0,0,0));
}

void RDHelper::GetWorldBoundBox (TimeValue t, INode* inode, ViewExp* vpt, Box3& box )
{
	Matrix3 tm;
	tm = inode->GetObjectTM(t);
	box = Box3 (tm.GetTrans(), tm.GetTrans());
}

/*
 * Hit point calculation
 */
int RDHelper::HitTest (TimeValue t, INode *inode, int type,
					   int crossing, int flags, IPoint2 *p, ViewExp *vpt)
{
	Matrix3 tm(1);	
	HitRegion hitRegion;
	DWORD	savedLimits;
	Point3 pt(0,0,0);
	
	vpt->getGW()->setTransform(tm);
	GraphicsWindow *gw = vpt->getGW();	
	Material *mtl = gw->getMaterial();
	
   	tm = inode->GetObjectTM(t);		
	MakeHitRegion(hitRegion, type, crossing, 4, p);
	
	gw->setRndLimits(((savedLimits = gw->getRndLimits())|GW_PICK)&~GW_ILLUM);
	gw->setHitRegion(&hitRegion);
	gw->clearHitCode();
	
	vpt->getGW()->setTransform(tm);
	vpt->getGW()->marker(&pt,X_MRKR);
	
	gw->setRndLimits(savedLimits);
	
	if((hitRegion.type != POINT_RGN) && !hitRegion.crossing)
		return TRUE;
	return gw->checkHitCode();
}

/*
* Snapping points
*/
void RDHelper::Snap (TimeValue t, INode* inode, SnapInfo *snap,
					 IPoint2 *p, ViewExp *vpt)
{
	if(suspendSnap)
		return;
	
	Matrix3 tm = inode->GetObjectTM(t);	
	GraphicsWindow *gw = vpt->getGW();	
	gw->setTransform(tm);
	
	Matrix3 invPlane = Inverse(snap->plane);
	
	// Make sure the vertex priority is active and at least as important as the best snap so far
	if(snap->vertPriority > 0 && snap->vertPriority <= snap->priority) {
		Point2 fp = Point2((float)p->x, (float)p->y);
		Point2 screen2;
		IPoint3 pt3;
		
		Point3 thePoint(0,0,0);
		// If constrained to the plane, make sure this point is in it!
		if(snap->snapType == SNAP_2D || snap->flags & SNAP_IN_PLANE) {
			Point3 test = thePoint * tm * invPlane;
			if(fabs(test.z) > 0.0001)	// Is it in the plane (within reason)?
				return;
		}
		gw->wTransPoint(&thePoint,&pt3);
		screen2.x = (float)pt3.x;
		screen2.y = (float)pt3.y;
		
		// Are we within the snap radius?
		int len = (int)Length(screen2 - fp);
		if(len <= snap->strength) {
			// Is this priority better than the best so far?
			if(snap->vertPriority < snap->priority) {
				snap->priority = snap->vertPriority;
				snap->bestWorld = thePoint * tm;
				snap->bestScreen = screen2;
				snap->bestDist = len;
			}
			else
				if(len < snap->bestDist) {
					snap->priority = snap->vertPriority;
					snap->bestWorld = thePoint * tm;
					snap->bestScreen = screen2;
					snap->bestDist = len;
				}
		}
	}
}

/*
 * Display the helper
 */
int RDHelper::Display (TimeValue t, INode* inode, ViewExp *vpt, int flags)
{
	Matrix3 tm(1);
	Point3 pt(0,0,0);
	
	vpt->getGW()->setTransform(tm);
	tm = inode->GetObjectTM(t);		

	vpt->getGW()->setTransform(tm);
	
	if(!inode->IsFrozen())
		vpt->getGW()->setColor(LINE_COLOR,GetUIColor(COLOR_POINT_OBJ));
	
	vpt->getGW()->marker(&pt,X_MRKR);
	
	return 0;
}

/*
 * Cloning (via references)
 */
RefTargetHandle RDHelper::Clone (RemapDir& remap)
{
	RDHelper *newob = new RDHelper();
	// Copy any instance vars across
	return newob;
}

/*
 * Loading and saving of helpers
 */
IOResult RDHelper::Load (ILoad *iload)
{
	IOResult res = IO_OK;
	
	while (IO_OK==(res=iload->OpenChunk())) {
		/*
		switch (iload->CurChunkID()) {
			// Check instance var chunks and load 'em here
		}*/
		
		res = iload->CloseChunk();
		if (res!=IO_OK)  return res;
	}
	
	return IO_OK;
}

IOResult RDHelper::Save (ISave *isave)
{
	// Save instance vars here
	
	return IO_OK;
}

/*
 * Beginning and ending of parameter edits
 */

// Param window handler
BOOL CALLBACK HelperParamProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	RDHelper *po = (RDHelper *)GetWindowLong(hWnd,GWL_USERDATA);
	if (!po && msg!=WM_INITDIALOG) return FALSE;
	
	switch (msg) {
	case WM_INITDIALOG:
		po = (RDHelper *)lParam;
		SetWindowLong (hWnd,GWL_USERDATA,lParam);
		// Set up stuff
		return FALSE;
		
	case WM_COMMAND:
		break;
		
	default:
		return FALSE;
	}
	
	return TRUE;
} 

void RDHelper::BeginEditParams (IObjParam *ip, ULONG flags,Animatable *prev)
{	
	objParams = ip;
	if (!paramWindow) {
		paramWindow = ip->AddRollupPage( 
			hInstance, 
			MAKEINTRESOURCE(IDD_HELPERPARAM),
			HelperParamProc, 
			GetString(IDS_PARAMS), 
			(LPARAM)this );
		ip->RegisterDlgWnd (paramWindow);
	} else {
		SetWindowLong(paramWindow,GWL_USERDATA,(LONG)this);
		// Set up window stuff here
	}
}

void RDHelper::EndEditParams(IObjParam *ip, ULONG flags,Animatable *next)
{	
	// Read window stuff here
	if (flags&END_EDIT_REMOVEUI) {
		ip->UnRegisterDlgWnd(paramWindow);
		ip->DeleteRollupPage(paramWindow);
		paramWindow = NULL;
	} else {
		SetWindowLong(paramWindow,GWL_USERDATA,0);
	}
	objParams = NULL;
}


/*
 * Red Dog helper class description
 */
class RDHelperClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create (BOOL loading = FALSE) {return new RDHelper();}
	const TCHAR *	ClassName() {return GetString(IDS_HELPER_CLASS_NAME);}
	SClass_ID		SuperClassID() {return HELPER_CLASS_ID;}
	Class_ID		ClassID() {return RDHELPER_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_HELPER_CATEGORY);}
	void			ResetClassParams (BOOL fileReset);
};

static RDHelperClassDesc RDHelperDesc;
ClassDesc* GetRDHelperDesc() {return &RDHelperDesc;}

//TODO: Should implement this method to reset the plugin params when Max is reset
void RDHelperClassDesc::ResetClassParams (BOOL fileReset) 
{

}

