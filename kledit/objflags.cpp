/*
 * ObjFlags.c
 */

#include "stdafx.h"
#include "ObjFlags.h"

ParamUIDesc OFdescParam[] = {
	ParamUIDesc (PB_CALC_MATRIX,	TYPE_SINGLECHEKBOX,	IDC_CALC_MATRIX),
	ParamUIDesc (PB_ID,				EDITTYPE_INT,		IDC_ID, IDC_IDSPINNER, 0, MAX_SUB_OBJECTS - 1, 0.1f),
	ParamUIDesc (PB_TARGETABLE,		TYPE_SINGLECHEKBOX,	IDC_TARGETABLE),
	ParamUIDesc (PB_HIGH_D_COLL,	TYPE_SINGLECHEKBOX,	IDC_HIGH_D_COLL),
	ParamUIDesc (PB_SHADOW,			TYPE_SINGLECHEKBOX,	IDC_CASTSHADOW),
	ParamUIDesc (PB_UNLIT,			TYPE_SINGLECHEKBOX,	IDC_UNLIT),
	ParamUIDesc (PB_ROUNDSHADOW,	TYPE_SINGLECHEKBOX,	IDC_ROUNDSHADOW),
	ParamUIDesc (PB_FACEON,			TYPE_SINGLECHEKBOX,	IDC_FACEON),
	ParamUIDesc (PB_STRATMOVE,	   	TYPE_SINGLECHEKBOX,	IDC_STRATMOVE),
	ParamUIDesc (PB_WATEREFFECT,   	TYPE_SINGLECHEKBOX,	IDC_WATEREFFECT),
	ParamUIDesc (PB_LAVAEFFECT,   	TYPE_SINGLECHEKBOX,	IDC_LAVAEFFECT),
};

ParamBlockDescID OFdescVer1[] = {
	{ TYPE_BOOL,	NULL, FALSE, PB_CALC_MATRIX },
	{ TYPE_INT,		NULL, FALSE, PB_ID },
};
#define VER1_SIZE 2

ParamBlockDescID OFdescVer2[] = {
	{ TYPE_BOOL,	NULL, FALSE, PB_CALC_MATRIX },
	{ TYPE_INT,		NULL, FALSE, PB_ID },
	{ TYPE_BOOL,	NULL, FALSE, PB_TARGETABLE },
};
#define VER2_SIZE 3

ParamBlockDescID OFdescVer3[] = {
	{ TYPE_BOOL,	NULL, FALSE, PB_CALC_MATRIX },
	{ TYPE_INT,		NULL, FALSE, PB_ID },
	{ TYPE_BOOL,	NULL, FALSE, PB_TARGETABLE },
	{ TYPE_BOOL,	NULL, FALSE, PB_HIGH_D_COLL },
};
#define VER3_SIZE 4
ParamBlockDescID OFdescVer4[] = {
	{ TYPE_BOOL,	NULL, FALSE, PB_CALC_MATRIX },
	{ TYPE_INT,		NULL, FALSE, PB_ID },
	{ TYPE_BOOL,	NULL, FALSE, PB_TARGETABLE },
	{ TYPE_BOOL,	NULL, FALSE, PB_HIGH_D_COLL },
	{ TYPE_BOOL,	NULL, FALSE, PB_SHADOW },
	{ TYPE_BOOL,	NULL, FALSE, PB_UNLIT },
};
#define VER4_SIZE 6
ParamBlockDescID OFdescVer5[] = {
	{ TYPE_BOOL,	NULL, FALSE, PB_CALC_MATRIX },
	{ TYPE_INT,		NULL, FALSE, PB_ID },
	{ TYPE_BOOL,	NULL, FALSE, PB_TARGETABLE },
	{ TYPE_BOOL,	NULL, FALSE, PB_HIGH_D_COLL },
	{ TYPE_BOOL,	NULL, FALSE, PB_SHADOW },
	{ TYPE_BOOL,	NULL, FALSE, PB_UNLIT },
	{ TYPE_BOOL,	NULL, FALSE, PB_ROUNDSHADOW },
};
#define VER5_SIZE 7
ParamBlockDescID OFdescVer6[] = {
	{ TYPE_BOOL,	NULL, FALSE, PB_CALC_MATRIX },
	{ TYPE_INT,		NULL, FALSE, PB_ID },
	{ TYPE_BOOL,	NULL, FALSE, PB_TARGETABLE },
	{ TYPE_BOOL,	NULL, FALSE, PB_HIGH_D_COLL },
	{ TYPE_BOOL,	NULL, FALSE, PB_SHADOW },
	{ TYPE_BOOL,	NULL, FALSE, PB_UNLIT },
	{ TYPE_BOOL,	NULL, FALSE, PB_ROUNDSHADOW },
	{ TYPE_BOOL,	NULL, FALSE, PB_FACEON },
};
#define VER6_SIZE 8

ParamBlockDescID OFdescVer7[] = {
	{ TYPE_BOOL,	NULL, FALSE, PB_CALC_MATRIX },
	{ TYPE_INT,		NULL, FALSE, PB_ID },
	{ TYPE_BOOL,	NULL, FALSE, PB_TARGETABLE },
	{ TYPE_BOOL,	NULL, FALSE, PB_HIGH_D_COLL },
	{ TYPE_BOOL,	NULL, FALSE, PB_SHADOW },
	{ TYPE_BOOL,	NULL, FALSE, PB_UNLIT },
	{ TYPE_BOOL,	NULL, FALSE, PB_ROUNDSHADOW },
	{ TYPE_BOOL,	NULL, FALSE, PB_FACEON },
	{ TYPE_BOOL,	NULL, FALSE, PB_STRATMOVE },
};
#define VER7_SIZE 9

ParamBlockDescID OFdescVer8[] = {
	{ TYPE_BOOL,	NULL, FALSE, PB_CALC_MATRIX },
	{ TYPE_INT,		NULL, FALSE, PB_ID },
	{ TYPE_BOOL,	NULL, FALSE, PB_TARGETABLE },
	{ TYPE_BOOL,	NULL, FALSE, PB_HIGH_D_COLL },
	{ TYPE_BOOL,	NULL, FALSE, PB_SHADOW },
	{ TYPE_BOOL,	NULL, FALSE, PB_UNLIT },
	{ TYPE_BOOL,	NULL, FALSE, PB_ROUNDSHADOW },
	{ TYPE_BOOL,	NULL, FALSE, PB_FACEON },
	{ TYPE_BOOL,	NULL, FALSE, PB_STRATMOVE },
	{ TYPE_BOOL,	NULL, FALSE, PB_WATEREFFECT },
	{ TYPE_BOOL,	NULL, FALSE, PB_LAVAEFFECT },
};
#define VER8_SIZE 11

// Array of old versions
static ParamVersionDesc versions[] = {
	ParamVersionDesc(OFdescVer1,VER1_SIZE,1),
		ParamVersionDesc(OFdescVer2,VER2_SIZE,2),
		ParamVersionDesc(OFdescVer3,VER3_SIZE,3),
		ParamVersionDesc(OFdescVer4,VER4_SIZE,4),
		ParamVersionDesc(OFdescVer5,VER5_SIZE,5),
		ParamVersionDesc(OFdescVer6,VER6_SIZE,6),
		ParamVersionDesc(OFdescVer7,VER7_SIZE,7),
};
#define NUM_OLDVERSIONS	7
// Current version
static ParamVersionDesc curVersion(OF_CURVERSIONDESC,
								   VER8_SIZE,				// remember to change this line
								   OF_CURRENT_VERSION);

/*******************************************************************************/

IParamMap	*ObjFlag::pmapParam;						// the default parameter map
IObjParam	*ObjFlag::objParams;
bool		ObjFlag::cCalcMatrix = false, ObjFlag::ownInstance = true;
float		ObjFlag::moi = 1.f, ObjFlag::radius = 10.f, ObjFlag::mass = 4000.f, ObjFlag::expRot = 1.f;
Point3		ObjFlag::friction = Point3(1.f,1.f,1.f);
Point3		ObjFlag::airFriction = Point3(1.f,1.f,1.f);
int			ObjFlag::uid = 0;
TSTR		ObjFlag::name = "";

ICustEdit *ObjFlagsParamDlg::nameCtrl;
ISpinnerControl *ObjFlagsParamDlg::radiusSpinner, *ObjFlagsParamDlg::massSpinner, 
			*ObjFlagsParamDlg::uidSpinner, *ObjFlagsParamDlg::moiSpinner, *ObjFlagsParamDlg::expSpinner, 
			*ObjFlagsParamDlg::frictionXSpinner, *ObjFlagsParamDlg::frictionYSpinner, *ObjFlagsParamDlg::frictionZSpinner,
			*ObjFlagsParamDlg::airFrictionXSpinner, *ObjFlagsParamDlg::airFrictionYSpinner, *ObjFlagsParamDlg::airFrictionZSpinner;
HWND		ObjFlagsParamDlg::instHwnd;

ObjFlagsParamDlg *ObjFlagsParamDlg::dlgInstance = NULL;

ObjFlag::ObjFlag (void)
{
	MakeRefByID(FOREVER, 0, CreateParameterBlock(OF_CURVERSIONDESC, OF_PARAMDESC_LENGTH, OF_CURRENT_VERSION));
	assert(pblock);

	pblock->SetValue (PB_CALC_MATRIX,	TimeValue(0), cCalcMatrix);
	pblock->SetValue (PB_ID,			TimeValue(0), 0);
	pblock->SetValue (PB_TARGETABLE,	TimeValue(0), 0);
	pblock->SetValue (PB_HIGH_D_COLL,	TimeValue(0), 0);
}

void ObjFlag::WindowFinal (IObjParam *ip)
{
	pblock->GetValue (PB_CALC_MATRIX,	ip->GetTime(), *((int *)&cCalcMatrix), FOREVER);
}

void ObjFlag::DrawMarker (GraphicsWindow *gw, bool Selected, bool Frozen)
{
	Point3 pt(0,0,0);
	if(!Frozen)
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_POINT_OBJ));
	gw->marker(&pt,SM_HOLLOW_BOX_MRKR);
}

void ObjFlagsParamDlg::Update (void)
{
	char name[256];
	nameCtrl->GetText (name, 256);
	ref->name = name;
	
	ref->ownInstance = IsDlgButtonChecked (instHwnd, IDC_INSTANCE) == BST_CHECKED ? true : false;

	ref->mass	= massSpinner->GetFVal();
	ref->moi	= moiSpinner->GetFVal();
	ref->radius = radiusSpinner->GetFVal();
	ref->expRot = expSpinner->GetFVal();
	ref->uid	= uidSpinner->GetIVal();
	ref->friction.x = frictionXSpinner->GetFVal();
	ref->friction.y = frictionYSpinner->GetFVal();
	ref->friction.z = frictionZSpinner->GetFVal();
	ref->airFriction.x = airFrictionXSpinner->GetFVal();
	ref->airFriction.y = airFrictionYSpinner->GetFVal();
	ref->airFriction.z = airFrictionZSpinner->GetFVal();
}
void ObjFlagsParamDlg::Update (TimeValue t) // max called
{
	if (ref) {
		if (nameCtrl)
			nameCtrl->SetText (ref->name);

		if (instHwnd)
			CheckDlgButton (instHwnd, IDC_INSTANCE, ref->ownInstance? BST_CHECKED : BST_UNCHECKED);

		if (radiusSpinner)
			radiusSpinner->SetValue (ref->radius, FALSE);
		if (moiSpinner)
			moiSpinner->SetValue (ref->moi, FALSE);
		if (uidSpinner)
			uidSpinner->SetValue (ref->uid, FALSE);
		if (massSpinner)
			massSpinner->SetValue (ref->mass, FALSE);
		if (expSpinner)
			expSpinner->SetValue (ref->expRot, FALSE);
		if (frictionXSpinner)
			frictionXSpinner->SetValue (ref->friction.x, FALSE);
		if (frictionYSpinner)
			frictionYSpinner->SetValue (ref->friction.y, FALSE);
		if (frictionZSpinner)
			frictionZSpinner->SetValue (ref->friction.z, FALSE);
		if (airFrictionXSpinner)
			airFrictionXSpinner->SetValue (ref->airFriction.x, FALSE);
		if (airFrictionYSpinner)
			airFrictionYSpinner->SetValue (ref->airFriction.y, FALSE);
		if (airFrictionZSpinner)
			airFrictionZSpinner->SetValue (ref->airFriction.z, FALSE);
	}
}

BOOL ObjFlagsParamDlg::DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		nameCtrl = GetICustEdit (GetDlgItem (hWnd, IDC_OBJECTNAME));
		assert (nameCtrl);

		instHwnd = hWnd;

		radiusSpinner = SetupFloatSpinner (hWnd, IDC_RADIUSSPINNER, IDC_RADIUS, 0.f, 10000.f, ref->radius);
		assert (radiusSpinner);
		uidSpinner = SetupIntSpinner (hWnd, IDC_UIDSPINNER, IDC_UID, 0, 65535, ref->uid);
		assert (uidSpinner);
		moiSpinner = SetupFloatSpinner (hWnd, IDC_MOISPINNER, IDC_MOI, 0.f, 10000.f, ref->moi);
		assert (moiSpinner);
		massSpinner = SetupFloatSpinner (hWnd, IDC_MASSSPINNER, IDC_MASS, 0.f, 10000.f, ref->mass);
		assert (massSpinner);
		expSpinner = SetupFloatSpinner (hWnd, IDC_EXPROTSPINNER, IDC_EXPROT, 0.f, 10000.f, ref->expRot);
		assert (expSpinner);
		frictionXSpinner = SetupFloatSpinner (hWnd, IDC_FRICTION_X_SPINNER, IDC_FRICTION_X, 0.f, 1.f, ref->friction.x);
		assert (frictionXSpinner);
		frictionYSpinner = SetupFloatSpinner (hWnd, IDC_FRICTION_Y_SPINNER, IDC_FRICTION_Y, 0.f, 1.f, ref->friction.y);
		assert (frictionXSpinner);
		frictionZSpinner = SetupFloatSpinner (hWnd, IDC_FRICTION_Z_SPINNER, IDC_FRICTION_Z, 0.f, 1.f, ref->friction.z);
		assert (frictionXSpinner);
		airFrictionXSpinner = SetupFloatSpinner (hWnd, IDC_FRICTION_X_SPINNER2, IDC_FRICTION_X2, 0.f, 1.f, ref->airFriction.x);
		assert (airFrictionXSpinner);
		airFrictionYSpinner = SetupFloatSpinner (hWnd, IDC_FRICTION_Y_SPINNER2, IDC_FRICTION_Y2, 0.f, 1.f, ref->airFriction.y);
		assert (airFrictionXSpinner);
		airFrictionZSpinner = SetupFloatSpinner (hWnd, IDC_FRICTION_Z_SPINNER2, IDC_FRICTION_Z2, 0.f, 1.f, ref->airFriction.z);
		assert (airFrictionXSpinner);

		nameCtrl->SetText (ref->name);
		return FALSE;
	case WM_DESTROY:
		Update();

		ReleaseICustEdit (nameCtrl);

		ReleaseISpinner (radiusSpinner);
		ReleaseISpinner (massSpinner);
		ReleaseISpinner (uidSpinner);
		ReleaseISpinner (expSpinner);
		ReleaseISpinner (moiSpinner);
		ReleaseISpinner (frictionXSpinner);
		ReleaseISpinner (frictionYSpinner);
		ReleaseISpinner (frictionZSpinner);
		ReleaseISpinner (airFrictionXSpinner);
		ReleaseISpinner (airFrictionYSpinner);
		ReleaseISpinner (airFrictionZSpinner);

		nameCtrl = NULL;
		radiusSpinner = massSpinner = uidSpinner = moiSpinner = expSpinner = 
			frictionXSpinner = frictionYSpinner = frictionZSpinner = 
			airFrictionXSpinner = airFrictionYSpinner = airFrictionZSpinner = 
			NULL;
		return TRUE;
	case WM_COMMAND:
		Update();
		break;
	case CC_SPINNER_CHANGE:
		Update();
		break;
	}
	return FALSE;
}

/*
 * Red Dog helper class description
 */
class ObjFlagsDesc : public ClassDesc
{
public:
	int 			IsPublic() {return 1;}
	void *			Create (BOOL loading = FALSE) {return new ObjFlag();}
	const TCHAR *	ClassName() {return GetString(IDS_OBJ_FLAGS_CLASS_NAME);}
	SClass_ID		SuperClassID() {return HELPER_CLASS_ID;}
	Class_ID		ClassID() {return OBJFLAGS_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_HELPER_CATEGORY);}
	void			ResetClassParams (BOOL fileReset);
	BOOL			NeedsToSave () { return TRUE; }
	IOResult		Save (ISave *save);
	IOResult		Load (ILoad *load);
};

static ObjFlagsDesc ObjFlagDesc;
ClassDesc* GetFlagDesc() {return &ObjFlagDesc;}

void ObjFlagsDesc::ResetClassParams (BOOL fileReset) 
{
	ObjFlag::name			= "";
	ObjFlag::mass			= 4000.f;
	ObjFlag::radius			= 10.f;
	ObjFlag::cCalcMatrix	= FALSE;
	ObjFlag::moi			= 1.f;
	ObjFlag::expRot			= 1.f;
	ObjFlag::uid			= 0;
	ObjFlag::friction		= Point3(1.f,1.f,1.f);
	ObjFlag::airFriction	= Point3(1.f,1.f,1.f);
	ObjFlag::ownInstance	= true;
}

#define CHUNK_MOI		1000
#define CHUNK_RADIUS	1010
#define CHUNK_MASS		1020
#define CHUNK_NAME		1030
#define CHUNK_UID		1040
#define CHUNK_FRICTION	1050
#define CHUNK_AIRFRIC	1060
#define CHUNK_EXPROT	1070
#define CHUNK_INST		1080

IOResult ObjFlagsDesc::Save (ISave *save)
{
	ULONG nb;
	save->BeginChunk (CHUNK_MOI);
	save->Write (&ObjFlag::moi, sizeof (float), &nb);
	save->EndChunk();
	save->BeginChunk (CHUNK_RADIUS);
	save->Write (&ObjFlag::radius, sizeof (float), &nb);
	save->EndChunk();
	save->BeginChunk (CHUNK_MASS);
	save->Write (&ObjFlag::mass, sizeof (float), &nb);
	save->EndChunk();
	save->BeginChunk (CHUNK_EXPROT);
	save->Write (&ObjFlag::expRot, sizeof (float), &nb);
	save->EndChunk();
	save->BeginChunk (CHUNK_UID);
	save->Write (&ObjFlag::uid, sizeof (int), &nb);
	save->EndChunk();
	save->BeginChunk (CHUNK_FRICTION);
	save->Write (&ObjFlag::friction, sizeof (Point3), &nb);
	save->EndChunk();
	save->BeginChunk (CHUNK_AIRFRIC);
	save->Write (&ObjFlag::airFriction, sizeof (Point3), &nb);
	save->EndChunk();
	save->BeginChunk (CHUNK_INST);
	save->Write (&ObjFlag::ownInstance, sizeof (bool), &nb);
	save->EndChunk();
	save->BeginChunk (CHUNK_NAME);
	int len = ObjFlag::name.Length() + 1;
	save->Write (&len, sizeof (len), &nb);
	save->Write ((char *)ObjFlag::name, sizeof (char) * len, &nb);
	save->EndChunk();
	return IO_OK;
}

IOResult ObjFlagsDesc::Load (ILoad *load)
{
	ULONG nb;
	IOResult res;

	ResetClassParams(TRUE);

	while ((res = load->OpenChunk()) == IO_OK) {
		switch (load->CurChunkID()) {
		case CHUNK_MOI:
			res = load->Read (&ObjFlag::moi, sizeof (float), &nb);
			break;
		case CHUNK_RADIUS:
			res = load->Read (&ObjFlag::radius, sizeof (float), &nb);
			break;
		case CHUNK_MASS:
			res = load->Read (&ObjFlag::mass, sizeof (float), &nb);
			break;
		case CHUNK_EXPROT:
			res = load->Read (&ObjFlag::expRot, sizeof (float), &nb);
			break;
		case CHUNK_UID:
			res = load->Read (&ObjFlag::uid, sizeof (int), &nb);
			break;
		case CHUNK_FRICTION:
			res = load->Read (&ObjFlag::friction, sizeof (Point3), &nb);
			break;
		case CHUNK_AIRFRIC:
			res = load->Read (&ObjFlag::airFriction, sizeof (Point3), &nb);
			break;
		case CHUNK_INST:
			res = load->Read (&ObjFlag::ownInstance, sizeof (bool), &nb);
			break;
		case CHUNK_NAME:
			{
				char maxFoo[1024];
				int len;
				res = load->Read (&len, sizeof (len), &nb);
				if (res==IO_OK)
					res = load->Read (maxFoo, sizeof (char) * len, &nb);
				if (res==IO_OK)
					ObjFlag::name = maxFoo;
			}
			break;
		}
		load->CloseChunk();
		if (res != IO_OK)
			return res;
	}
	// Update the display
	if (ObjFlagsParamDlg::dlgInstance)
		ObjFlagsParamDlg::dlgInstance->Update (TimeValue(0));
	return IO_OK;
}

IOResult ObjFlag::Load (ILoad *load)
{
	load->RegisterPostLoadCallback(
		new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,0));
	return IO_OK;
}

int ObjFlagCreatorClass::proc (ViewExp *vpt, int msg, int point, 
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

