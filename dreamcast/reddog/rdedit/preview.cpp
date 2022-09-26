/*
 * Preview.cpp
 * Previews a doobry wossname
 * Object, y'know
 * Also converts materials to the new red dog format
 */

#include "stdafx.h"
#include "Resource.h"
#include <process.h>
#include <sys/stat.h>
#include "RDstratmap.h"

#define PREVIEW_CLASS_ID Class_ID(0x39893e51, 0x215f55f9)

extern bool kludgeAllCalcMatrix;
extern BOOL GlobalCancelled;

static TCHAR levelName[128];
extern float MIN_BBOX_POLYS;
extern float MAX_BBOX_SIZE, MAX_POLY_DENSITY, BASE_CARVE_AMOUNT_X, BASE_CARVE_AMOUNT_Y;
static int ids[] = { IDC_EDIT1, IDC_EDIT2, IDC_EDIT3, IDC_EDIT4, IDC_EDIT5 };
static float *idPtr[] = { &MIN_BBOX_POLYS, &MAX_BBOX_SIZE, &MAX_POLY_DENSITY, &BASE_CARVE_AMOUNT_X, &BASE_CARVE_AMOUNT_Y };
static ICustEdit *salaryCtrl, *bonusCtrl, *bonusAmtCtrl;


static FILETIME lpCreationTime, lpLastAccessTime;

bool GetTheFileTime (const char *lpszFileName, FILETIME *lpLastWriteTime)
{
	HANDLE hFile = ::CreateFile(lpszFileName, GENERIC_READ|GENERIC_WRITE,
		FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
		NULL);
	
	if (hFile == INVALID_HANDLE_VALUE)
		return false;
	
	if (!GetFileTime((HANDLE)hFile, &lpCreationTime, &lpLastAccessTime, lpLastWriteTime))
		return false;
	
	if (!::CloseHandle(hFile))
		return false;

	return true;
}

void SetTheFileTime (const char *lpszFileName, const FILETIME *lpLastWriteTime)
{
	HANDLE hFile = ::CreateFile(lpszFileName, GENERIC_READ|GENERIC_WRITE,
		FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
		NULL);
	
	if (hFile == INVALID_HANDLE_VALUE)
		return;
	
	if (!SetFileTime((HANDLE)hFile, &lpCreationTime, &lpLastAccessTime, lpLastWriteTime))
		return;
	
	if (!::CloseHandle(hFile))
		return;
}

//FILTER TYPES
#define ALLSTRATS 0
#define STARTSTRATS 1
#define NONDELETEDSTRATS 2
int VIEWMODE = ALLSTRATS;
// Converts materials in a node
static void ConvertNode (INode *node)
{
	if (!node)
		return;
	Mtl *mat = node->GetMtl();
	if (mat) {
		if (mat->ClassID() == Class_ID (DMTL_CLASS_ID, 0)) {
			MGMaterial *equiv = new MGMaterial (false);
			equiv->setMat (mat);
			node->SetMtl(equiv);
		} else {
			if (mat->IsMultiMtl() && mat->NumSubMtls()) {
				for (int i = 0; i < mat->NumSubMtls(); ++i) {
					Mtl *sub = mat->GetSubMtl(i);
					if (sub && sub->ClassID() == Class_ID (DMTL_CLASS_ID, 0)) {
						MGMaterial *equiv = new MGMaterial (false);
						equiv->setMat (sub);
						mat->SetSubMtl(i, equiv);
					}
				}
			}
		}
	}
	for (int child = 0; child < node->NumberOfChildren(); ++child)
		ConvertNode (node->GetChildNode(child));
}

class PreviewClassDesc : public ClassDesc
{
public:
	int 			IsPublic() {return 1;}
	void *			Create (BOOL loading = FALSE) {return new Preview();}
	const TCHAR *	ClassName() {return GetString(IDS_PREVIEW_CLASS_NAME);}
	SClass_ID		SuperClassID() {return UTILITY_CLASS_ID;}
	Class_ID		ClassID() {return PREVIEW_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_PREVIEW_CATEGORY);}
	void			ResetClassParams (BOOL fileReset) { levelName[0] = '\0'; 
	MIN_BBOX_POLYS		= 200;
	MAX_BBOX_SIZE		= 15.f,
		MAX_POLY_DENSITY	= 0.3f,
		BASE_CARVE_AMOUNT_X = BASE_CARVE_AMOUNT_Y = 60.f;
	}
	BOOL			NeedsToSave () { return TRUE; }
	IOResult		Save (ISave *save)
	{
		ULONG nb;
		save->BeginChunk (1);
		save->WriteCString (levelName);
		save->EndChunk();
		save->BeginChunk (2);
		save->Write (&MIN_BBOX_POLYS, 4, &nb);
		save->Write (&MAX_BBOX_SIZE, 4, &nb);
		save->Write (&MAX_POLY_DENSITY, 4, &nb);
		save->Write (&BASE_CARVE_AMOUNT_X, 4, &nb);
		save->EndChunk();
		save->BeginChunk (3);
		save->Write (&BASE_CARVE_AMOUNT_Y, 4, &nb);
		save->EndChunk();
		return IO_OK;
	}
	IOResult		Load (ILoad *load)
	{
		IOResult res;
		ULONG nb;
		while ((res = load->OpenChunk()) == IO_OK) {
			switch (load->CurChunkID()) {
			case 1:
				char *foo;
				res = load->ReadCStringChunk (&foo);
				strcpy (levelName, foo);
		  	 	if (LEVNAMEBOX)
					LEVNAMEBOX->nameupdate (levelName);

				break;
			case 2:
				load->Read (&MIN_BBOX_POLYS, 4, &nb);
				load->Read (&MAX_BBOX_SIZE, 4, &nb);
				load->Read (&MAX_POLY_DENSITY, 4, &nb);
				load->Read (&BASE_CARVE_AMOUNT_X, 4, &nb);
		  	 	if (LEVNAMEBOX)
					LEVNAMEBOX->nameupdate (levelName);
				break;
			case 3:
				load->Read (&BASE_CARVE_AMOUNT_Y, 4, &nb);
				break;
			}
			load->CloseChunk();
			if (res != IO_OK)
				return res;
		}
		return res;
	}
};

static PreviewClassDesc aPreviewDesc;

ClassDesc *GetPreviewDesc()
{
	return &aPreviewDesc;
}

HWND		Preview::paramWindow;

HTREEITEM MAINTREE;
INode *NamedNode;




void FindNamedNode(INode *node,char *name)
{
	int i,numchild;
	INode *oldnode=node;
		
	numchild = node->NumberOfChildren();

	TCHAR *nodename = node->GetName();
	if 	(
		(!(strnicmp(nodename,name,strlen(nodename)))) &&
	    (!(strnicmp(name,nodename,strlen(name))))
	
		)
  	{
	  	NamedNode = node;
	  	return;
	}

	if (!numchild)
		return;

	for (i=0;i<numchild;i++)
	{
		node = oldnode->GetChildNode(i);
		FindNamedNode(node,name);
	}

}





void LookForLinks(INode *BaseNode,INode *node,HWND listBox, HTREEITEM after)
{
	TV_INSERTSTRUCT insert;
	int i,numchild;
	INode *oldnode=node;
  	char buf[256];
		
	numchild = node->NumberOfChildren();
	INode *parent = node->GetParentNode();


	if (parent==BaseNode)
  	{
		HTREEITEM tree;
		insert.hParent = after;
		insert.hInsertAfter = TVI_SORT;
		insert.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	   	sprintf(buf,"%s %s","DELETES STRAT :",(char*)node->GetName());
		insert.item.pszText = (TCHAR*)buf;

		insert.item.pszText = node->GetName();

		//insert.item.lParam = (LPARAM) after;
		insert.item.lParam = (LPARAM) node;
		insert.item.iImage = 2;
		insert.item.iSelectedImage = 1;
		after = 
			tree = TreeView_InsertItem (listBox, &insert);
		MAINTREE = tree;

	} 

	if (!numchild)
		return;

  	for (i=0;i<numchild;i++)
	{
		TCHAR *name=node->GetName();
		node = oldnode->GetChildNode(i);
		LookForLinks(BaseNode,node,listBox,after);
	}

}

void LookForDeletes(INode *BaseNode,INode *node,HWND listBox, HTREEITEM after)
{
	TV_INSERTSTRUCT insert;
	int i,numchild;
	INode *oldnode=node;
  //	char buf[64];
		
	numchild = node->NumberOfChildren();

  //	Object *object = node->EvalWorldState(GetCOREInterface()->GetTime()).obj;
	Object *object = node->GetObjectRef();
   //	object = node->GetObjOrWSMRef();

	if ((object) && (object->ClassID() == RDOBJECT_CLASS_ID))
  	{
	   	object = object->FindBaseObject();
		RDObject *rd = (RDObject*)object;


	if (rd->deletioneventpoint==BaseNode)
  	{
		HTREEITEM tree;
		insert.hParent = after;
		insert.hInsertAfter = TVI_SORT;
		insert.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
//	   	sprintf(buf,"%s %s","GENERATES STRAT :",(char*)node->GetName());
//		insert.item.pszText = (TCHAR*)buf;

		insert.item.pszText = node->GetName();

	   //	insert.item.lParam = (LPARAM) after;
		insert.item.lParam = (LPARAM) node;
		insert.item.iImage = 2;
		insert.item.iSelectedImage = 1;
		after = 
			tree = TreeView_InsertItem (listBox, &insert);
		MAINTREE = tree;

	} 
	}


  	for (i=0;i<numchild;i++)
	{
		TCHAR *name=node->GetName();
		node = oldnode->GetChildNode(i);
		LookForDeletes(BaseNode,node,listBox,after);
	}
	return;

}


//checks to see if a strat is created on an event occurring
static int AttachedToCreationEvent(RDObject *rd)
{
 	INode *node = rd->GetMyNode();

	INode *parent = node->GetParentNode();

	
	if (!parent)
		return(0);

	if (parent)
	{
//		TCHAR array1[512];

		TCHAR *name = parent->GetName();

		TCHAR* ArrPt = name;

		//GET RID OF LEADING SPACES

		 while (*ArrPt == ' ')
		 	ArrPt++;

	   //	 sscanf(ArrPt,"%s ",&array1);

	   //	 if (strlen(array1) > 256)
	   //		 int test=1;
		 if (!(strnicmp(ArrPt,"EVENT",5)))
			return (1);
	}
	return(0);


}

//returns TRUE if a strat has no creation trigger and is not activated on an event
//IE: IF THE STRAT IS ACTIVE FROM THE START

static int Filter(RDObject *rd)
{
	int Filter = 0;
	if ((!rd->trigpoint) && (!AttachedToCreationEvent(rd)))
		Filter = 1;

	return(Filter);


}

//returns TRUE if a strat is not connected to a deletion event

static int NonDeleted(RDObject *rd)
{
	int Filter = 0;
	if ((!rd->deletioneventpoint))
		Filter = 1;

	return(Filter);


}



void Prepare(INode *node,HWND listBox, HTREEITEM after)
{
	TV_INSERTSTRUCT insert;
	int i,numchild;
	int Valid;
	INode *oldnode=node;
		
	numchild = node->NumberOfChildren();

	Object *object = node->GetObjectRef();



	if (object)
	{
		object = object->FindBaseObject();
	 	if (object->ClassID() == RDOBJECT_CLASS_ID)
		{
			RDObject *rd = (RDObject*)object;
		
			HTREEITEM tree;

			if (VIEWMODE == ALLSTRATS)
				Valid = 1;
			else
			{
			 	if (VIEWMODE == NONDELETEDSTRATS)
					Valid = NonDeleted(rd);
			   	else
					Valid = Filter(rd);
			}

			if (Valid)
			{
				insert.hParent = after;
				insert.hInsertAfter = TVI_SORT;
				insert.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
				insert.item.pszText = node->GetName();
				//insert.item.lParam = (LPARAM) after;
			   	insert.item.lParam = (LPARAM) node;
				insert.item.iImage = 2;
				insert.item.iSelectedImage = 1;
				after = 
				tree = TreeView_InsertItem (listBox, &insert);
				//does the strat have an event ?
				if (rd->eventpoint)
				{	
					insert.hParent = after;
					insert.hInsertAfter = TVI_SORT;
					insert.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	  
					// 	sprintf(buf,"%s %s","LINKED TO EVENT :",(char*)rd->eventpoint->GetName());
					//insert.item.pszText = (TCHAR*)buf;
					insert.item.pszText = rd->eventpoint->GetName();
					//insert.item.lParam = (LPARAM) after;
			   		insert.item.lParam = (LPARAM) rd->eventpoint;
					insert.item.iImage = 2;
					insert.item.iSelectedImage = 1;
					after = 
					tree = TreeView_InsertItem (listBox, &insert);
					MAINTREE = tree;
					LookForLinks(rd->eventpoint,GetCOREInterface()->GetRootNode(),listBox,tree);
					LookForDeletes(rd->eventpoint,GetCOREInterface()->GetRootNode(),listBox,tree);
					tree = MAINTREE;
				}
			}
		} 
	}
	if (!numchild)
		return;

	for (i=0;i<numchild;i++)
	{
		node = oldnode->GetChildNode(i);
		Prepare(node,listBox,after);
	}
}


// Recursively gets the filename of a listbox item
static INode *GetItemName (HWND treeItem, char *buffer, HTREEITEM tree)
{
	TV_ITEM newInfo;
   	char textBuf[256];
	// Get the information from this tree node
	memset (&newInfo, 0, sizeof (newInfo));
	newInfo.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM;
	newInfo.hItem = tree;
	newInfo.pszText = &textBuf[0];
   	
	newInfo.cchTextMax = sizeof (textBuf);
	TreeView_GetItem (treeItem, &newInfo);
	
	strcat (buffer, textBuf);
	return ((INode*)newInfo.lParam);


}


static BOOL CALLBACK dialogFunc (HWND dBox, UINT msg, WPARAM wp, LPARAM lp)
{
	static HWND treeItem;
	static HIMAGELIST imageList;
	switch (msg) {
	case WM_INITDIALOG:
		
		
		treeItem = GetDlgItem (dBox, IDC_OBJECT_TREE);
	  	Prepare (GetCOREInterface()->GetRootNode(),treeItem, NULL);
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
	   //	case TVN_SELCHANGED:
			char textBuf[MAX_PATH];
			INode *node;
			
			textBuf[0]='\0';
			if (pnmh->code == TVN_SELCHANGED)
			{
				// Get a pointer to the tree event block
				NM_TREEVIEW FAR *pnmtv = (NM_TREEVIEW FAR *) pnmh;
				// Check for a valid selection
				node = GetItemName (treeItem, &textBuf[0], pnmtv->itemNew.hItem);
			} else
			{
				HTREEITEM tree;
				tree = TreeView_GetSelection (treeItem);
				node = GetItemName (treeItem, &textBuf[0], tree);
			}
			theHold.Begin();
			//FindNamedNode(GetCOREInterface()->GetRootNode(),textBuf);
			//INode *node = NamedNode;
		   	GetCOREInterface()->SelectNode(node);
			GetCOREInterface()->ExecuteMAXCommand(MAXCOM_ZOOMEXT_SEL);
			GetCOREInterface()->ExecuteMAXCommand(MAXCOM_MODIFY_MODE);
			TSTR undostr; undostr.printf("Select");
			theHold.Accept(undostr); 
			EndDialog (dBox, TRUE);
			return TRUE;




			return TRUE;
		}
	}
	return FALSE;
}

#if ARTIST
	#define WADDIR  "D:\\DREAMCAST\\WADS\\"
#else
#if GODMODEMACHINE
	#define WADDIR  "D:\\DREAMCAST\\WADS\\"
#else
	#define WADDIR  "C:\\DREAMCAST\\WADS\\"

#endif

#endif
#define SYSWADDIR  "P:\\WADS\\"

void MakeWadTxt(char *mapname)
{
	char DBFile[128];
	char line[128];
	FILE *wadfile;
	FILE *sysfile;


	sprintf(DBFile,"%s%s%s",WADDIR,"WAD",".TXT");	
	if (!(wadfile=fopen(DBFile, "w")))
		return;


	//BRING ACROSS THE SYSTEM ESSENTIALS INTO OUR WAD
	sprintf(DBFile,"%s%s%s",SYSWADDIR,"SYSWAD",".TXT");	
	if ((sysfile=fopen(DBFile, "r")))
	{
		for (;;)
		{
			if (!(fgets(line,128,sysfile)))
				break;

			fputs(line,wadfile);
		}
		fclose(sysfile);
	}


			
			
	sprintf(DBFile,"\n%s\t%s\t%s\n",
			"CreateWad",
			mapname,
			mapname);
		fputs(DBFile,wadfile);


			//QUICK DOME HACK, EVENTUALLY THIS SHOULD GO IN THE LEVEL MAP WAD FILE ITSELF
		   //	sprintf(DBFile,"\t%s\n",
		   //		"ADDOBJECT	MISC\\DOME_LEVEL2");
		   //	fputs(DBFile,wadfile);

			sprintf(DBFile,"\t%s\t%s\n",
				"AddMap",
				mapname);
			fputs(DBFile,wadfile);

			sprintf(DBFile,"CloseWad\n");
		fputs(DBFile,wadfile);

			
	fclose(wadfile);

}

/*DELETES ALL RDOBJECTS AND THEIR ASSOCIATED WAYPATHS AND TRIGGERS*/

typedef struct names
{
	char *oldname;
	char *newname;
}NAMES;

#define MAXNAME 39

/*UPDATES OLD DST WITH NEW NAMES*/
NAMES CheckName[MAXNAME] = 
{
	"LAUNCHER.DST",				"Deactivator.dst",
	"FOOTSOLDIERMARKSMAN.DST",	"MARKSMAN_FootSoldier.dst",
	"FOOTSOLDIERPARENT.DST",	"FootSoldier.dst",
	"CAMOSENTRY.DST",			"PopUpGun.dst",
	"FANOUTPOST.DST",			"ENV_Fan.dst",
	"FENCEGATE.DST",			"ENV_WireGate.dst",
	"FLYINGASSASIN.DST",		"CruiseMissile.dst",
	"BRIDGE.DST",				"ENV_Bridge.dst",
	"GROUNDSPEEDERSTATIC.DST",	"MARKSMAN_GroundSpeeder.dst",
	"BIGRAPIER.DST",			"RAPIER_Big.dst",
	"GUNTURRET.DST",			"ENV_GunTurret.dst",
	"HONEYMINE.DST",			"MINE_PopUp.dst",
	"WATERMINE.DST",			"MINE_Water.dst",
	"HONEYMINEDOWN.DST",		"MINE_PopUp.dst",
	"STATICMINE.DST",			"MINE_Proximity.dst",
	"HOVERFIGHTERSIMPLE.DST",	"HOVERBOT_Simple.dst",
	"HOVERGUIDE.DST",			"HOVERBOT_Normal.dst",
	"JETPACKERPARENT.DST",		"JETPACKER_Normal.dst",
	"OBJECTEXPLODE.DST",		"ENV_ObjectExplodeOnImpact.dst",
	"OBJECTEXPLODEONSHOOT.DST", "ENV_ObjectExplodeWhenShot.dst",
	"PLAYERNORMALSTATE.DST",	"STATECHANGE_Normal.dst",
	"ENDLEVEL.DST",				"STATECHANGE_EndLevel.dst",
	"PLAYERPATHCONTROL.DST",	"STATECHANGE_TowerLiftPathControl.dst",
	"POPUPMINEHOLE.DST",		"MinePopUpGenerator.dst",
	"RAIL_JETPACKER.DST",		"JETPACKER_Formation.dst",
	"RAINCTRL.DST",				"ENV_FXRain.dst",
	"SCAPEFIRE.DST",			"ENV_FXFire.dst",
	"SMOKEPUFF.DST",			"ENV_FXLavaSmoke.dst",
	"A00MINELAYER.DST",			"MineAerialLayer.dst",
	"SPLINEMINIMINER.DST",		"MineStaticLayer.dst",
	"SUN.DST",					"ENV_FXSun.dst",
	"WATER.DST",				"ENV_FXWater.dst",
	"WATERWHEEL.DST",			"ENV_WaterWheel.dst",
	"MINIRAPIERANIM.DST",	 	"RAPIER_Mini.dst",
	"A00MINIRAPIER.DST",	 	"RAPIER_Mini.dst",
  	"BATTLETANKNETWORK.DST",   	"BattleTank.dst",
	"HOVERBOTELECTRIC.DST",    "HoverBot.dst",
	"HOVERBOT_NORMAL.DST",      "HoverBot.dst",
	"HOVERBOT_SIMPLE.DST",      "HoverBot.dst"
















};



void NameUpdate2(INode *node)
{
		int i,numchild;
		INode *oldnode=node;
		
		numchild = node->NumberOfChildren();

	   //	Object *object = node->EvalWorldState(GetCOREInterface()->GetTime()).obj;
	 	Object *object = node->GetObjectRef();
   //	object = node->GetObjOrWSMRef();

		if (object)
	  	{
			object = object->FindBaseObject();
			if (object->ClassID() == RDOBJECT_CLASS_ID)
			{
				RDObject *rd = (RDObject*)object;
		 		char *name = rd->stratSource;
				
				for (int loop=0;loop<MAXNAME;loop++)
				{
					if (!strnicmp(name,CheckName[loop].oldname,strlen(name)))
				 		strcpy(rd->stratSource,CheckName[loop].newname);
				}
			}
		}

		if (!numchild)
			return;

		for (i=0;i<numchild;i++)
		{
			node = oldnode->GetChildNode(i);
			NameUpdate2(node);
		}





}
void NameUpdate(INode *node)
{
		int i,numchild;
		INode *oldnode=node;
		
		numchild = node->NumberOfChildren();

	 // 	Object *object = node->EvalWorldState(GetCOREInterface()->GetTime()).obj;
	  	Object *object = node->GetObjectRef();
  //	object = node->GetObjOrWSMRef();

		if (object)
	  	{
			object = object->FindBaseObject();
			if (object->ClassID() == RDOBJECT_CLASS_ID)
			{
				RDObject *rd = (RDObject*)object;
		 		INode *rdnode = rd->GetMyNode();
		 		if (!strncmp(rdnode->GetName(),"RedDogObject",12))
					UpdateNodeName(rd,rd->stratSource);
			}
		}

		if (!numchild)
			return;

		for (i=0;i<numchild;i++)
		{
			node = oldnode->GetChildNode(i);
			NameUpdate(node);
		}





}


//INode *delnodes[1024];
INode **delnodes;
int NODEY;

void AddToDeletionSet(INode *node)
{
	int loop;
	int found=0;

	for (loop=0;loop<NODEY;loop++)
	{
		if (delnodes[loop] == node)
		{
			found++;
			break;

		}

	}

	if (!found)
	{

		delnodes[NODEY]= node;
	  	
		NODEY ++;

	}


}



void StratDelete(INode *node)
{
		int i,numchild;
		INode *oldnode=node;
		numchild = node->NumberOfChildren();


	  	Object *object = node->GetObjectRef();

	  	if (object)
	  	{
   
			object = object->FindBaseObject();
			TCHAR *name;
		 
			if (object->ClassID() == camcontrols_CLASS_ID)
					AddToDeletionSet(node);

			if (object->ClassID() == levelflags_CLASS_ID)
					AddToDeletionSet(node);

			
			
			if (object->ClassID() == RDOBJECT_CLASS_ID)
			{
				RDObject *rd = (RDObject*)object;
		
				if (rd->netpath)
					AddToDeletionSet(rd->netpath);

				if (rd->waypoint)
				{
					INode *parent = rd->waypoint->GetParentNode();
				   	
					if (parent)
					{
						
						//delete all the children
						name = parent->GetName();
						if (!(strnicmp(name,"BBox",4)))
						{
							int child = parent->NumberOfChildren();

							for (int loop=0;loop<child;loop++)
							{
								INode *bboxchild = parent->GetChildNode(loop);
								AddToDeletionSet(bboxchild);

								int numchildchild = bboxchild->NumberOfChildren();
							   
								for (int loop2 =0;loop2<numchildchild;loop2++)
								{
									INode *bboxchildchild = bboxchild->GetChildNode(loop2);
									AddToDeletionSet(bboxchildchild);
								}
							}

							AddToDeletionSet(parent);

						}
					}


					AddToDeletionSet(rd->waypoint);
				}

				if (rd->eventpoint)
					AddToDeletionSet(rd->eventpoint);

				if (rd->deletioneventpoint)
					AddToDeletionSet(rd->deletioneventpoint);

				if (rd->trigpoint)
					AddToDeletionSet(rd->trigpoint);
			
				for (int loop=0;loop<5;loop++)
				{
					if (rd->actpoint[loop])
						AddToDeletionSet(rd->actpoint[loop]);
				}
		 
				
				AddToDeletionSet(node);
				
				
			 	return;	
			}
		
		}

		if (!numchild)
			return;

		for (i=0;i<numchild;i++)
		{
			node = oldnode->GetChildNode(i);
			StratDelete(node);
		}





}

void UpdateNames()
{
		INode *newnode = GetCOREInterface()->GetRootNode();

	 
		NameUpdate(newnode);
		NameUpdate2(newnode);

}



void DeleteStrats()
{
		INode *newnode = GetCOREInterface()->GetRootNode();

		delnodes = (INode**)malloc(16536);
		NODEY = 0;
		StratDelete(newnode);
  		if (!theHold.Holding())
			theHold.Begin();

		 for (int i=0;i<NODEY;i++)
		 {
		  	// theHold.Put(new RestoreObj(delnodes[i]));
			 delnodes[i]->Delete(GetCOREInterface()->GetTime(),TRUE);
		 }
		 theHold.End();

		 GetCOREInterface()->ForceCompleteRedraw();
		free(delnodes);
}


void FreezeStrats()
{
		INode *newnode = GetCOREInterface()->GetRootNode();

		delnodes = (INode**)malloc(16536);
		
		NODEY = 0;
		StratDelete(newnode);
  
		 for (int i=0;i<NODEY;i++)
		 {
		  	 if (delnodes[i]->IsGroupMember())
			 {
				INode *parent = delnodes[i]->GetParentNode();
				parent->Freeze(TRUE);

			 }
			 delnodes[i]->Freeze(TRUE);

		 }
		 GetCOREInterface()->ForceCompleteRedraw();
		free(delnodes);
}

Preview *LEVNAMEBOX = NULL;


typedef struct godmodescript
{
	char SourceFile[128];
	char XPortName[32];


}	GODMODESCRIPT;

#if 0
void DelNode(INode *node)
{
	int i,numchild;
	INode *oldnode=node;
		
	numchild = node->NumberOfChildren();
	   
	Object *object = node->GetObjectRef();
	if (object)
	{
		object = object->FindBaseObject();
		if (object->ClassID() == RDOBJECT_CLASS_ID)
		{
			RDObject *rd = (RDObject*)object;
		  	rd->Clean();
			//	delete(object);
		}
	}

	for (i=0;i<numchild;i++)
	{
		node = oldnode->GetChildNode(i);
		DelNode(node);
	}
	//node->Delete(GetCOREInterface()->GetTime(),0);
}
#endif


void HACKLOOP()
{
	return;
	for (int hack=0;hack < 100000;hack++)
					{
						for (int hacki=0;hacki < 100000;hacki++)
						{
						}

					}
}



GODMODESCRIPT GodScripts[64];
#define MAX_LINE 128

/********************************************************************************/

BOOL CALLBACK PreviewParamProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	Preview *po = (Preview *)GetWindowLong(hWnd,GWL_USERDATA);
 	FILE *godscript;
	int Ginsu=0;
	if (!po && msg!=WM_INITDIALOG) return FALSE;
	
	switch (msg) {
	case WM_INITDIALOG:
		LEVNAMEBOX = po = (Preview *)lParam;
		salaryCtrl = GetICustEdit (GetDlgItem (hWnd, IDC_SALARY));
		bonusCtrl = GetICustEdit (GetDlgItem (hWnd, IDC_BONUS));
		bonusAmtCtrl = GetICustEdit (GetDlgItem (hWnd, IDC_BONUS_AMT));
		SetWindowLong (hWnd,GWL_USERDATA,lParam);
		CheckDlgButton(hWnd, IDC_AUTORUN_GDS, GDSRUN);
		CheckDlgButton(hWnd, IDC_AUTORUN_CODE, CODERUN);
		
		// Set up stuff
		return FALSE;
   case WM_DESTROY:
		LEVNAMEBOX = NULL;
		ReleaseICustEdit (salaryCtrl);
		ReleaseICustEdit (bonusCtrl);
		ReleaseICustEdit (bonusAmtCtrl);
		
		// Set up stuff
		return FALSE;
		
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		default:
			break;
		case IDC_STATS:
			if (po->iface->ExportToFile ("C:\\WINDOWS\\TEMP\\tempobj.RDO", TRUE)) {
				char foo[1024];
				float amt;
				extern int stripPolys, stripStrip;
				amt = 2.f - ((((float)stripStrip / (float)stripPolys)) - 1.f);
				sprintf (foo, "Render cost = %d\n%.3f%% stripped", 
					stripStrip,
					amt * 50.f);
				MessageBox (NULL, foo, "Statistics", 0);
				unlink ("C:\\WINDOWS\\TEMP\\tempobj.RDO");
			}
			return TRUE;
		case IDC_CONVERT:
			//po->iface->ExportToFile ("!CONVERT!.RDO", TRUE);
			_spawnl (_P_NOWAIT, "P:\\UTILS\\CONVERT.EXE", "P:\\UTILS\\CONVERT.EXE", NULL);
			return TRUE;
		case IDC_CONVERTMAT:
			{
				INode *root = po->iface->GetRootNode();
				ConvertNode(root);
				po->iface->ForceCompleteRedraw();
			}
			return TRUE;
		case IDC_LEVELNAME:
		case IDC_EDIT1:
		case IDC_EDIT2:
		case IDC_EDIT3:
		case IDC_EDIT4:
			if (HIWORD(wParam) == EN_CHANGE) {
				po->edit->GetText (levelName, sizeof (levelName));
				for (int i = 0; i < asize(idPtr); ++i) {
					char buf[256];
					po->saveParams[i]->GetText (buf, sizeof (buf));
					*idPtr[i] = (float)atof (buf);
				}
			}
			break;
		case IDC_SALARY:
		case IDC_BONUS:
			if (HIWORD(wParam) == EN_CHANGE) {
				static char salText[32], bonText[32], resText[32];
				salaryCtrl->GetText (salText, sizeof (salText));
				bonusCtrl->GetText (bonText, sizeof (bonText));
				sprintf (resText, "%f", 100.f * atof(bonText) / (atof(salText)/4));
				bonusAmtCtrl->SetText (resText);
			}
			break;
		case IDC_EDITOR:
			_spawnl (_P_WAIT, "C:\\PROGRAM FILES\\ACCESSORIES\\WORDPAD.EXE", "C:\\PROGRAM FILES\\ACCESSORIES\\WORDPAD.EXE", NULL, NULL);
			break;
		case IDC_AUTORUN_GDS:
			GDSRUN = !GDSRUN;
			break;
		case IDC_AUTORUN_CODE:
			CODERUN = !CODERUN;
			break;
		case IDC_CONVNAMES:
			UpdateNames();
		 	return TRUE;
			break;
		
		case IDC_DELETESTRATS:
			if (MessageBox(hWnd, "YOU ARE ABOUT TO DELETE ALL THE STRATS AND ASSOCIATED PATHS \nARE YOU ALRIGHT IN THE HEAD?", "Warning!", MB_YESNO) == IDYES)
		   		DeleteStrats();
		 	return TRUE;
			break;
		
		case IDC_FREEZESTRATS:
			FreezeStrats();
		 	return TRUE;
			break;
		
		case IDC_SAVESCAPE:
		case IDC_SAVEMAP:
			{
				char buf2[1024];
				if (levelName[0] == '\0') {
					MessageBox (hWnd, "Please set the level name box to be the level name", "Message", MB_OK);
				} else {
					if (LOWORD(wParam) == IDC_SAVESCAPE)
					{
						sprintf (buf2, "P:\\GAME\\NEWSCAPES\\%s.RDL", levelName);
						po->iface->ExportToFile (buf2, TRUE);
						if (GetCOREInterface()->GetCancel() || GlobalCancelled)
							break;
						//ENSURE THE WADDER IS CALLED
						//WAD TXT MUST BE MADE UP CORRECTLY AS THE SCR IS NOT CALLED
						MakeWadTxt(levelName);

						if (GetCOREInterface()->GetCancel() || GlobalCancelled)
							break;

						#if ARTIST
							_spawnl (_P_WAIT, "p:\\UTILS\\WADDER\\wadder.exe",NULL, NULL);
						#else
#if GODMODEMACHINE
							_spawnl (_P_WAIT, "D:\\DREAMCAST\\REDDOG\\WADDER\\RELEASE\\wadder.exe",NULL, NULL);
#else
							_spawnl (_P_WAIT, "c:\\DREAMCAST\\REDDOG\\WADDER\\RELEASE\\wadder.exe",NULL, NULL);
							
#endif
		#endif
					
					}
					if (LOWORD(wParam) == IDC_SAVEMAP) {
						#if ARTIST
							sprintf (buf2, "D:\\DREAMCAST\\%s.SCR", levelName);
						#else
#if GODMODEMACHINE
							sprintf (buf2, "D:\\DREAMCAST\\REDDOG\\GAME\\STRATS\\%s.SCR", levelName);
#else
							sprintf (buf2, "C:\\DREAMCAST\\REDDOG\\GAME\\STRATS\\%s.SCR", levelName);
#endif
						#endif							
						#if GODMODEMACHINE
							ALLMAPMODE = GODMODEFINAL;
						#else
	  						ALLMAPMODE = 1;
						#endif
							po->iface->ExportToFile (buf2, TRUE);
					}
				}
			}
			return true;
		case IDC_SAVEBOTH:
				char buf2[1024];
				if (levelName[0] == '\0') 
					MessageBox (hWnd, "Please set the level name box to be the level name", "Message", MB_OK);
				else
				{
					sprintf (buf2, "P:\\GAME\\NEWSCAPES\\%s.RDL", levelName);
					po->iface->ExportToFile (buf2, TRUE);
						if (GetCOREInterface()->GetCancel() || GlobalCancelled)
							break;
					#if ARTIST
						sprintf (buf2, "D:\\DREAMCAST\\%s.SCR", levelName);
					#else
#if GODMODEMACHINE
						sprintf (buf2, "D:\\DREAMCAST\\REDDOG\\GAME\\STRATS\\%s.SCR", levelName);
#else
						sprintf (buf2, "C:\\DREAMCAST\\REDDOG\\GAME\\STRATS\\%s.SCR", levelName);
#endif
	#endif
					#if GODMODEMACHINE
				   		ALLMAPMODE = GODMODEFINAL;
				   	#else
						ALLMAPMODE = 0;
					#endif
					po->iface->ExportToFile (buf2, TRUE);
					ALLMAPMODE = 1;
				}
				return true;

		case IDC_SAVEOBJ:
			{
				char buf2[1024];
				if (levelName[0] == '\0') 
					MessageBox (hWnd, "Please set the name box to be the object name", "Message", MB_OK);
				else
				{
					sprintf (buf2, "P:\\GAME\\NEWOBJECTS\\%s.RDO", levelName);
					po->iface->ExportToFile (buf2, TRUE);
				}
				return true;
			}

		case IDC_SAVESOLEMAP:
			{
				char buf2[1024];
			   	//first off run the delete gamemaps util
#if !GODMODEMACHINE
#if	ARTIST
				system("P:\\UTILS\\DELART.bat");
#else
			 	system("P:\\UTILS\\DELMAPS.bat");
#endif
#endif

				HACKLOOP();
				if (levelName[0] == '\0') 
					MessageBox (hWnd, "Please set the level name box to be the level name", "Message", MB_OK);
				else
				{
					DisableAccelerators();
					#if ARTIST
						sprintf (buf2, "D:\\DREAMCAST\\%s.SCR", levelName);
					#else
#if GODMODEMACHINE
					  	sprintf (buf2, "D:\\DREAMCAST\\REDDOG\\GAME\\STRATS\\%s.SCR", levelName);
#else
					  	sprintf (buf2, "C:\\DREAMCAST\\REDDOG\\GAME\\STRATS\\%s.SCR", levelName);
						
#endif
	#endif
					#if GODMODEMACHINE
				   		ALLMAPMODE = GODMODEFINAL;
				   	#else
						ALLMAPMODE = 0;
					#endif
					po->iface->ExportToFile (buf2, TRUE);
					ALLMAPMODE=1;
					EnableAccelerators();
				}
			}
			return true;
			break;
		case IDC_VIEWSTRATS:
			VIEWMODE = ALLSTRATS;
			DialogBoxParam (hInstance, MAKEINTRESOURCE (IDD_STRATLIST),
			hWnd, dialogFunc, NULL);
			break;
		case IDC_FILTER:
			VIEWMODE = STARTSTRATS;
			DialogBoxParam (hInstance, MAKEINTRESOURCE (IDD_STRATLIST),
			hWnd, dialogFunc, NULL);
		 	break;
		case IDC_NONDELETEDSTRATS:
			VIEWMODE = NONDELETEDSTRATS;
			DialogBoxParam (hInstance, MAKEINTRESOURCE (IDD_STRATLIST),
			hWnd, dialogFunc, NULL);
		 	break;
		case IDC_CTRL_CAMERA:
			// Get the current camera INode
			{
				ViewExp *view = GetCOREInterface()->GetActiveViewport();
				ICustButton *button = GetICustButton (GetDlgItem (hWnd, IDC_CTRL_CAMERA));
				if (button) {
					button->SetHighlightColor (GREEN_WASH);
					button->SetCheckHighlight (TRUE);
					button->SetCheck(TRUE);
				} else MessageBox (NULL, "Unable to get button", "", 0);
				if (view) {
					INode *camera = view->GetViewCamera();
					if (camera) {
						Matrix3 mOrig = camera->GetNodeTM (GetCOREInterface()->GetTime());
						bool finished = false;
						while (!finished) {
							MSG msg;
							while (PeekMessage(&msg,GetCOREInterface()->GetMAXHWnd(),0,0,TRUE)) {
								Point3 p;
								float xRot=0, yRot=0;
								switch (msg.message) {
								case WM_KEYDOWN:
									switch (msg.wParam) {
									case 27: // escape
										finished = TRUE;
										break;
									default:
										p = Point3(0,0,0);
										xRot = yRot = 0;
										if (GetAsyncKeyState('W') < 0) {
											p += Point3(0, 0, -1);
										}
										if (GetAsyncKeyState('S') < 0) {
											p += Point3(0, 0, 1);
										}
										if (GetAsyncKeyState('A') < 0) {
											p += Point3(-1, 0, 0);
										}
										if (GetAsyncKeyState('D') < 0) {
											p += Point3(1, 0, 0);
										}
										if (GetAsyncKeyState('R') < 0) {
											p += Point3(0, 1, 0);
										}
										if (GetAsyncKeyState('F') < 0) {
											p += Point3(0, -1, 0);
										}
										if (GetAsyncKeyState(VK_UP) < 0) {
											xRot += 1.f;
										}
										if (GetAsyncKeyState(VK_DOWN) < 0) {
											xRot -= 1.f;
										}
										if (GetAsyncKeyState(VK_LEFT) < 0) {
											yRot += 1.f;
										}
										if (GetAsyncKeyState(VK_RIGHT) < 0) {
											yRot -= 1.f;
										}
										if (p.x || p.y || p.z || xRot || yRot) {
											p = p * Point3(5, 5, 5);
											camera->Move (GetCOREInterface()->GetTime(), camera->GetNodeTM(GetCOREInterface()->GetTime()),
												p);
											mOrig = camera->GetNodeTM (GetCOREInterface()->GetTime());
											Matrix3 m = mOrig;
											m.PreRotateX (xRot * 0.01f);
											m.PreRotateY (yRot * 0.01f);
											camera->SetNodeTM (GetCOREInterface()->GetTime(), m);
											GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
										}
										break;

									}
								break;
								case WM_LBUTTONDOWN:
								case WM_RBUTTONDOWN:
									finished = true;
									break;
									// falls:
								case WM_PAINT:
								case WM_PAINTICON:
									GetCOREInterface()->TranslateAndDispatchMAXMessage(msg);
									break;

								}
							}
						}
						MessageBox (hWnd, "Done it", "", 0);
					}
					GetCOREInterface()->ReleaseViewport(view);

				} else 
					MessageBox (hWnd, "Unable to do it", "", 0);
				if (button) {
					button->SetCheck (FALSE);
					ReleaseICustButton (button);
				}
			}
			break;

		case IDC_GINSU:
			Ginsu = 1;
		case IDC_GODMODE:
			int result = 0;

			if (Ginsu)
				result = MessageBox(hWnd, "HELLO !! ANYONE AT HOME !!  \n DO YOU REALLY WANT TO GIVE GIN TO SUE ?", "MAJOR BRAVERY ALERT!", MB_YESNO);
			else
				result = MessageBox(hWnd, "HELLO !! ANYONE AT HOME !!  \n YOU DO KNOW THE NATURE OF THIS WAREZ ?", "MAJOR BRAVERY ALERT!", MB_YESNO);


			//if (MessageBox(hWnd, "HELLO !! ANYONE AT HOME !!  \n YOU DO KNOW THE NATURE OF THIS WAREZ ?", "MAJOR BRAVERY ALERT!", MB_YESNO) == IDYES)
			if (result == IDYES)
			{
				#if ARTIST
	   			 	MessageBox (NULL, "SORRY..YOUR WAREZY RATING IS NEGATIVE..TRY AGAIN LATER","GOD'S MESSAGE TO MINIONS",MB_OK);
				#else
					//first off run the delete gamemaps util
#if GODMODEMACHINE
					//DO SOME PARANOIA CLEANING
					system("command /e:8192 /cP:\\game\\artstrat\\GODclean.bat");
					system("P:\\UTILS\\DELGODMAPS.bat");
#else
				system("P:\\UTILS\\DELMAPS.bat");
#endif
					HACKLOOP();

					unlink ("p:\\godmode.log");

					//OPEN UP OUR GOD MODE'S RIGHT-HAND ANGEL
					if (Ginsu)
					{
#if GODMODEMACHINE
						sprintf (buf2, "D:\\DREAMCAST\\REDDOG\\GAME\\GINSU.SCR");
#else
						sprintf (buf2, "C:\\DREAMCAST\\REDDOG\\GAME\\GINSU.SCR");
#endif

					}
					else
					{
#if GODMODEMACHINE
						sprintf (buf2, "D:\\DREAMCAST\\REDDOG\\GAME\\GODMODE.SCR");
#else
						sprintf (buf2, "C:\\DREAMCAST\\REDDOG\\GAME\\GODMODE.SCR");
#endif
					}
					int mapnum = 0;
					if ((godscript=fopen(buf2, "r")))
					{
						Clear(buf2,1024);
						char *buffer;
			   
						while (fgets(buf2,MAX_LINE,godscript))
						{
							buffer = WhiteSpaceRead(buf2);
							buffer = ReadToEnd(GodScripts[mapnum].SourceFile,buffer);
							buffer = WhiteSpaceRead(buffer);
							ReadToEnd(GodScripts[mapnum].XPortName,buffer);
							Clear(buf2,1024);
							mapnum++;
						}
			  
						fclose(godscript);

						for (int maploop=0;maploop<mapnum;maploop++)
						{
							GetCOREInterface()->ExecuteMAXCommand(MAXCOM_OVERRIDE);
							GetCOREInterface()->FileReset(1);
							if (maploop == (mapnum-1))
								ALLMAPMODE = GODMODEFINAL;
							else
								ALLMAPMODE = GODMODEINTERMEDIATE;
							//load the max file
				  			GetCOREInterface()->LoadFromFile(GodScripts[maploop].SourceFile,1);
							sprintf (buf2, "P:\\UTILS\\BUILDLOGS\\%s.LOG",GodScripts[maploop].XPortName );
							godlog = fopen(buf2,"w");

							// NEW:
							// Only export the scape if it has changed in any way
							FILETIME sourceTime, destTime;
							GetTheFileTime (GodScripts[maploop].SourceFile, &sourceTime);

							//xport the scape

							sprintf (buf2, "P:\\GAME\\NEWSCAPES\\%s.RDL",GodScripts[maploop].XPortName );
							bool ok = GetTheFileTime (buf2, &destTime);
							bool exported = false;
							if (!ok ||
								sourceTime.dwHighDateTime != destTime.dwHighDateTime ||
								sourceTime.dwLowDateTime != destTime.dwLowDateTime) {
								GetCOREInterface()->ExportToFile (buf2, TRUE);
								exported = true;
								// We exported: set the timestamp of dest to be that of source
								SetTheFileTime (buf2, &sourceTime);
							}

							{
								FILE *fUpdate = fopen ("p:\\godmode.log", "a+");
								fprintf (fUpdate, "Exported %s.SCR\n", GodScripts[maploop].XPortName);
								if (exported)
									fprintf (fUpdate, "\tExported %s.RDL\n", GodScripts[maploop].XPortName);
								fclose (fUpdate);
							}
							//xport the strats, compiling and rewadding on the last one
#if GODMODEMACHINE
							sprintf (buf2, "D:\\DREAMCAST\\REDDOG\\GAME\\STRATS\\%s.SCR", GodScripts[maploop].XPortName);
#else
							sprintf (buf2, "C:\\DREAMCAST\\REDDOG\\GAME\\STRATS\\%s.SCR", GodScripts[maploop].XPortName);
#endif
							GetCOREInterface()->ExportToFile (buf2, TRUE);
							fclose(godlog);
							//INode *newnode = GetCOREInterface()->GetRootNode();
	 
							//DelNode(newnode);

			  
						}
				}
				#endif
			}		 
		 	break;


		}
		break;
		
	default:
		return FALSE;
	}
	
	return TRUE;
} 

void Preview::BeginEditParams (Interface *ip, IUtil *iu)
{	
	iface = ip;
	if (!paramWindow) {
		paramWindow = ip->AddRollupPage( 
			hInstance, 
			MAKEINTRESOURCE(IDD_PREVIEW),
			PreviewParamProc, 
			GetString(IDS_PREVIEW_CATEGORY), 
			(LPARAM)this );
		ip->RegisterDlgWnd (paramWindow);
		edit = GetICustEdit(GetDlgItem (paramWindow, IDC_LEVELNAME));
		edit->SetText (levelName);
		for (int i = 0 ; i < asize(ids); ++i) {
			char buf[255];
			saveParams[i] = GetICustEdit (GetDlgItem(paramWindow, ids[i]));
			sprintf (buf, "%.2f", *idPtr[i]);
			saveParams[i]->SetText(buf);
		}
	} else {
		SetWindowLong(paramWindow,GWL_USERDATA,(LONG)this);
	}
}

void Preview::EndEditParams(Interface *ip, IUtil *iu)
{	
	// Read window stuff here
	edit->GetText (levelName, sizeof (levelName));
	ReleaseICustEdit (edit);
	edit = NULL;
	for (int i = 0; i < asize(ids); ++i) {
		char buf[256];
		saveParams[i]->GetText (buf, sizeof (buf));
		*idPtr[i] = (float)atof (buf);
		ReleaseICustEdit (saveParams[i]);
		saveParams[i] = NULL;
	}
	ip->UnRegisterDlgWnd(paramWindow);
	ip->DeleteRollupPage(paramWindow);
	paramWindow = NULL;
	iface = NULL;
}
void Preview::nameupdate (TCHAR* name)
{
	if (edit)
		edit->SetText(name);
	for (int i = 0 ; i < asize(ids); ++i) {
		char buf[255];
		sprintf (buf, "%.2f", *idPtr[i]);
		if (saveParams[i])
			saveParams[i]->SetText(buf);
	}
}
