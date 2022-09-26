/*===========================================================================*\
  How To Create a Procedural Object Plug-In

  FILE: sphere_c.cpp
 
  DESCRIPTION:  Sphere object, Revised implementation

  CREATED BY: Rolf Berteig

  HISTORY: created 10/10/95

  Copyright (c) 1994, All Rights Reserved.
\*===========================================================================*/
#include <max.h>
#include <iparamm.h>
#include <simpobj.h>
#include "stdafx.h"
#include <surf_api.h>


#include "resource.h"

// This is the DLL instance handle passed in when the plug-in is 
// loaded at startup.
#if 0
HINSTANCE hInstance;

// This function returns a pointer to a string in the string table of
// the resource library.  Note that this function maintains the buffer
// and that only one string is loaded at a time.  Therefore if you intend
// to use this string, you must copy to another buffer since it will 
// be overwritten on the next GetString() call.
TCHAR *GetString(int id) {
	static TCHAR buf[256];
	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
	}
#endif

// Unique Class ID.  It is specified as two 32-bit quantities.
// These must be unique.  Use the Class_ID generator program to create
// these values.  This program is available from the main menu of the
// help file.
#define SPHERE_C_CLASS_ID Class_ID(0x3d7f05b6, 0x63657b8)

// The sphere object class definition.  It is derived from SimpleObject and
// IParamArray.  SimpleObject is the class to derive objects from which have 
// geometry are renderable, and represent themselves using a mesh.  
// IParamArray is used as part of the Parameter Map scheme used to manage
// the user interface parameters.
class SphereObject : public GenSphere, public IParamArray {
	public:			
		// Class variables
		// There is only one set of these variables shared by all instances
		// of this class.  This is OK because there is only one sphere
		// being edited at a time.
		static IParamMap *pmapCreate;
		static IParamMap *pmapTypeIn;
		static IParamMap *pmapParam;
		static int dlgSegments;
		static int dlgCreateMeth;
		static int dlgSmooth;
		static Point3 crtPos;		
		static float crtRadius;
		// This is the Interface pointer into MAX.  It is used to call
		// functions implemented in MAX itself.  
		static IObjParam *ip;

		// --- Inherited virtual methods of Animatable ---
		void DeleteThis() {delete this;}
		Class_ID ClassID() { return SPHERE_C_CLASS_ID; } 
		void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);

		// --- Inherited virtual methods of ReferenceMaker ---
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		// --- Inherited virtual methods of ReferenceTarget ---
		RefTargetHandle Clone(RemapDir& remap = NoRemap());

		// --- Inherited virtual methods of BaseObject ---
		CreateMouseCallBack* GetCreateMouseCallBack();
		TCHAR *GetObjectName() { return GetString(IDS_BOUNDREGION); }

		// --- Inherited virtual methods of Object ---
		int CanConvertToType(Class_ID obtype);
		Object* ConvertToType(TimeValue t, Class_ID obtype);
		void GetCollapseTypes(Tab<Class_ID> &clist,Tab<TSTR*> &nlist);
		BOOL HasUVW();
		void SetGenUVW(BOOL sw);
		BOOL IsParamSurface() {return TRUE;}
		Point3 GetSurfacePoint(TimeValue t, float u, float v,Interval &iv);
		int IntersectRay(TimeValue t, Ray& ray, float& at, Point3& norm);

		// --- Inherited virtual methods of SimpleObject ---
		void BuildMesh(TimeValue t);
		BOOL OKtoDisplay(TimeValue t);
		void InvalidateUI();
		ParamDimension *GetParameterDim(int pbIndex);
		TSTR GetParameterName(int pbIndex);		

		// --- Inherited virtual methods of GenSphere ---
		void SetParams(float rad, int segs, BOOL smooth=TRUE, BOOL genUV=TRUE,
			 float hemi=0.0f, BOOL squash=FALSE, BOOL recenter=FALSE);

		// --- Inherited virtual methods of IParamArray ---
		BOOL SetValue(int i, TimeValue t, int v);
		BOOL SetValue(int i, TimeValue t, float v);
		BOOL SetValue(int i, TimeValue t, Point3 &v);
		BOOL GetValue(int i, TimeValue t, int &v, Interval &ivalid);
		BOOL GetValue(int i, TimeValue t, float &v, Interval &ivalid);
		BOOL GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid);

		// --- Methods of SphereObject ---
		SphereObject();		
	};

// Misc stuff
#define MAX_SEGMENTS	200
#define MIN_SEGMENTS	4

#define MIN_RADIUS		float(0)
#define MAX_RADIUS		float(1.0E30)

#define MIN_SMOOTH		0
#define MAX_SMOOTH		1

#define DEF_SEGMENTS	16
#define DEF_RADIUS		float(0.0)

#define SMOOTH_ON	1
#define SMOOTH_OFF	0


//--- Class vars ---------------------------------
// Initialize the class vars
int SphereObject::dlgSegments       = DEF_SEGMENTS;
int SphereObject::dlgCreateMeth     = 1; // create_radius
int SphereObject::dlgSmooth         = SMOOTH_ON;
IParamMap *SphereObject::pmapCreate = NULL;
IParamMap *SphereObject::pmapParam  = NULL;
IParamMap *SphereObject::pmapTypeIn = NULL;
IObjParam *SphereObject::ip         = NULL;
Point3 SphereObject::crtPos         = Point3(0,0,0);
float SphereObject::crtRadius       = 0.0f;

//--- Parameter map/block descriptors -------------------------------
// The parameter map descriptors define the properties of a parameter
// such as the type (spinner, radio button, check box, etc.), which
// resource ID they refer to, and which index into the virtual array
// they use.

// Parameter block indices
#define PB_RADIUS	0
#define PB_SEGS		1
#define PB_SMOOTH	2
#define PB_HEMI		3
#define PB_SQUASH	4
#define PB_RECENTER	5
#define PB_GENUVS	6

// Non-parameter block indices
#define PB_CREATEMETHOD		0
#define PB_TI_POS			1
#define PB_TI_RADIUS		2


//
//
//	Creation method

static int createMethIDs[] = {IDC_BOUNDPARENT,IDC_BOUNDRADIUS};

static ParamUIDesc descCreate[] = {
	// Diameter/radius
	ParamUIDesc(PB_CREATEMETHOD,TYPE_RADIO,createMethIDs,2)
	};
#define CREATEDESC_LENGH 1


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
		SPIN_AUTOSCALE),
	
	// Radius
	ParamUIDesc(
		PB_TI_RADIUS,
		EDITTYPE_UNIVERSE,
		IDC_RADIUS,IDC_RADSPINNER,
		MIN_RADIUS,MAX_RADIUS,
		SPIN_AUTOSCALE)	
	};
#define TYPEINDESC_LENGH 2


//
//
// Parameters

static int squashIDs[] = {IDC_HEMI_CHOP,IDC_HEMI_SQUASH};

static ParamUIDesc descParam[] = {
	// Radius
	ParamUIDesc(
		PB_RADIUS,
		EDITTYPE_UNIVERSE,
		IDC_RADIUS,IDC_RADSPINNER,
		MIN_RADIUS,MAX_RADIUS,
		SPIN_AUTOSCALE),	
	
	// Segments
	ParamUIDesc(
		PB_SEGS,
		EDITTYPE_INT,
		IDC_SEGMENTS,IDC_SEGSPINNER,
		(float)MIN_SEGMENTS,(float)MAX_SEGMENTS,
		0.1f),
	
	// Smooth
	ParamUIDesc(PB_SMOOTH,TYPE_SINGLECHEKBOX,IDC_OBSMOOTH),

	// Hemisphere
	ParamUIDesc(
		PB_HEMI,
		EDITTYPE_FLOAT,
		IDC_HEMISPHERE,IDC_HEMISPHERESPINNER,
		0.0f,1.0f,
		0.005f),

	// Chop/squash
	ParamUIDesc(PB_SQUASH,TYPE_RADIO,squashIDs,2),

	// Recenter
	ParamUIDesc(PB_RECENTER,TYPE_SINGLECHEKBOX,IDC_HEMI_RECENTER),

	// Gen UVs
	ParamUIDesc(PB_GENUVS,TYPE_SINGLECHEKBOX,IDC_GENTEXTURE),
	};
#define PARAMDESC_LENGH 7


// The parameter block descriptor defines the type of value represented
// by the parameter (int, float, Color...) and if it is animated or not.

// This class requires these values to be initialized:
// { - ParameterType, 
//   - Not Used, must be set to NULL, 
//   - Flag which indicates if the parameter is animatable,
//   - ID of the parameter used to match a corresponding ID in the 
//     other version of the parameter.  This is used to automatically
//     convert between and older version of the parameter block
//     structure and a newer one.
//  }

// This one is the older version.
static ParamBlockDescID descVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },		
	{ TYPE_INT, NULL, TRUE, 1 },
	{ TYPE_INT, NULL, TRUE, 2 } };

static ParamBlockDescID descVer1[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },	
	{ TYPE_INT, NULL, TRUE, 1 },
	{ TYPE_INT, NULL, TRUE, 2 },
	{ TYPE_FLOAT, NULL, TRUE, 3 },
	{ TYPE_INT, NULL, FALSE, 4 },
	{ TYPE_INT, NULL, FALSE, 5 } };

static ParamBlockDescID descVer2[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },	
	{ TYPE_INT, NULL, TRUE, 1 },
	{ TYPE_BOOL, NULL, TRUE, 2 },
	{ TYPE_FLOAT, NULL, TRUE, 3 },
	{ TYPE_INT, NULL, FALSE, 4 },
	{ TYPE_INT, NULL, FALSE, 5 },
	{ TYPE_INT, NULL, FALSE, 6 } };

#define PBLOCK_LENGTH	7

// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(descVer0,3,0),
	ParamVersionDesc(descVer1,6,1)
	};
#define NUM_OLDVERSIONS	2

// Current version
static ParamVersionDesc curVersion(descVer2,PBLOCK_LENGTH,2);
#define CURRENT_VERSION	2

//--- TypeInDlgProc --------------------------------

// This is the method called when the user clicks on the Create button
// in the Keyboard Entry rollup.  It was registered as the dialog proc
// for this button by the SetUserDlgProc() method called from 
// BeginEditParams().
class SphereTypeInDlgProc : public ParamMapUserDlgProc {
	public:
		SphereObject *so;

		SphereTypeInDlgProc(SphereObject *s) {so=s;}
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
		void DeleteThis() {delete this;}
	};

BOOL SphereTypeInDlgProc::DlgProc(
		TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_TI_CREATE: {
					if (so->crtRadius==0.0) return TRUE;
					
					// We only want to set the value if the object is 
					// not in the scene.
					if (so->TestAFlag(A_OBJ_CREATING)) {
						so->pblock->SetValue(PB_RADIUS,0,so->crtRadius);
						}

					Matrix3 tm(1);
					tm.SetTrans(so->crtPos);
					so->suspendSnap = FALSE;
					so->ip->NonMouseCreate(tm);					
					// NOTE that calling NonMouseCreate will cause this
					// object to be deleted. DO NOT DO ANYTHING BUT RETURN.
					return TRUE;	
					}
				}
			break;	
		}
	return FALSE;
	}

// Class SphereObjCreateCallBack
// Declare a class derived from CreateMouseCallBack to handle
// the user input during the creation phase of the sphere.
class SphereObjCreateCallBack : public CreateMouseCallBack {
	IPoint2 sp0;
	SphereObject *ob;
	Point3 p0;
	public:
		int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
		void SetObj(SphereObject *obj) {ob = obj;}
	};

// --- Inherited virtual methods of CreateMouseCallBack ---
// This is the method that actually handles the user input
// during the sphere creation.
int SphereObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) {
	float r;
	Point3 p1,center;

	#ifdef _OSNAP
	if (msg == MOUSE_FREEMOVE)
	{
		#ifdef _3D_CREATE
			vpt->SnapPreview(m,m,NULL, SNAP_IN_3D);
		#else
			vpt->SnapPreview(m,m,NULL, SNAP_IN_PLANE);
		#endif
	}
	#endif

	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
		switch(point) {
			case 0:  // only happens with MOUSE_POINT msg
				ob->pblock->SetValue(PB_RADIUS,0,0.0f);
				ob->suspendSnap = TRUE;				
				sp0 = m;
				#ifdef _3D_CREATE	
					p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_3D);
				#else	
					p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				#endif
				mat.SetTrans(p0);
				break;
			case 1:
				mat.IdentityMatrix();
				//mat.PreRotateZ(HALFPI);
				#ifdef _3D_CREATE	
					p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_3D);
				#else	
					p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
				#endif
				if (ob->dlgCreateMeth) {
					r = Length(p1-p0);
					mat.SetTrans(p0);
					}
				else {
					center = (p0+p1)/float(2);
					mat.SetTrans(center);
					r = Length(center-p0);
					} 
				ob->pblock->SetValue(PB_RADIUS,0,r);
				ob->pmapParam->Invalidate();

				if (flags&MOUSE_CTRL) {
					float ang = (float)atan2(p1.y-p0.y,p1.x-p0.x);					
					mat.PreRotateZ(ob->ip->SnapAngle(ang));
					}

				if (msg==MOUSE_POINT) {
					ob->suspendSnap = FALSE;
					return (Length(m-sp0)<3 || Length(p1-p0)<0.1f)?CREATE_ABORT:CREATE_STOP;
					}
				break;					   
			}
		}
	else
	if (msg == MOUSE_ABORT) {		
		return CREATE_ABORT;
		}

	return TRUE;
	}

// A single instance of the callback object.
static SphereObjCreateCallBack sphereCreateCB;

// --- Global Functions ---
// Triangular patch layout:
//
//   A---> ac ----- ca <---C
//   |                    / 
//   |                  /
//   v    i1    i3    /
//   ab            cb
//
//   |           /
//   |    i2   /
// 
//   ba     bc
//   ^     /
//   |   /
//   | /
//   B
//
// vertices ( a b c d ) are in counter clockwise order when viewed from 
// outside the surface

// Vector length for unit circle
#define CIRCLE_VECTOR_LENGTH 0.5517861843f

// Build a Patch rep of the sphere for use with ConvertToType().
static void BuildSpherePatch(PatchMesh& amesh, float radius, int smooth, BOOL textured)
	{
	Point3 p;	
	int np=0,nv=0;
	
	int nverts = 6;
	int nvecs = 48;
	int npatches = 8;
	amesh.setNumVerts(nverts);
	amesh.setNumTVerts(textured ? 13 : 0);
	amesh.setNumVecs(nvecs);
	amesh.setNumPatches(npatches);
	amesh.setNumTVPatches(textured ? npatches : 0);

	Point3 v0(0.0f, 0.0f, radius);		// Top
	Point3 v1(0.0f, 0.0f, -radius);		// Bottom
	Point3 v2(0.0f, -radius, 0.0f);		// Front
	Point3 v3(radius, 0.0f, 0.0f);		// Right
	Point3 v4(0.0f, radius, 0.0f);		// Back
	Point3 v5(-radius, 0.0f, 0.0f);		// Left

	// Create the vertices.
	amesh.verts[0].flags = PVERT_COPLANAR;
	amesh.verts[1].flags = PVERT_COPLANAR;
	amesh.verts[2].flags = PVERT_COPLANAR;
	amesh.verts[3].flags = PVERT_COPLANAR;
	amesh.verts[4].flags = PVERT_COPLANAR;
	amesh.verts[5].flags = PVERT_COPLANAR;
	amesh.setVert(0, v0);
	amesh.setVert(1, v1);
	amesh.setVert(2, v2);
	amesh.setVert(3, v3);
	amesh.setVert(4, v4);
	amesh.setVert(5, v5);

	if(textured) {
		amesh.setTVert(0, UVVert(0.125f,1.0f,0.0f));
		amesh.setTVert(1, UVVert(0.375f,1.0f,0.0f));
		amesh.setTVert(2, UVVert(0.625f,1.0f,0.0f));
		amesh.setTVert(3, UVVert(0.875f,1.0f,0.0f));
		amesh.setTVert(4, UVVert(0.0f,0.5f,0.0f));
		amesh.setTVert(5, UVVert(0.25f,0.5f,0.0f));
		amesh.setTVert(6, UVVert(0.5f,0.5f,0.0f));
		amesh.setTVert(7, UVVert(0.75f,0.5f,0.0f));
		amesh.setTVert(8, UVVert(1.0f,0.5f,0.0f));
		amesh.setTVert(9, UVVert(0.125f,0.0f,0.0f));
		amesh.setTVert(10, UVVert(0.375f,0.0f,0.0f));
		amesh.setTVert(11, UVVert(0.625f,0.0f,0.0f));
		amesh.setTVert(12, UVVert(0.875f,0.0f,0.0f));

		amesh.getTVPatch(0).setTVerts(3,7,8);
		amesh.getTVPatch(1).setTVerts(0,4,5);
		amesh.getTVPatch(2).setTVerts(1,5,6);
		amesh.getTVPatch(3).setTVerts(2,6,7);
		amesh.getTVPatch(4).setTVerts(12,8,7);
		amesh.getTVPatch(5).setTVerts(9,5,4);
		amesh.getTVPatch(6).setTVerts(10,6,5);
		amesh.getTVPatch(7).setTVerts(11,7,6);
		}

	// Create the edge vectors
	float vecLen = CIRCLE_VECTOR_LENGTH * radius;
	Point3 xVec(vecLen, 0.0f, 0.0f);
	Point3 yVec(0.0f, vecLen, 0.0f);
	Point3 zVec(0.0f, 0.0f, vecLen);
	amesh.setVec(0, v0 - yVec);
	amesh.setVec(2, v0 + xVec);
	amesh.setVec(4, v0 + yVec);
	amesh.setVec(6, v0 - xVec);
	amesh.setVec(8, v1 - yVec);
	amesh.setVec(10, v1 + xVec);
	amesh.setVec(12, v1 + yVec);
	amesh.setVec(14, v1 - xVec);
	amesh.setVec(9, v2 - zVec);
	amesh.setVec(16, v2 + xVec);
	amesh.setVec(1, v2 + zVec);
	amesh.setVec(23, v2 - xVec);
	amesh.setVec(11, v3 - zVec);
	amesh.setVec(18, v3 + yVec);
	amesh.setVec(3, v3 + zVec);
	amesh.setVec(17, v3 - yVec);
	amesh.setVec(13, v4 - zVec);
	amesh.setVec(20, v4 - xVec);
	amesh.setVec(5, v4 + zVec);
	amesh.setVec(19, v4 + xVec);
	amesh.setVec(15, v5 - zVec);
	amesh.setVec(22, v5 - yVec);
	amesh.setVec(7, v5 + zVec);
	amesh.setVec(21, v5 + yVec);
	
	// Create the patches
	amesh.MakeTriPatch(np++, 0, 0, 1, 2, 16, 17, 3, 3, 2, 24, 25, 26, smooth);
	amesh.MakeTriPatch(np++, 0, 2, 3, 3, 18, 19, 4, 5, 4, 27, 28, 29, smooth);
	amesh.MakeTriPatch(np++, 0, 4, 5, 4, 20, 21, 5, 7, 6, 30, 31, 32, smooth);
	amesh.MakeTriPatch(np++, 0, 6, 7, 5, 22, 23, 2, 1, 0, 33, 34, 35, smooth);
	amesh.MakeTriPatch(np++, 1, 10, 11, 3, 17, 16, 2, 9, 8, 36, 37, 38, smooth);
	amesh.MakeTriPatch(np++, 1, 12, 13, 4, 19, 18, 3, 11, 10, 39, 40, 41, smooth);
	amesh.MakeTriPatch(np++, 1, 14, 15, 5, 21, 20, 4, 13, 12, 42, 43, 44, smooth);
	amesh.MakeTriPatch(np++, 1, 8, 9, 2, 23, 22, 5, 15, 14, 45, 46, 47, smooth);

	// Create all the interior vertices and make them non-automatic
	float chi = 0.5893534f * radius;

	int interior = 24;
	amesh.setVec(interior++, Point3(chi, -chi, radius)); 
	amesh.setVec(interior++, Point3(chi, -radius, chi)); 
	amesh.setVec(interior++, Point3(radius, -chi, chi)); 

	amesh.setVec(interior++, Point3(chi, chi, radius)); 
	amesh.setVec(interior++, Point3(radius, chi, chi)); 
	amesh.setVec(interior++, Point3(chi, radius, chi)); 

	amesh.setVec(interior++, Point3(-chi, chi, radius)); 
	amesh.setVec(interior++, Point3(-chi, radius, chi)); 
	amesh.setVec(interior++, Point3(-radius, chi, chi)); 

	amesh.setVec(interior++, Point3(-chi, -chi, radius)); 
	amesh.setVec(interior++, Point3(-radius, -chi, chi)); 
	amesh.setVec(interior++, Point3(-chi, -radius, chi)); 

	amesh.setVec(interior++, Point3(chi, -chi, -radius)); 
	amesh.setVec(interior++, Point3(radius, -chi, -chi)); 
	amesh.setVec(interior++, Point3(chi, -radius, -chi)); 

	amesh.setVec(interior++, Point3(chi, chi, -radius)); 
	amesh.setVec(interior++, Point3(chi, radius, -chi)); 
	amesh.setVec(interior++, Point3(radius, chi, -chi)); 

	amesh.setVec(interior++, Point3(-chi, chi, -radius)); 
	amesh.setVec(interior++, Point3(-radius, chi, -chi)); 
	amesh.setVec(interior++, Point3(-chi, radius, -chi)); 

	amesh.setVec(interior++, Point3(-chi, -chi, -radius)); 
	amesh.setVec(interior++, Point3(-chi, -radius, -chi)); 
	amesh.setVec(interior++, Point3(-radius, -chi, -chi)); 

	for(int i = 0; i < 8; ++i)
		amesh.patches[i].SetAuto(FALSE);

	// Finish up patch internal linkages (and bail out if it fails!)
	assert(amesh.buildLinkages());

	// Calculate the interior bezier points on the PatchMesh's patches
	amesh.computeInteriors();
	amesh.InvalidateGeomCache();
	}

// Build a NURBS rep of the sphere for use with ConvertToType().

Object *
BuildNURBSSphere(float radius, float hemi, BOOL recenter, BOOL genUVs)
{
	NURBSSet nset;

	Point3 center(0,0,0);
	Point3 northAxis(0,0,1);
	Point3 refAxis(0,-1,0);

	float startAngleU = -PI;
	float endAngleU = PI;
	float startAngleV = -HALFPI + (hemi * PI);
	float endAngleV = HALFPI;
	if (recenter)
		center = Point3(0.0, 0.0, -cos((1.0f-hemi) * PI) * radius);

	NURBSCVSurface *surf = new NURBSCVSurface();
	nset.AppendObject(surf);
	surf->SetGenerateUVs(genUVs);

	surf->SetTextureUVs(0, 0, Point2(0.0f, hemi));
	surf->SetTextureUVs(0, 1, Point2(0.0f, 1.0f));
	surf->SetTextureUVs(0, 2, Point2(1.0f, hemi));
	surf->SetTextureUVs(0, 3, Point2(1.0f, 1.0f));

	surf->FlipNormals(TRUE);
	surf->Renderable(TRUE);
	char bname[80];
	char sname[80];
	strcpy(bname, GetString(IDS_RB_SPHERE));
	sprintf(sname, "%s%s", bname, GetString(IDS_CT_SURF));
	surf->SetName(sname);
	GenNURBSSphereSurface(radius, center, northAxis, refAxis,
					startAngleU, endAngleU, startAngleV, endAngleV, FALSE, *surf);


#define F(s1, s2, s1r, s1c, s2r, s2c) \
	fuse.mSurf1 = (s1); \
	fuse.mSurf2 = (s2); \
	fuse.mRow1 = (s1r); \
	fuse.mCol1 = (s1c); \
	fuse.mRow2 = (s2r); \
	fuse.mCol2 = (s2c); \
	nset.mSurfFuse.Append(1, &fuse);

	NURBSFuseSurfaceCV fuse;

	// poles
	for (int f = 1; f < surf->GetNumVCVs(); f++) {
		if (hemi <= 0.0f) {
			// south pole
			F(0, 0, 0, 0, 0, f);
		}
		//north pole
		F(0, 0, surf->GetNumUCVs()-1, 0, surf->GetNumUCVs()-1, f);
	}
	// seam
	for (f = 0; f < surf->GetNumUCVs()-1; f++) {
		F(0, 0, f, 0, f, surf->GetNumVCVs()-1);
	}

	if (hemi > 0.0f) {
		// Cap
		// we need to make a cap that is a NURBS surface one edge of which matches
		// the south polar edge and the rest is degenerate...
		Point3 bot;
		if (recenter)
			bot = Point3(0,0,0);
		else
			bot = Point3(0.0, 0.0, cos((1.0-hemi) * PI)*radius);

		NURBSCVSurface *s0 = (NURBSCVSurface*)nset.GetNURBSObject(0);
		NURBSCVSurface *s = new NURBSCVSurface();
		nset.AppendObject(s);

		// we'll be cubic in on direction and match the sphere in the other
		s->SetUOrder(4);
		s->SetNumUKnots(8);
		for (int i = 0; i < 4; i++) {
			s->SetUKnot(i, 0.0);
			s->SetUKnot(i+4, 1.0);
		}

		s->SetVOrder(s0->GetVOrder());
		s->SetNumVKnots(s0->GetNumVKnots());
		for (i = 0; i < s->GetNumVKnots(); i++)
			s->SetVKnot(i, s0->GetVKnot(i));

		int numU, numV;
		s0->GetNumCVs(numU, numV);
		s->SetNumCVs(4, numV);

		for (int v = 0; v < numV; v++) {
			Point3 edge = s0->GetCV(0, v)->GetPosition(0);
			double w = s0->GetCV(0, v)->GetWeight(0);
			for (int u = 0; u < 4; u++) {
				NURBSControlVertex ncv;
				ncv.SetPosition(0, bot + ((edge - bot)*((float)u/3.0f)));
				ncv.SetWeight(0, w);
				s->SetCV(u, v, ncv);
			}
			// fuse the cap to the hemi
			F(1, 0, 3, v, 0, v);

			// fuse the center degeneracy
			if (v > 0) {
				F(1, 1, 0, 0, 0, v);
			}
		}

		// fuse the remaining two end sections
		F(1, 1, 1, 0, 1, numV-1);
		F(1, 1, 2, 0, 2, numV-1);

		s->SetGenerateUVs(genUVs);

		s->SetTextureUVs(0, 0, Point2(1.0f, 1.0f));
		s->SetTextureUVs(0, 1, Point2(0.0f, 1.0f));
		s->SetTextureUVs(0, 2, Point2(1.0f, 0.0f));
		s->SetTextureUVs(0, 3, Point2(0.0f, 0.0f));

		s->FlipNormals(TRUE);
		s->Renderable(TRUE);
		sprintf(sname, "%s%s%01", bname, GetString(IDS_CT_CAP));
		s->SetName(sname);
	}

	Matrix3 mat;
	mat.IdentityMatrix();
	Object *ob = CreateNURBSObject(NULL, &nset, mat);
	return ob;
}


// --- Inherited virtual methods of Animatable ---
// This method is called by the system when the user needs 
// to edit the objects parameters in the command panel.  
void SphereObject::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
	{
	// We subclass off SimpleObject so we must call its
	// BeginEditParams() method first.
	SimpleObject::BeginEditParams(ip,flags,prev);
	this->ip = ip;

	if (pmapCreate && pmapParam) {
		
		// Left over from last sphere ceated
		pmapCreate->SetParamBlock(this);
		pmapTypeIn->SetParamBlock(this);
		pmapParam->SetParamBlock(pblock);
	} else {
		
		// Gotta make a new one.
		if (flags&BEGIN_EDIT_CREATE) {
			// Here we create each new rollup page in the command panel
			// using our descriptors.
			pmapCreate = CreateCPParamMap(
				descCreate,CREATEDESC_LENGH,
				this,
				ip,
				hInstance,
				MAKEINTRESOURCE(IDD_SPHEREPARAM1),
				GetString(IDS_RB_CREATIONMETHOD),
				0);

			pmapTypeIn = CreateCPParamMap(
				descTypeIn,TYPEINDESC_LENGH,
				this,
				ip,
				hInstance,
				MAKEINTRESOURCE(IDD_SPHEREPARAM3),
				GetString(IDS_RB_KEYBOARDENTRY),
				APPENDROLL_CLOSED);
			}

		pmapParam = CreateCPParamMap(
			descParam,PARAMDESC_LENGH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_SPHEREPARAM2),
			GetString(IDS_RB_PARAMETERS),
			0);
		}

	if(pmapTypeIn) {
		// A callback for the type in.
		// This handles processing the Create button in the 
		// Keyboard Entry rollup page.
		pmapTypeIn->SetUserDlgProc(new SphereTypeInDlgProc(this));
		}
	}
		
// This is called by the system to terminate the editing of the
// parameters in the command panel.  
void SphereObject::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
	{		
	SimpleObject::EndEditParams(ip,flags,next);
	this->ip = NULL;

	if (flags&END_EDIT_REMOVEUI ) {
		// Remove the rollup pages from the command panel.
		if (pmapCreate) DestroyCPParamMap(pmapCreate);
		if (pmapTypeIn) DestroyCPParamMap(pmapTypeIn);
		DestroyCPParamMap(pmapParam);
		pmapParam  = NULL;
		pmapTypeIn = NULL;
		pmapCreate = NULL;
		}

	// Save these values in class variables so the next object created 
	// will inherit them.
	pblock->GetValue(PB_SEGS,ip->GetTime(),dlgSegments,FOREVER);
	pblock->GetValue(PB_SMOOTH,ip->GetTime(),dlgSmooth,FOREVER);	
	}

// --- Inherited virtual methods of ReferenceMaker ---
#define NEWMAP_CHUNKID	0x0100

// Called by MAX when the sphere object is loaded from disk.
IOResult SphereObject::Load(ILoad *iload) 
	{
	ClearAFlag(A_PLUGIN1);

	IOResult res;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {	
			case NEWMAP_CHUNKID:
				SetAFlag(A_PLUGIN1);
				break;
			}
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}

	// This is the callback that corrects for any older versions
	// of the parameter block structure found in the MAX file 
	// being loaded.
	iload->RegisterPostLoadCallback(
		new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,0));
	return IO_OK;
	}

IOResult SphereObject::Save(ISave *isave)
	{
	if (TestAFlag(A_PLUGIN1)) {
		isave->BeginChunk(NEWMAP_CHUNKID);
		isave->EndChunk();
		}
 	return IO_OK;
	}


// --- Inherited virtual methods of ReferenceTarget ---
RefTargetHandle SphereObject::Clone(RemapDir& remap) 
	{
	SphereObject* newob = new SphereObject();	
	newob->ReplaceReference(0,pblock->Clone(remap));
	newob->ivalid.SetEmpty();	
	return(newob);
	}

// --- Inherited virtual methods of BaseObject ---
CreateMouseCallBack* SphereObject::GetCreateMouseCallBack() 
	{
	sphereCreateCB.SetObj(this);
	return(&sphereCreateCB);
	}

// --- Inherited virtual methods of Object ---
Object* SphereObject::ConvertToType(TimeValue t, Class_ID obtype)
	{
	if (obtype == patchObjectClassID) {
		Interval valid = FOREVER;
		float radius;
		int smooth, genUVs;
		pblock->GetValue(PB_RADIUS,t,radius,valid);
		pblock->GetValue(PB_SMOOTH,t,smooth,valid);	
		pblock->GetValue(PB_GENUVS,t,genUVs,valid);
		PatchObject *ob = new PatchObject();
		BuildSpherePatch(ob->patch,radius,smooth,genUVs);
		ob->SetChannelValidity(TOPO_CHAN_NUM,valid);
		ob->SetChannelValidity(GEOM_CHAN_NUM,valid);
		ob->UnlockObject();
		return ob;
	} else  if (obtype == EDITABLE_SURF_CLASS_ID) {
		Interval valid = FOREVER;
		float radius, hemi;
		int recenter, genUVs;
		pblock->GetValue(PB_RADIUS,t,radius,valid);
		pblock->GetValue(PB_HEMI,t,hemi,valid);	
		pblock->GetValue(PB_RECENTER,t,recenter,valid);
		pblock->GetValue(PB_GENUVS,t,genUVs,valid);
		Object *ob = BuildNURBSSphere(radius, hemi, recenter,genUVs);
		ob->SetChannelValidity(TOPO_CHAN_NUM,valid);
		ob->SetChannelValidity(GEOM_CHAN_NUM,valid);
		ob->UnlockObject();
		return ob;
		
	} else{
		return SimpleObject::ConvertToType(t,obtype);
		}
	}

int SphereObject::CanConvertToType(Class_ID obtype)
	{
	if (obtype==patchObjectClassID || obtype==defObjectClassID ||
		obtype==triObjectClassID || obtype==EDITABLE_SURF_CLASS_ID) {
		return 1;
	} else {
		return SimpleObject::CanConvertToType(obtype);
		}
	}

void SphereObject::GetCollapseTypes(Tab<Class_ID> &clist,Tab<TSTR*> &nlist)
{
   	return;
#if 0
	Object::GetCollapseTypes(clist, nlist);
    Class_ID id = EDITABLE_SURF_CLASS_ID;
    TSTR *name = new TSTR(GetString(IDS_SM_NURBS_SURFACE));
    clist.Append(1,&id);
    nlist.Append(1,&name);
#endif
}

Point3 SphereObject::GetSurfacePoint(
		TimeValue t, float u, float v,Interval &iv)
	{
	float rad;
	pblock->GetValue(PB_RADIUS, t, rad, iv);
	Point3 pos;	
	v -= 0.5f;
	float ar = (float)cos(v*PI);
	pos.x = rad * float(cos(u*TWOPI)) * ar;
	pos.y = rad * float(sin(u*TWOPI)) * ar;
	pos.z = rad * float(sin(v*PI));
	return pos;
	}

BOOL SphereObject::HasUVW() { 
	BOOL genUVs;
	Interval v;
	pblock->GetValue(PB_GENUVS, 0, genUVs, v);
	return genUVs; 
	}

void SphereObject::SetGenUVW(BOOL sw) {  
	if (sw==HasUVW()) return;
	pblock->SetValue(PB_GENUVS,0, sw);				
	}

int SphereObject::IntersectRay(
		TimeValue t, Ray& ray, float& at, Point3& norm)
	{
	int smooth, recenter;
	pblock->GetValue(PB_SMOOTH,t,smooth,FOREVER);
	pblock->GetValue(PB_RECENTER,t,recenter,FOREVER);	
	float hemi;
	pblock->GetValue(PB_HEMI,t,hemi,FOREVER);
	if (!smooth || hemi!=0.0f || recenter) {
		return SimpleObject::IntersectRay(t,ray,at,norm);
		}	
	
	float r;
	float a, b, c, ac4, b2, at1, at2;
	float root;
	BOOL neg1, neg2;

	pblock->GetValue(PB_RADIUS,t,r,FOREVER);

	a = DotProd(ray.dir,ray.dir);
	b = DotProd(ray.dir,ray.p) * 2.0f;
	c = DotProd(ray.p,ray.p) - r*r;
	
	ac4 = 4.0f * a * c;
	b2 = b*b;

	if (ac4 > b2) return 0;

	// We want the smallest positive root
	root = float(sqrt(b2-ac4));
	at1 = (-b + root) / (2.0f * a);
	at2 = (-b - root) / (2.0f * a);
	neg1 = at1<0.0f;
	neg2 = at2<0.0f;
	if (neg1 && neg2) return 0;
	else
	if (neg1 && !neg2) at = at2;
	else 
	if (!neg1 && neg2) at = at1;
	else
	if (at1<at2) at = at1;
	else at = at2;
	
	norm = Normalize(ray.p + at*ray.dir);

	return 1;
	}

// --- Inherited virtual methods of SimpleObject ---
// Return TRUE if it is OK to display the mesh at the time requested,
// return FALSE otherwise.
BOOL SphereObject::OKtoDisplay(TimeValue t) 
	{
	float radius;
	pblock->GetValue(PB_RADIUS,t,radius,FOREVER);
	if (radius==0.0f) return FALSE;
	else return TRUE;
	}

// This method is called when the user interface controls need to be
// updated to reflect new values because of the user moving the time
// slider.  Here we simply call a method of the parameter map to 
// handle this for us.
void SphereObject::InvalidateUI() 
	{
	if (pmapParam) pmapParam->Invalidate();
	}

// This method returns the dimension of the parameter requested.
// This dimension describes the type and magnitude of the value
// stored by the parameter.
ParamDimension *SphereObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_RADIUS:
			return stdWorldDim;			
		case PB_HEMI:
			return stdNormalizedDim;
		case PB_SEGS:
			return stdSegmentsDim;			
		case PB_SMOOTH:
			return stdNormalizedDim;			
		default:
			return defaultDim;
		}
	}

// This method returns the name of the parameter requested.
TSTR SphereObject::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_RADIUS:
			return TSTR(GetString(IDS_RB_RADIUS));			
		case PB_HEMI:
			return GetString(IDS_RB_HEMISPHERE);
		case PB_SEGS:
			return TSTR(GetString(IDS_RB_SEGS));			
		case PB_SMOOTH:
			return TSTR(GetString(IDS_RB_SMOOTH));			
		default:
			return TSTR(_T(""));
		}
	}

// Builds the mesh representation for the sphere based on the
// state of it's parameters at the time requested.
void SphereObject::BuildMesh(TimeValue t)
	{
	Point3 p;	
	int ix,na,nb,nc,nd,jx,kx;
	int nf=0,nv=0;
	float delta, delta2;
	float a,alt,secrad,secang,b,c;
	int segs, smooth;
	float radius;
	float hemi;
	BOOL noHemi = FALSE;	
	int squash;
	int recenter;
	BOOL genUVs = TRUE;
	float startAng = 0.0f;
	if (TestAFlag(A_PLUGIN1)) startAng = HALFPI;

	// Start the validity interval at forever and whittle it down.
	ivalid = FOREVER;
	pblock->GetValue(PB_RADIUS, t, radius, ivalid);
	pblock->GetValue(PB_SEGS, t, segs, ivalid);
	pblock->GetValue(PB_SMOOTH, t, smooth, ivalid);
	pblock->GetValue(PB_HEMI, t, hemi, ivalid);
	pblock->GetValue(PB_SQUASH, t, squash, ivalid);
	pblock->GetValue(PB_RECENTER, t, recenter, ivalid);
	pblock->GetValue(PB_GENUVS, t, genUVs, ivalid);
	LimitValue(segs, MIN_SEGMENTS, MAX_SEGMENTS);
	LimitValue(smooth, MIN_SMOOTH, MAX_SMOOTH);
	LimitValue(radius, MIN_RADIUS, MAX_RADIUS);
	LimitValue(hemi, 0.0f, 1.0f);

	if (hemi<0.00001f) noHemi = TRUE;
	if (hemi>=1.0f) hemi = 0.9999f;
	hemi = (1.0f-hemi) * PI;

	if (!noHemi && squash) {
		delta  = 2.0f*hemi/float(segs-2);
		delta2 = 2.0f*PI/(float)segs;
	} else {
		delta  = 2.0f*PI/(float)segs;
		delta2 = delta;
		}

	int rows;
	if (noHemi || squash) {
		rows = (segs/2-1);
	} else {
		rows = int(hemi/delta) + 1;
		}
	int nverts = rows * segs + 2;
	int nfaces = rows * segs * 2;
	mesh.setNumVerts(nverts);
	mesh.setNumFaces(nfaces);
	mesh.setSmoothFlags(smooth != 0);

	// Top vertex 
	mesh.setVert(nv, 0.0f, 0.0f, radius);
	nv++;

	// Middle vertices 
	alt=delta;
	for(ix=1; ix<=rows; ix++) {		
		if (!noHemi && ix==rows) alt = hemi;
		a = (float)cos(alt)*radius;		
		secrad = (float)sin(alt)*radius;
		secang = startAng; //0.0f
		for(jx=0; jx<segs; ++jx) {
			b = (float)cos(secang)*secrad;
			c = (float)sin(secang)*secrad;
			mesh.setVert(nv++,b,c,a);
			secang+=delta2;
			}
				
		alt+=delta;		
		}

	/* Bottom vertex */
	if (noHemi) {
		mesh.setVert(nv++, 0.0f, 0.0f,-radius);
		}
	else {
		a = (float)cos(hemi)*radius;
		mesh.setVert(nv++, 0.0f, 0.0f, a);
		}

	// Now make faces 
	
	// Make top conic cap
	for(ix=1; ix<=segs; ++ix) {
		nc=(ix==segs)?1:ix+1;
		mesh.faces[nf].setEdgeVisFlags(1,1,1);
		mesh.faces[nf].setSmGroup(smooth?1:0);
		mesh.faces[nf].setVerts(0, ix, nc);
		mesh.faces[nf].setMatID(1);
		nf++;
		}

	/* Make midsection */
	for(ix=1; ix<rows; ++ix) {
		jx=(ix-1)*segs+1;
		for(kx=0; kx<segs; ++kx) {
			na = jx+kx;
			nb = na+segs;
			nc = (kx==(segs-1))? jx+segs: nb+1;
			nd = (kx==(segs-1))? jx : na+1;
			
			mesh.faces[nf].setEdgeVisFlags(1,1,0);
			mesh.faces[nf].setSmGroup(smooth?1:0);
			mesh.faces[nf].setVerts(na,nb,nc);
			mesh.faces[nf].setMatID(1);
			nf++;

			mesh.faces[nf].setEdgeVisFlags(0,1,1);
			mesh.faces[nf].setSmGroup(smooth?1:0);
			mesh.faces[nf].setVerts(na,nc,nd);
			mesh.faces[nf].setMatID(1);
			nf++;
			}
	 	}

	// Make bottom conic cap
	na = mesh.getNumVerts()-1;
	jx = (rows-1)*segs+1;
	for(ix=0; ix<segs; ++ix) {
		nc = ix + jx;
		nb = (ix==segs-1)?jx:nc+1;
		mesh.faces[nf].setEdgeVisFlags(1,1,1);
		mesh.faces[nf].setSmGroup(smooth?1:0);
		mesh.faces[nf].setVerts(na, nb, nc);
		if (smooth) {
			mesh.faces[nf].setSmGroup(noHemi?1:2);
		} else {
			mesh.faces[nf].setSmGroup(0);
			}
		mesh.faces[nf].setMatID(noHemi?1:0);
		nf++;
		}

	// Put the flat part of the hemisphere at z=0
	if (recenter) {
		float shift = (float)cos(hemi) * radius;
		for (ix=0; ix<mesh.getNumVerts(); ix++) {
			mesh.verts[ix].z -= shift;
			}
		}

	if (genUVs) {
		int ntverts = (rows+2)*(segs+1);
		mesh.setNumTVerts(ntverts);
		mesh.setNumTVFaces(nfaces);
		nv = 0;
		delta  = 2.0f*PI/(float)segs;  // make the texture squash too
		alt = 0.0f; // = delta;

		for(ix=0; ix < rows+2; ix++) {		
		//	if (!noHemi && ix==rows) alt = hemi;		
			secang = 0.0f; //angle;
			for(jx=0; jx <= segs; ++jx) {
				mesh.setTVert(nv++, secang/TWOPI, 1.0f-alt/PI, 0.0f);
				secang += delta2;
				}
			alt += delta;		
			}

		nf = 0;
		// Make top conic cap
		for(ix=0; ix<segs; ++ix) {
			mesh.tvFace[nf++].setTVerts(ix,ix+segs+1,ix+segs+2);
			}

		/* Make midsection */
		for(ix=1; ix<rows; ++ix) {
			jx = ix*(segs+1);
			for(kx=0; kx<segs; ++kx) {
				na = jx+kx;
				nb = na+segs+1;
				nc = nb+1;
				nd = na+1;
				assert(nc<ntverts);
				assert(nd<ntverts);
				mesh.tvFace[nf++].setTVerts(na,nb,nc);
				mesh.tvFace[nf++].setTVerts(na,nc,nd);
				}
			}
		// Make bottom conic cap
		jx = rows*(segs+1);
		for(ix=0; ix<segs; ++ix) {
			nc = ix + jx;
			na = ix + jx + segs+1;
			nb = nc + 1;
			assert(na<ntverts);
			assert(nb<ntverts);
			assert(nc<ntverts);
			mesh.tvFace[nf++].setTVerts(na,nb,nc);
			}
		assert(nf==nfaces);
		}
	else {
		mesh.setNumTVerts(0);
		mesh.setNumTVFaces(0);
		}

	mesh.InvalidateGeomCache();
	mesh.BuildStripsAndEdges();
	}

// --- Inherited virtual methods of GenSphere ---
void SphereObject::SetParams(float rad, int segs, BOOL smooth, BOOL genUV,
	 float hemi, BOOL squash, BOOL recenter) {
	pblock->SetValue(PB_RADIUS,0, rad);				
	pblock->SetValue(PB_HEMI,0, hemi);				
	pblock->SetValue(PB_SEGS,0, segs);				
	pblock->SetValue(PB_SQUASH,0, squash);				
	pblock->SetValue(PB_SMOOTH,0, smooth);				
	pblock->SetValue(PB_RECENTER,0, recenter);				
	pblock->SetValue(PB_GENUVS,0, genUV);				
	}			   

// --- Inherited virtual methods of IParamArray ---
// These methods allow the parameter map to access our class variables.
// Based on the virtual array index passed in we set or get the 
// variable requested.
BOOL SphereObject::SetValue(int i, TimeValue t, int v) 
	{
	switch (i) {
		case PB_CREATEMETHOD: dlgCreateMeth = v; break;
		}		
	return TRUE;
	}

BOOL SphereObject::SetValue(int i, TimeValue t, float v)
	{
	switch (i) {				
		case PB_TI_RADIUS: crtRadius = v; break;
		}	
	return TRUE;
	}

BOOL SphereObject::SetValue(int i, TimeValue t, Point3 &v) 
	{
	switch (i) {
		case PB_TI_POS: crtPos = v; break;
		}		
	return TRUE;
	}

BOOL SphereObject::GetValue(int i, TimeValue t, int &v, Interval &ivalid) 
	{
	switch (i) {
		case PB_CREATEMETHOD: v = dlgCreateMeth; break;
		}
	return TRUE;
	}

BOOL SphereObject::GetValue(int i, TimeValue t, float &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_RADIUS: v = crtRadius; break;
		}
	return TRUE;
	}

BOOL SphereObject::GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid) 
	{	
	switch (i) {		
		case PB_TI_POS: v = crtPos; break;		
		}
	return TRUE;
	}

// --- Methods of SphereObject ---
SphereObject::SphereObject()
	{
	// Create the parameter block and make a reference to it.
	SetAFlag(A_PLUGIN1);
	MakeRefByID(FOREVER, 0, CreateParameterBlock(descVer2, PBLOCK_LENGTH, CURRENT_VERSION));
	assert(pblock);

	// Initialize the default values.
	pblock->SetValue(PB_RADIUS,0,crtRadius);
	pblock->SetValue(PB_SMOOTH,0,dlgSmooth);
	pblock->SetValue(PB_SEGS,0,dlgSegments);	
	pblock->SetValue(PB_SQUASH,0,0);
	}

// See the Advanced Topics section on DLL Functions and Class Descriptors
// for more information.
/*===========================================================================*\
 | The Class Descriptor
\*===========================================================================*/
class SphereClassDesc:public ClassDesc {
	public:
	// This method should return TRUE if the plug-in can be picked
	// and assigned by the user. Some plug-ins may be used privately by other
	// plug-ins implemented in the same DLL and should not appear in lists for
	// user to choose from (such plug-ins would return FALSE).
	int 			IsPublic() { return 1; }
	// This is the method that actually creates a new instance of
	// the plug-in class.  The system calls the correspoding 
	// Animatable::DeleteThis() method of the plug-in to free the memory.
	// This implementations use 'new' and 'delete'.
	void			*Create(BOOL loading = FALSE) { return new SphereObject; }
	// This is the name that appears on the button in the MAX user interface.
	const TCHAR		*ClassName() { return GetString(IDS_RB_SPHERE_CLASS); }
	// The system calls this method at startup to determine the type of plug-in
	// this is. The possible options are defined in PLUGAPI.H
	SClass_ID		SuperClassID() { return GEOMOBJECT_CLASS_ID; }
	// The system calls this method to retrieve the unique Class ID 
	// for this object.
	Class_ID		ClassID() { return SPHERE_C_CLASS_ID; }
	// The category is selected in the bottom most drop down list in the 
	// reate branch. If this is set to be an exiting category for example 
	// "Standard Primitives" then the plug-in will appear in that category. 
	// If the category doesn't exists then it is created.
	const TCHAR		*Category() { return GetString(IDS_RB_HOWTO); }
	// When the user executes File / Reset this method is called.  The plug-in
	// can respond by resetting itself to use its default values.
	void			ResetClassParams(BOOL fileReset);
	};

// Declare a single static instance of the class descriptor.
static SphereClassDesc sphereDesc;

// This function returns the address of the descriptor.  We call it from 
// the LibClassDesc() function, which is called by the system when loading
// the DLLs at startup.
ClassDesc* GetSphereDesc() { return &sphereDesc; }

void SphereClassDesc::ResetClassParams(BOOL fileReset)
	{
	SphereObject::dlgSegments    = DEF_SEGMENTS;
	SphereObject::dlgCreateMeth  = 1; // create_radius
	SphereObject::dlgSmooth      = SMOOTH_ON;
	SphereObject::crtPos         = Point3(0,0,0);
	SphereObject::crtRadius      = 0.0f;
	}

// The following five functions are used by every plug-in DLL.
/*===========================================================================*\
 | The DLL and Library Functions
\*===========================================================================*/
// This function is called by Windows when the DLL is loaded.  This 
// function may also be called many times during time critical operations
// like rendering.  Therefore developers need to be careful what they
// do inside this function.  In the code below, note how after the DLL is
// loaded the first time only a few statements are executed.
#if 0
int controlsInit = FALSE;
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) 
	{	
	// Hang on to this DLL's instance handle.
	hInstance = hinstDLL;

	if (! controlsInit) {
		controlsInit = TRUE;
		
		// Initialize MAX's custom controls
		InitCustomControls(hInstance);
		
		// Initialize Win95 controls
		InitCommonControls();
	}
	
	return(TRUE);
	}
#endif

#if 0
// This function returns the number of plug-in classes this DLL implements
__declspec( dllexport ) int LibNumberClasses() {return 1;}

// This function return the ith class descriptor
__declspec( dllexport ) ClassDesc *LibClassDesc(int i) {
	switch(i) {
		case 0: return GetSphereDesc();		
		default: return 0;
		}
	}


// This function returns a string that describes the DLL.  This string appears in 
// the File / Summary Info / Plug-In Info dialog box.
__declspec( dllexport ) const TCHAR *LibDescription() { 
	return GetString(IDS_LIB_DESC);
	}


// This function returns a pre-defined constant indicating the version of 
// the system under which it was compiled.  It is used to allow the system
// to catch obsolete DLLs.
__declspec( dllexport ) ULONG LibVersion() { return VERSION_3DSMAX; }
#endif
