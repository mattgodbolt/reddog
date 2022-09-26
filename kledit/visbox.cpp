#include "StdAfx.h"
#include "RDEdit.h"
#include "HelpBase.h"
#include "resource.h"
#include "util.h"
#include "VisBox.h"
#include "RedDog.h"

IParamMap	*VisBox::pmapParam;						// the default parameter map
IObjParam	*VisBox::objParams;

static int radButs[] = { IDC_INCLUSIVE, IDC_EXCLUSIVE };

#define PB_INDEX_TYPE		0
#define PB_EXCLUDE_AREAS	1
#define PB_BOX_ID			2
#define PB_SIZE				3

ParamUIDesc VBdescParam[] = 
{
//	ParamUIDesc (PB_INDEX_TYPE,			TYPE_RADIO, radButs, asize(radButs)),
	ParamUIDesc (PB_BOX_ID,				EDITTYPE_POS_INT, IDC_VISBOX_ID, IDC_VISBOX_SPIN, 0, 31, 1)
};
ParamBlockDescID VBdescVer1[] = {
	{ TYPE_INT,		NULL, FALSE, PB_INDEX_TYPE },
	{ TYPE_INT,		NULL, FALSE, PB_BOX_ID },
	{ TYPE_INT,		NULL, FALSE, PB_EXCLUDE_AREAS },
};
#define CURRENT_VERSION 1
#define DESC_SIZE 3

VisBox::VisBox (void)
{
	MakeRefByID(FOREVER, 0, CreateParameterBlock(VBdescVer1, DESC_SIZE, CURRENT_VERSION));
	assert(pblock);

	pblock->SetValue (PB_BOX_ID,		TimeValue(0), 0);
	pblock->SetValue (PB_INDEX_TYPE,	TimeValue(0), 0);
	pblock->SetValue (PB_EXCLUDE_AREAS,	TimeValue(0), 0);
}

VISParamDlg::VISParamDlg (VISBOX *r) { ref = (VisBox *)r; }
int VISCreatorClass::proc (ViewExp *vpt, int msg, int point, 
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
}

static int CheckBox[] =
{
	IDC_CHECK1,
	IDC_CHECK2,
	IDC_CHECK3,
	IDC_CHECK4,
	IDC_CHECK5,
	IDC_CHECK6,
	IDC_CHECK7,
	IDC_CHECK8,

	IDC_CHECK9,
	IDC_CHECK10,
	IDC_CHECK11,
	IDC_CHECK12,
	IDC_CHECK13,
	IDC_CHECK14,
	IDC_CHECK15,
	IDC_CHECK16,

	IDC_CHECK34,
	IDC_CHECK17,
	IDC_CHECK18,
	IDC_CHECK19,
	IDC_CHECK20,
	IDC_CHECK21,
	IDC_CHECK22,
	IDC_CHECK23,
	IDC_CHECK26,

	IDC_CHECK27,
	IDC_CHECK28,
	IDC_CHECK29,
	IDC_CHECK30,
	IDC_CHECK31,
	IDC_CHECK32,
	IDC_CHECK33,
};

BOOL VISParamDlg::DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	int i;
	switch (msg) {
	case WM_INITDIALOG:
		{ 
			unsigned int id;
			id = ref->GetExcludeAreas();
			for (i = 0; i < asize(CheckBox); ++i) {
				CheckDlgButton (hWnd, CheckBox[i], (id & (1<<i))? TRUE : FALSE);
			}
		}
		return TRUE;
	case WM_DESTROY:
		return TRUE;
	case WM_COMMAND:
		for (i = 0; i < asize(CheckBox); ++i) {
			if (LOWORD(wParam) == CheckBox[i]) {
				unsigned int id;
				id = ref->GetExcludeAreas();
				if (IsDlgButtonChecked (hWnd, LOWORD(wParam))) {
					id |= (1<<i);
				} else {
					id &= ~(1<<i);
				}
				ref->SetExcludeAreas(id);
			}
		}
		break;
	}
	return FALSE;
}

/*
 * Red Dog helper class description
 */
class VISBoxDesc : public ClassDesc
{
public:
	int 			IsPublic() {return 1;}
	void *			Create (BOOL loading = FALSE) {return new VisBox();}
	const TCHAR *	ClassName() {return GetString(IDS_VIS_CLASS_NAME);}
	SClass_ID		SuperClassID() {return HELPER_CLASS_ID;}
	Class_ID		ClassID() {return VISBOX_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_HELPER_CATEGORY);}
	void			ResetClassParams (BOOL fileReset);
};

static VISBoxDesc VisPointDesc;
ClassDesc* GetVISDesc() {return &VisPointDesc;}

//TODO: Should implement this method to reset the plugin params when Max is reset
void VISBoxDesc::ResetClassParams (BOOL fileReset) 
{

}

void VisBox::DrawMarker (GraphicsWindow *gw, bool Selected, bool Frozen)
{
	int id;
	static Point3 left[] = 
	{
		Point3( 0,  0,  0),
		Point3( 0,  0,  1),
		Point3( 0,  1,  1),
		Point3( 0,  1,  0),
		Point3( 0,  0,  0)
	};
	static Point3 right[] = 
	{
		Point3(1,  0,  0 ),
		Point3(1,  0,  1 ),
		Point3(1,  1,  1 ),
		Point3(1,  1,  0 ),
		Point3(0,  0,  0 )
	};
	static Point3 lines[4][3] = 
	{
		{
			Point3( 0,  0,  1 ),
			Point3( 1,  0,  1 ),
			Point3( 0,  0,  0 ),
		},
		{
			Point3( 0,  1,  1 ),
			Point3( 1,  1,  1 ),
			Point3( 0,  0,  0 ),
		},		{
			Point3( 0,  0,  0 ),
			Point3( 1,  0,  0 ),
			Point3( 0,  0,  0 ),
		},
		{
			Point3( 0,  1,  0 ),
			Point3( 1,  1,  0 ),
			Point3( 0,  0,  0 ),
		}
	};

	pblock->GetValue (PB_BOX_ID, TimeValue(0), id, FOREVER);
	if (!Frozen) {
		if (!Selected) {
			gw->setColor(LINE_COLOR, 0.25f * (id & 3), 0.25f * ((id>>2) & 3), 0.25f * ((id>>4) & 3));
		} else {
			gw->setColor(LINE_COLOR, 1, 1, 1);
		}
	}
	
	gw->polyline(4, left,		NULL, NULL, true, NULL);
	gw->polyline(4, right,		NULL, NULL, true, NULL);

	for (int i = 0; i < 4; ++i) 
		gw->polyline (2, lines[i], NULL, NULL, false, NULL);
}

/*
 * Get a red dog box
 */
RedDog::VisBox VisBox::GetBox(Matrix3 &objToWorld)
{
	RedDog::VisBox retVal;
	Matrix3 matrix = Inverse(objToWorld);

	// Copy in the matrix
	retVal.worldToBoxSpace.m[0][0]	= matrix.GetRow(0).x;
	retVal.worldToBoxSpace.m[0][1]	= matrix.GetRow(0).y;
	retVal.worldToBoxSpace.m[0][2]	= matrix.GetRow(0).z;
	retVal.worldToBoxSpace.m[0][3]	= 0.f;
	retVal.worldToBoxSpace.m[1][0]	= matrix.GetRow(1).x;
	retVal.worldToBoxSpace.m[1][1]	= matrix.GetRow(1).y;
	retVal.worldToBoxSpace.m[1][2]	= matrix.GetRow(1).z;
	retVal.worldToBoxSpace.m[1][3]	= 0.f;
	retVal.worldToBoxSpace.m[2][0]	= matrix.GetRow(2).x;
	retVal.worldToBoxSpace.m[2][1]	= matrix.GetRow(2).y;
	retVal.worldToBoxSpace.m[2][2]	= matrix.GetRow(2).z;
	retVal.worldToBoxSpace.m[2][3]	= 0.f;
	retVal.worldToBoxSpace.m[3][0]	= matrix.GetRow(3).x;
	retVal.worldToBoxSpace.m[3][1]	= matrix.GetRow(3).y;
	retVal.worldToBoxSpace.m[3][2]	= matrix.GetRow(3).z;
	retVal.worldToBoxSpace.m[3][3]	= 1.f;

	int id, excludes;
	pblock->GetValue (PB_BOX_ID, TimeValue(0), id, FOREVER);
	pblock->GetValue (PB_EXCLUDE_AREAS, TimeValue(0), excludes, FOREVER);
	retVal.ID = id; retVal.canAlsoSee = excludes;

	return retVal;
}

int VisBox::GetExcludeAreas()
{
	int id;
	pblock->GetValue (PB_EXCLUDE_AREAS, TimeValue(0), id, FOREVER);
	return id;
}

void VisBox::SetExcludeAreas(int areas)
{
	pblock->SetValue (PB_EXCLUDE_AREAS, TimeValue(0), areas);
}