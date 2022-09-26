/**********************************************************************
 *<
	FILE: circle.cpp

	DESCRIPTION:  A Circle object implementation

	CREATED BY: Tom Hudson

	HISTORY: created 29 October 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/
#include <max.h>
// This is based on the simple spline object...
#include "stdafx.h"
#include "splshape.h"
#include "linshape.h"
#include "iparamm.h"
#include "evrouter.h"
#include <simpspl.h>
#include "resource.h"
#include "KLeditspline.h"
  
// Knot type indexes for creation and a handy lookup table

#define INDEX_CORNER 0
#define INDEX_SMOOTH 1
#define INDEX_BEZIER 2
static int KnotTypeTable[] = { KTYPE_CORNER, KTYPE_AUTO, KTYPE_BEZIER };

#define DEF_RENDERABLE FALSE
#define DEF_THICKNESS 1.0f
#define DEF_GENUVS FALSE


class BBOXSplineObject: public SplineShape, public IParamArray {			   
	friend class SplineObjCreateCallBack;
	friend BOOL CALLBACK SplineParamDialogProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
	friend class SplineTypeInDlgProc;
	friend class BackspaceUser;

	public:
		// Class vars
		static IParamMap *pmapCreate;
		static IParamMap *pmapTypeIn;
		static Point3 crtPos;		
		static int crtInitialType;
		static int crtDragType;
		static Spline3D TIspline;	// Type-in construction spline
		static BOOL useTI;			// If TRUE, constructor copies TIspline into our work spline & resets this
		static float dlgThickness;
		static BOOL dlgGenUVs;
		static BOOL dlgRenderable;

		// Flag set when initial creation is complete.
		BOOL createDone;

		// Special flag for loading old files -- Temporarily sets up refs
		BOOL loadingOldFile;

		// Old interpolation parameter block (used when loading old files)
		IParamBlock *ipblock;	// Interpolation parameter block

		// Flag to suspend snapping -- Used during creation
		BOOL suspendSnap;

		BBOXSplineObject();
		~BBOXSplineObject();

		//  inherited virtual methods:

		CreateMouseCallBack* GetCreateMouseCallBack();
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		TCHAR *GetObjectName() { return GetString(IDS_TH_LINE); }
		void InitNodeName(TSTR& s) { s = GetString(IDS_TH_LINE); }		
		Class_ID ClassID() { return  BBOXSPLINE_CLASS_ID ;}
	   //		Class_ID(0x480c3cf6, 0x1ae92cf0); }
	   //		SPLINE3D_CLASS_ID,0); }  
		void GetClassName(TSTR& s) { s = TSTR(GetString(IDS_TH_LINE_CLASS)); }
		RefTargetHandle Clone(RemapDir& remap = NoRemap());
		int NumRefs();
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);		

		// From IParamArray
		BOOL SetValue(int i, TimeValue t, int v);
		BOOL SetValue(int i, TimeValue t, float v);
		BOOL SetValue(int i, TimeValue t, Point3 &v);
		BOOL GetValue(int i, TimeValue t, int &v, Interval &ivalid);
		BOOL GetValue(int i, TimeValue t, float &v, Interval &ivalid);
		BOOL GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid);

		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex);

		void AssignSpline(Spline3D *s);
		void FlushTypeInSpline() { TIspline.NewSpline(); useTI = FALSE; }
		int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
		BOOL ValidForDisplay(TimeValue t);

		void CenterPivot(IObjParam *ip);

		// From SplineShape (Editable Spline)
		Object* ConvertToType(TimeValue t, Class_ID obtype);

		// IO
		IOResult Save(ISave *isave);
		IOResult Load(ILoad *iload);
	};				

//------------------------------------------------------

class SplineObjClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new BBOXSplineObject; }
	const TCHAR *	ClassName() { return GetString(IDS_TH_LINE_CLASS); }
	SClass_ID		SuperClassID() { return SHAPE_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(0x1b3b11fc, 0x178d4d91);}
	//SPLINE3D_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_TH_SPLINES);  }
	void			ResetClassParams(BOOL fileReset);
	};

static SplineObjClassDesc splineObjDesc;

ClassDesc* GetSplineDesc() { return &splineObjDesc; }

// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;

// class variable for spline class.
float BBOXSplineObject::dlgThickness = DEF_THICKNESS;
BOOL BBOXSplineObject::dlgGenUVs = DEF_GENUVS;
BOOL BBOXSplineObject::dlgRenderable = DEF_RENDERABLE;
IParamMap *BBOXSplineObject::pmapCreate = NULL;
IParamMap *BBOXSplineObject::pmapTypeIn = NULL;
IObjParam *BBOXSplineObject::ip         = NULL;
Point3 BBOXSplineObject::crtPos         = Point3(0,0,0);
int BBOXSplineObject::crtInitialType    = INDEX_CORNER;
int BBOXSplineObject::crtDragType       = INDEX_BEZIER;

// Type-in creation statics:
Spline3D BBOXSplineObject::TIspline;
BOOL BBOXSplineObject::useTI            = FALSE;

void SplineObjClassDesc::ResetClassParams(BOOL fileReset)
	{
	BBOXSplineObject::crtPos         = Point3(0,0,0);
	BBOXSplineObject::crtInitialType = INDEX_CORNER;
	BBOXSplineObject::crtDragType    = INDEX_BEZIER;
	BBOXSplineObject::useTI          = FALSE;
	BBOXSplineObject::dlgThickness   = DEF_THICKNESS;
	BBOXSplineObject::dlgGenUVs      = DEF_GENUVS;
	BBOXSplineObject::dlgRenderable  = DEF_RENDERABLE;
	}

// Parameter map indices
#define PB_DUMMY		0	// Not actually used!

// Non-parameter block indices
#define PB_INITIALTYPE		0
#define PB_DRAGTYPE			1
#define PB_TI_POS			2

//
//
// Creation Parameters

static int initialTypeIDs[] = {IDC_ICORNER,IDC_ISMOOTH};
static int dragTypeIDs[] = {IDC_DCORNER,IDC_DSMOOTH,IDC_DBEZIER};

static ParamUIDesc descCreate[] = {
	// Initial point type	
	ParamUIDesc(PB_INITIALTYPE,TYPE_RADIO,initialTypeIDs,2),
	// Dragged point type
	ParamUIDesc(PB_DRAGTYPE,TYPE_RADIO,dragTypeIDs,3)
	};
#define CREATEDESC_LENGTH 2

//
//
// Type in

static ParamUIDesc descTypeIn[] = {
	
	// Position
	ParamUIDesc(
		PB_TI_POS,
		EDITTYPE_UNIVERSE,
		IDC_TI_POSX,IDC_TI_POSXSPIN,
		IDC_TI_POSY,IDC_TI_POSYSPIN,
		IDC_TI_POSZ,IDC_TI_POSZSPIN,
		-99999999.0f,99999999.0f,
		SPIN_AUTOSCALE)
	
	};
#define TYPEINDESC_LENGTH 1

//--- TypeInDlgProc --------------------------------

class SplineTypeInDlgProc : public ParamMapUserDlgProc {
	public:
		BBOXSplineObject *ob;

		SplineTypeInDlgProc(BBOXSplineObject *n) {ob=n;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}
	};

BOOL SplineTypeInDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	static Point3 previous;
	switch (msg) {
		case WM_DESTROY:
			ob->suspendSnap = FALSE;
			ob->FlushTypeInSpline();
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_TI_ADDPOINT: {
					// Return focus to the top spinner
					SetFocus(GetDlgItem(hWnd, IDC_TI_POSX));
					ob->suspendSnap = TRUE;
					// If first point, start up the new object
					if(ob->TIspline.KnotCount() == 0) {
						Matrix3 tm(1);
						previous = ob->crtPos;
						BBOXSplineObject *newob = (BBOXSplineObject *)ob->ip->NonMouseCreate(tm);
						newob->TIspline.AddKnot(SplineKnot(KnotTypeTable[ob->crtInitialType],LTYPE_CURVE,ob->crtPos,ob->crtPos,ob->crtPos));
						// Stuff the spline into the shape
						while(newob->shape.SplineCount())
							newob->shape.DeleteSpline(0);
						Spline3D *spline = newob->shape.NewSpline();
						*spline = newob->TIspline;
						newob->shape.UpdateSels();	// Make sure it readies the selection set info
						newob->shape.InvalidateGeomCache();
						return TRUE;
						}					
					if(ob->TIspline.KnotCount() > 0 && ob->crtPos == previous) {
						ob->ip->DisplayTempPrompt(GetString(IDS_TH_ERRORTWOCONSECUTIVE));
						Beep(800, 100);
						return TRUE;
						}
					previous = ob->crtPos;
					ob->TIspline.AddKnot(SplineKnot(KnotTypeTable[ob->crtInitialType],LTYPE_CURVE,ob->crtPos,ob->crtPos,ob->crtPos));
					// Stuff the revised spline into the shape
					while(ob->shape.SplineCount())
						ob->shape.DeleteSpline(0);
					Spline3D *spline = ob->shape.NewSpline();
					*spline = ob->TIspline;
					ob->shape.UpdateSels();	// Make sure it readies the selection set info
					ob->shape.InvalidateGeomCache();
					ob->NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
					return TRUE;
					}
				case IDC_TI_CLOSE:
					// Return focus to the top spinner
					SetFocus(GetDlgItem(hWnd, IDC_TI_POSX));
					
					if(ob->TIspline.KnotCount() > 1) {
						ob->TIspline.SetClosed();

						common_finish:
						ob->suspendSnap = FALSE;
						ob->TIspline.ComputeBezPoints();
						ob->AssignSpline(&ob->TIspline);
						ob->CenterPivot(ob->ip);

						// Zero out the type-in spline
						ob->FlushTypeInSpline();
						return TRUE;
						}
					else {
						ob->ip->DisplayTempPrompt(GetString(IDS_TH_NEEDTWOSPLINEPOINTS));
						Beep(500, 100);
						}
					break;
				case IDC_TI_FINISH:
					if(ob->TIspline.KnotCount() > 1)
						goto common_finish;
					else {
						ob->ip->DisplayTempPrompt(GetString(IDS_TH_NEEDTWOSPLINEPOINTS));
						Beep(500, 100);
						}
					break;
				}
			break;	
		}
	return FALSE;
	}

void BBOXSplineObject::CenterPivot(IObjParam *ip) {
	// Find the center of the knot points
	Point3 composite(0,0,0);
	Spline3D *spline = shape.splines[0];
	int knots = spline->KnotCount();
	for(int i = 0; i < knots; ++i)
		composite += spline->GetKnotPoint(i);
	composite /= (float)knots;
	// Translate the entire spline to this new center
	Matrix3 xlate = TransMatrix(-composite);
	spline->Transform(&xlate);
	shape.InvalidateGeomCache();
	// Set the new matrix center
	Matrix3 tm(1);
	tm.SetTrans(composite);
	// Assign the revised transform
	ip->NonMouseCreateFinish(tm);
	}

void BBOXSplineObject::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
	{
	SplineShape::BeginEditParams(ip,flags,prev);
	// This shouldn't be necessary, but the ip is getting hosed after the previous call!
	this->ip = ip;

	if (pmapCreate) {
		
		// Left over from last one ceated
		pmapCreate->SetParamBlock(this);
		pmapTypeIn->SetParamBlock(this);
	} else {
		
		// Gotta make a new one.
		if (flags&BEGIN_EDIT_CREATE) {
			pmapCreate = CreateCPParamMap(
				descCreate,CREATEDESC_LENGTH,
				this,
				ip,
				hInstance,
				MAKEINTRESOURCE(IDD_SPLINEPARAM1),
				GetString(IDS_TH_CREATION_METHOD),
				0);

			pmapTypeIn = CreateCPParamMap(
				descTypeIn,TYPEINDESC_LENGTH,
				this,
				ip,
				hInstance,
				MAKEINTRESOURCE(IDD_SPLINEPARAM2),
				GetString(IDS_TH_KEYBOARD_ENTRY),
				APPENDROLL_CLOSED);
			}
		}

	if(pmapTypeIn) {
		// A callback for the type in.
		pmapTypeIn->SetUserDlgProc(new SplineTypeInDlgProc(this));
		}
	}
		
void BBOXSplineObject::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
	{
	if(TIspline.KnotCount()) {
		if(TIspline.KnotCount() == 1) {
			Spline3D *spline = shape.splines[0];
			Point3 p = spline->GetKnotPoint(0);
			spline->AddKnot(SplineKnot(KnotTypeTable[crtInitialType],LTYPE_CURVE,p,p,p));
			shape.UpdateSels();	// Make sure it readies the selection set info
			shape.InvalidateGeomCache();
			}
		CenterPivot(ip);
		suspendSnap = FALSE;
		NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
		FlushTypeInSpline();		
		}

	SplineShape::EndEditParams(ip,flags,next);

	if (flags&END_EDIT_REMOVEUI ) {
		if (pmapCreate) DestroyCPParamMap(pmapCreate);
		if (pmapTypeIn) DestroyCPParamMap(pmapTypeIn);
		pmapTypeIn = NULL;
		pmapCreate = NULL;
		}
	// Save these values in class variables so the next object created will inherit them.
	dlgThickness = GetThickness();
	dlgRenderable = GetRenderable();
	dlgGenUVs = GetGenUVs();
	}

BBOXSplineObject::BBOXSplineObject() : SplineShape() 
	{
	createDone = FALSE;
	loadingOldFile = FALSE;
	suspendSnap = FALSE;
	ipblock = NULL;
	if(useTI) {
		AssignSpline(&TIspline);
		FlushTypeInSpline();
		}
	SetRenderable(dlgRenderable);
	SetThickness(dlgThickness);	// Not in param block -- In ShapeObject
	SetGenUVs(dlgGenUVs);
	}

BBOXSplineObject::~BBOXSplineObject()
	{
	DeleteAllRefsFromMe();
	}

class SplineObjCreateCallBack: public CreateMouseCallBack {
	BBOXSplineObject *ob;
	public:
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
		int override(int mode) { return CLICK_DOWN_POINT; }		// Override the mouse manager's mode!
		void SetObj(BBOXSplineObject *obj) { ob = obj; }
	};

class BackspaceUser : public EventUser {
	BBOXSplineObject *ob;
	public:
		void Notify();
		void SetObj(BBOXSplineObject *obj) { ob = obj; }
	};

void BackspaceUser::Notify() {
	Spline3D *spline = ob->shape.GetSpline(0);
	if(spline->KnotCount() > 2) {
		// Tell the spline we just backspaced to remove the last point
		spline->Create(NULL, -1, 0, 0, IPoint2(0,0), NULL,ob->ip);
		ob->shape.InvalidateGeomCache();
		ob->NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
		ob->ip->RedrawViews(ob->ip->GetTime(),REDRAW_INTERACTIVE);
		}
	}

static BackspaceUser pBack;

int SplineObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) {
#ifdef _OSNAP
		if(msg == MOUSE_FREEMOVE)
		{
			vpt->SnapPreview(m,m,NULL,SNAP_IN_3D);
			return TRUE;
		}
#endif

	if(ob->createDone) {
		ob->suspendSnap = FALSE;
		return CREATE_STOP;
		}
	if(point == 0) {
		ob->suspendSnap = TRUE;
		pBack.SetObj(ob);
		backspaceRouter.Register(&pBack);
		ob->shape.NewShape();
		ob->shape.NewSpline(KnotTypeTable[ob->crtInitialType],KnotTypeTable[ob->crtDragType]);
		ob->FlushTypeInSpline();	// Also get rid of the type-in spline, which we aren't using
		}
	Spline3D *theSpline = ob->shape.GetSpline(0);
	if(!theSpline) {
		assert(0);
		return FALSE;
		}
	int res = theSpline->Create(vpt,msg,point,flags,m,NULL,ob->ip);
	switch(res) {
		case CREATE_ABORT:
			ob->suspendSnap = FALSE;
			ob->createDone = TRUE;
			backspaceRouter.UnRegister(&pBack);
			return res;
		case CREATE_STOP:
			ob->suspendSnap = FALSE;
			ob->createDone = TRUE;
			if(theSpline->KnotCount() == 0) {
				backspaceRouter.UnRegister(&pBack);
				return CREATE_ABORT;
				}
			// Find the center of the knot points
			Point3 composite(0,0,0);
			for(int i = 0; i < theSpline->KnotCount(); ++i)
				composite += theSpline->GetKnotPoint(i);
			composite /= (float)theSpline->KnotCount();
			// Translate the entire spline to this new center
			Matrix3 xlate = TransMatrix(-composite);
			theSpline->Transform(&xlate);
			// Set the new matrix center
			mat.SetTrans(composite * mat);
			backspaceRouter.UnRegister(&pBack);
			break;
		}
	ob->shape.UpdateSels();	// Make sure it readies the selection set info
	ob->shape.InvalidateGeomCache();
	ob->NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
	return res;
	}

static SplineObjCreateCallBack splineCreateCB;

CreateMouseCallBack* BBOXSplineObject::GetCreateMouseCallBack() {
	splineCreateCB.SetObj(this);
	return(&splineCreateCB);
	}

//
// Reference Managment:
//

RefTargetHandle BBOXSplineObject::Clone(RemapDir& remap) {
	BBOXSplineObject* newob = new BBOXSplineObject();
	newob->SplineShapeClone(this);
	return(newob);
	}

ParamDimension *BBOXSplineObject::GetParameterDim(int pbIndex) 
	{
//	switch (pbIndex) {
//		}
	return defaultDim;
	}

TSTR BBOXSplineObject::GetParameterName(int pbIndex) 
	{
//	switch (pbIndex) {
//		}
	return TSTR(_T(""));
	}

void BBOXSplineObject::AssignSpline(Spline3D *s) {
	shape.NewShape();
	*(shape.NewSpline()) = *s;
	shape.UpdateSels();
	shape.InvalidateGeomCache();
	}

// From ParamArray
BOOL BBOXSplineObject::SetValue(int i, TimeValue t, int v) 
	{
	switch (i) {
		case PB_INITIALTYPE: crtInitialType = v; break;
		case PB_DRAGTYPE: crtDragType = v; break;
		}		
	return TRUE;
	}

BOOL BBOXSplineObject::SetValue(int i, TimeValue t, float v)
	{
//	switch (i) {				
//		}	
	return TRUE;
	}

BOOL BBOXSplineObject::SetValue(int i, TimeValue t, Point3 &v) 
	{
	switch (i) {
		case PB_TI_POS: crtPos = v; break;
		}		
	return TRUE;
	}

BOOL BBOXSplineObject::GetValue(int i, TimeValue t, int &v, Interval &ivalid) 
	{
	switch (i) {
		case PB_INITIALTYPE: v = crtInitialType; break;
		case PB_DRAGTYPE: v = crtDragType; break;
		}
	return TRUE;
	}

BOOL BBOXSplineObject::GetValue(int i, TimeValue t, float &v, Interval &ivalid) 
	{	
//	switch (i) {		
//		}
	return TRUE;
	}

BOOL BBOXSplineObject::GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_POS: v = crtPos; break;		
		}
	return TRUE;
	}

#define DISP_NODEFAULTCOLOR (1<<12)
#define COMP_NODEFAULTCOLOR (1<<10)

int BBOXSplineObject::Display(TimeValue t, INode *inode, ViewExp *vpt, int flags) {
	if(!ValidForDisplay(t))
		return 0;

	GraphicsWindow *gw = vpt->getGW();
	gw->setTransform(inode->GetObjectTM(t));
	// If creating, show the vertex ticks
	BOOL ticksSet = FALSE;
	if((drawTicks || suspendSnap) && !(shape.dispFlags & DISP_VERTTICKS)) {
		shape.dispFlags |= DISP_VERTTICKS;
		ticksSet = TRUE;
		}

	shape.Render( gw, gw->getMaterial(),
		(flags&USE_DAMAGE_RECT) ? &vpt->GetDammageRect() : NULL, 
		COMP_ALL | ((flags&DISP_SHOWSUBOBJECT)?COMP_OBJSELECTED:0) |
		((flags&DISP_NODEFAULTCOLOR)?COMP_NODEFAULTCOLOR:0) |
		(suspendSnap ? COMP_OBJSELECTED:0), 
		NULL);

	// Turn off the ticks if we set 'em
	if(ticksSet)
		shape.dispFlags &= ~DISP_VERTTICKS;

	return(0);
	}

BOOL BBOXSplineObject::ValidForDisplay(TimeValue t) {
	if(shape.SplineCount() == 0)
		return FALSE;
	return (shape.GetSpline(0)->KnotCount() > 1) ? TRUE : FALSE;
	}

// We're based on the SplineShape object, but the conversion to an editable spline
// should turn us into a general SplineShape, not the Line object that we are.
// If asked to turn into a SplineShape, we'll do the conversion, otherwise, we'll
// let the SplineShape code do the work, which may mean just returning a pointer
// to ourself.
Object* BBOXSplineObject::ConvertToType(TimeValue t, Class_ID obtype) {
	if (obtype == splineShapeClassID) {
		SplineShape* newob = new SplineShape;
		// Gotta copy the ShapeObject parts
		newob->CopyBaseData(*this);
		*newob = *this;
		newob->SetChannelValidity(TOPO_CHAN_NUM,ConvertValidity(t));
		newob->SetChannelValidity(GEOM_CHAN_NUM,ConvertValidity(t));
		return newob;
		}
	return SplineShape::ConvertToType(t, obtype);
	}

int BBOXSplineObject::NumRefs() {
	if(loadingOldFile)
		return 2;
	return SplineShape::NumRefs();
	}

RefTargetHandle BBOXSplineObject::GetReference(int i) {
	if(loadingOldFile)
		return (i==1) ? ipblock : NULL;
	return SplineShape::GetReference(i);
	}

void BBOXSplineObject::SetReference(int i, RefTargetHandle rtarg) {
	if(loadingOldFile) {
		if(i==1)
			ipblock=(IParamBlock*)rtarg;
		}
	else
		SplineShape::SetReference(i, rtarg);
	}		

// IO
#define OLD_SPLINE_CHUNK 0x1000
#define NEW_SPLINE_CHUNK 0x2000

IOResult BBOXSplineObject::Save(ISave *isave) {
	isave->BeginChunk(NEW_SPLINE_CHUNK);
	SplineShape::Save(isave);
	isave->EndChunk();
	return IO_OK;
	}

class SplinePostLoadCallback : public PostLoadCallback {
	public:
		BBOXSplineObject *so;
		SplinePostLoadCallback(BBOXSplineObject *s) {so=s;}
		void proc(ILoad *iload) {
			int steps, optimize, adaptive;
			if(so->ipblock) {
				so->ipblock->GetValue(IPB_STEPS, 0, steps, FOREVER);
				so->ipblock->GetValue(IPB_OPTIMIZE, 0, optimize, FOREVER);
				so->ipblock->GetValue(IPB_ADAPTIVE, 0, adaptive, FOREVER);
				so->steps = so->shape.steps = steps;
				so->shape.optimize = optimize;
				if(adaptive)
					so->shape.steps = -1;
				}
			so->DeleteAllRefsFromMe();
			so->loadingOldFile = FALSE;
			delete this;
			}
	};

IOResult  BBOXSplineObject::Load(ILoad *iload) {
	IOResult res;
	loadingOldFile = FALSE;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(iload->CurChunkID())  {
			case OLD_SPLINE_CHUNK: {	// Pre-2.0 line object chunk
				loadingOldFile = TRUE;
				iload->RegisterPostLoadCallback(new SplinePostLoadCallback(this));
				shape.NewShape();
				Spline3D *s = shape.NewSpline();
				res = s->Load(iload);
				iload->SetObsolete();
				shape.UpdateSels();
				shape.InvalidateGeomCache();
				}
				break;
			case NEW_SPLINE_CHUNK:
				SplineShape::Load(iload);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
		}
	return IO_OK;
	}
