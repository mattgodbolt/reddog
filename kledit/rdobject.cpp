/*
 * RDObject.cpp
 * Interface to placing Red Dog objects onto the map
 */

#include "stdafx.h"
#include "ObjFlags.h"
#include "splshape.h"
#include "PLUGAPI.h"
#include "kleditspline.h"


#define NONDISPLAY 0
#define DISPLAY 1
#define LOADING 2
int Id;

/* 
 * The static variables
 */
IObjParam	*RDObject::ip;					// A handle to call MAX with
BOOL		RDObject::dlgHasStrat = FALSE;	// Default - does it have a strat?
IParamMap	*RDObject::pmapParam = NULL;	// the parameter map
char		RDObject::dlgObjSource[MAX_FNAME_SIZE] = "NONE";
char		RDObject::dlgStratSource[MAX_FNAME_SIZE] = "DUMMY.DST";

PickOperand	RDObject::pickCB;
RDObject::DisplayType RDObject::dlgDisplayType = RDObject::DISPLAY_BOX;


ICustEdit *RDObjectParamDlg::nameCtrl;
//ICustEdit cust;
 

//ISpinnerControl *armourSpinner=NULL;
//ISpinnerControl *healthSpinner=NULL; 
//ISpinnerControl *nodeSpinner=NULL; 
//ISpinnerControl *routeSpinner=NULL; 
//ISpinnerControl *intparamSpinner=NULL; 
//ISpinnerControl *floatparamSpinner=NULL; 
//ISpinnerControl *delaySpinner=NULL; 

ISpinnerControl  *RDObjectParamDlg::armourSpinner=NULL;
ISpinnerControl  *RDObjectParamDlg::healthSpinner=NULL; 
ISpinnerControl  *RDObjectParamDlg::nodeSpinner=NULL; 
ISpinnerControl  *RDObjectParamDlg::routeSpinner=NULL; 
ISpinnerControl  *RDObjectParamDlg::intparamSpinner=NULL; 
ISpinnerControl  *RDObjectParamDlg::floatparamSpinner=NULL; 
ISpinnerControl  *RDObjectParamDlg::delaySpinner=NULL; 

#define PB_HAS_STRAT		0
#define PB_DISPLAY			1
#define PB_FLAGSET			2



static int dispIDs[] = { IDC_BOX, IDC_DISPLAY_ARROW, IDC_MESH };
//static int flagIDs[] = { IDC_FRIENDLY, IDC_ENEMY};
//static int wayflagIDs[] = { IDC_NORMPATH, IDC_OFFPATH};

static ParamUIDesc descParam[] = {
	ParamUIDesc (PB_HAS_STRAT,	TYPE_SINGLECHEKBOX,	IDC_OBJECT_HAS_STRAT),
	ParamUIDesc (PB_DISPLAY,	TYPE_RADIO,			dispIDs,	3),
//	ParamUIDesc (PB_HAS_STRAT,	TYPE_RADIO,			flagIDs,	2)
//	ParamUIDesc (PB_DISPLAY,	TYPE_RADIO,			wayflagIDs,	2)
};


#define PARAMDESC_LENGTH 2

static ParamBlockDescID descVer1[] = {
	{ TYPE_BOOL,	NULL, FALSE, PB_HAS_STRAT },
	{ TYPE_INT,		NULL, FALSE, PB_DISPLAY }
};
#define PBLOCK_LENGTH	2
#define CURRENT_VERSION	1

#define SOME	0
#define ALL		1 
#define LEGACY	2		

void error()
{
	
 	MessageBox (NULL, "FUCKING","MEMORY WAREZ",MB_OK);

}

void FreeVals(RDObject *ref,int mode);

/*
 * Constructors + destructors
 */
RDObject::RDObject()
{
	int i;
	MakeRefByID(FOREVER, 0, CreateParameterBlock(descVer1, PBLOCK_LENGTH, CURRENT_VERSION));
  //	assert(pblock);
	
  //	pblock->SetValue (PB_HAS_STRAT,	TimeValue(0), dlgHasStrat);
  //	pblock->SetValue (PB_DISPLAY, TimeValue(0), (int) dlgDisplayType);

	// Copy in the previous object's source
//	strcpy (objSource, dlgObjSource);
//	ObjLoad (objSource, objMesh);
	parent = NULL;
	suspendSnap = FALSE;
//	hasStrat = dlgHasStrat;
	displayType = dlgDisplayType;
	flag = STRAT_ENEMY;
//	flag = STRAT_FRIENDLY;
//	strcpy (stratSource, dlgStratSource);
	wayflag=WAY_NORM;
	actnum = 0;
	SetReference(1,NULL);
	SetReference(2,NULL);
	SetReference(3,NULL);
	SetReference(4,NULL);
	SetReference(5,NULL);
	SetReference(6,NULL);
	SetReference(7,NULL);
	SetReference(8,NULL);
	SetReference(9,NULL);
	SetReference(10,NULL);
	SetReference(11,NULL);
	strcpy ( dlgStratSource,"DUMMY.DST");
	strcpy (stratSource, dlgStratSource);
	strcpy (dlgObjSource,"NONE");
	strcpy (objSource, dlgObjSource);

	
	//LOAD UP OUR OBJECT
//	objMesh = NULL;
	ObjLoad (objSource, objMesh, 0);
	MeshLoaded = FALSE;
	dlgDisplayType = DISPLAY_BOX;
	armour = 0.0f;
	health = 0.0f;
  	node = 0;
  	nodemax = 0;
  	path = 0;
  	pathmax = 0;
	entirenet = 0;
 //	entirenet = 1;
	ownrot = 0;
	ownpos = 0;
	actnum=0;
	waypoint = NULL;
	trigpoint = NULL;
	eventpoint = NULL;
	deletioneventpoint = NULL;
	for (i=0;i<5;i++)
		actpoint[i]=NULL;
	floatparam = -1;
	floatparammax = 0;
	intparam = -1;
	intparammax =0;
	internalintmax = 0;
	internalfloatmax = 0;


	for (i=0;i<MAX_PARAMS;i++)
	{
		intparams[i].value.ival = 0;
		intparams[i].paramname = NULL;
		floatparams[i].value.fval = 0.0;
		floatparams[i].paramname = NULL;
		Internalfloatparams[i].paramname = NULL;
		Internalintparams[i].paramname = NULL;
	}
	delay = 0;
}

//HANDLE TO ENSURE HOUSEKEEPING IS PERFORMED ON MALLOC'D MEM 
RDObject::~RDObject()
{
	int loop;
	for (loop=0;loop<NumRefs();loop++)
	{
		if (GetReference(loop))
			DeleteReference(loop);
	}
	FreeVals(this,ALL);
	mesh.FreeAll();
	objMesh.FreeAll();
	
}

void RDObject::Delete()
{
	int loop;
	for (loop=0;loop<NumRefs();loop++)
	{
		if (GetReference(loop))
			DeleteReference(loop);
	}
	FreeVals(this,ALL);
	mesh.FreeAll();
	objMesh.FreeAll();

}
#if 0
void RDObject::Clean()
{
	FreeVals(this,ALL);
	mesh.FreeAll();
	objMesh.FreeAll();
}
#endif

static void MakeQuad (Face *f, int a, int b, int c, int d)
{
	f[0].setVerts (b, a, c);
	f[0].setSmGroup (0);
	f[0].setEdgeVisFlags (1,0,1);
	f[1].setVerts (d, c, a);
	f[1].setSmGroup (0);
	f[1].setEdgeVisFlags (1,0,1);
}

static void MakeTri (Face *f, int a, int b, int c)
{
	f[0].setVerts (a, b, c);
	f[0].setSmGroup (0);
	f[0].setEdgeVisFlags (1,0,1);
}


#define MAKE_QUAD(na,nb,nc,nd)	{MakeQuad (&(mesh.faces[face]),na, nb, nc, nd);face+=2;}
#define MAKE_TRI(na,nb,nc)		{MakeTri (&(mesh.faces[face]),na, nb, nc);face+=1;}

/*
 * Builds the mesh (member function inherited from SimpleObject)
 */
void RDObject::BuildMesh (TimeValue t)
{
	int face = 0;
	int displayType;

	ivalid = FOREVER;

	//ANOTHER KLUDGE FOR CLONING OBJECTS
	if (t != -1)
		pblock->GetValue(PB_DISPLAY, t, displayType, ivalid);
	else
	{
		
		if (MeshLoaded)
		{
			displayType = DISPLAY_MESH;
			MeshLoaded = FALSE;
		}
		else
		displayType = DISPLAY_BOX;
	}
	Box3 boundBox = objMesh.getBoundingBox();
	float minX = boundBox.pmin.x;
	float minY = boundBox.pmin.y;
	float minZ = boundBox.pmin.z;
	float maxX = boundBox.pmax.x;
	float maxY = boundBox.pmax.y;
	float maxZ = boundBox.pmax.z;
	float midX = (maxX + minX) / 2.f;
	float midY = (maxY + minY) / 2.f;
	float midZ = (maxZ + minZ) / 2.f;
	float aBit = (maxZ - minZ) * 0.2f;

	switch (displayType) {
	case DISPLAY_BOX:	
		mesh.FreeAll();

		BoxMake(mesh,minX,minY,minZ,maxX,maxY,maxZ);
		break;

	case DISPLAY_ARROW:

		
		// The object is displayed by the displayer
		mesh.FreeAll();
		mesh.setNumVerts(13);
		mesh.setNumFaces(22);

		// Head vertices
		mesh.setVert (0, midX, maxY, midZ);
		mesh.setVert (1, minX, midY, minZ);
		mesh.setVert (2, minX, midY, maxZ);
		mesh.setVert (3, maxX, midY, minZ);
		mesh.setVert (4, maxX, midY, maxZ);


		// Body vertices
		mesh.setVert (5, midX - aBit, midY, midZ - aBit);
		mesh.setVert (6, midX - aBit, midY, midZ + aBit);
		mesh.setVert (7, midX + aBit, midY, midZ - aBit);
		mesh.setVert (8, midX + aBit, midY, midZ + aBit);

		mesh.setVert (9, midX - aBit, minY, midZ - aBit);
		mesh.setVert(10, midX - aBit, minY, midZ + aBit*2.f);
		mesh.setVert(11, midX + aBit, minY, midZ - aBit);
		mesh.setVert(12, midX + aBit, minY, midZ + aBit*2.f);

		// Head faces
		MAKE_TRI (0, 2, 4);
		MAKE_TRI (0, 3, 1);
		MAKE_TRI (0, 1, 2);
		MAKE_TRI (0, 4, 3);

		// Back of the head
		MAKE_QUAD (8, 6, 2, 4);
		MAKE_QUAD (5, 1, 2, 6);
		MAKE_QUAD (3, 7, 8, 4);
		MAKE_QUAD (7, 3, 1, 5);

		// Body
		MAKE_QUAD (12, 10, 6, 8);
		MAKE_QUAD (9, 11, 7, 5);
		MAKE_QUAD (9, 5, 6, 10);
		MAKE_QUAD (7, 11, 12, 8);

		// End cap
		MAKE_QUAD (10, 12, 11, 9);
		break;

	case DISPLAY_MESH:
		
		if (strncmp(objSource,"NONE",strlen(objSource)+1))
		{		
			if (!MeshLoaded)
			{
				mesh.FreeAll();
				objMesh.FreeAll();
				ObjLoad (objSource, objMesh,1);
				MeshLoaded = TRUE;
			//	ip->PipeSelLevelChanged();
				//UpdateMesh(ip->GetTime());
			 //	UpdateValidity(TOPO_CHAN_NUM,FOREVER);
			//	MeshInvalid();
//		   		ip->ForceCompleteRedraw();
			}
		}
		mesh = objMesh;
		break;
	}


	mesh.InvalidateTopologyCache();
	mesh.InvalidateGeomCache();
	mesh.BuildStripsAndEdges();
	//FORCE THE REDRAW
	GetCOREInterface()->ForceCompleteRedraw(TRUE);
}

/*
 * Display a cross for the object
 */
int RDObject::Display (TimeValue t, INode *node, ViewExp *view, int flags)
{
		return SimpleObject::Display (t, node, view, flags);
}

/*
 * Retrieves the one and only instance of class RDObjectMouseCallBack
 * which implements the callback during creation
 */
class RDObjectMouseCallBack : public CreateMouseCallBack
{
private:
	RDObject	*ref;	// a pointer to the object being created
	Point3		p0;		// Various points used in construction

public:
	RDObjectMouseCallBack() {}
	void setRef (RDObject *object) { ref = object; }
	int proc (ViewExp *vpt,int msg, int point, int flags, IPoint2 m, 
		Matrix3& mat);
};
static RDObjectMouseCallBack objectCallBack;

CreateMouseCallBack *RDObject::GetCreateMouseCallBack()
{ 
	objectCallBack.setRef (this);
	return &objectCallBack;
}

/*
 * Called during creation as the mouse moves
 */
int RDObjectMouseCallBack::proc (ViewExp *vpt, int msg, int point,
								 int flags, IPoint2 m, Matrix3& mat)
{
	// Check the mouse message
	switch (msg) {
	case MOUSE_FREEMOVE:
		vpt->SnapPreview(m,m,NULL, SNAP_IN_3D);
		break;
	case MOUSE_POINT:
	case MOUSE_MOVE:
		switch (point) {
		case 0: // Only at the beginning
			// Set up the translation based on the mouse position
			ref->suspendSnap = TRUE;
			p0 = vpt->SnapPoint (m, m, NULL, SNAP_IN_3D);
			mat.SetTrans(p0);
			break;
		case 1:
			p0 = vpt->SnapPoint (m, m, NULL, SNAP_IN_3D);
			mat.SetTrans(p0);

			if (msg==MOUSE_POINT) {										
				ref->suspendSnap = FALSE;
				return CREATE_STOP;
			}
			break;
		}
		break;

	case MOUSE_ABORT:
		ref->suspendSnap = FALSE;
		return CREATE_ABORT;
	}
	return CREATE_CONTINUE;
}

/*
 * Create the rollup pages on an edit begin
 */

enum {OBJECTS,STRATS};

TSTR selectedModelName;// a pointer to the currently selected model
static int fillTree (char *parentDir, char *dir, char *current, HWND listBox, HTREEITEM after,int Mode)
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
			fillTree (buf, findData.cFileName, current, listBox, tree,Mode);
		}
		if (!FindNextFile(hSearch, &findData))
			break;
	}
	FindClose (hSearch);

	// Now add all the .DST files

	if (Mode == STRATS)
	{
		if (parentDir)
			sprintf (buf, "%s\\%s\\*.DST", parentDir, dir);
		else
			sprintf (buf, "%s\\*.DST", dir);
	}
	else
	{
		if (parentDir)
			sprintf (buf, "%s\\%s\\*.RDO", parentDir, dir);
		else
			sprintf (buf, "%s\\*.RDO", dir);
	}

	if ((hSearch = FindFirstFile (buf, &findData)) == INVALID_HANDLE_VALUE)
		return 0;
	while (1) {
		char buf2[MAX_PATH];
		HTREEITEM item;
		strcpy (buf2, findData.cFileName);
		buf2[strlen(buf2)-4] = '\0';
		char *tx = &buf2[strlen(buf2)-4];

		int valid;

		if (Mode == OBJECTS)
			valid = strnicmp(tx,"_PAL",4);
		else
			valid = strnicmp(buf2,"SPAWN_",6);		

	   //	if 	(
	   //		(strnicmp(tx,"_PAL",4)) &&
	   //		(strnicmp(buf2,"SPAWN_",6))
	   //		)
	  
		if (valid)
		{

	  //	if (strnicmp(buf2,"SPAWN_",6))
	  //	{


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

		 	if (Mode == OBJECTS)
			{
				if (!stricmp (&buf2[strlen (STRAT_OBJECT_ROOT) + 1], current))
					TreeView_SelectItem (listBox, item);
			}
			else
			{
				if (!stricmp (&buf2[strlen (STRAT_ROOT) + 1], current))
					TreeView_SelectItem (listBox, item);
			}
		}
		if (!FindNextFile(hSearch, &findData))
			break;
	}
	FindClose (hSearch);

	return 1;
}

// Recursively gets the filename of a listbox item
static void GetItemName (HWND treeItem, char *buffer, HTREEITEM tree,int Mode)
{
	TV_ITEM newInfo;
	char textBuf[256];
	static int itemCount = 0;

	// Append the root name
	if (itemCount==0)
		if (Mode == OBJECTS)
			strcpy (buffer, STRAT_OBJECT_ROOT);
		else
			strcpy (buffer, STRAT_ROOT);

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
		GetItemName (treeItem, buffer, (HTREEITEM) newInfo.lParam,Mode);

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
		
		
		if (Id == OBJECTS)
		{
			treeItem = GetDlgItem (dBox, IDC_OBJECT_TREE);
			fillTree (NULL, STRAT_OBJECT_ROOT, (char *)lp, treeItem, NULL,OBJECTS);
			imageList = ImageList_LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_IMAGELIST),
				16, 0, RGB (0, 0, 255));
			TreeView_SetImageList (treeItem, imageList, TVSIL_NORMAL);
			EnableWindow (GetDlgItem (dBox, IDOK), FALSE);

		}
		else
		{
			treeItem = GetDlgItem (dBox, IDC_OBJECT_TREE);
			fillTree (NULL, STRAT_ROOT, (char *)lp, treeItem, NULL,STRATS);
			imageList = ImageList_LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_IMAGELIST),
				16, 0, RGB (0, 0, 255));
			TreeView_SetImageList (treeItem, imageList, TVSIL_NORMAL);
			EnableWindow (GetDlgItem (dBox, IDOK), FALSE);
		}
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
				GetItemName (treeItem, &textBuf[0], pnmtv->itemNew.hItem,Id);
			} else {
				HTREEITEM tree;
				tree = TreeView_GetSelection (treeItem);
				GetItemName (treeItem, &textBuf[0], tree,Id);
			}
			if (Id == OBJECTS)
				strcat (textBuf, ".RDO");
			else
				strcat (textBuf, ".DST");

			if (FILE *f = fopen (textBuf, "rb")) 
			{
				// Note the selected model name
				selectedModelName = textBuf;
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

#if 0
INode *ReturnMyNode(RDObject *ref)
{

	//enumerate trhough the scene until root child = the node we are dealing with 'ref'
	
	INode *newnode = GetCOREInterface()->GetRootNode();
	int numchild = newnode->NumberOfChildren();

	INode *childnode;

	int found = 0;
	for (int i=0;i<numchild;i++)
	{
		childnode = newnode->GetChildNode(i);
		Object *obj = childnode->EvalWorldState(GetCOREInterface()->GetTime()).obj;
		if (obj == ref)
		{
			found++;
			break;
		}
	}						 //IMPLEMENT FOUND++ IF FOUND
	return(childnode);

}
#endif

//GIVEN A SELECTION OF A NETWORK OR PATH, THIS WILL GO THROUGH THE ATTACHED STRUCTURE
//AND EXTRACT THE MAXIMUM NODES THAT MAY BE PICKED FROM FOR THE STRAT'S START POSITION
//class NullView: public View {
//	public:
//		Point2 ViewToScreen(Point3 p) { return Point2(p.x,p.y); }
//		NullView() { worldToView.IdentityMatrix(); screenW=640.0f; screenH = 480.0f; }
//	};


//int Set = 0;

void	SetUpNodeChoice(RDObject *ref,TimeValue t,HWND win)
{

	Object *obj = ref->waypoint->EvalWorldState(t).obj;
  	Matrix3 pathmat = ref->waypoint->GetObjectTM(0);
	ShapeObject* shape = (ShapeObject*)obj;
	PolyShape pShape;
	//shape->MakePolyShape(t, pShape);
	int numLines;

	Point3 p;

	TCHAR* nodename = ref->waypoint->GetName();	

	// BOLLOX SO I MAY EVENTUALLY GET SPLINE RE-COLOURISATION WORKING

	// 	Mesh *mesh = ((GeomObject*)obj)->GetRenderMesh(t,ref->waypoint,view,needDel);


  //	SplineShape *spline = (SplineShape*)obj; 
  //	spline->shape.SetDispFlag(DISP_UNSELECTED);
  //	spline->shape.selLevel = SHAPE_VERTEX;
  //	spline->shape.UpdateSels();
  // THIS WORKS FINE	spline->shape.DeleteSpline(t);

 //  	if (!Set)
 //  	{
   //	BOOL needDel = TRUE;
  // 	NullView view;
  //	Mesh *mesh = shape->GetRenderMesh(t,ref->waypoint,view,needDel);
  //	mesh->SetDispFlag(DISP_VERTTICKS);
   //	MeshInvalid();
	 //((SimpleObject*)shape)->UpdateValidity(TOPO_CHAN_NUM,FOREVER);
			//	MeshInvalid();

   //	((SimpleObject*)shape)->MeshInvalid();
   //	((SimpleObject*)shape)->UpdateMesh(t);
	 //	pShape.dispFlags = DISP_VERTTICKS;
	   //	shape->CopyBaseData(&pShape);
 	 //	Set++;
//		shape->SetThickness(100000.0f);
//   	}
//	return;
	//	mesh = &obj->mesh;

	if ((!(strnicmp(nodename,"network",7))))
	{

//		MessageBox (win, "TEST", "TEST ME", MB_YESNOCANCEL);
		//NETWORK MODE GET THE LINKED PATH

		int numchild = ref->waypoint->NumberOfChildren();

		if ((!numchild) || (ref->entirenet))
		{
		  //	MessageBox (win, "NEEDS AN ATTACHED PATH",nodename, MB_OK);
		  //	ref->nodemax = 0;
		  //	ref->node = 0;
		  //	return;
		  
		  	INode *path = ref->waypoint;			

			//int a=ref->MakeRefByID(FOREVER,3,path);
			//ref->netpath=(INode*)ref->GetReference(3);

			obj = path->EvalWorldState(t).obj;
			obj->DoOwnSelectHilite();
			pathmat = path->GetObjectTM(0);
			shape = (ShapeObject*)obj;
			shape->MakePolyShape(t, pShape);
			numLines = pShape.numLines;
			int numPts = 0;

			for (int line=0;line<numLines;line++)
		 		numPts += pShape.lines[line].numPts;

			ref->nodemax = numPts-1;


		}	
		else
		{
			//sat removed
			//	INode *path = ref->waypoint->GetChildNode(0);			
			if ((ref->path+1) > numchild)
			{
				//reconnect to correct child as a patrol spline has gone walkabout
				for (int child=0;child < numchild;child++)
				{
					INode *netchild = ref->waypoint->GetChildNode(child);
					if (ref->netpath == netchild)
						ref->path = child;

				}
				
				ref->path = 0;
			}
		  	INode *path = (INode*)(ref->waypoint->GetChildNode(ref->path));			

			if (ref->GetReference(3))
				ref->ReplaceReference(3,path,1);
			else
		   		int a=ref->MakeRefByID(FOREVER,3,path);
			ref->netpath=(INode*)ref->GetReference(3);

			obj = path->EvalWorldState(t).obj;
			obj->DoOwnSelectHilite();
  			pathmat = path->GetObjectTM(0);
			shape = (ShapeObject*)obj;
			shape->MakePolyShape(t, pShape);
			numLines = pShape.numLines;
			int numPts = 0;

			for (int line=0;line<numLines;line++)
		 		numPts += pShape.lines[line].numPts;

			ref->nodemax = numPts-1;

		}

   		 int storeline=0,storepoint=0;

   		 int numPts = 0;
		 for (int line=0;line<numLines;line++)
			for (int point=0;point<pShape.lines[line].numPts;point++)
			{
				if (numPts == ref->node)
				{
					storeline = line;
					storepoint = point;
				}
				 
				 numPts++;
			}

		 p=pShape.lines[storeline].pts[storepoint].p * pathmat;
		
	}
	else
	{
		obj->DoOwnSelectHilite();
		shape->MakePolyShape(t, pShape);
		ref->nodemax = pShape.lines->numPts-1;
		p=pShape.lines[0].pts[ref->node].p * pathmat;


	}
 
	//find this node in the scene
	INode *childnode = ref->GetMyNode();
	
	if ((childnode) && (!(ref->ownpos)))
	{
//		Matrix3 transform = ref->waypoint->GetNodeTM(GetCOREInterface()->GetTime());
		Matrix3 transform = childnode->GetObjectTM(GetCOREInterface()->GetTime());
		nodename = childnode->GetName();	

		transform.SetTrans(p);
		childnode->SetNodeTM(GetCOREInterface()->GetTime(),transform);
	}

	pShape.NewShape();
	//ELSE FATAL ERROR REPORT NEEDED HERE	
}



void	SetUpRouteChoice(RDObject *ref,TimeValue t,HWND win)
{

	Object *obj = ref->waypoint->EvalWorldState(t).obj;
  	Matrix3 pathmat = ref->waypoint->GetObjectTM(0);
	ShapeObject* shape = (ShapeObject*)obj;
	PolyShape pShape;
	shape->MakePolyShape(t, pShape);
	int numLines = pShape.numLines;

	Point3 p;

	TCHAR* nodename = ref->waypoint->GetName();	

	if (!(strnicmp(nodename,"network",7)))
	{

//		MessageBox (win, "TEST", "TEST ME", MB_YESNOCANCEL);
		//NETWORK MODE GET THE LINKED PATH

		int numchild = ref->waypoint->NumberOfChildren();

		if (!numchild)
		{
		  //	MessageBox (win, "NEEDS AN ATTACHED PATH",nodename, MB_OK);
			ref->nodemax = 0;
			ref->node = 0;
		  //	return;
		}	
		else
		{
			ref->pathmax = numchild - 1;
			//ref->path = 0;
		}

	}
	pShape.NewShape();
}


#define MAX_LINE 512
#define INVALID -1
#define PARAMINT 0
#define PARAMFLOAT 1
#define	ENDFLOAT	2
#define	ENDINT	3
#define ENDPARAMS 4
#define ASIZE(a) (sizeof(a)/sizeof((a)[0]))


/******* PARAM SEARCH KEYWORDS *******/

static struct {
	char *word;
	short length;
	short token;

#define K(word,token) {word,sizeof(word)-1,token,},
} Keywords[] = {
	K("PARAMINT",	  	PARAMINT)
	K("PARAMFLOAT",	  	PARAMFLOAT)
	K("LOCALFLOAT",	  	ENDFLOAT)
	K("LOCALINT",	  	ENDINT)
	K("/* ENDPARAMS */",	  	ENDPARAMS)


};
#undef K

char  *WhiteSpaceRead(char *cursor)
{	int charfound=0;
	int len;
	char *max;

	//SAFETY CHECK TO ENSURE NULL * IS NOT CHECKED

	if (!cursor )
		return (NULL);

	len = strlen(cursor)+1;
	max = cursor+MAX_LINE;



	for (cursor;cursor < max;cursor++)
	{
		if ((strncmp(cursor," ",1)) && (strncmp(cursor,"\t",1)))
		{
			if ((strncmp(cursor,"\n",1))  && (strncmp(cursor,"\0",1)))
				charfound=1;
			break;
		}
	}

	if (charfound)
		return (cursor);
	else
		return (NULL);
}

char *ReadToEnd(char *dest,char*src)
{
	while	(
			(strncmp(src," ",1)) &&
			(strncmp(src,"\0",1)) &&
			(strncmp(src,"\t",1)) &&
			(strncmp(src,"\n",1)) )
	{

		strncpy(dest,src,1);
	 	src++;
		dest++;
	}
	*dest='\0';
	return(src);
}

void Clear(char *input,int amount)
{
	// innocent whistling of MattG here...
//	memset (input, 0, amount);
 	
 	int i;
	for (i=0;i<amount;i++)
		input[i]='\0';
	
}




#define MODEINT 0
#define MODEFLOAT 1

int CheckVal(PARAM *param,int mode,int max, PARAM *src)
{
	int i,res;

	for (i=0;i<max;i++)
	{
	   //	if (mode == MODEINT)
			if (strlen(param->paramname) != strlen((src+i)->paramname))
				res = 1;
			else
		  		res = strncmp(param->paramname,(src+i)->paramname,strlen((src+i)->paramname));
	   //	else
		 //	res = strncmp(param->paramname,Internalfloatparams[i].paramname,strlen(Internalfloatparams[i].paramname);

		if (!res)
		{
			if (mode == MODEINT)
				param->value.ival = (src+i)->value.ival;
			else
				param->value.fval = (src+i)->value.fval;
			return(1);
		}


	}

	return(0);


}


int UpdateInternalVal(PARAM *param,int mode,int max, PARAM *src)
{
	int i,res;

	for (i=0;i<max;i++)
	{
		if (strlen(param->paramname) != strlen((src+i)->paramname))
			res = 1;
		else
			res = strncmp(param->paramname,(src+i)->paramname,strlen((src+i)->paramname));

		if (!res)
		{
			if (mode == MODEINT)
				(src+i)->value.ival = param->value.ival ;
			else
				(src+i)->value.fval = param->value.fval ;
			return(1);
		}


	}

	return(0);


}


//OPENS UP THE STRAT FILE TO REGEN THE INTERNAL LIST
void RegenInternalList(RDObject *ref,char *name)
{
  char filename[256];
  FILE *fp;

  sprintf(filename,"%s\\%s",STRAT_ROOT,name);

  fp = fopen (filename, "r");

  //if (fp)



  char line[MAX_LINE];

  int linenum=0,token,i;
  int endint=0,endfloat=0,endfound=0;
  char *linest=line;

  ref->internalintmax = ref->internalfloatmax = 0;
  
  
  while ((fgets(line,MAX_LINE,fp)) && (!endfound))
  {
		linenum ++;
		linest = line;
		endfound = endint & endfloat;

		if ((strnicmp(linest,"//",2)) && ((strnicmp(linest,"/*",2))))
		{

			token=INVALID;	
			for(i=0; i< ASIZE(Keywords); i++)
			{
					
				if(!strncmp(Keywords[i].word,linest,Keywords[i].length))
				{
					token = Keywords[i].token;
					linest += Keywords[i].length;
					break;
				}		
			}

			if (token != INVALID)
			{
			   	if (linest = WhiteSpaceRead(linest))
				{
			   
					switch (token)
					{
						case PARAMINT:
						
							ref->Internalintparams[ref->internalintmax].paramname = (char*)malloc(strlen(linest)+1);
						   	if (ref->Internalintparams[ref->internalintmax].paramname)
							{
								linest = ReadToEnd(ref->Internalintparams[ref->internalintmax].paramname,linest);

								if (!(linest = WhiteSpaceRead(linest)))
									ref->Internalintparams[ref->internalintmax].value.ival = 0;
								else
									ref->Internalintparams[ref->internalintmax].value.ival = atoi(linest);
								ref->internalintmax++;
							}
							else
								error();
							break;
						case PARAMFLOAT:
							ref->Internalfloatparams[ref->internalfloatmax].paramname = (char*)malloc(strlen(linest)+1);
						   	if (ref->Internalfloatparams[ref->internalfloatmax].paramname)
							{
								linest = ReadToEnd(ref->Internalfloatparams[ref->internalfloatmax].paramname,linest);

								if (!(linest = WhiteSpaceRead(linest)))
									ref->Internalfloatparams[ref->internalfloatmax].value.fval = 0;
								else
									ref->Internalfloatparams[ref->internalfloatmax].value.fval = (float)atof(linest);
								ref->internalfloatmax++;
							}
							else
								error();
							break;
						case	ENDFLOAT:
							endfloat = 1;
							break;
						case	ENDINT:
							endint = 1;
							break;
						case ENDPARAMS:
							endint = endfloat = 1;
							break;
					
					}

				}
			}

		}
	  
  }

  fclose(fp);


}


//OPENS UP THE STRAT FILE TO READ THE NUMBER OF PARAMETER VARS THAT MAY BE SET
void SetupMaxParams(RDObject *ref,char *name,int mode,HWND hWnd)
{
  char filename[256];
  FILE *fp;
  int endint=0,endfloat=0,endfound=0;

//  if (mode == NONDISPLAY)
//	ref->paraminterr=ref->paramfloaterr = 0;
//	return;  
  int oldfloatparammax = ref->floatparammax;
  int oldintparammax = ref->intparammax;

  sprintf(filename,"%s\\%s",STRAT_ROOT,name);

  fp = fopen (filename, "r");

  if (!(fp))
  {
   	MessageBox (hWnd, "MISSING STRAT FILE",filename, MB_TASKMODAL | MB_OK);
	return;
  }



  char line[MAX_LINE];

  ref->intparam = -1;
  ref->intparammax = 0;

  ref->floatparam = -1;
  ref->floatparammax = 0;

  int linenum=0,token,i;
  char *linest=line;
  while ((fgets(line,MAX_LINE,fp)) && (!endfound))
  {
		linenum ++;
		linest = line;
		endfound = endint & endfloat;


		if ((strnicmp(linest,"//",2)) && ((strnicmp(linest,"/*",2))))
		{

			token=INVALID;	
			for(i=0; i< ASIZE(Keywords); i++)
			{
					
				if(!strncmp(Keywords[i].word,linest,Keywords[i].length))
				{
					token = Keywords[i].token;
					linest += Keywords[i].length;
					break;
				}		
			}

			if (token != INVALID)
			{
			   	if (linest = WhiteSpaceRead(linest))
				{
			   
					switch (token)
					{
						case PARAMINT:
							ref->intparam++;
							ref->intparammax++;
						
							ref->intparams[ref->intparam].paramname = (char*)malloc(strlen(linest)+1);
						   	if (ref->intparams[ref->intparam].paramname)
							{
								linest = ReadToEnd(ref->intparams[ref->intparam].paramname,linest);

								if (!(CheckVal(&ref->intparams[ref->intparam],MODEINT,ref->internalintmax,ref->Internalintparams)))
								{
									if (!(linest = WhiteSpaceRead(linest)))
										ref->intparams[ref->intparam].value.ival = 0;
									else
									{
										ref->intparams[ref->intparam].value.ival =
										ref->intparams[ref->intparam].def.ival = atoi(linest);
									}
								}
								else
								{
									if ((linest = WhiteSpaceRead(linest)))
										ref->intparams[ref->intparam].def.ival = atoi(linest);

								}
							}
							else
								error();

							break;
						case PARAMFLOAT:
							ref->floatparam++;
							ref->floatparammax++;
							ref->floatparams[ref->floatparam].paramname = (char*)malloc(strlen(linest)+1);
							if (ref->floatparams[ref->floatparam].paramname)
							{
								linest = ReadToEnd(ref->floatparams[ref->floatparam].paramname,linest);
								if (!(CheckVal(&ref->floatparams[ref->floatparam],MODEFLOAT,ref->internalfloatmax,ref->Internalfloatparams)))
								{
									if (!(linest = WhiteSpaceRead(linest)))
										ref->floatparams[ref->floatparam].value.fval = 0.0;
									else
									{
										ref->floatparams[ref->floatparam].value.fval =
										ref->floatparams[ref->floatparam].def.fval = (float)atof(linest);
									}
								}
								else
								{
									if ((linest = WhiteSpaceRead(linest)))
										ref->floatparams[ref->floatparam].def.fval = (float)atof(linest);
							
								}
							}
							else
								error();
							break;
						case ENDFLOAT:
							endfloat=1;
							break;
						case ENDINT:
							endint=1;
							break;
						case ENDPARAMS:
							endint = endfloat = 1;
							break;

				 	}
				}
				else
				   	MessageBox (hWnd, "INVALID PARAM SET IN STRAT",filename, MB_TASKMODAL | MB_OK);



			}

		}
	  
  }

  fclose(fp);

  //return;

 // if (mode == LOADING)
  {
	if ((ref->internalintmax != ref->intparammax) || (ref->internalfloatmax != ref->floatparammax))
	{
		PARAM TempIntParams[32];
		PARAM TempFloatParams[32];
		int loopint  = ref->intparammax;
		int loopfloat  = ref->floatparammax;


	
		
		for (int i =0; i < loopint; i++)
		{
			TempIntParams[i].paramname = (char*)malloc(strlen(ref->intparams[i].paramname) + 1);
		
		
			strcpy(TempIntParams[i].paramname,ref->intparams[i].paramname);
			TempIntParams[i].value.ival = ref->intparams[i].value.ival;

		}
	
	//	return;

		for (i =0; i < loopfloat; i++)
		{
			TempFloatParams[i].paramname = (char*)malloc(strlen(ref->floatparams[i].paramname) + 1);
			strcpy(TempFloatParams[i].paramname,ref->floatparams[i].paramname);
			TempFloatParams[i].value.fval = ref->floatparams[i].value.fval;

		}

	

	    FreeVals(ref,LEGACY);
	  	RegenInternalList(ref,ref->stratSource);

		for (i =0; i < loopint; i++)
		{
			int foundint = 0;
			for (int loop = 0; loop < ref->intparammax;loop++)
			{
				if (strlen(TempIntParams[i].paramname) == strlen(ref->intparams[loop].paramname))
				{
				if (!(strncmp(TempIntParams[i].paramname,ref->intparams[loop].paramname,strlen(TempIntParams[i].paramname))))
				{
					foundint++;
					break;
				}
				}

			}									 

			if (foundint)
			{
				ref->intparams[loop].value.ival = TempIntParams[i].value.ival;
				ref->Internalintparams[loop].value.ival = ref->intparams[loop].value.ival;
			
			}
			free(TempIntParams[i].paramname);


		}
	 
		for (i =0; i < loopfloat; i++)
		{
			int foundfloat = 0;
			for (int loop = 0; loop < ref->floatparammax;loop++)
			{
			   if (strlen(TempFloatParams[i].paramname) == strlen(ref->floatparams[loop].paramname))
				{
				if (!(strncmp(TempFloatParams[i].paramname,ref->floatparams[loop].paramname,strlen(TempFloatParams[i].paramname))))
				{
					foundfloat++;
					break;
				}
			   }

			}

			if (foundfloat)
			{
				ref->floatparams[loop].value.fval = TempFloatParams[i].value.fval;
				ref->Internalfloatparams[loop].value.fval = ref->floatparams[loop].value.fval;
			
			}
			free(TempFloatParams[i].paramname);


		}




	}
	

  }  


//  {
/*
	 if (oldintparammax != ref->intparammax) 
	    if ((mode == DISPLAY) && (ref->paraminterr))
		   	MessageBox (hWnd, "THE NUMBER OF INT PARAMS HAVE CHANGED SINCE THE LAST EDIT\nPLEASE ENSURE THE VALUES ARE STILL CORRECT",filename, MB_TASKMODAL | MB_OK);
		else
			ref->paraminterr++;

	 if (oldfloatparammax != ref->floatparammax)
	    if ((mode == DISPLAY) && (ref->paramfloaterr))
			MessageBox (hWnd, "THE NUMBER OF FLOAT PARAMS HAVE CHANGED SINCE THE LAST EDIT\nPLEASE ENSURE THE VALUES ARE STILL CORRECT",filename, MB_TASKMODAL | MB_OK);
		else
  		   ref->paramfloaterr++;
  
//  }


  int oldline = linenum;
 */


}

#define INTDEFS 0
#define FLOATDEFS 1

//OPENS UP THE STRAT FILE TO READ THE NUMBER OF PARAMETER VARS THAT MAY BE SET
void ResetParams(RDObject *ref,int mode)
{
  	int loop;

	if (mode == FLOATDEFS)
	{
		for (loop=0;loop<ref->floatparammax;loop++)
		{
			ref->floatparams[loop].value.fval = ref->floatparams[loop].def.fval;
			ref->Internalfloatparams[loop].value.fval = ref->floatparams[loop].def.fval;
		}
	}

	if (mode == INTDEFS)
	{
		for (loop=0;loop<ref->intparammax;loop++)
		{
			ref->intparams[loop].value.ival = ref->intparams[loop].def.ival;
			ref->Internalintparams[loop].value.ival = ref->intparams[loop].def.ival;
		}
	 }
}

void FreeVals(RDObject *ref,int mode)
{
	int loop;

	if ((mode == ALL) || (mode == LEGACY))
	{
		for (loop=0;loop<(ref->internalintmax);loop++)
		{
			if (ref->Internalintparams[loop].paramname)
				free(ref->Internalintparams[loop].paramname);
			ref->Internalintparams[loop].paramname=NULL;
		 }


		for (loop=0;loop<(ref->internalfloatmax);loop++)
		{
			if (ref->Internalfloatparams[loop].paramname)
				free(ref->Internalfloatparams[loop].paramname);
			ref->Internalfloatparams[loop].paramname=NULL;
		 }


		ref->internalintmax = 0;
		ref->internalfloatmax = 0;

	}

	//FREE UP USED MEM IF NEEDED
	if (mode != LEGACY)
	{
		if (ref->intparam >=0)
		{
			for (loop=0;loop<(ref->intparam+1);loop++)
			{
				if (ref->intparams[loop].paramname)
					free(ref->intparams[loop].paramname);
				ref->intparams[loop].paramname=NULL;
			 }
		}
		if (ref->floatparam >=0)
		{
			for (loop=0;loop<(ref->floatparam+1);loop++)
			{
				if (ref->floatparams[loop].paramname)
					free(ref->floatparams[loop].paramname);
				ref->floatparams[loop].paramname=NULL;
		   	}
		}
	}


}


void ParamUpdate(RDObject *ref,TCHAR *name,HWND hWnd)
{
  	char num[4];

	FreeVals(ref,SOME);

   	//if (strnicmp(name,"DUMMY.DST",9))
	SetupMaxParams(ref,name,DISPLAY,hWnd);
 
  	if (ref->intparammax)
  	{
		ref->intparam = 0;
  		SetDlgItemText (hWnd, IDC_IPARAM, ref->intparams[ref->intparam].paramname);
  		if (RDObjectParamDlg::intparamSpinner)
  			ReleaseISpinner(RDObjectParamDlg::intparamSpinner);
  	   //		ReleaseICustEdit(intparamSpinner);
  		RDObjectParamDlg::intparamSpinner = SetupIntSpinner (hWnd, IDC_INTPARAMSPINNER, IDC_INTPARAM, -65536,65535 , ref->intparams[ref->intparam].value.ival);
  		assert (RDObjectParamDlg::intparamSpinner);
	  	itoa(ref->intparam,num,10);
		SetDlgItemText (hWnd, IDC_IPARAMNUM, num);

  	}
	else
	{
		ref->intparam = -1;
		SetDlgItemText (hWnd, IDC_IPARAM, "NO INT PARAMS");
		SetDlgItemText (hWnd, IDC_IPARAMNUM, " ");
  		if (RDObjectParamDlg::intparamSpinner)
  			ReleaseISpinner(RDObjectParamDlg::intparamSpinner);
	}
			

  	if (ref->floatparammax)
  	{
		ref->floatparam = 0;
  		SetDlgItemText (hWnd, IDC_FPARAM, ref->floatparams[ref->floatparam].paramname);
  		if (RDObjectParamDlg::floatparamSpinner)
  			ReleaseISpinner(RDObjectParamDlg::floatparamSpinner);
  		RDObjectParamDlg::floatparamSpinner = SetupFloatSpinner (hWnd, IDC_FLOATPARAMSPINNER, IDC_FLOATPARAM, -65536.0,65535.0 , ref->floatparams[ref->floatparam].value.fval);
  		assert (RDObjectParamDlg::floatparamSpinner);
	  	itoa(ref->floatparam,num,10);
		SetDlgItemText (hWnd, IDC_FPARAMNUM, num);

  	}
	else
	{
		ref->floatparam = -1;

  		SetDlgItemText (hWnd, IDC_FPARAM, "NO FLOAT PARAMS");
	  	SetDlgItemText (hWnd, IDC_FPARAMNUM, " ");
		if (RDObjectParamDlg::floatparamSpinner)
  			ReleaseISpinner(RDObjectParamDlg::floatparamSpinner);

	}
}

void UpdateNodeName(RDObject *ref,char *source)
{
	char buf[128];
	int i=0;
	*buf= '_';
	i++;

	while (*source != '.')
	{
		buf[i] = *source++;
		i++;
	}
	buf[i] = '\0';
   
	INode *mynode =  ref->GetMyNode();

	if (mynode)
		mynode->SetName(buf);
}


HWND DiaSave=NULL;
enum {NOPICK,WAYPOINT,TRIGPOINT,ACTPOINT,EVENTPOINT,PARENT,DELETIONEVENTPOINT};
int PickMode;
int NUMACT;

class RDObjectParamMapUserDlgProc : public ParamMapUserDlgProc
{
private:
	RDObject	&ref;
	char		num[4];

public:
	RDObjectParamMapUserDlgProc (RDObject &r) : ref(r) {}
  

				void DeleteThis() { delete this; }
   	BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{

		
		if (ref.path==-1)
			ref.path=0;


		switch (msg) {
		case WM_INITDIALOG:

				RDObjectParamDlg::armourSpinner = SetupFloatSpinner (hWnd, IDC_ARMOURSPINNER, IDC_ARMOUR, 0.0f, 1.0f, ref.armour);
				RDObjectParamDlg::healthSpinner = SetupFloatSpinner (hWnd, IDC_HEALTHSPINNER, IDC_HEALTH, 0.0f, 1.0f, ref.health);
				RDObjectParamDlg::delaySpinner = SetupIntSpinner (hWnd, IDC_DELAYSPINNER, IDC_DELAY, 0, 10000, ref.delay);

				if (ref.waypoint)
				{
					SetUpNodeChoice(&ref,t,hWnd);

					RDObjectParamDlg::nodeSpinner = SetupIntSpinner (hWnd, IDC_NODESPINNER, IDC_NODE, 0, ref.nodemax, ref.node);
					if (!(ref.entirenet))
						SetUpRouteChoice(&ref,t,hWnd);
					RDObjectParamDlg::routeSpinner = SetupIntSpinner (hWnd, IDC_ROUTESPINNER, IDC_ROUTENUM, 0, ref.pathmax, ref.path);
				}

				//char num[4];
				itoa(ref.actnum,num,10);
				SetDlgItemText (hWnd, IDC_ACTNUM, num);

				//INIT OUR PANEL'S DIALOGUE BOXES
				SetDlgItemText (hWnd, IDC_OBJECT_TYPE, ref.stratSource);


				//update our params
			  	ParamUpdate(&ref,ref.stratSource,hWnd);
				
				
					
				SetDlgItemText (hWnd, IDC_STRAT_OBJECTS, ref.objSource);
				
				if (ref.flag & STRAT_FRIENDLY)
					CheckRadioButton(hWnd, IDC_FRIENDLY,IDC_ENEMY,IDC_FRIENDLY);
				else
					CheckRadioButton(hWnd, IDC_FRIENDLY,IDC_ENEMY,IDC_ENEMY);

				CheckDlgButton(hWnd, IDC_GRAVITYONC, 0);
				CheckDlgButton(hWnd, IDC_CALCRELVELC, 0);
				CheckDlgButton(hWnd, IDC_COLLFLOORC, 0);
				CheckDlgButton(hWnd, IDC_COLLSTRATC, 0);
				
				if (ref.entirenet)
					CheckDlgButton(hWnd, IDC_ENTIRENET, 1);

				if (ref.ownrot)
					CheckDlgButton(hWnd, IDC_OWNROT, 1);
			   
				if (ref.ownpos)
					CheckDlgButton(hWnd, IDC_OWNPOS, 1);




				if (ref.flag & STRAT_GRAVITYON)
					CheckDlgButton(hWnd, IDC_GRAVITYONC, 1);
				//	CheckRadioButton(hWnd, IDC_GRAVITYON,IDC_GRAVITYON,IDC_GRAVITYON);
				if (ref.flag & STRAT_CALCRELVEL)
					CheckDlgButton(hWnd, IDC_CALCRELVELC, 1);
				//	CheckRadioButton(hWnd, IDC_CALCRELVEL,IDC_CALCRELVEL,IDC_CALCRELVEL);
				if (ref.flag & STRAT_COLLFLOOR)
					CheckDlgButton(hWnd, IDC_COLLFLOORC, 1);
				//	CheckRadioButton(hWnd, IDC_COLLFLOOR,IDC_COLLFLOOR,IDC_COLLFLOOR);
				if (ref.flag & STRAT_COLLSTRAT)
					CheckDlgButton(hWnd, IDC_COLLSTRATC, 1);
				//	CheckRadioButton(hWnd, IDC_COLLSTRAT,IDC_COLLSTRAT,IDC_COLLSTRAT);

				//THESE SHOULD BE MUTUALLY EXCLUSIVE..TO DO
				if (ref.flag & STRAT_HOVER)
					CheckRadioButton(hWnd, IDC_HOVERMOVE,IDC_HOVERMOVE,IDC_HOVERMOVE);
				if (ref.flag & STRAT_PLANEMOVE)
					CheckRadioButton(hWnd, IDC_PLANEMOVE,IDC_PLANEMOVE,IDC_PLANEMOVE);
				
				
				
				if (ref.flag & STRAT_AGGRESSIVE)
					CheckRadioButton(hWnd, IDC_AGGRESSIVE,IDC_PATH,IDC_AGGRESSIVE);
				else
					if (ref.flag & STRAT_DEFENSIVE)
						CheckRadioButton(hWnd, IDC_AGGRESSIVE,IDC_PATH,IDC_DEFENSIVE);
					else
						if (ref.flag & STRAT_HOLDING)
						CheckRadioButton(hWnd, IDC_AGGRESSIVE,IDC_PATH,IDC_HOLDING);
					else		
						CheckRadioButton(hWnd, IDC_AGGRESSIVE,IDC_PATH,IDC_PATH);
				
				
				if (ref.wayflag & WAY_OFF)
					CheckRadioButton(hWnd, IDC_OFFPATH,IDC_OFFPATH,IDC_OFFPATH);
				else
					CheckRadioButton(hWnd, IDC_NORMPATH,IDC_NORMPATH,IDC_NORMPATH);
				
				//DISPLAY OUR PATH	
				if (ref.waypoint)
					SetDlgItemText (hWnd, IDC_PATHPICK1, ref.waypoint->GetName());

				//DISPLAY OUR TRIGGER POINT	
				if (ref.trigpoint)
					SetDlgItemText (hWnd, IDC_TRIGPOINT1, ref.trigpoint->GetName());

				//DISPLAY OUR EVENT POINT	
				if (ref.eventpoint)
					SetDlgItemText (hWnd, IDC_EVENTPOINT, ref.eventpoint->GetName());


				//DISPLAY OUR DELETION EVENT POINT	
				if (ref.deletioneventpoint)
					SetDlgItemText (hWnd, IDC_DELETIONEVENTPOINT, ref.deletioneventpoint->GetName());

				if (ref.actpoint[ref.actnum])
					SetDlgItemText (hWnd, IDC_ACTTRIGPOINT, ref.actpoint[ref.actnum]->GetName());


				//SET UP PARENT
				if (ref.parent)
					SetDlgItemText (hWnd, IDC_PARENT, ref.parent->GetName());





				PickMode = NOPICK;
				// Fall thru
				return TRUE;
				break;
		case WM_DESTROY:
			Update();

			//FREE OUR SPINNERS
			ReleaseISpinner(RDObjectParamDlg::delaySpinner);
			ReleaseISpinner(RDObjectParamDlg::nodeSpinner);
			ReleaseISpinner(RDObjectParamDlg::routeSpinner);
			ReleaseISpinner(RDObjectParamDlg::healthSpinner);
			ReleaseISpinner(RDObjectParamDlg::armourSpinner);
			ReleaseISpinner(RDObjectParamDlg::intparamSpinner);
			ReleaseISpinner(RDObjectParamDlg::floatparamSpinner);

			RDObjectParamDlg::delaySpinner = RDObjectParamDlg::intparamSpinner = RDObjectParamDlg::floatparamSpinner =
			RDObjectParamDlg::routeSpinner = RDObjectParamDlg::nodeSpinner = RDObjectParamDlg::healthSpinner = RDObjectParamDlg::armourSpinner = NULL;

   /*			int loop;
			if (ref.intparam >=0)
			{
				for (loop=0;loop<(ref.intparam+1);loop++)
				{
				 	free(ref.intparams[loop].paramname);
					ref.intparams[loop].paramname=NULL;
				}
			}
			if (ref.floatparam >=0)
			{
				for (loop=0;loop<(ref.floatparam+1);loop++)
				{
					free(ref.floatparams[loop].paramname);
					ref.floatparams[loop].paramname=NULL;
				}
			}

	 */

			selectedModelName = "";
			return TRUE;
		case CC_SPINNER_CHANGE:
			switch (LOWORD(wParam))
			{

			case IDC_DELAYSPINNER:
				ref.delay	= RDObjectParamDlg::delaySpinner->GetIVal();
				return (TRUE);
				break;

			case IDC_ARMOURSPINNER:
				ref.armour	= RDObjectParamDlg::armourSpinner->GetFVal();
				return (TRUE);
				break;

			case IDC_HEALTHSPINNER:
				ref.health	= RDObjectParamDlg::healthSpinner->GetFVal();
				return (TRUE);
				break;

			case IDC_NODESPINNER:
				if (ref.waypoint)
				{
					ref.node = RDObjectParamDlg::nodeSpinner->GetIVal();
					SetUpNodeChoice(&ref,t,hWnd);
				}
				return (TRUE);
				break;

			case IDC_INTPARAMSPINNER:
				if (ref.intparam >= 0)
				{
					ref.intparams[ref.intparam].value.ival = RDObjectParamDlg::intparamSpinner->GetIVal();
					UpdateInternalVal(&ref.intparams[ref.intparam],MODEINT,ref.internalintmax,ref.Internalintparams);
				}
				break;

			case IDC_FLOATPARAMSPINNER:
				if (ref.floatparam >= 0)
				{
					ref.floatparams[ref.floatparam].value.fval = RDObjectParamDlg::floatparamSpinner->GetFVal();
					UpdateInternalVal(&ref.floatparams[ref.floatparam],MODEFLOAT,ref.internalfloatmax,ref.Internalfloatparams);
				}
				break;

			
			
			case IDC_ROUTESPINNER:
				if ((ref.waypoint) && (!(ref.entirenet)))
				{
					ref.path = RDObjectParamDlg::routeSpinner->GetIVal();
					ref.node = 0;
					SetUpNodeChoice(&ref,t,hWnd);
					if (RDObjectParamDlg::nodeSpinner)
						ReleaseISpinner(RDObjectParamDlg::nodeSpinner);
					RDObjectParamDlg::nodeSpinner = SetupIntSpinner (hWnd, IDC_NODESPINNER, IDC_NODE, 0, ref.nodemax, ref.node);

				   //	SetUpRouteChoice(&ref,t,hWnd);
				}  
				return (TRUE);
				break;

			}
  
			return TRUE;
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {

			case IDC_CLEARMODEL:
				ref.mesh.FreeAll();
				ref.objMesh.FreeAll();
			   	strcpy (ref.dlgObjSource,"NONE");
				strcpy (ref.objSource, ref.dlgObjSource);
			  //	ref.MeshLoaded = FALSE;
			  	SetDlgItemText (hWnd, IDC_STRAT_OBJECTS, ref.objSource);
				ObjLoad (ref.objSource, ref.objMesh, 0);
				ref.dlgDisplayType = RDObject::DISPLAY_BOX;
			 	ref.MeshLoaded=FALSE;
			// 	ref.SetValue (PB_DISPLAY, ref.ip->GetTime(), ref.DISPLAY_MESH);
			// 	ref.BuildMesh(ref.ip->GetTime());
			   	ref.BuildMesh(ref.ip->GetTime());
				break;
			
			
			case IDC_IFLOATDEF:
				if (ref.floatparam >=0)
				{
					ResetParams(&ref,FLOATDEFS);
					if (RDObjectParamDlg::floatparamSpinner)
						ReleaseISpinner(RDObjectParamDlg::floatparamSpinner);
					
					RDObjectParamDlg::floatparamSpinner = SetupFloatSpinner (hWnd, IDC_FLOATPARAMSPINNER, IDC_FLOATPARAM, -65536.0,65535.0 , ref.floatparams[ref.floatparam].value.fval);
				}
				break;



			case IDC_IINTDEF:
				if (ref.intparam >=0)
				{
					ResetParams(&ref,INTDEFS);
					if (RDObjectParamDlg::intparamSpinner)
						ReleaseISpinner(RDObjectParamDlg::intparamSpinner);
					
					RDObjectParamDlg::intparamSpinner = SetupIntSpinner (hWnd, IDC_INTPARAMSPINNER, IDC_INTPARAM, -65536,65535 , ref.intparams[ref.intparam].value.ival);
				}
				break;


			case IDC_IPARAMNUM:
				if (ref.intparam >=0)
				{
					ref.intparam++;
					if ((ref.intparam+1) > ref.intparammax)
						ref.intparam=0;
					SetDlgItemText (hWnd, IDC_IPARAM, ref.intparams[ref.intparam].paramname);
				  //	char num[4];
					itoa(ref.intparam,num,10);
					SetDlgItemText (hWnd, IDC_IPARAMNUM, num);

					if (RDObjectParamDlg::intparamSpinner)
						ReleaseISpinner(RDObjectParamDlg::intparamSpinner);
					
					RDObjectParamDlg::intparamSpinner = SetupIntSpinner (hWnd, IDC_INTPARAMSPINNER, IDC_INTPARAM, -65536,65535 , ref.intparams[ref.intparam].value.ival);
				}

				break;


			case IDC_FPARAMNUM:
				if (ref.floatparam >=0)
				{
					ref.floatparam++;
					if ((ref.floatparam+1) > ref.floatparammax)
						ref.floatparam=0;
					SetDlgItemText (hWnd, IDC_FPARAM, ref.floatparams[ref.floatparam].paramname);
					//char num[4];
					itoa(ref.floatparam,num,10);
					SetDlgItemText (hWnd, IDC_FPARAMNUM, num);

					if (RDObjectParamDlg::floatparamSpinner)
						ReleaseISpinner(RDObjectParamDlg::floatparamSpinner);
					
					RDObjectParamDlg::floatparamSpinner = SetupFloatSpinner (hWnd, IDC_FLOATPARAMSPINNER, IDC_FLOATPARAM, -65536.0,65535.0 , ref.floatparams[ref.floatparam].value.fval);
				}

				break;


			case IDC_ACTNUM:
			   //	char num[4];
			 	ref.actnum++;
				if (ref.actnum==5)
					ref.actnum=0;
				itoa(ref.actnum,num,10);
				SetDlgItemText (hWnd, IDC_ACTNUM, num);
				if (ref.actpoint[ref.actnum])
					SetDlgItemText (hWnd, IDC_ACTTRIGPOINT, ref.actpoint[ref.actnum]->GetName());
				else
					SetDlgItemText (hWnd, IDC_ACTTRIGPOINT, "Select Activation");
				break;

			case IDC_PARENT:
				if (ref.ip->GetCommandMode()->ID() == CID_STDPICK) 
				{
					int a=10;
					int b=20;
				}
				else
				{
					//SAVE THE DIALOGUE WINDOW HANDLE FOR LATER USE
					DiaSave = hWnd;
					PickMode=PARENT;
					ref.pickCB.pickObj = &ref;
					ref.ip->SetPickMode(&ref.pickCB);
				}
				break;

			case IDC_PATHPICK1:
				if (ref.ip->GetCommandMode()->ID() == CID_STDPICK) 
				{
					int a=10;
					int b=20;
				}
				else
				{
					//SAVE THE DIALOGUE WINDOW HANDLE FOR LATER USE
					DiaSave = hWnd;
					PickMode=WAYPOINT;
					ref.pickCB.pickObj = &ref;
					ref.ip->SetPickMode(&ref.pickCB);
				}
				break;

			case IDC_TRIGPOINT1:
				if (ref.ip->GetCommandMode()->ID() == CID_STDPICK) 
				{
					int a=10;
					int b=20;
				}
				else
				{
					//SAVE THE DIALOGUE WINDOW HANDLE FOR LATER USE
					DiaSave = hWnd;
					PickMode=TRIGPOINT;
					ref.pickCB.pickObj = &ref;
					ref.ip->SetPickMode(&ref.pickCB);
				}
				break;

			case IDC_EVENTPOINT:
				if (ref.ip->GetCommandMode()->ID() != CID_STDPICK) 
			  /*	{
					int a=10;
					int b=20;
				}
				else */
				{
					//SAVE THE DIALOGUE WINDOW HANDLE FOR LATER USE
					DiaSave = hWnd;
					PickMode=EVENTPOINT;
					ref.pickCB.pickObj = &ref;
					ref.ip->SetPickMode(&ref.pickCB);
				}
				break;
		  
			case IDC_DELETIONEVENTPOINT:
				if (ref.ip->GetCommandMode()->ID() != CID_STDPICK) 
			  /*	{
					int a=10;
					int b=20;
				}
				else */
				{
					//SAVE THE DIALOGUE WINDOW HANDLE FOR LATER USE
					DiaSave = hWnd;
					PickMode=DELETIONEVENTPOINT;
					ref.pickCB.pickObj = &ref;
					ref.ip->SetPickMode(&ref.pickCB);
				}
				break;


			case IDC_ACTTRIGPOINT:
				if (ref.ip->GetCommandMode()->ID() == CID_STDPICK) 
				{
					int a=10;
					int b=20;
				}
				else
				{
					//SAVE THE DIALOGUE WINDOW HANDLE FOR LATER USE
					DiaSave = hWnd;
					PickMode=ACTPOINT;
					ref.pickCB.pickObj = &ref;
					ref.ip->SetPickMode(&ref.pickCB);
				}
				break;

				//STILL DON'T SEE THE CORRECT PLACE TO PUT THIS WAZ
			case IDC_ENTIRENET:	 
				
				if (ref.waypoint)
				{
					ref.entirenet = !ref.entirenet;
					if (ref.entirenet)
					{
						//clear any info pertaining to starting patrol
						ref.path=-1;
						ref.pathmax=0;
					}
					else
					{
					 	ref.path=0;
						ref.pathmax=0;

						SetUpRouteChoice(&ref,t,hWnd);
					}
					SetUpNodeChoice(&ref,t,hWnd);
					if (RDObjectParamDlg::nodeSpinner)
						ReleaseISpinner(RDObjectParamDlg::nodeSpinner);
					if (RDObjectParamDlg::routeSpinner)
						ReleaseISpinner(RDObjectParamDlg::routeSpinner);
					RDObjectParamDlg::nodeSpinner = SetupIntSpinner (hWnd, IDC_NODESPINNER, IDC_NODE, 0, ref.nodemax, ref.node);
					RDObjectParamDlg::routeSpinner = SetupIntSpinner (hWnd, IDC_ROUTESPINNER, IDC_ROUTENUM, 0, ref.pathmax, ref.path);
				}
				return(TRUE);
				break;			
			case IDC_OWNROT:		
				ref.ownrot = !ref.ownrot;
				return(TRUE);
				break;			
			case IDC_OWNPOS:		
				ref.ownpos = !ref.ownpos;
				return(TRUE);
				break;			
			case IDC_FRIENDLY:		
				ref.flag &= (0xffffffff)-STRAT_ENEMY;
				ref.flag |= STRAT_FRIENDLY;


				return(TRUE);
				break;			
			case IDC_ENEMY:
				//DON'T ALLOW THE MAINPLAYER STRAT TO CHANGE TYPES
				if (strnicmp(ref.stratSource,"MAINPLAYER.DST",strlen(ref.stratSource)+1))
				{
					ref.flag &= (0xffffffff)-STRAT_FRIENDLY;
					ref.flag |= STRAT_ENEMY;
				}
				return(TRUE);
				break;
			case IDC_GRAVITYONC:
				ref.flag ^= STRAT_GRAVITYON;
				return(TRUE);
				break;
			case IDC_CALCRELVELC:				
				ref.flag ^= STRAT_CALCRELVEL;
				return(TRUE);
				break;
			case IDC_COLLFLOORC:
				ref.flag ^= STRAT_COLLFLOOR;
				return(TRUE);
				break;
			case IDC_COLLSTRATC:
				ref.flag ^= STRAT_COLLSTRAT;
				return(TRUE);
				break;

			case IDC_PLANEMOVE:
				ref.flag ^= STRAT_PLANEMOVE;
				return(TRUE);
				break;
			case IDC_HOVERMOVE:
				ref.flag ^= STRAT_HOVER;
				return(TRUE);
				break;

			case IDC_AGGRESSIVE:
				ref.flag &= (0xffffffff)-(STRAT_DEFENSIVE + STRAT_HOLDING);
				ref.flag |= STRAT_AGGRESSIVE;
				return(TRUE);
				break;
			case IDC_DEFENSIVE:
				ref.flag &= (0xffffffff)-(STRAT_AGGRESSIVE + STRAT_HOLDING);
				ref.flag |= STRAT_DEFENSIVE;
				return(TRUE);
				break;
			case IDC_HOLDING:
				ref.flag &= (0xffffffff)-(STRAT_AGGRESSIVE + STRAT_DEFENSIVE);
				ref.flag |= STRAT_HOLDING;
				return(TRUE);
				break;
			case IDC_PATH:
				ref.flag &= (0xffffffff)-(STRAT_AGGRESSIVE + STRAT_DEFENSIVE + STRAT_HOLDING);
				return(TRUE);
				break;

			case IDC_CLEARACT:
				if (ref.actpoint[ref.actnum])
				{
					ref.DeleteReference(4+ref.actnum);
					ref.SetReference(4+ref.actnum,NULL);
					ref.actpoint[ref.actnum]=NULL;
					SetDlgItemText (hWnd, IDC_ACTTRIGPOINT, "Select Activation");
				
				}
				
				return(TRUE);
				break;


			case IDC_CLEARPARENT:
				if (ref.parent)
				{
					ref.DeleteReference(10);
					ref.SetReference(10,NULL);
					ref.parent=NULL;
					SetDlgItemText (hWnd, IDC_PARENT, "ORPHAN");
				
				}
				
				return(TRUE);
				break;


			case IDC_CLEARWAY:
				if (ref.waypoint)
				{
					ref.DeleteReference(1);
					ref.SetReference(1,NULL);
					ref.waypoint=NULL;
					ref.wayflag=WAY_NORM;
					ref.path = -1;
					ref.path = 0;
					ref.pathmax = 0;
					ref.node = 0;
					ref.nodemax = 0;
					ref.entirenet = 0;
					CheckDlgButton(hWnd, IDC_ENTIRENET, 0);
					ref.ownrot = 0;
					ref.ownpos = 0;
					CheckDlgButton(hWnd, IDC_OWNROT, 0);
					CheckDlgButton(hWnd, IDC_OWNPOS, 0);
					//CLEAN UP ANY SET PATH REFERENCE
					if (ref.GetReference(3))
					{
						ref.DeleteReference(3);
						ref.SetReference(3,NULL);
					}
					if (RDObjectParamDlg::nodeSpinner)
						ReleaseISpinner(RDObjectParamDlg::nodeSpinner);

					if (RDObjectParamDlg::routeSpinner)
						ReleaseISpinner(RDObjectParamDlg::routeSpinner);

					RDObjectParamDlg::nodeSpinner = SetupIntSpinner (hWnd, IDC_NODESPINNER, IDC_NODE, 0, ref.nodemax, ref.node);
					RDObjectParamDlg::routeSpinner = SetupIntSpinner (hWnd, IDC_ROUTESPINNER, IDC_ROUTENUM, 0, ref.pathmax, ref.path);
					SetDlgItemText (hWnd, IDC_PATHPICK1, "Select Path");


				}	
				
				return(TRUE);
				break;


			case IDC_CLEARCREATE:
				if (ref.trigpoint)
				{
					ref.DeleteReference(2);
					ref.SetReference(2,NULL);
					ref.trigpoint=NULL;
					SetDlgItemText (hWnd, IDC_TRIGPOINT1, "Select Creation");
				}
				return(TRUE);
				break;


			case IDC_CLEAREVENT:
				if (ref.eventpoint)
				{
					ref.DeleteReference(9);
					ref.SetReference(9,NULL);
					ref.eventpoint=NULL;
					SetDlgItemText (hWnd, IDC_EVENTPOINT, "Select Event");
				}
				return(TRUE);
				break;

			case IDC_CLEARDELETIONEVENT:
				if (ref.deletioneventpoint)
				{
					ref.DeleteReference(11);
					ref.SetReference(11,NULL);
					ref.deletioneventpoint=NULL;
					SetDlgItemText (hWnd, IDC_DELETIONEVENTPOINT, "Select Deletion");
				}
				return(TRUE);
				break;


			
			
			case IDC_NORMPATH:
				if (ref.waypoint)	
				{
					ref.wayflag &= (0xffff)-WAY_OFF;
					ref.wayflag |= WAY_NORM;
					return(TRUE);
				}
				else
					return(FALSE);
				break;
			case IDC_OFFPATH:
				if (ref.waypoint)	
				{
					ref.wayflag &= (0xffff)-WAY_NORM;
					ref.wayflag |= WAY_OFF;
					return(TRUE);
				}
				else
					return(FALSE);
				break;

		
			case IDC_BOX: case IDC_DISPLAY_ARROW: case IDC_MESH:
				ref.BuildMesh (0);
				return (TRUE);
				
			case IDC_STRAT_OBJECTS:
			case IDC_OBJECT_TYPE:
				if (LOWORD(wParam) == IDC_OBJECT_TYPE)
					Id = STRATS;
				else
					Id = OBJECTS;
				int result = DialogBoxParam (hInstance, MAKEINTRESOURCE (IDD_CHOOSE_OBJECT),
				hWnd, dialogFunc, (LPARAM) ref.objSource);
				// Did the model name change?				
				if (result && selectedModelName != "") 
				{
					if (LOWORD(wParam) == IDC_OBJECT_TYPE)
					{
						// Set the button name to be the model pathname minus the object root (plus the \)
						strcpy (ref.stratSource, selectedModelName + strlen (STRAT_ROOT) + 1);
					   	UpdateNodeName(&ref,ref.stratSource);
						SetDlgItemText (hWnd, IDC_OBJECT_TYPE, ref.stratSource);
						//update our params
						FreeVals(&ref,ALL);
						RegenInternalList(&ref,ref.stratSource);

						ParamUpdate(&ref,ref.stratSource,hWnd);
			
						
						//CODE TO ENSURE THAT MAINPLAYER STRAT IS ALWAYS SET TO FRIENDLY
						if (!strnicmp(ref.stratSource,"MAINPLAYER.DST",strlen(ref.stratSource)+1))
						{
							ref.flag &= (0xffffffff)-STRAT_ENEMY;
							ref.flag |= STRAT_FRIENDLY;
							CheckRadioButton(hWnd, IDC_FRIENDLY,IDC_ENEMY,IDC_FRIENDLY);
						}
					
						//CODE TO ENSURE THAT CAMCONTROL STRATS HAVE CORRECT TYPE
						//if (!strnicmp(ref.stratSource,"CAMCONTROL.DST",strlen(ref.stratSource)+1))
						//	ref.Type = CAMCONTROL;					
					
				   
					}
					else
					{
						// Set the button name to be the model pathname minus the object root (plus the \)
						strcpy (ref.objSource, selectedModelName + strlen (STRAT_OBJECT_ROOT) + 1);
						SetDlgItemText (hWnd, IDC_STRAT_OBJECTS, ref.objSource);
						ref.MeshLoaded=FALSE;
						ref.SetValue (PB_DISPLAY, ref.ip->GetTime(), ref.DISPLAY_MESH);
						ref.BuildMesh(ref.ip->GetTime());
					}
					selectedModelName = "";
				}
				else
				{

					if (LOWORD(wParam) == IDC_OBJECT_TYPE)
					{
						SetDlgItemText (hWnd, IDC_OBJECT_TYPE, ref.stratSource);
						FreeVals(&ref,ALL);
						RegenInternalList(&ref,ref.stratSource);

						ParamUpdate(&ref,ref.stratSource,hWnd);
					}
					else
						SetDlgItemText (hWnd, IDC_STRAT_OBJECTS, ref.objSource);

				}
				return(TRUE);
			}
		}
		
		return FALSE;
	}

	void	Update (void)
	{
			ref.armour	= RDObjectParamDlg::armourSpinner->GetFVal();
			ref.health	= RDObjectParamDlg::healthSpinner->GetFVal();
	}

};

void RDObject::BeginEditParams (IObjParam *ip,ULONG flags,Animatable *prev)
{
	// We subclass off SimpleObject so we must call its
	// BeginEditParams() method first.
	SimpleObject::BeginEditParams (ip,flags,prev);
	// Save the interface pointer.
	this->ip = ip;
	
	if (pmapParam) {
		// Left over from last object ceated
		pmapParam->SetParamBlock(pblock);
	} else {
		// Create new pages
		
		pmapParam = CreateCPParamMap(
			descParam,PARAMDESC_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_PANEL),
			GetString (IDS_RB_PARAMETERS),
			0);
	}
	// Set up the dialog proc
	if (pmapParam)
		pmapParam->SetUserDlgProc (new RDObjectParamMapUserDlgProc (*this));
}

// This is called by the system to terminate the editing of the
// parameters in the command panel.  
void RDObject::EndEditParams (IObjParam *ip, ULONG flags,Animatable *next)
{		
	SimpleObject::EndEditParams (ip,flags,next);
	this->ip = NULL;
	
	// Save these values in class variables so the next object 
	// created will inherit them.
	int awesomeKludge;
	pblock->GetValue (PB_HAS_STRAT,		ip->GetTime(), dlgHasStrat,			FOREVER);
	pblock->GetValue (PB_DISPLAY,		ip->GetTime(), awesomeKludge,		FOREVER);
	dlgDisplayType = (DisplayType) awesomeKludge;


	//dlgFlag = flag;
	strcpy (dlgObjSource, objSource);
	strcpy (dlgStratSource, stratSource);

	if (flags & END_EDIT_REMOVEUI) {
		DestroyCPParamMap (pmapParam);
		pmapParam  = NULL;
	}
	DiaSave=NULL;

//	int loop;
   /*	if (intparam >=0)
	{
		for (loop=0;loop<(intparam+1);loop++)
//			free(paramints[loop]);
			free(intparams[loop].paramname);
	}
	if (floatparam >=0)
	{
		for (loop=0;loop<(floatparam+1);loop++)
//			free(paramfloats[loop]);
			free(floatparams[loop].paramname);
	}*/
}


void RDObject::InvalidateUI() 
{
	if (pmapParam) pmapParam->Invalidate();
}

/*
 * Setting and getting parameters
 */
BOOL RDObject::SetValue(int i, TimeValue t, float v) { return FALSE; }
BOOL RDObject::SetValue(int i, TimeValue t, Point3 &v) { return FALSE; }
BOOL RDObject::GetValue(int i, TimeValue t, float &v, Interval &ivalid) { return FALSE; }
BOOL RDObject::GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid) { return FALSE; }

BOOL RDObject::GetValue (int i, TimeValue t, int &v, Interval &ivalid)
{
	switch (i) {
	case PB_DISPLAY:
		v = (int) displayType;
		break;
	default:
		//return FALSE;
		break;
	}
	return TRUE;
}

BOOL RDObject::SetValue (int i, TimeValue t, int v)
{
	switch (i) {
	case PB_DISPLAY:
		displayType = (DisplayType) v;
		break;
	default:
		//return FALSE;
		break;
	}
	return TRUE;
}

ParamDimension *RDObject::GetParameterDim(int pbIndex) 
{
	switch (pbIndex) {
	case PB_HAS_STRAT:
		return stdWorldDim;
	case PB_DISPLAY:
		return stdWorldDim;
	default:
		return stdWorldDim;
	}
	return defaultDim;
}

TSTR RDObject::GetParameterName(int pbIndex) 
{
	switch (pbIndex) {
	case PB_HAS_STRAT:
		return TSTR(GetString(IDS_RB_HAS_STRAT));
	case PB_DISPLAY:
		return TSTR(GetString(IDS_RB_DISPLAY));
	default:
		return TSTR(GetString(IDS_RB_HAS_STRAT));
//		return TSTR(_T(""));
	}
}


BOOL PickOperand::HitTest(
		IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags)
{	
	int IdcType;

	//GET THE NODE AND DISPLAY IN DIALOGUE BOX IF VALID

	INode *node = ip->PickNode(hWnd,m,this);
	
	switch (PickMode)
	{
		case (WAYPOINT):
			IdcType = IDC_PATHPICK1;
			break;

		case (TRIGPOINT):
			IdcType = IDC_TRIGPOINT1;
			break;

		case (EVENTPOINT):
			IdcType = IDC_EVENTPOINT;
			break;
	   
		case (DELETIONEVENTPOINT):
			IdcType = IDC_DELETIONEVENTPOINT;
			break;


		case (ACTPOINT):
			IdcType = IDC_ACTTRIGPOINT;
			break;

		case (PARENT):
			IdcType = IDC_PARENT;
			break;
		default:
			break;
	}


   //	if (PickMode == WAYPOINT)
   //		IdcType = IDC_PATHPICK1;
   //	else
	//	IdcType = IDC_TRIGPOINT1;
	
	
	if (node) 
	{
		TCHAR *name = node->GetName();	

		SetDlgItemText (DiaSave, IdcType, node->GetName());


	}
	else
		if (PickMode == PARENT)
			SetDlgItemText (DiaSave, IdcType, "ORPHAN");
		else
			SetDlgItemText (DiaSave, IdcType, "Select Path");

	// Is it a Red Dog Object and part of the landscape?
	
	return node ? TRUE : FALSE;
}

BOOL PickOperand::RightClick(IObjParam *ip,ViewExp *vpt)
{
//	if (PickMode==WAYPOINT)
//		if (pickObj->waypoint)
//			SetDlgItemText (DiaSave, IDC_PATHPICK1, pickObj->waypoint->GetName());
//	else
//		if (pickObj->trigpoint)
//			SetDlgItemText (DiaSave, IDC_TRIGPOINT1, pickObj->trigpoint->GetName());

	switch (PickMode)
	{
		case (WAYPOINT):
			if (pickObj->waypoint)
				SetDlgItemText (DiaSave, IDC_PATHPICK1, pickObj->waypoint->GetName());
			break;

		case (TRIGPOINT):
			if (pickObj->trigpoint)
				SetDlgItemText (DiaSave, IDC_TRIGPOINT1, pickObj->trigpoint->GetName());
			break;

		case (EVENTPOINT):
			if (pickObj->eventpoint)
				SetDlgItemText (DiaSave, IDC_EVENTPOINT, pickObj->eventpoint->GetName());
			break;
	  
		case (DELETIONEVENTPOINT):
			if (pickObj->deletioneventpoint)
				SetDlgItemText (DiaSave, IDC_DELETIONEVENTPOINT, pickObj->deletioneventpoint->GetName());
			break;


		case (ACTPOINT):
			if (pickObj->actpoint[pickObj->actnum])
				SetDlgItemText (DiaSave, IDC_ACTTRIGPOINT, pickObj->actpoint[pickObj->actnum]->GetName());
			break;
		case (PARENT):
			if (pickObj->parent)
				SetDlgItemText (DiaSave, IDC_PARENT, pickObj->parent->GetName());
			break;

		default:
			break;
	}



	return (TRUE);
}


//extra checks on selection fro filtering out items picked from the MAX selection menu
BOOL PickOperand::Pick(IObjParam *ip,ViewExp *vpt)
{
	//Save the node and update the dialogue box
	
	INode *node = vpt->GetClosestHit();

	
	int a;
	switch (PickMode)
	{	
		case (WAYPOINT):
			if (node) 
			{

//				pickObj->waypoint=node;		
				SetDlgItemText (DiaSave, IDC_PATHPICK1, node->GetName());
				
				if (pickObj->GetReference(1))
					pickObj->ReplaceReference(1,node,1);
				else
					a=pickObj->MakeRefByID(FOREVER,1,node);
				pickObj->waypoint=(INode*)pickObj->GetReference(1);
 
  			   //	if (!(pickObj->entirenet))
				//	pickObj->path = 0;
			   //	else
					pickObj->path = -1;

					pickObj->path = 0;

				pickObj->entirenet = 1;
				CheckDlgButton(DiaSave, IDC_ENTIRENET, 1);

				SetUpNodeChoice(pickObj,ip->GetTime(),DiaSave);
				pickObj->node = 0;

				if (RDObjectParamDlg::nodeSpinner)
					ReleaseISpinner(RDObjectParamDlg::nodeSpinner);

				RDObjectParamDlg::nodeSpinner = SetupIntSpinner (DiaSave, IDC_NODESPINNER, IDC_NODE, 0, pickObj->nodemax, pickObj->node);

				SetUpRouteChoice(pickObj,ip->GetTime(),DiaSave);
				if (RDObjectParamDlg::routeSpinner)
					ReleaseISpinner(RDObjectParamDlg::routeSpinner);

				RDObjectParamDlg::routeSpinner = SetupIntSpinner (DiaSave, IDC_ROUTESPINNER, IDC_ROUTENUM, 0, pickObj->pathmax, pickObj->path);

				PickMode=NOPICK;
				a=REF_SUCCEED;
				return TRUE;
			}
			SetDlgItemText (DiaSave, IDC_PATHPICK1, "INVALID PATH");
			break;
		case (TRIGPOINT):
			if (node) 
			{

	  //			pickObj->trigpoint=node;		
				//valid selection ? ie: of circle class
				Object *obj = node->EvalWorldState(pickObj->ip->GetTime()).obj;
				if 	(obj->ClassID() == Class_ID(CIRCLE_CLASS_ID,0))
				{
			   
					SetDlgItemText (DiaSave, IDC_TRIGPOINT1, node->GetName());
				
					if (pickObj->GetReference(2))
						pickObj->ReplaceReference(2,node,1);
					else
						a=pickObj->MakeRefByID(FOREVER,2,node);
					pickObj->trigpoint=(INode*)pickObj->GetReference(2);
				}
				PickMode=NOPICK;
				a=REF_SUCCEED;
				return TRUE;
			}
			SetDlgItemText (DiaSave, IDC_TRIGPOINT1, "INVALID PATH");
			break;

		case (EVENTPOINT):
			if (node) 
			{

	  //			pickObj->trigpoint=node;		
				if (!(strnicmp(node->GetName(),"EVENT",5)))
				{
					SetDlgItemText (DiaSave, IDC_EVENTPOINT, node->GetName());
				
					if (pickObj->GetReference(9))
						pickObj->ReplaceReference(9,node,1);
					else
						a=pickObj->MakeRefByID(FOREVER,9,node);
					pickObj->eventpoint=(INode*)pickObj->GetReference(9);
				}
				PickMode=NOPICK;
				a=REF_SUCCEED;
				return TRUE;
			}
			SetDlgItemText (DiaSave, IDC_EVENTPOINT, "INVALID PATH");
			break;
	   
		case (DELETIONEVENTPOINT):
			if (node) 
			{

	  //			pickObj->trigpoint=node;		
		 		//ensure a correctly named event is picked
				if (!(strnicmp(node->GetName(),"EVENT",5)))
				{
					SetDlgItemText (DiaSave, IDC_DELETIONEVENTPOINT, node->GetName());
				
					if (pickObj->GetReference(11))
						pickObj->ReplaceReference(11,node,1);
					else
						a=pickObj->MakeRefByID(FOREVER,11,node);
					pickObj->deletioneventpoint=(INode*)pickObj->GetReference(11);
				}
				PickMode=NOPICK;
				a=REF_SUCCEED;
				return TRUE;
			}
			SetDlgItemText (DiaSave, IDC_DELETIONEVENTPOINT, "INVALID PATH");
			break;


		case (PARENT):
			if (node) 
			{

	  //			pickObj->trigpoint=node;		
				SetDlgItemText (DiaSave, IDC_PARENT, node->GetName());
				
				if (pickObj->GetReference(10))
					pickObj->ReplaceReference(10,node,1);
				else
					a=pickObj->MakeRefByID(FOREVER,10,node);
				pickObj->parent=(INode*)pickObj->GetReference(10);
			
				PickMode=NOPICK;
				a=REF_SUCCEED;
				return TRUE;
			}
			SetDlgItemText (DiaSave, IDC_PARENT, "INVALID PATH");
			break;

		case (ACTPOINT):
			if (node) 
			{

		//		pickObj->actpoint=node;		
				SetDlgItemText (DiaSave, IDC_ACTTRIGPOINT, node->GetName());
				
				if (pickObj->GetReference(4+pickObj->actnum))
					pickObj->ReplaceReference(4+pickObj->actnum,node,1);
				else
					a=pickObj->MakeRefByID(FOREVER,4+pickObj->actnum,node);
				pickObj->actpoint[pickObj->actnum]=(INode*)pickObj->GetReference(4+pickObj->actnum);
			
				PickMode=NOPICK;
				a=REF_SUCCEED;
				return TRUE;
			}
			SetDlgItemText (DiaSave, IDC_ACTTRIGPOINT, "INVALID PATH");
			break;


	}	
	
	
	
	return TRUE;
}



BOOL validpick;

void ValidatePick(INode *node,int parentvalid, INode *requested)
{
		int i,numchild;
		INode *oldnode=node;
		
		numchild = node->NumberOfChildren();

 	if (node == requested)
	   	{
  			if (parentvalid)
				validpick = FALSE;
			else
				validpick = TRUE;
		}

		for (i=0;i<numchild;i++)
		{
		   
			if (!(strnicmp(oldnode->GetName(),"network",7)))
				parentvalid = 1;
			else
				parentvalid = 0;

			node = oldnode->GetChildNode(i);
			ValidatePick(node,parentvalid,requested);
		}

	   //	parentvalid = 0;
		
}



//APPLIES A FILTER TO ONLY ALLOW PATHS TO BE PICKED
BOOL PickOperand::Filter(INode *node)
{
   	int res = FALSE;
	
	switch (PickMode)
	{
		case (WAYPOINT):
		{
			if (node)
			{
				Object *obj = node->EvalWorldState(pickObj->ip->GetTime()).obj;
			  	if ((obj->SuperClassID() == SHAPE_CLASS_ID) &&
			  		(obj->ClassID() != Class_ID(CIRCLE_CLASS_ID,0)))
				
			  //	if (obj->ClassID() == BBOXSPLINE_CLASS_ID)
			   	{ 
				 	//ensure it's not a patrol route connected to a network
					INode *newnode = GetCOREInterface()->GetRootNode();
					
					validpick = FALSE;
					ValidatePick(newnode,0,node);
					res = validpick;
					//res = TRUE;
				}
			}
		break;

		}
		case (PARENT):
		{
			if (node)
			{
				Object *obj = node->EvalWorldState(pickObj->ip->GetTime()).obj;
				if (obj->ClassID() == RDOBJECT_CLASS_ID)
						res = TRUE;
			}

			break;
		}
		
		default:
			if (node)
			{
				Object *obj = node->EvalWorldState(pickObj->ip->GetTime()).obj;
				if (obj->ClassID() == Class_ID(CIRCLE_CLASS_ID,0))
						res = TRUE;
			}
			break;

	}
  
	return(res);

/*	
	if (PickMode == WAYPOINT)
	{								  
		if (node) 
		{	
			Object *obj = node->EvalWorldState(pickObj->ip->GetTime()).obj;
			if ((obj->SuperClassID() == SHAPE_CLASS_ID) &&
				(obj->ClassID() != Class_ID(CIRCLE_CLASS_ID,0)))

				return TRUE;
		}
	}
	else
	{

		//trigger point being picked circle/radius action

		if (node) 
		{	
			Object *obj = node->EvalWorldState(pickObj->ip->GetTime()).obj;
			if (obj->ClassID() == Class_ID(CIRCLE_CLASS_ID,0))
				return TRUE;
		}

	}
	return FALSE;
*/
}

void PickOperand::EnterMode(IObjParam *ip)
{
}

void PickOperand::ExitMode(IObjParam *ip)
{
}


/*
 * Conversions
 */
int RDObject::CanConvertToType (Class_ID obtype)
{
	return 0;
}

Object *RDObject::ConvertToType (TimeValue t, Class_ID obtype)
{
	return NULL;
}

void RDObject::GetCollapseTypes(Tab<Class_ID> &clist,Tab<TSTR*> &nlist)
{
	return;
}


/*
 * Cloning (via references)
 */
RefTargetHandle RDObject::Clone (RemapDir& remap)
{
	RDObject *newob = new RDObject();
	newob->SetReference(1,NULL);
	newob->SetReference(2,NULL);
	newob->SetReference(3,NULL);
	newob->SetReference(4,NULL);
	newob->SetReference(5,NULL);
	newob->SetReference(6,NULL);
	newob->SetReference(7,NULL);
	newob->SetReference(8,NULL);
	newob->SetReference(9,NULL);
	newob->SetReference(10,NULL);
	newob->SetReference(11,NULL);


	// Copy any instance vars across
	strcpy (newob->stratSource, stratSource);
	newob->flag = flag;
	newob->MeshLoaded = MeshLoaded;
	newob->dlgDisplayType = dlgDisplayType;
	newob->displayType = displayType;
	newob->entirenet = entirenet;
	newob->waypoint = waypoint;

	int res;
	
   	if (newob->waypoint)
   		res = newob->MakeRefByID(FOREVER,1,newob->waypoint);

	newob->trigpoint = trigpoint;
   	if (newob->trigpoint)
		res = newob->MakeRefByID(FOREVER,2,newob->trigpoint);

	newob->eventpoint = eventpoint;
   	if (newob->eventpoint)
		res = newob->MakeRefByID(FOREVER,9,newob->eventpoint);
   
	newob->deletioneventpoint = deletioneventpoint;
   	if (newob->deletioneventpoint)
		res = newob->MakeRefByID(FOREVER,11,newob->deletioneventpoint);


	newob->parent = parent;
   	if (newob->parent)
		res = newob->MakeRefByID(FOREVER,10,newob->parent);

	newob->actnum = actnum;
	for (res=0;res<5;res++)
	{
		newob->actpoint[res] = actpoint[res];

		if (newob->actpoint[res])
			newob->MakeRefByID(FOREVER,4+res,newob->actpoint[res]);
	}

  //	for (res=0;res<MAX_PARAMS;res++)
  //	{
  //		intparams[res].paramname=NULL;
  //		floatparams[res].paramname=NULL;
  //	}


	newob->path = path;
	newob->node = node;
						
	
	strcpy (newob->objSource, objSource);
//	newob->hasStrat = hasStrat;
	newob->BuildMesh (-1);


	newob->ownrot = ownrot;
	newob->ownpos = ownpos;
	newob->floatparam = floatparam;
	newob->intparam = intparam;
	newob->floatparammax = floatparammax;
	newob->intparammax = intparammax;

/*	int len,loop;
  	for (loop=0;loop<newob->floatparammax;loop++)
	{
		len = strlen(floatparams[loop].paramname)+1;
		newob->floatparams[loop].paramname = (char*)malloc(len);
		memcpy(newob->floatparams[loop].paramname,floatparams[loop].paramname,len);
		newob->floatparams[loop].value.fval = 
		newob->floatparams[loop].def.fval = floatparams[loop].def.fval;

	}


	for (loop=0;loop<newob->intparammax;loop++)
	{
		len = strlen(intparams[loop].paramname)+1;
		newob->intparams[loop].paramname = (char*)malloc(len);
		memcpy(newob->intparams[loop].paramname,intparams[loop].paramname,len);
		newob->intparams[loop].value.ival = 
		newob->intparams[loop].def.ival = intparams[loop].def.ival;

	}*/

		 //matt
	RegenInternalList(newob,newob->stratSource);
	SetupMaxParams(newob,newob->stratSource,0,NULL);



	delay = 0;



	//ObjLoad (objSource, newob->objMesh,0);
	return newob;
}


//TO TEST TOMMORROW 
RefTargetHandle RDObject::GetReference (int i)
{
	switch (i)
	{
		case (0):
			return (pblock);
		case (1):
			return (waypoint);
		case (2):
			return (trigpoint);
		case (3):
			return (netpath);
		case (4):
			return (actpoint[0]);
		case (5):
			return (actpoint[1]);
		case (6):
			return (actpoint[2]);
		case (7):
			return (actpoint[3]);
		case (8):
			return (actpoint[4]);
		case (9):
			return (eventpoint);
		case (10):
			return (parent);
		case (11):
			return (deletioneventpoint);
		default:
		//	break;
			return (NULL);
	}
}

void RDObject::SetReference(int i, RefTargetHandle rtarg)
{
	switch (i)
	{
		case (0):
			pblock = (IParamBlock*)rtarg;
			break;
		case (1):
			waypoint = (INode*)rtarg;
			break;
		case (2):
			trigpoint = (INode*)rtarg;
			break;
		case (3):
			netpath = (INode*)rtarg;
			break;
		case (4):
			actpoint[0] = (INode*)rtarg;
			break;
		case (5):
			actpoint[1] = (INode*)rtarg;
			break;
		case (6):
			actpoint[2] = (INode*)rtarg;
			break;
		case (7):
			actpoint[3] = (INode*)rtarg;
			break;
		case (8):
			actpoint[4] = (INode*)rtarg;
			break;
		case (9):
			eventpoint = (INode*)rtarg;
			break;
		case (10):
			parent = (INode*)rtarg;
			break;
		case (11):
			deletioneventpoint = (INode*)rtarg;
			break;
		default:
			break;;
	}
}		


//REEVALUATES THE POSITIONS AND LINKS TO NEWTWORKS, NODES AND PATROL ROUTES FOR EACH RDOBJECT
//THE DELETION OF A PATROL ROUTE USED BY AT LEAST ONE RDOBJECT

void NetworkValidate(INode *node)
{
	int i,numchild;
	INode *oldnode=node;
		
	numchild = node->NumberOfChildren();

  	Object *object = node->GetObjectRef();
   	if ((object) && (object->ClassID() == RDOBJECT_CLASS_ID))
   	{
   		RDObject *RDObj = (RDObject*)object;
   		if (RDObj->waypoint)
		{
	   		RDObj->StoreTransform = node->GetObjectTM(GetCOREInterface()->GetTime());
	   		SetUpNodeChoice(RDObj,0,DiaSave);

		}
	}
	for (i=0;i<numchild;i++)
	{
		node = oldnode->GetChildNode(i);
		NetworkValidate(node);
	}

}




RefResult RDObject::NotifyRefChanged(Interval i, RefTargetHandle rtarg, PartID& partID, RefMessage message)
{
	INode *node;
	if (message == REFMSG_TARGET_DELETED)
	{
		if ((INode*)rtarg == waypoint)	
		{
		   //	node = ReturnMyNode(this);
			node = this->GetMyNode();
			node->SetNodeTM(GetCOREInterface()->GetTime(),StoreTransform);


			waypoint = NULL;
			wayflag=WAY_NORM;
			path = -1;
			path = 0;
			pathmax = 0;
			this->node = 0;
			nodemax = 0;
			entirenet = 0;
			ownrot = 0;
			ownpos = 0;
			if (DiaSave)
			{
				CheckDlgButton(DiaSave, IDC_ENTIRENET, 0);
				CheckDlgButton(DiaSave, IDC_OWNROT, 0);
				CheckDlgButton(DiaSave, IDC_OWNPOS, 0);
			}
		  //	DeleteReference(1);		//DELETE THE WAYPATH REFERENCE
		  	if (GetReference(3))
		  		DeleteReference(3);		//DELETE THE PATROL REFERENCE, IF IT HAS ONE
	 		netpath = NULL;

		}

		if ((INode*)rtarg == netpath)	
		{
			//waypoint = NULL;
			//wayflag=WAY_NORM;
			path = -1;
			path=0;
			pathmax = 0;
			this->node = 0;
			nodemax = 0;
	 		netpath = NULL;
		  	if (GetReference(3))
				DeleteReference(3);		//DELETE THE PATROL REFERENCE

		   	//ADDED THE BELOW STUFF, SAT 20TH JUNE '99
			//ENSURE I'M RESET TO USE THE ENTIRE NETWORK
			entirenet = 1;
			//paranoia setupnodechoice for others on this network
			INode *newnode = GetCOREInterface()->GetRootNode();
			NetworkValidate(newnode);
			//if (DiaSave)
			//	CheckDlgButton(DiaSave, IDC_ENTIRENET, 1);


		}

		if ((INode*)rtarg == trigpoint)	
			trigpoint = NULL;

		if ((INode*)rtarg == parent)	
			parent = NULL;

		if ((INode*)rtarg == eventpoint)	
			eventpoint = NULL;
	  
		if ((INode*)rtarg == deletioneventpoint)	
			deletioneventpoint = NULL;


		//LOOP 0 T0 5 NEEDED HERE
		int loop;
		for (loop=0;loop<5;loop++)
		if ((INode*)rtarg == actpoint[loop])	
			actpoint[loop] = NULL;
		
	}
   	else
	{
	if (message == REFMSG_CHANGE)
	{
		if (
		//   	((INode*)rtarg == netpath) || ((INode*)rtarg == waypoint))	
		 	((INode*)rtarg == waypoint))							  
			{
				if (rtarg)
				{
				  //	node = ReturnMyNode(this);
					node = this->GetMyNode();
					//ensure check as undelete = this happens before node gets created
				  	if (node)
					{
						StoreTransform = node->GetObjectTM(GetCOREInterface()->GetTime());
						SetUpNodeChoice(this,0,DiaSave);
					}
				}
			}
		}
	}
	
	return(REF_SUCCEED);
}		

/*
 * Loading and saving of helpers
 */

#define CHUNKID_OBJ_SOURCE		0x1100
#define CHUNKID_STRAT_SOURCE	0x1101
#define CHUNKID_FLAG_SOURCE		0x1110
#define CHUNKID_WAYFLAG_SOURCE	0x1111
#define CHUNKID_ARMOUR			0x1000
#define CHUNKID_HEALTH			0x1001
#define CHUNKID_STARTNODE	   	0x1010
#define CHUNKID_STARTPATH	   	0x1011
#define CHUNKID_ENTIRENET	   	0xffff
#define CHUNKID_OWNROT		   	0xfff0
#define CHUNKID_IPARAMSNUM	   	0xff00
#define CHUNKID_IPARAMS		   	0xf000
#define CHUNKID_FPARAMSNUM	   	0xeeee
#define CHUNKID_FPARAMS		   	0xeee0
#define CHUNKID_TIMER		   	0xee00
#define CHUNKID_INTERNAL_FPARAMS 0xe000
#define CHUNKID_INTERNAL_IPARAMS 0xdddd
#define CHUNKID_OWNPOS			 0xddd0

IOResult RDObject::Load (ILoad *iload)
{
	ULONG nb;
	int i,len;
	IOResult res = IO_OK;
	
	while (IO_OK==(res=iload->OpenChunk())) {

		switch (iload->CurChunkID()) {
			// Check instance var chunks and load 'em here
		case	CHUNKID_INTERNAL_IPARAMS:
			//the number of internal params
			res = iload->Read(&internalintmax,sizeof(int),&nb);
			for (i=0;i<internalintmax;i++)
			{
				//THE VALUE
				res = iload->Read(&Internalintparams[i].value.ival,sizeof(int),&nb);
				//THE LENGTH OF THE ATTACHED CHARACTER NAME STRING
				res = iload->Read(&len,sizeof(int),&nb);
		   
				//THE STRING ITSELF
				Internalintparams[i].paramname = (char*)malloc(sizeof(char)*len);
				if (Internalintparams[i].paramname)
					res = iload->Read(Internalintparams[i].paramname,sizeof(char) * len,&nb);
				if (len > 32)
					int test = 1;
			  //	free(Internalintparams[i].paramname);
			  //	Internalintparams[i].paramname = NULL;
			}
			break;

		case	CHUNKID_INTERNAL_FPARAMS:
			//the number of internal params
			res = iload->Read(&internalfloatmax,sizeof(int),&nb);
			for (i=0;i<internalfloatmax;i++)
			{
				//THE VALUE
				res = iload->Read(&Internalfloatparams[i].value.fval,sizeof(float),&nb);
				//THE LENGTH OF THE ATTACHED CHARACTER NAME STRING
				res = iload->Read(&len,sizeof(int),&nb);
				//THE STRING ITSELF
				Internalfloatparams[i].paramname = (char*)malloc(sizeof(char)*len);
				if (Internalfloatparams[i].paramname)
					res = iload->Read(Internalfloatparams[i].paramname,sizeof(char) * len,&nb);
		 		if (len > 32)
					int test = 1;
			//	free(Internalfloatparams[i].paramname);
			//	Internalfloatparams[i].paramname = NULL;
			}
			break;

		case CHUNKID_OBJ_SOURCE:
			res = iload->Read(&objSource,sizeof(objSource),&nb);
		//	if (strlen(objSource) > 200)
		//		int test = 1;
//			ObjLoad (objSource, objMesh,0);
			break;
		case CHUNKID_STRAT_SOURCE:
			res = iload->Read(&stratSource,sizeof(stratSource),&nb);
	  //		if (strlen(stratSource) > 200)
	  //			int test = 1;
			
			
			SetupMaxParams(this,stratSource,LOADING,NULL);
			//CODE TO ENSURE THAT MAINPLAYER STRAT IS ALWAYS SET TO FRIENDLY
			if (!strnicmp(stratSource,"MAINPLAYER.DST",strlen(stratSource)+1))
			{
				flag &= (0xffffffff)-STRAT_ENEMY;
				flag |= STRAT_FRIENDLY;
			}
			


			//CODE TO ENSURE THAT CAMCONTROL STRATS HAVE CORRECT TYPE
			//if (!strnicmp(stratSource,"CAMCONTROL.DST",strlen(stratSource)+1))
			//	Type = CAMCONTROL;					
							
			break;
		case CHUNKID_FLAG_SOURCE:
			res = iload->Read(&flag,sizeof(int),&nb);
			//THE NEW DEFAULT SETTING IS ENEMY, SO ENSURE OLD FRIENDLY STRATS
			//ARE STILL CORRECT
			if (flag & STRAT_FRIENDLY)
				flag = flag & (0xffffffff - STRAT_ENEMY);
			
			break;
		case CHUNKID_WAYFLAG_SOURCE:
			res = iload->Read(&wayflag,sizeof(int),&nb);
			break;
		case CHUNKID_ARMOUR:
			res = iload->Read(&armour,sizeof(float),&nb);
			break;
		case CHUNKID_HEALTH:
			res = iload->Read(&health,sizeof(float),&nb);
			break;

		case CHUNKID_STARTNODE:
			res = iload->Read(&node,sizeof(int),&nb);
			break;

		case CHUNKID_STARTPATH:
			res = iload->Read(&path,sizeof(int),&nb);
			break;

		case CHUNKID_ENTIRENET:
			res = iload->Read(&entirenet,sizeof(int),&nb);
			break;

		case CHUNKID_OWNROT:
			res = iload->Read(&ownrot,sizeof(int),&nb);
			break;
		case CHUNKID_OWNPOS:
			res = iload->Read(&ownpos,sizeof(int),&nb);
			break;
		//case CHUNKID_IPARAMSNUM:
		//	res = iload->Read(&intparammax,sizeof(int),&nb);
		//	break;
		//case CHUNKID_IPARAMS:
		//	for (i=0;i<intparammax;i++)
		//		res = iload->Read(&intparams[i].value.ival,sizeof(int),&nb);
		//	break;
			
	   //	case CHUNKID_FPARAMSNUM:
	   //		res = iload->Read(&floatparammax,sizeof(int),&nb);
	   //		break;
	   //	case CHUNKID_FPARAMS:
		//	for (i=0;i<floatparammax;i++)
		//		res = iload->Read(&floatparams[i].value.fval,sizeof(int),&nb);
		//	break;
		case CHUNKID_TIMER:
			res = iload->Read(&delay,sizeof(short),&nb);
			break;
			
		}
		
		res = iload->CloseChunk();
		if (res!=IO_OK)  return res;
	}
	
	return IO_OK;
}

IOResult RDObject::Save (ISave *isave)
{
	// Save instance vars here
	ULONG nb;
	int len,i;
  //	int test;
	
	isave->BeginChunk(CHUNKID_INTERNAL_IPARAMS);
   //		   	if (!strnicmp(stratSource,"BOMBER.DST",strlen(stratSource)+1))
  //		   		test = 1;
	//the number of internal params
	isave->Write(&intparammax,sizeof(int),&nb);
	for (i=0;i<intparammax;i++)
	{
	 	//THE VALUE
		isave->Write(&intparams[i].value.ival,sizeof(int),&nb);
		//THE LENGTH OF THE ATTACHED CHARACTER NAME STRING
 		len = strlen(intparams[i].paramname)+1;
		isave->Write(&len,sizeof(int),&nb);
		//THE STRING ITSELF
		isave->Write(intparams[i].paramname,sizeof(char)*len,&nb);

	}
	isave->EndChunk();

	isave->BeginChunk(CHUNKID_INTERNAL_FPARAMS);
	//the number of internal params
	isave->Write(&floatparammax,sizeof(int),&nb);
	for (i=0;i<floatparammax;i++)
	{
	 	//THE VALUE
		isave->Write(&floatparams[i].value.fval,sizeof(float),&nb);
		//THE LENGTH OF THE ATTACHED CHARACTER NAME STRING
 		len = strlen(floatparams[i].paramname)+1;
		isave->Write(&len,sizeof(int),&nb);
		//THE STRING ITSELF
		isave->Write(floatparams[i].paramname,sizeof(char)*len,&nb);

	}
	isave->EndChunk();

	isave->BeginChunk(CHUNKID_OBJ_SOURCE);
	isave->Write(&objSource,sizeof(objSource),&nb);
	isave->EndChunk();

	isave->BeginChunk(CHUNKID_STRAT_SOURCE);
	isave->Write(&stratSource,sizeof(stratSource),&nb);
	isave->EndChunk();
	
	isave->BeginChunk(CHUNKID_FLAG_SOURCE);
	isave->Write(&flag,sizeof(int),&nb);
	isave->EndChunk();

	isave->BeginChunk(CHUNKID_WAYFLAG_SOURCE);
	isave->Write(&flag,sizeof(int),&nb);
	isave->EndChunk();

	isave->BeginChunk(CHUNKID_ARMOUR);
	isave->Write(&armour,sizeof(float),&nb);
	isave->EndChunk();

	isave->BeginChunk(CHUNKID_HEALTH);
	isave->Write(&health,sizeof(float),&nb);
	isave->EndChunk();

	isave->BeginChunk(CHUNKID_STARTNODE);
	isave->Write(&node,sizeof(int),&nb);
	isave->EndChunk();

	isave->BeginChunk(CHUNKID_STARTPATH);
	isave->Write(&path,sizeof(int),&nb);
	isave->EndChunk();

	isave->BeginChunk(CHUNKID_ENTIRENET);
	isave->Write(&entirenet,sizeof(int),&nb);
	isave->EndChunk();

	isave->BeginChunk(CHUNKID_OWNROT);
	isave->Write(&ownrot,sizeof(int),&nb);
	isave->EndChunk();

	isave->BeginChunk(CHUNKID_OWNPOS);
	isave->Write(&ownpos,sizeof(int),&nb);
	isave->EndChunk();


/*	isave->BeginChunk(CHUNKID_IPARAMSNUM);
	isave->Write(&intparammax,sizeof(int),&nb);
	isave->EndChunk();


	isave->BeginChunk(CHUNKID_IPARAMS);
	for (i=0;i<intparammax;i++)
		isave->Write(&intparams[i].value.ival,sizeof(int),&nb);
	isave->EndChunk();


	isave->BeginChunk(CHUNKID_FPARAMSNUM);
	isave->Write(&floatparammax,sizeof(int),&nb);
	isave->EndChunk();


	isave->BeginChunk(CHUNKID_FPARAMS);
	for (int loop=0;loop<floatparammax;loop++)
		isave->Write(&floatparams[i].value.fval,sizeof(float),&nb);
	isave->EndChunk();
  */
	isave->BeginChunk(CHUNKID_TIMER);
	isave->Write(&delay,sizeof(short),&nb);
	isave->EndChunk();



	return IO_OK;
}


/*************************************************************************************/

/*
 * Object exporter
 */

class RDObjectExporter : public SceneExport {
public:
	/*
     * MAX API
	 */
	// Returns the number of supported extensions
	virtual int ExtCount() { return 1; }
	// Returns the file extension for a given number
	virtual const TCHAR *Ext(int i) { assert(i==0); return _T("RDO"); };
	// Returns the short and long descriptions, and other strings
	virtual const TCHAR *LongDesc() { return _T("Argonaut Software Red Dog object"); }
	virtual const TCHAR *ShortDesc() { return _T("Red Dog object"); }
	virtual const TCHAR *AuthorName() { return _T("Argonaut Software"); }
	virtual const TCHAR *CopyrightMessage() { return _T("(c) 1998 Argonaut Software Ltd"); }
	virtual const TCHAR *OtherMessage1() { return _T(""); }
	virtual const TCHAR *OtherMessage2() { return _T(""); }
	// Returns the version of the plugin
	virtual unsigned int Version() { return RDO_VERSION; }
	// Show the About box
	virtual void ShowAbout (HWND parent) { return; }
	// Actually do the export
	#if MAX3
		virtual int DoExport (const TCHAR *name, ExpInterface *ei, Interface *i, BOOL suppressPrompts /* = FALSE */,DWORD options)
	#else
		virtual int DoExport (const TCHAR *name, ExpInterface *ei, Interface *i, BOOL suppressPrompts /* = FALSE */)
	#endif
	{
		FILE *output;
		// Hack to add convert materials
		if (!strcmp (name, "!CONVERT!.RDO")) {
 			output = NULL;
		} else {
			// Open the output stream
			output = fopen(name, "wb");
			
			// Check the output stream
			if (!output)
				return FALSE;
		}
		extern Point3 grossCOM;
		extern int stripPolys, stripStrip, numStrip;
		stripPolys = stripStrip = numStrip = 0;

		grossCOM = Point3(0,0,0);

		// Find the selected object(s) and save them
		ObjSave (output, ei->theScene, i);

		// Tack a pNode structure at the end of the file
		if (output) {
			RedDog::PNode node;
			memset (&node, 0, sizeof (node));
			node.obj			= (RedDog::Object *)1;
			node.mass			= ObjFlag::mass;
			node.radius			= ObjFlag::radius;
			node.collForceRatio	= ObjFlag::moi;
			node.flag		= ObjFlag::ownInstance ? 0 : OBJECT_SHARE_OBJECT_DATA;
			node.typeId			= ObjFlag::uid;
			node.com.x			= grossCOM.y;
			node.com.y			= -grossCOM.x;
			node.com.z			= grossCOM.z;
			node.expRot			= ObjFlag::expRot;

			node.groundFriction.x	= ObjFlag::friction.x;
			node.groundFriction.y	= ObjFlag::friction.y;
			node.groundFriction.z	= ObjFlag::friction.z;
			node.airFriction.x		= ObjFlag::airFriction.x;
			node.airFriction.y		= ObjFlag::airFriction.y;
			node.airFriction.z		= ObjFlag::airFriction.z;
			fwrite (&node, sizeof(node), 1, output);
			int len = ObjFlag::name.Length() + 1;
			fwrite (&len, sizeof (int), 1, output);
			fwrite ((char *)ObjFlag::name, 1, len, output);
		}
				
		// Close the output file
		if (output)
			fclose (output);
		
		FILE *fOut = fopen ("P:\\STRIP.TXT", "a+");
		if (fOut) {
			fprintf (fOut, "Model '%s' has %d polys and %d stripverts (%f ratio) - %f polys per strip\n", 
				GetCOREInterface()->GetCurFilePath(),
				stripPolys, stripStrip,
				(float)stripStrip / (float)stripPolys, (float)stripPolys / (float) numStrip);
			fclose (fOut);
		}
		
		return TRUE;
	}
};




/*
 * Red Dog object class description
 */

class RDObjectClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create (BOOL loading = FALSE) {return new RDObject;}
	const TCHAR *	ClassName() {return GetString(IDS_OBJECT_CLASS_NAME);}
	SClass_ID		SuperClassID() {return GEOMOBJECT_CLASS_ID;}
	Class_ID		ClassID() {return RDOBJECT_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_OBJECT_CATEGORY);}
	void			ResetClassParams (BOOL fileReset);
};

static RDObjectClassDesc RDObjectDesc;
ClassDesc* GetRDObjectDesc() {return &RDObjectDesc;}

//TODO: Should implement this method to reset the plugin params when Max is reset
void RDObjectClassDesc::ResetClassParams (BOOL fileReset) 
{
	RDObject::dlgHasStrat			= FALSE;
	RDObject::dlgDisplayType		= RDObject::DISPLAY_BOX;
	strcpy (RDObject::dlgObjSource,	"DUMMY");
	strcpy (RDObject::dlgStratSource,	"DUMMY");
	DiaSave = NULL;
}


/*
 * Red Dog object export class description
 */

class RDObjectExportClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create (BOOL loading = FALSE) {return new RDObjectExporter();}
	const TCHAR *	ClassName() {return GetString(IDS_OBJEXP_CLASS_NAME);}
	SClass_ID		SuperClassID() {return SCENE_EXPORT_CLASS_ID;}
	Class_ID		ClassID() {return RDOBJECTEXPORT_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_CATEGORY);}
	void			ResetClassParams (BOOL fileReset);
};

static RDObjectExportClassDesc RDObjectExportDesc;
ClassDesc* GetRDObjectExportDesc() {return &RDObjectExportDesc;}

//TODO: Should implement this method to reset the plugin params when Max is reset
void RDObjectExportClassDesc::ResetClassParams (BOOL fileReset) 
{

}

