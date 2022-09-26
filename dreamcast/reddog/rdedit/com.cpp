#include "StdAfx.h"
#include "CoM.h"

IParamMap	*COMpoint::pmapParam;						// the default parameter map
IObjParam	*COMpoint::objParams;
bool		COMpoint::cRotates = TRUE;

COMpoint::COMpoint (void)
{
	MakeRefByID(FOREVER, 0, CreateParameterBlock(CPdescVer1, 0, 1));
	assert(pblock);

//	pblock->SetValue (PB_IGN_PARENT, TimeValue(0), cRotates);
}
void COMpoint::WindowFinal (IObjParam *ip)
{
//	pblock->GetValue (PB_IGN_PARENT,	ip->GetTime(), *((int *)&cRotates), FOREVER);
}
COMParamDlg::COMParamDlg (COMPOINT *r) { ref = (COMpoint *)r; }
int COMCreatorClass::proc (ViewExp *vpt, int msg, int point, 
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


BOOL COMParamDlg::DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		return FALSE;
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
class COMpointDesc : public ClassDesc
{
public:
	int 			IsPublic() {return 1;}
	void *			Create (BOOL loading = FALSE) {return new COMpoint();}
	const TCHAR *	ClassName() {return GetString(IDS_COM_CLASS_NAME);}
	SClass_ID		SuperClassID() {return HELPER_CLASS_ID;}
	Class_ID		ClassID() {return COMPOINT_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_HELPER_CATEGORY);}
	void			ResetClassParams (BOOL fileReset);
};

static COMpointDesc ComPointDesc;
ClassDesc* GetCOMDesc() {return &ComPointDesc;}

//TODO: Should implement this method to reset the plugin params when Max is reset
void COMpointDesc::ResetClassParams (BOOL fileReset) 
{

}

