/*
 * levelflags.c
 */

#include "stdafx.h"
#include "CamControl.h"


/*#define MAXCAMS 64
typedef struct camlinks
{
	INode *camera;
	struct camlinks *next;
} CAMLINKS;

CAMLINKS camarray[MAXCAMS];
CAMLINKS *nextcam;
  */
ParamUIDesc CCdescParam[] = {
  	ParamUIDesc (PB_ID,				EDITTYPE_INT,		IDC_AGREEN, IDC_AGREENSPINNER, 0, 256, 0),
//  	ParamUIDesc (PB_CALC_MATRIX,	TYPE_SINGLECHEKBOX,	IDC_CALC_MATRIX),
  //	ParamUIDesc (PB_ID,				EDITTYPE_INT,		IDC_ID, IDC_IDSPINNER, 0, MAX_SUB_OBJECTS - 1, 0.1f),
  //	ParamUIDesc (PB_TARGETABLE,		TYPE_SINGLECHEKBOX,	IDC_TARGETABLE),
  //	ParamUIDesc (PB_HIGH_D_COLL,	TYPE_SINGLECHEKBOX,	IDC_HIGH_D_COLL),
};

ParamBlockDescID CCdescVer1[] = {
	{ TYPE_BOOL,	NULL, FALSE, PB_CALC_MATRIX },
	{ TYPE_INT,		NULL, FALSE, PB_ID },
};
#define VER1_SIZE 2

ParamBlockDescID CCdescVer2[] = {
	{ TYPE_BOOL,	NULL, FALSE, PB_CALC_MATRIX },
	{ TYPE_INT,		NULL, FALSE, PB_ID },
	{ TYPE_BOOL,	NULL, FALSE, PB_TARGETABLE },
};
#define VER2_SIZE 3

ParamBlockDescID CCdescVer3[] = {
	{ TYPE_BOOL,	NULL, FALSE, PB_CALC_MATRIX },
	{ TYPE_INT,		NULL, FALSE, PB_ID },
  	{ TYPE_BOOL,	NULL, FALSE, PB_TARGETABLE },
   	{ TYPE_BOOL,	NULL, FALSE, PB_HIGH_D_COLL },
};


IParamMap	*camcontrol::pmapParam;						// the default parameter map
IObjParam	*camcontrol::objParams;
//float		camcontrol::pitchang = 0.1f;
//char		camcontrol::name[MAX_FNAME_SIZE] = "CHOOSE DOME";

ICustEdit *camcontrolsParamDlg::nameCtrl;
ISpinnerControl  
*camcontrolsParamDlg::amountSpinner, 
*camcontrolsParamDlg::speedSpinner; 
/**camcontrolsParamDlg::upxposSpinner, 
*camcontrolsParamDlg::upyposSpinner, 
*camcontrolsParamDlg::upzposSpinner, 
*camcontrolsParamDlg::downxposSpinner, 
*camcontrolsParamDlg::downyposSpinner, 
*camcontrolsParamDlg::downzposSpinner;*/ 
HWND		camcontrolsParamDlg::instHwnd;

camcontrolsParamDlg *camcontrolsParamDlg::dlgInstance = NULL;

camcontrol::camcontrol (void)
{
   	MakeRefByID(FOREVER, 0, CreateParameterBlock(CCdescVer3, CC_PARAMDESC_LENGTH, CC_CURRENT_VERSION));
   	assert(pblock);
	ObjLoad (NULL, objMesh, 0);
	speed = 0.05f;
	amount = 15.0f;
//	upxpos = 0.0f;
	objMesh.InvalidateTopologyCache();
	objMesh.InvalidateGeomCache();
	objMesh.BuildStripsAndEdges();
//	AddCamControl();


	//FORCE THE REDRAW
	GetCOREInterface()->ForceCompleteRedraw(TRUE);

//	pblock->SetValue (PB_CALC_MATRIX,	TimeValue(0), cCalcMatrix);
//	pblock->SetValue (PB_ID,			TimeValue(0), 0);
//	pblock->SetValue (PB_TARGETABLE,	TimeValue(0), 0);
//	pblock->SetValue (PB_HIGH_D_COLL,	TimeValue(0), 0);
}

void camcontrol::WindowFinal (IObjParam *ip)
{
  //	pblock->GetValue (PB_CALC_MATRIX,	ip->GetTime(), *((int *)&cCalcMatrix), FOREVER);
}

void camcontrol::DrawMarker (GraphicsWindow *gw, bool Selected, bool Frozen)
{
	Point3 pt(0,0,0);
	if(!Frozen)
		gw->setColor(LINE_COLOR,GetUIColor(COLOR_POINT_OBJ));

  	gw->marker(&pt,SM_HOLLOW_BOX_MRKR);
}


void camcontrol::Reset()
{
//	nextcam = &camarray[0];
//	nextcam->next = NULL;
}


void camcontrolsParamDlg::Update (void)
{

  	ref->speed = speedSpinner->GetFVal();
  	ref->amount = amountSpinner->GetFVal();
  /*	ref->upxpos = upxposSpinner->GetFVal();
	ref->upypos = upyposSpinner->GetFVal();
	ref->upzpos = upzposSpinner->GetFVal();
	ref->downxpos = downxposSpinner->GetFVal();
	ref->downypos = downyposSpinner->GetFVal();
	ref->downzpos = downzposSpinner->GetFVal();*/
}

void camcontrolsParamDlg::Update (TimeValue t) // max called
{
  	ref->speed = speedSpinner->GetFVal();
  	ref->amount = amountSpinner->GetFVal();
  /*	ref->upxpos = upxposSpinner->GetFVal();
	ref->upypos = upyposSpinner->GetFVal();
	ref->upzpos = upzposSpinner->GetFVal();
	ref->downxpos = downxposSpinner->GetFVal();
	ref->downypos = downyposSpinner->GetFVal();
	ref->downzpos = downzposSpinner->GetFVal();*/
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


BOOL camcontrolsParamDlg::DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		instHwnd = hWnd;
	 	speedSpinner = SetupFloatSpinner (hWnd, IDC_ASPEEDSPINNER, IDC_ASPEED, -30.0f, 30.0f, ref->speed);
	  	assert (speedSpinner);

	 	amountSpinner = SetupFloatSpinner (hWnd, IDC_AAMOUNTSPINNER, IDC_AAMOUNT, -30.0f, 30.0f, ref->amount);
	  	assert (amountSpinner);

		return FALSE;
	case WM_DESTROY:
		Update();
		ReleaseISpinner (speedSpinner);
		ReleaseISpinner (amountSpinner);
		speedSpinner = 
		amountSpinner = 
			NULL;
		return TRUE;
	case WM_COMMAND:
			Update();
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
class camcontrolsDesc : public ClassDesc
{
public:
	int 			IsPublic() {return 1;}
	void *			Create (BOOL loading = FALSE) {return new camcontrol();}
	const TCHAR *	ClassName() {return GetString(IDS_CAMCONTROLS);}
	SClass_ID		SuperClassID() {return HELPER_CLASS_ID;}
	Class_ID		ClassID() {return camcontrols_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_HELPER_CATEGORY);}
	void			ResetClassParams (BOOL fileReset);
	BOOL			NeedsToSave () { return TRUE; }
//	IOResult		Save (ISave *save);
//	IOResult		Load (ILoad *load);
};


static camcontrolsDesc camcontrolDesc;
ClassDesc* GetcamcontrolDesc() {return &camcontrolDesc;}

void camcontrolsDesc::ResetClassParams (BOOL fileReset) 
{

//	nextcam = &camarray[0];
//	nextcam->next = NULL;
}

#define CHUNK_AMOUNT		1000
#define CHUNK_SPEED		1010



IOResult camcontrol::Save (ISave *save)
{
	ULONG nb;
	
	save->BeginChunk (CHUNK_AMOUNT);
	save->Write (&amount, sizeof (float), &nb);
	save->EndChunk();
	save->BeginChunk (CHUNK_SPEED);
	save->Write (&speed, sizeof (float), &nb);
	save->EndChunk();
	return IO_OK;
}

IOResult camcontrol::Load (ILoad *load)
{
	ULONG nb;
	IOResult res;
   //	ResetClassParams(TRUE);
   //	return(IO_OK);

	// Array of old versions
	static ParamVersionDesc versions[] = {
		ParamVersionDesc(CCdescVer1,VER1_SIZE,1),
		ParamVersionDesc(CCdescVer2,VER2_SIZE,2)
	};

	while ((res = load->OpenChunk()) == IO_OK) {
		switch (load->CurChunkID())
		{
			case CHUNK_AMOUNT:
				res = load->Read (&amount, sizeof (float), &nb);
				break;
			case CHUNK_SPEED:
				res = load->Read (&speed, sizeof (float), &nb);
				break;
		}
		load->CloseChunk();
		if (res != IO_OK)
			return res;
	}
	
	return IO_OK;
}


int camcontrolCreatorClass::proc (ViewExp *vpt, int msg, int point, 
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

