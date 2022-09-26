/*
 * levelflags.c
 */

#include "stdafx.h"
#include "LevelFlags.h"

ParamUIDesc LFdescParam[] = {
  	ParamUIDesc (PB_ID,				EDITTYPE_INT,		IDC_AGREEN, IDC_AGREENSPINNER, 0, 256, 0),
//  	ParamUIDesc (PB_CALC_MATRIX,	TYPE_SINGLECHEKBOX,	IDC_CALC_MATRIX),
  //	ParamUIDesc (PB_ID,				EDITTYPE_INT,		IDC_ID, IDC_IDSPINNER, 0, MAX_SUB_OBJECTS - 1, 0.1f),
  //	ParamUIDesc (PB_TARGETABLE,		TYPE_SINGLECHEKBOX,	IDC_TARGETABLE),
  //	ParamUIDesc (PB_HIGH_D_COLL,	TYPE_SINGLECHEKBOX,	IDC_HIGH_D_COLL),
};

ParamBlockDescID LFdescVer1[] = {
	{ TYPE_BOOL,	NULL, FALSE, PB_CALC_MATRIX },
	{ TYPE_INT,		NULL, FALSE, PB_ID },
};
#define VER1_SIZE 2

ParamBlockDescID LFdescVer2[] = {
	{ TYPE_BOOL,	NULL, FALSE, PB_CALC_MATRIX },
	{ TYPE_INT,		NULL, FALSE, PB_ID },
	{ TYPE_BOOL,	NULL, FALSE, PB_TARGETABLE },
};
#define VER2_SIZE 3

ParamBlockDescID LFdescVer3[] = {
	{ TYPE_BOOL,	NULL, FALSE, PB_CALC_MATRIX },
	{ TYPE_INT,		NULL, FALSE, PB_ID },
  	{ TYPE_BOOL,	NULL, FALSE, PB_TARGETABLE },
   	{ TYPE_BOOL,	NULL, FALSE, PB_HIGH_D_COLL },
};


IParamMap	*levelflag::pmapParam;						// the default parameter map
IObjParam	*levelflag::objParams;
float		levelflag::fov = 0.7f;
float		levelflag::farz = 500.0f;
float		levelflag::cameraHeight = 4.0f;
float		levelflag::cameraDist = -11.0f;
float		levelflag::cameralookHeight = 0.0f;
float		levelflag::fognear = 100.0f, levelflag::fogfar=1000.0f;
float		levelflag::stratintensity = 1.0f, levelflag::scapeintensity = 1.0f;
int			levelflag::ambientgreen = 255,levelflag::ambientblue = 255, levelflag::ambientred = 255;
int			levelflag::foggreen = 255,levelflag::fogblue = 255, levelflag::fogred = 255;
char		levelflag::name[MAX_FNAME_SIZE] = "CHOOSE DOME";

ICustEdit *levelflagsParamDlg::nameCtrl;
ISpinnerControl  
*levelflagsParamDlg::fovSpinner, 
*levelflagsParamDlg::farzSpinner, 
*levelflagsParamDlg::cameraheightSpinner, 
*levelflagsParamDlg::cameradistSpinner, 
*levelflagsParamDlg::cameralookheightSpinner, 
			*levelflagsParamDlg::scapeintensitySpinner, 
			*levelflagsParamDlg::stratintensitySpinner, 
			*levelflagsParamDlg::fognearSpinner, 
			*levelflagsParamDlg::fogfarSpinner, 
			*levelflagsParamDlg::aredSpinner, 
			*levelflagsParamDlg::agreenSpinner, *levelflagsParamDlg::ablueSpinner,
			*levelflagsParamDlg::fredSpinner, 
			*levelflagsParamDlg::fgreenSpinner, *levelflagsParamDlg::fblueSpinner;
HWND		levelflagsParamDlg::instHwnd;

levelflagsParamDlg *levelflagsParamDlg::dlgInstance = NULL;

levelflag::levelflag (void)
{
   	MakeRefByID(FOREVER, 0, CreateParameterBlock(LFdescVer3, LF_PARAMDESC_LENGTH, LF_CURRENT_VERSION));
   	assert(pblock);

//	pblock->SetValue (PB_CALC_MATRIX,	TimeValue(0), cCalcMatrix);
//	pblock->SetValue (PB_ID,			TimeValue(0), 0);
//	pblock->SetValue (PB_TARGETABLE,	TimeValue(0), 0);
//	pblock->SetValue (PB_HIGH_D_COLL,	TimeValue(0), 0);
}

void levelflag::WindowFinal (IObjParam *ip)
{
  //	pblock->GetValue (PB_CALC_MATRIX,	ip->GetTime(), *((int *)&cCalcMatrix), FOREVER);
}

void levelflagsParamDlg::Update (void)
{

	
	
	ref->ambientgreen = agreenSpinner->GetIVal();
	ref->ambientred = aredSpinner->GetIVal();
	ref->ambientblue = ablueSpinner->GetIVal();
	ref->foggreen = fgreenSpinner->GetIVal();
	ref->fogred = fredSpinner->GetIVal();
	ref->fogblue = fblueSpinner->GetIVal();
	ref->fognear = fognearSpinner->GetFVal();
	ref->fogfar = fogfarSpinner->GetFVal();
	ref->stratintensity = stratintensitySpinner->GetFVal();
	ref->scapeintensity = scapeintensitySpinner->GetFVal();
	ref->fov = fovSpinner->GetFVal();
	ref->farz = farzSpinner->GetFVal();
	ref->cameraHeight = cameraheightSpinner->GetFVal();
	ref->cameraDist = cameradistSpinner->GetFVal();
	ref->cameralookHeight = cameralookheightSpinner->GetFVal();
	#if MAX3
		SetSaveRequiredFlag(TRUE);
	#else
		SetSaveRequired(1);
	#endif
  }

void levelflagsParamDlg::Update (TimeValue t) // max called
{
/*	if (ref) {
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
	}*/ 
}

static char *selectedModelName = NULL;			// a pointer to the currently selected model


static int fillTree (char *parentDir, char *dir, char *current, HWND listBox, HTREEITEM after)
{
	TV_INSERTSTRUCT insert;
	HANDLE hSearch;
	WIN32_FIND_DATA findData;
	char buf[MAX_PATH];

	// First find all the directories
	if (parentDir)
		sprintf (buf, "%s\\%s\\*.*", parentDir, dir);
	else
		sprintf (buf, "%s\\*.*", dir);

	if ((hSearch = FindFirstFile (buf, &findData)) == INVALID_HANDLE_VALUE)
		return 0;
	while (1) {
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY
			&& strcmp (findData.cFileName, ".")
			&& strcmp (findData.cFileName, "..")) {
			HTREEITEM tree;
			insert.hParent = after;
			insert.hInsertAfter = TVI_SORT;
			insert.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
			insert.item.pszText = findData.cFileName;
			insert.item.lParam = (LPARAM) after;
			insert.item.iImage = 2;
			insert.item.iSelectedImage = 1;
			tree = TreeView_InsertItem (listBox, &insert);
			// Now fill in the children
			if (parentDir)
				sprintf (buf, "%s\\%s", parentDir, dir);
			else
				strcpy (buf, dir);
			fillTree (buf, findData.cFileName, current, listBox, tree);
		}
		if (!FindNextFile(hSearch, &findData))
			break;
	}
	FindClose (hSearch);

	// Now add all the .DST files

	if (parentDir)
		sprintf (buf, "%s\\%s\\*.RDO", parentDir, dir);
	else
		sprintf (buf, "%s\\*.RDO", dir);

	if ((hSearch = FindFirstFile (buf, &findData)) == INVALID_HANDLE_VALUE)
		return 0;
	while (1) {
		char buf2[MAX_PATH];
		HTREEITEM item;
		strcpy (buf2, findData.cFileName);
		buf2[strlen(buf2)-4] = '\0';

		insert.hParent = after;
		insert.hInsertAfter = TVI_SORT;
		insert.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		insert.item.pszText = buf2;
		insert.item.lParam = (LPARAM) after;
		insert.item.iImage = 4;
		insert.item.iSelectedImage = 3;
		item = TreeView_InsertItem (listBox, &insert);

		/*
		 * Check to see if this is the currently selected badger
		 */
		if (parentDir)
			sprintf (buf2, "%s\\%s\\%s", parentDir, dir, findData.cFileName);
		else
			sprintf (buf2, "%s\\%s", dir, findData.cFileName);

		if (!stricmp (&buf2[strlen (DOME_OBJECT_ROOT) + 1], current))
			TreeView_SelectItem (listBox, item);
		if (!FindNextFile(hSearch, &findData))
			break;
	}
	FindClose (hSearch);

	return 1;
}

// Recursively gets the filename of a listbox item
static void GetItemName (HWND treeItem, char *buffer, HTREEITEM tree)
{
	TV_ITEM newInfo;
	char textBuf[256];
	static int itemCount = 0;

	// Append the root name
	if (itemCount==0)
		strcpy (buffer, DOME_OBJECT_ROOT);
	itemCount++;

	// Get the information from this tree node
	memset (&newInfo, 0, sizeof (newInfo));
	newInfo.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM;
	newInfo.hItem = tree;
	newInfo.pszText = &textBuf[0];
	newInfo.cchTextMax = sizeof (textBuf);
	TreeView_GetItem (treeItem, &newInfo);
	
	// Does it have a parent?
	if (newInfo.lParam)
		GetItemName (treeItem, buffer, (HTREEITEM) newInfo.lParam);

	// Place the text into the name
	strcat (buffer, "\\");
	strcat (buffer, textBuf);

	// Reduce the recursion depth
	--itemCount;
}


static BOOL CALLBACK dialogFunc (HWND dBox, UINT msg, WPARAM wp, LPARAM lp)
{
	static HWND treeItem;
	static HIMAGELIST imageList;
	switch (msg) {
	case WM_INITDIALOG:
		
		
		treeItem = GetDlgItem (dBox, IDC_OBJECT_TREE);
		fillTree (NULL, DOME_OBJECT_ROOT, (char *)lp, treeItem, NULL);
		imageList = ImageList_LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_IMAGELIST),
			16, 0, RGB (0, 0, 255));
		TreeView_SetImageList (treeItem, imageList, TVSIL_NORMAL);
		EnableWindow (GetDlgItem (dBox, IDOK), FALSE);

		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wp)) {
		case IDOK:
			EndDialog (dBox, TRUE);
			return TRUE;
		case IDCANCEL:
			EndDialog (dBox, FALSE);
			return TRUE;
		}
	case WM_NOTIFY:
		LPNMHDR pnmh = (LPNMHDR) lp; 
		switch (pnmh->code) {
		case NM_DBLCLK:
		case TVN_SELCHANGED:
			char textBuf[MAX_PATH];
			if (pnmh->code == TVN_SELCHANGED) {
				// Get a pointer to the tree event block
				NM_TREEVIEW FAR *pnmtv = (NM_TREEVIEW FAR *) pnmh;
				// Check for a valid selection
				GetItemName (treeItem, &textBuf[0], pnmtv->itemNew.hItem);
			} else {
				HTREEITEM tree;
				tree = TreeView_GetSelection (treeItem);
				GetItemName (treeItem, &textBuf[0], tree);
			}
				strcat (textBuf, ".RDO");
			if (FILE *f = fopen (textBuf, "rb")) 
			{
				// Note the selected model name
				if (selectedModelName)
					free (selectedModelName);
				selectedModelName = strdup (textBuf);
				EnableWindow (GetDlgItem (dBox, IDOK), TRUE);
				fclose (f);
				// If double-clicked, then EndDialog now
				if (pnmh->code == NM_DBLCLK) {
					EndDialog (dBox, TRUE);
			}

			} else if (pnmh->code != NM_DBLCLK)	// the SelChanged message will set this part
				EnableWindow (GetDlgItem (dBox, IDOK), FALSE);
			return TRUE;
		}
	}
	return FALSE;
}


BOOL levelflagsParamDlg::DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
	  //	nameCtrl = GetICustEdit (GetDlgItem (hWnd, IDC_OBJECTNAME));
	  //	assert (nameCtrl);

		instHwnd = hWnd;
		SetDlgItemText (hWnd, IDC_OBJECTNAME, ref->name);
	   //	radiusSpinner = SetupFloatSpinner (hWnd, IDC_RADIUSSPINNER, IDC_RADIUS, 0.f, 10000.f, ref->radius);
	   //	assert (radiusSpinner);
	 	agreenSpinner = SetupIntSpinner (hWnd, IDC_AGREENSPINNER, IDC_AGREEN, 0, 255, ref->ambientgreen);
	  	assert (agreenSpinner);

	 	aredSpinner = SetupIntSpinner (hWnd, IDC_AREDSPINNER, IDC_ARED, 0, 255, ref->ambientred);
	  	assert (aredSpinner);

	 	ablueSpinner = SetupIntSpinner (hWnd, IDC_ABLUESPINNER, IDC_ABLUE, 0, 255, ref->ambientblue);
	  	assert (ablueSpinner);

	 	fgreenSpinner = SetupIntSpinner (hWnd, IDC_FGREENSPINNER, IDC_FGREEN, 0, 255, ref->foggreen);
	  	assert (fgreenSpinner);

	 	fredSpinner = SetupIntSpinner (hWnd, IDC_FREDSPINNER, IDC_FRED, 0, 255, ref->fogred);
	  	assert (fredSpinner);

	 	fblueSpinner = SetupIntSpinner (hWnd, IDC_FBLUESPINNER, IDC_FBLUE, 0, 255, ref->fogblue);
	  	assert (fblueSpinner);

	 	fognearSpinner = SetupFloatSpinner (hWnd, IDC_FOGNEARSPINNER, IDC_FOGNEAR, 0, 3000.0f, ref->fognear);
	  	assert (fognearSpinner);

	 	fogfarSpinner = SetupFloatSpinner (hWnd, IDC_FOGFARSPINNER, IDC_FOGFAR, 0, 3000.0f, ref->fogfar);
	  	assert (fogfarSpinner);

	 	scapeintensitySpinner = SetupFloatSpinner (hWnd, IDC_SCAPEINTENSITYSPINNER, IDC_SCAPEINTENSITY, 0, 1.0f, ref->scapeintensity);
	  	assert (scapeintensitySpinner);

	 	stratintensitySpinner = SetupFloatSpinner (hWnd, IDC_STRATINTENSITYSPINNER, IDC_STRATINTENSITY, 0, 1.0f, ref->stratintensity);
	  	assert (stratintensitySpinner);

	 	fovSpinner = SetupFloatSpinner (hWnd, IDC_FOVSPINNER, IDC_FOV, 0, 3000.0f, ref->fov);
	  	assert (fovSpinner);

		farzSpinner = SetupFloatSpinner (hWnd, IDC_FARZSPINNER, IDC_FARZ, 0, 3000.0f, ref->farz);
	  	assert (farzSpinner);

		cameraheightSpinner = SetupFloatSpinner (hWnd, IDC_HEIGHTSPINNER, IDC_HEIGHT, -300.0f, 3000.0f, ref->cameraHeight);
	  	assert (cameraheightSpinner);

		cameradistSpinner = SetupFloatSpinner (hWnd, IDC_DISTSPINNER, IDC_DIST, -300.0f, 3000.0f, ref->cameraDist);
	  	assert (cameradistSpinner);

		cameralookheightSpinner = SetupFloatSpinner (hWnd, IDC_LOOKHEIGHTSPINNER, IDC_LOOKHEIGHT, -300.0f, 3000.0f, ref->cameralookHeight);
	  	assert (cameralookheightSpinner);

		//		nameCtrl->SetText (ref->name);
		return FALSE;
	case WM_DESTROY:
		Update();
		ReleaseISpinner (agreenSpinner);
		ReleaseISpinner (aredSpinner);
		ReleaseISpinner (ablueSpinner);
		ReleaseISpinner (fgreenSpinner);
		ReleaseISpinner (fredSpinner);
		ReleaseISpinner (fblueSpinner);
		ReleaseISpinner (fognearSpinner);
		ReleaseISpinner (fogfarSpinner);

		ReleaseISpinner (scapeintensitySpinner);
		ReleaseISpinner (stratintensitySpinner);

		ReleaseISpinner (fovSpinner);
		ReleaseISpinner (farzSpinner);

		ReleaseISpinner (cameraheightSpinner);
		ReleaseISpinner (cameradistSpinner);
		ReleaseISpinner (cameralookheightSpinner);

		/*ReleaseICustEdit (nameCtrl);

		ReleaseISpinner (expSpinner);
		ReleaseISpinner (moiSpinner);
		ReleaseISpinner (frictionXSpinner);
		ReleaseISpinner (frictionYSpinner);
		ReleaseISpinner (frictionZSpinner);
		ReleaseISpinner (airFrictionXSpinner);
		ReleaseISpinner (airFrictionYSpinner);
		ReleaseISpinner (airFrictionZSpinner);
		*/
		nameCtrl = NULL;
		fovSpinner = farzSpinner = cameraheightSpinner = cameradistSpinner = cameralookheightSpinner =
		fredSpinner = fblueSpinner = fgreenSpinner =
	   scapeintensitySpinner = stratintensitySpinner = fognearSpinner = fogfarSpinner = agreenSpinner = aredSpinner = ablueSpinner =
			NULL;
		return TRUE;
	case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				int result;
				case IDC_OBJECTNAME:
					result = DialogBoxParam (hInstance, MAKEINTRESOURCE (IDD_CHOOSE_OBJECT),
					hWnd, dialogFunc, (LPARAM) ref->name);
					// Did the model name change?				
					if (result && selectedModelName) 
					{
							// Set the button name to be the model pathname minus the object root (plus the \)
							strcpy (ref->name, selectedModelName + strlen (DOME_OBJECT_ROOT) + 1);
							SetDlgItemText (hWnd, IDC_OBJECTNAME, ref->name);
					}
					break;
				case IDC_CLEARDOME:
					strcpy (ref->name,"CHOOSE DOME");
					SetDlgItemText (hWnd, IDC_OBJECTNAME, ref->name);
					break;
				default:		
				Update();
	 			break;
 		}
			break;
	case CC_SPINNER_CHANGE:
		Update();
		break;
	}
	return TRUE;
}

/*
 * Red Dog helper class description
 */
class levelflagsDesc : public ClassDesc
{
public:
	int 			IsPublic() {return 1;}
	void *			Create (BOOL loading = FALSE) {return new levelflag();}
	const TCHAR *	ClassName() {return GetString(IDS_LEVEL_FLAGS_CLASS_NAME);}
	SClass_ID		SuperClassID() {return HELPER_CLASS_ID;}
	Class_ID		ClassID() {return levelflags_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_HELPER_CATEGORY);}
	void			ResetClassParams (BOOL fileReset);
	BOOL			NeedsToSave () { return TRUE; }
	IOResult		Save (ISave *save);
	IOResult		Load (ILoad *load);
};


static levelflagsDesc levelflagDesc;
ClassDesc* GetLevelFlagDesc() {return &levelflagDesc;}

void levelflagsDesc::ResetClassParams (BOOL fileReset) 
{

	levelflag::ambientgreen = 255;
	levelflag::ambientblue = 255;
	levelflag::ambientred = 255;
	levelflag::stratintensity = 1.0f;
	levelflag::scapeintensity = 1.0f;
	levelflag::fognear = 100.0f;
	levelflag::fogfar = 1000.0f;
	levelflag::foggreen = 255;
	levelflag::fogblue = 255;
	levelflag::fogred = 255;
	levelflag::fov = 0.7f;
	levelflag::farz = 500.0f;
	levelflag::cameraHeight = 4.0f;
	levelflag::cameraDist = -11.0f; 
	levelflag::cameralookHeight = 0.0f;
	strcpy (levelflag::name,"CHOOSE DOME");
}

#define CHUNK_AGREEN		1000
#define CHUNK_ARED			1010
#define CHUNK_ABLUE			1020
#define CHUNK_SCAPE			1030
#define CHUNK_STRAT			1040

#define CHUNK_FGREEN		1050
#define CHUNK_FRED			1060
#define CHUNK_FBLUE			1070
#define CHUNK_FOGFAR		1080
#define CHUNK_FOGNEAR		1090


#define CHUNK_FOV			1100
#define CHUNK_FARZ			1110
#define CHUNK_HEIGHT		1120
#define CHUNK_DIST			1130
#define CHUNK_LOOKHEIGHT	1140
#define CHUNK_DOMENAME		1150



IOResult levelflagsDesc::Save (ISave *save)
{
	ULONG nb;
	save->BeginChunk (CHUNK_DOMENAME);
	save->Write (&levelflag::name, sizeof (levelflag::name), &nb);
	save->EndChunk();

	save->BeginChunk (CHUNK_AGREEN);
	save->Write (&levelflag::ambientgreen, sizeof (int), &nb);
	save->EndChunk();

	save->BeginChunk (CHUNK_ABLUE);
	save->Write (&levelflag::ambientblue, sizeof (int), &nb);
	save->EndChunk();

	save->BeginChunk (CHUNK_ARED);
	save->Write (&levelflag::ambientred, sizeof (int), &nb);
	save->EndChunk();

	save->BeginChunk (CHUNK_SCAPE);
	save->Write (&levelflag::scapeintensity, sizeof (float), &nb);
	save->EndChunk();


	save->BeginChunk (CHUNK_STRAT);
	save->Write (&levelflag::stratintensity, sizeof (float), &nb);
	save->EndChunk();



	save->BeginChunk (CHUNK_FGREEN);
	save->Write (&levelflag::foggreen, sizeof (int), &nb);
	save->EndChunk();

	save->BeginChunk (CHUNK_FBLUE);
	save->Write (&levelflag::fogblue, sizeof (int), &nb);
	save->EndChunk();

	save->BeginChunk (CHUNK_FRED);
	save->Write (&levelflag::fogred, sizeof (int), &nb);
	save->EndChunk();


	save->BeginChunk (CHUNK_FOGNEAR);
	save->Write (&levelflag::fognear, sizeof (float), &nb);
	save->EndChunk();


	save->BeginChunk (CHUNK_FOGFAR);
	save->Write (&levelflag::fogfar, sizeof (float), &nb);
	save->EndChunk();

	save->BeginChunk (CHUNK_FOV);
	save->Write (&levelflag::fov, sizeof (float), &nb);
	save->EndChunk();


	save->BeginChunk (CHUNK_FARZ);
	save->Write (&levelflag::farz, sizeof (float), &nb);
	save->EndChunk();


	save->BeginChunk (CHUNK_HEIGHT);
	save->Write (&levelflag::cameraHeight, sizeof (float), &nb);
	save->EndChunk();


	save->BeginChunk (CHUNK_DIST);
	save->Write (&levelflag::cameraDist, sizeof (float), &nb);
	save->EndChunk();


	save->BeginChunk (CHUNK_LOOKHEIGHT);
	save->Write (&levelflag::cameralookHeight, sizeof (float), &nb);
	save->EndChunk();






 //	save->EndChunk();
	return IO_OK;
}

IOResult levelflagsDesc::Load (ILoad *load)
{
	ULONG nb;
	IOResult res;
	ResetClassParams(TRUE);
   //	return(IO_OK);

	while ((res = load->OpenChunk()) == IO_OK) {
		switch (load->CurChunkID()) {
		case CHUNK_DOMENAME:
			res = load->Read (&levelflag::name, sizeof (levelflag::name), &nb);
		 			
			break;
		case CHUNK_ARED:
			res = load->Read (&levelflag::ambientred, sizeof (int), &nb);
			break;
		case CHUNK_AGREEN:
			res = load->Read (&levelflag::ambientgreen, sizeof (int), &nb);
			break;
		case CHUNK_ABLUE:
			res = load->Read (&levelflag::ambientblue, sizeof (int), &nb);
			break;
		case CHUNK_SCAPE:
			res = load->Read (&levelflag::scapeintensity, sizeof (float), &nb);
			break;
		case CHUNK_STRAT:
			res = load->Read (&levelflag::stratintensity, sizeof (float), &nb);
			break;
		case CHUNK_FRED:
			res = load->Read (&levelflag::fogred, sizeof (int), &nb);
			break;
		case CHUNK_FGREEN:
			res = load->Read (&levelflag::foggreen, sizeof (int), &nb);
			break;
		case CHUNK_FBLUE:
			res = load->Read (&levelflag::fogblue, sizeof (int), &nb);
			break;
	   	case CHUNK_FOGNEAR:
			res = load->Read (&levelflag::fognear, sizeof (float), &nb);
			break;
		case CHUNK_FOGFAR:
			res = load->Read (&levelflag::fogfar, sizeof (float), &nb);
			break;
	   	case CHUNK_FOV:
			res = load->Read (&levelflag::fov, sizeof (float), &nb);
			break;
		case CHUNK_FARZ:
			res = load->Read (&levelflag::farz, sizeof (float), &nb);
			break;
	   	case CHUNK_HEIGHT:
			res = load->Read (&levelflag::cameraHeight, sizeof (float), &nb);
			break;
		case CHUNK_DIST:
			res = load->Read (&levelflag::cameraDist, sizeof (float), &nb);
			break;
		case CHUNK_LOOKHEIGHT:
			res = load->Read (&levelflag::cameralookHeight, sizeof (float), &nb);
			break;
		}
		load->CloseChunk();
		if (res != IO_OK)
			return res;
	}
	// Update the display
	if (levelflagsParamDlg::dlgInstance)
		levelflagsParamDlg::dlgInstance->Update (TimeValue(0));
	return IO_OK;
}

IOResult levelflag::Load (ILoad *load)
{
	// Array of old versions
	static ParamVersionDesc versions[] = {
		ParamVersionDesc(LFdescVer1,VER1_SIZE,1),
		ParamVersionDesc(LFdescVer2,VER2_SIZE,2)
	};
#define NUM_OLDVERSIONS	2
	// Current version
   //	static ParamVersionDesc curVersion(LFdescVer3,LF_PARAMDESC_LENGTH,LF_CURRENT_VERSION);

   //	load->RegisterPostLoadCallback(
   //		new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,0));
	return IO_OK;
}

int levelflagCreatorClass::proc (ViewExp *vpt, int msg, int point, 
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

