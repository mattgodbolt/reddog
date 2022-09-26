/*
 * animcontrol.c
 */
#include "stdafx.h"

ANIM	anims[MAX_ANIMS];

ParamUIDesc ACdescParam[] = {
  	ParamUIDesc (PB_ID,				EDITTYPE_INT,		IDC_AGREEN, IDC_AGREENSPINNER, 0, 256, 0),
};

ParamBlockDescID ACdescVer1[] = {
	{ TYPE_BOOL,	NULL, FALSE, PB_CALC_MATRIX },
	{ TYPE_INT,		NULL, FALSE, PB_ID },
};
#define VER1_SIZE 2

ParamBlockDescID ACdescVer2[] = {
	{ TYPE_BOOL,	NULL, FALSE, PB_CALC_MATRIX },
	{ TYPE_INT,		NULL, FALSE, PB_ID },
	{ TYPE_BOOL,	NULL, FALSE, PB_TARGETABLE },
};
#define VER2_SIZE 3

ParamBlockDescID ACdescVer3[] = {
	{ TYPE_BOOL,	NULL, FALSE, PB_CALC_MATRIX },
	{ TYPE_INT,		NULL, FALSE, PB_ID },
  	{ TYPE_BOOL,	NULL, FALSE, PB_TARGETABLE },
   	{ TYPE_BOOL,	NULL, FALSE, PB_HIGH_D_COLL },
};

int interpo=0;
int createid=0;

IParamMap	*animcontrol::pmapParam;						// the default parameter map
IObjParam	*animcontrol::objParams;
//float		camcontrol::pitchang = 0.1f;
//char		camcontrol::name[MAX_FNAME_SIZE] = "CHOOSE DOME";

ICustEdit *animcontrolParamDlg::nameCtrl;
ISpinnerControl *animcontrolParamDlg::frameSpinner; 
ISpinnerControl	*animcontrolParamDlg::endframeSpinner; 
HWND		animcontrolParamDlg::instHwnd;

animcontrolParamDlg *animcontrolParamDlg::dlgInstance = NULL;


void ClearAnims()  
{
	//RESET THE PARAMETERS FOR THE ANIMATION TABLE
	for (int i=0;i<MAX_ANIMS;i++)
	{
		strcpy(anims[i].name,"DEFAULT");
		anims[i].keyframe = 0;
		anims[i].endframe = 0;
	}
}	

animcontrol::animcontrol (void)
{
   	MakeRefByID(FOREVER, 0, CreateParameterBlock(ACdescVer3, AC_PARAMDESC_LENGTH, AC_CURRENT_VERSION));
   	assert(pblock);
	ObjLoad (NULL, objMesh, 0);

	currentanim = 0;
//	interpo = 0;	
	objMesh.InvalidateTopologyCache();
	objMesh.InvalidateGeomCache();
	objMesh.BuildStripsAndEdges();

	//FORCE THE REDRAW
	GetCOREInterface()->ForceCompleteRedraw(TRUE);

}

void animcontrol::WindowFinal (IObjParam *ip)
{
}

void animcontrol::DrawMarker (GraphicsWindow *gw, bool Selected, bool Frozen)
{
	Point3 pt(0,0,0);
	if(!Frozen)
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_POINT_OBJ));

  	gw->marker(&pt,SM_HOLLOW_BOX_MRKR);
}


void animcontrol::Reset()
{
}


/*void animcontrolParamDlg::Update (void)
{
} */

void animcontrolParamDlg::Update (TimeValue t) // max called
{
   //	anims[ref->currentanim].keyframe = frameSpinner->GetIVal();
   //	anims[ref->currentanim].endframe = endframeSpinner->GetIVal();
}

void animcontrolParamDlg::VarUpdate()
{
   //	anims[ref->currentanim].keyframe = frameSpinner->GetIVal();
   //	anims[ref->currentanim].endframe = endframeSpinner->GetIVal();

}
void	 animcontrolParamDlg::UpdateAnimInfo(HWND hWnd)
{  char num[4];

itoa(ref->currentanim,num,10);
				  SetDlgItemText (hWnd, IDC_ANIMNUM, num);
				  frameSpinner->SetValue(anims[ref->currentanim].keyframe,TRUE);
				  endframeSpinner->SetValue(anims[ref->currentanim].endframe,TRUE);
				  nameCtrl->SetText (anims[ref->currentanim].name);

}

BOOL animcontrolParamDlg::DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
   	TimeValue start = GetCOREInterface()->GetAnimRange().Start();
   	TimeValue end = GetCOREInterface()->GetAnimRange().End();
	int delta = GetTicksPerFrame();

	int numframes = (int)((end - start) / delta);
   	numframes = end/delta;
	start = start/delta;

	switch (msg) {
	case WM_INITDIALOG:
		instHwnd = hWnd;

	  	nameCtrl = GetICustEdit (GetDlgItem (hWnd, IDC_ANIMNAME));
	  	assert (nameCtrl);
		nameCtrl->SetText (anims[ref->currentanim].name);



	 	frameSpinner = SetupIntSpinner 
			(hWnd, IDC_AKEYSPINNER, IDC_AKEY, start,numframes, anims[ref->currentanim].keyframe);
	  	assert (frameSpinner);

	 	endframeSpinner = SetupIntSpinner 
			(hWnd, IDC_AENDKEYSPINNER, IDC_AENDKEY, start,numframes, anims[ref->currentanim].endframe);
	  	assert (endframeSpinner);

		CheckDlgButton(hWnd, IDC_INTERPOLATE, interpo);
		CheckDlgButton(hWnd, IDC_OBJIDCREATE, createid);


		return FALSE;
	case WM_DESTROY:
		anims[ref->currentanim].keyframe = frameSpinner->GetIVal();
	 	anims[ref->currentanim].endframe = endframeSpinner->GetIVal();

		//VarUpdate();
		//release name controller ? check out
		ReleaseISpinner (frameSpinner);
		ReleaseISpinner (endframeSpinner);
		frameSpinner = NULL; 
		endframeSpinner = NULL; 
		return TRUE;
	case WM_COMMAND:
		   //	char num[4];
			switch (LOWORD(wParam))
			{
				case (IDC_INTERPOLATE):
					if (interpo)
						interpo = 0;
					else
						interpo = 1;
				 //	interpo = !interpo;
					#if MAX3
						SetSaveRequiredFlag(TRUE);
					#else
						SetSaveRequired(1);
					#endif
				  break;
			   
				case (IDC_OBJIDCREATE):
				 	if (createid)
						createid = 0;
					else
						createid = 1;
				//	createid = !createid;
					#if MAX3
						SetSaveRequiredFlag(TRUE);
					#else
						SetSaveRequired(1);
					#endif
				  break;
			
			
			
				case (IDC_NEXTANIM):
				  ref->currentanim++;
				  if (ref->currentanim == MAX_ANIMS)
				  	 ref->currentanim = 0;
				
				  UpdateAnimInfo(hWnd);
			  //	  itoa(ref->currentanim,num,10);
			  //	  SetDlgItemText (hWnd, IDC_ANIMNUM, num);
			  //	  frameSpinner->SetValue(anims[ref->currentanim].keyframe,TRUE);
			  //	  endframeSpinner->SetValue(anims[ref->currentanim].endframe,TRUE);
			  //	  nameCtrl->SetText (anims[ref->currentanim].name);
				  
				  
				  break;

				case (IDC_PREVANIM):
				  ref->currentanim--;
				  if (ref->currentanim < 0)
				  	 ref->currentanim = MAX_ANIMS-1;
				
				  UpdateAnimInfo(hWnd);

				   break;

				case IDC_ANIMNAME:
					//ENSURE THE SAVE BIT IS SET IN THE MAX FILE
					#if MAX3
						SetSaveRequiredFlag(TRUE);
					#else
						SetSaveRequired(1);
					#endif
					nameCtrl->GetText(anims[ref->currentanim].name,32);
					break;
				case IDC_RESETANIMS:
					#if MAX3
						SetSaveRequiredFlag(TRUE);
					#else
						SetSaveRequired(1);
					#endif
	  				frameSpinner->SetValue(anims[ref->currentanim].keyframe,TRUE);
	  				endframeSpinner->SetValue(anims[ref->currentanim].endframe,TRUE);
					nameCtrl->GetText(anims[ref->currentanim].name,32);
					ClearAnims();
					ReleaseISpinner (frameSpinner);
					ReleaseISpinner (endframeSpinner);
					frameSpinner = NULL; 
					endframeSpinner = NULL; 
				 	frameSpinner = SetupIntSpinner 
						(hWnd, IDC_AKEYSPINNER, IDC_AKEY, start,numframes, anims[ref->currentanim].keyframe);
				  	assert (frameSpinner);

				 	endframeSpinner = SetupIntSpinner 
						(hWnd, IDC_AENDKEYSPINNER, IDC_AENDKEY, start,numframes, anims[ref->currentanim].endframe);
				 	assert (endframeSpinner);
					ref->currentanim = 0;
					  frameSpinner->SetValue(anims[ref->currentanim].keyframe,TRUE);
					  endframeSpinner->SetValue(anims[ref->currentanim].endframe,TRUE);
					  nameCtrl->SetText (anims[ref->currentanim].name);

					break;
					
			}
			//Update();
 			break;
	case CC_SPINNER_CHANGE:
	  	switch (LOWORD(wParam))
	  	{
			case (IDC_AKEYSPINNER):
				//ENSURE THE SAVE BIT IS SET IN THE MAX FILE
				 #if MAX3
					SetSaveRequiredFlag(TRUE);
				#else
					SetSaveRequired(1);
				#endif
				anims[ref->currentanim].keyframe = frameSpinner->GetIVal();
				break;
			case (IDC_AENDKEYSPINNER):
				#if MAX3
					SetSaveRequiredFlag(TRUE);
				#else
					SetSaveRequired(1);
				#endif
				anims[ref->currentanim].endframe = endframeSpinner->GetIVal();
				break;
			default:
				break;

		
		}	
	}
	return TRUE;
}

/*
 * Red Dog helper class description
 */
class animcontrolsDesc : public ClassDesc
{
public:
	int 			IsPublic() {return 1;}
	void *			Create (BOOL loading = FALSE) {return new animcontrol();}
	const TCHAR *	ClassName() {return GetString(IDS_ANIMCONTROL);}
	SClass_ID		SuperClassID() {return HELPER_CLASS_ID;}
	Class_ID		ClassID() {return animcontrol_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_HELPER_CATEGORY);}
	void			ResetClassParams (BOOL fileReset);
	BOOL			NeedsToSave () { return TRUE; }
	IOResult		Save (ISave *save);
	IOResult		Load (ILoad *load);
};


static animcontrolsDesc animcontrolDesc;
ClassDesc* GetanimcontrolDesc() {return &animcontrolDesc;}

void animcontrolsDesc::ResetClassParams (BOOL fileReset) 
{

//	nextcam = &camarray[0];
//	nextcam->next = NULL;
}

#define CHUNK_ANIM	1000
#define CHUNK_SPEED		1010
#define CHUNK_INTERPOLATE		1020
#define CHUNK_OBJIDCREATE		1030



IOResult animcontrolsDesc::Save (ISave *save)
{
	ULONG nb;
	
	save->BeginChunk (CHUNK_ANIM);
	for (int i=0;i<MAX_ANIMS;i++)
	{
		save->Write (&anims[i].keyframe, sizeof (int), &nb);
		save->Write (&anims[i].name, 32, &nb);
		save->Write (&anims[i].endframe, sizeof (int), &nb);
	}
	save->EndChunk();

	save->BeginChunk (CHUNK_INTERPOLATE);
	if (interpo == 3)
		interpo = 1;
   	save->Write (&interpo, sizeof(int), &nb);
	save->EndChunk();
	save->BeginChunk (CHUNK_OBJIDCREATE);
   	save->Write (&createid, sizeof(int), &nb);
	save->EndChunk();
	return IO_OK;
}

IOResult animcontrolsDesc::Load (ILoad *load)
{
	ULONG nb;
	IOResult res;
	int i;
	//	ResetClassParams(TRUE);
   //	return(IO_OK);

	// Array of old versions
	static ParamVersionDesc versions[] = {
		ParamVersionDesc(ACdescVer1,VER1_SIZE,1),
		ParamVersionDesc(ACdescVer2,VER2_SIZE,2)
	};

	while ((res = load->OpenChunk()) == IO_OK) {
		switch (load->CurChunkID())
		{
		   	case CHUNK_ANIM:
				for (i=0;i<MAX_ANIMS;i++)
				{
					res = load->Read (&anims[i].keyframe, sizeof (int), &nb);
					if (res == IO_OK)
						res = load->Read (&anims[i].name, 32, &nb);
					if (res == IO_OK)
					   	res = load->Read (&anims[i].endframe, sizeof (int), &nb);
				}
		   //		res = load->Read (&amount, sizeof (float), &nb);
		   		break;
		   	case CHUNK_INTERPOLATE:
		   		res = load->Read (&interpo, sizeof (int), &nb);
		   		break;
		   	case CHUNK_OBJIDCREATE:
		   		res = load->Read (&createid, sizeof (int), &nb);
		   		break;

		}
		load->CloseChunk();
		if (res != IO_OK)
			return res;
	}
	
	return IO_OK;
}


int animcontrolCreatorClass::proc (ViewExp *vpt, int msg, int point, 
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

