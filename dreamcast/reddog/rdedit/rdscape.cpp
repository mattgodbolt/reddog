/*
 * RDScape.cpp - the map file format
 */

#include "stdafx.h"
#include "VisBox.h"
#include "camcontrol.h"
#include "ApplyVC\EvalCol.h"
#include "ShadBox.h"

// The progress box
static Interface *progress = NULL;

BOOL GlobalCancelled = FALSE;

static DWORD WINAPI fn (LPVOID arg)
{
	return 0;
}

BOOL Canceled()
{
	assert (progress);
	return (progress->GetCancel());
}

void setProgressText (char *text, int pc /* = 0 */)
{
	assert (progress);
	static int PC = 0;
	if (text)
		progress->ProgressUpdate (pc, FALSE, _T(text));
	else {
		if (pc != PC) {
			progress->ProgressUpdate (pc);
			PC = pc;
		}
	}
}

// Counts the tris in the accumulated mesh
int RDScape::getTriCount ()
{
	return accMesh.numFaces;
}

static BOOL CALLBACK progressProc (HWND dBox, UINT msg, WPARAM wp, LPARAM lp)
{
	return FALSE;
}

void setProgressName (char *name)
{
	assert (progress);
	progress->ProgressEnd();
	progress->ProgressStart(_T(name), TRUE, fn, NULL);
}

void RecurseGetShadBoxes (Tab<IShadBox *> &list, INode *node)
{
	Object *obj = node->EvalWorldState(0).obj;
	if (obj && obj->ClassID() == SHADBOX_CLASSID) {
		IShadBox *box = (IShadBox *)obj;
		box->SetMat (node->GetObjectTM(0));
		list.Append (1, &box, 32);
	}
	for (int i = 0; i < node->NumberOfChildren(); ++i) 
		RecurseGetShadBoxes (list, node->GetChildNode(i));
}

// Prepares the map data ready for output
void RDScape::Prepare ()
{
	extern int stripPolys, stripStrip, numStrip;
	stripPolys = stripStrip = numStrip = 0;
	// Initialise the progress stuff
	progress = iface;

	setProgressText (NULL, 0);
	Tab<IShadBox *> shadowBoxes;
	RecurseGetShadBoxes(shadowBoxes, GetCOREInterface()->GetRootNode());
	box = new BBoxLandscape (accMesh, &uvTLib[0], matLib, shadowBoxes);
	//box->IterateShowBoxes (iface);

	progress->ProgressEnd();
	progress = NULL;
}

// Outputs the converted mesh
void RDScape::Output (FILE *f)
{
	extern int stripPolys, stripStrip, numStrip;
	FILE *fOut;

	iface->ProgressStart (_T("Saving map"), TRUE, fn, NULL);
	box->ExportMap (f, matLib, visibilityBoxes);
	iface->ProgressEnd();

	fOut = fopen ("P:\\STRIP.TXT", "a+");
	if (fOut) {
		fprintf (fOut, "Scape '%s' has %d polys,%d stripverts (%f ratio) - %f polys per strip\n", 
			GetCOREInterface()->GetCurFilePath(),
			stripPolys, stripStrip,
			(float)stripStrip / (float)stripPolys, (float)stripPolys / (float)numStrip);
		fclose (fOut);
	}
}

/*
 * The saving enumerator
 */
class RDExportEnumerator : public ITreeEnumProc {
private:
	// The interface to max
	Interface *maxInterface;
	int num, maxNum;
	int mode;
	int curPc;
	// The map we're building
	RDScape &map;

public:
	RDExportEnumerator (IScene *scene, Interface *i, RDScape &m) : map(m)
	{
		assert (i != NULL);
		// Set up the variables
		maxInterface = i;
		num = 0;
		mode = 0;
		scene->EnumTree (this);
		maxNum = num;
		num = 0;
		mode = 1;
		curPc = -1;
		scene->EnumTree (this);
	}
	
	// A function called once per object to save it
	int callback (INode *node)
	{
	   	Object *obj = node->EvalWorldState(maxInterface->GetTime()).obj;
		if (obj == NULL)
			return TREE_CONTINUE;
	   //	Object *obj = node->GetObjectRef();

		TCHAR* nodename = node->GetName();	
		int InvalidModel = 0;
		if (!(strncmp(nodename,"BBox",4)))
			InvalidModel=1;
		// Is the node hidden?
		if (node->IsNodeHidden())
			InvalidModel=1;

		// Is it a Red Dog Object and part of the landscape?
		// or a circular trigger point ?

		if ((obj->ClassID() == RDOBJECT_CLASS_ID)
			||
			(obj->ClassID() == camcontrols_CLASS_ID)
			|| 
			(obj->ClassID() == Class_ID(CIRCLE_CLASS_ID,0))
			||
			(obj->SuperClassID() == SHAPE_CLASS_ID)
			||
			(InvalidModel))
		{
			//if (obj->ClassID() != Class_ID(CIRCLE_CLASS_ID,0))
			if (mode==1)
				map.AddObject ((RDObject *)node);
			return TREE_CONTINUE;
		} else if (obj->ClassID() == VISBOX_CLASS_ID ||
					obj->ClassID() == SHADBOX_CLASSID) {
			if (mode==1) {
				if (obj->ClassID() == SHADBOX_CLASSID) {
/*					IShadBox *box = (IShadBox *)obj;
					map.AddShadBox (&box);*/
				} else {
					VisBox *box = (VisBox *)obj;
					RedDog::VisBox rdBox = box->GetBox(node->GetObjectTM(0));
					map.AddBox (&rdBox);
				}
			}
			return TREE_CONTINUE;
		} else {
			if (mode==0) {
				num++;
			} else {
				SingleVertexColor colourTab;
				// Prelight the object
				calcMixedVertexColors(node, 0, 
					LIGHT_SCENELIGHT, 
					colourTab.vertexColors);

				// Retrieve the TriObject from the node
				int deleteIt;
				TriObject *triObject = GetTriObjectFromNode (maxInterface, node, deleteIt);
				
				// Use the TriObject if available
				if (!triObject) return TREE_CONTINUE;
				
				// Add this mesh to the map
				Matrix3 transform = node->GetObjectTM(0);
				map.AddMesh (node->GetMtl(), triObject->mesh, &transform, node->GetName(), &colourTab.vertexColors);
				
				num++;
				int pc = (int)(100.f * (float)num / (float)maxNum);
				if (pc != curPc) {
					maxInterface->ProgressUpdate (pc);
					curPc = pc;
				}

				// Delete it when done...
				if (deleteIt) triObject->DeleteMe();
			}
			if (GetCOREInterface()->GetCancel())
				return TREE_ABORT;
			else
				return TREE_CONTINUE;
		}
	}
};

class RDExport : public SceneExport {
public:
	/*
     * MAX API
	 */
	// Returns the number of supported extensions
	virtual int ExtCount() { return 1; }
	// Returns the file extension for a given number
	virtual const TCHAR *Ext(int i) { assert(i==0); return _T("RDL"); };
	// Returns the short and long descriptions, and other strings
	virtual const TCHAR *LongDesc() { return _T("Argonaut Software Red Dog Landscape"); }
	virtual const TCHAR *ShortDesc() { return _T("Red Dog landscape"); }
	virtual const TCHAR *AuthorName() { return _T("Argonaut Software"); }
	virtual const TCHAR *CopyrightMessage() { return _T("(c) 1998 Argonaut Software Ltd"); }
	virtual const TCHAR *OtherMessage1() { return _T(""); }
	virtual const TCHAR *OtherMessage2() { return _T(""); }
	// Returns the version of the plugin
	virtual unsigned int Version() { return RDM_VERSION; }
	// Show the About box
	virtual void ShowAbout (HWND parent) { return; }
	// Actually do the export
#if MAX3
	virtual int DoExport (const TCHAR *name, ExpInterface *ei, Interface *i, BOOL suppressPrompts /* = FALSE */, DWORD options)
#else
	virtual int DoExport (const TCHAR *name, ExpInterface *ei, Interface *i, BOOL suppressPrompts /* = FALSE */)
#endif
	{
		FILE *f = fopen (name, "wb");
		GlobalCancelled = FALSE;
		if (f==NULL)
			return FALSE;
		// Enumerate through all the objects in the scene, and add them to a map
		RDScape map (i);
		i->ProgressStart (_T("Generating mesh"), TRUE, fn, NULL);
		RDExportEnumerator saveEnumerator(ei->theScene, i, map);
		if (i->GetCancel()) {
			GlobalCancelled = TRUE;
			i->ProgressEnd();
			return FALSE;
		} else
			i->ProgressEnd();

		// Prepare the map
		map.Prepare ();

		if (GetCOREInterface()->GetCancel())
			return FALSE;

		// Save the map
		map.Output (f);
		
		if (GetCOREInterface()->GetCancel())
			return FALSE;

		// Close the output file
		fclose (f);

		return TRUE;
	}
};


/*
 * Red Dog Export Class description
 */

class RDExportClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create (BOOL loading = FALSE) {return new RDExport();}
	const TCHAR *	ClassName() {return GetString(IDS_CLASS_NAME);}
	SClass_ID		SuperClassID() {return SCENE_EXPORT_CLASS_ID;}
	Class_ID		ClassID() {return RDEXPORT_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_CATEGORY);}
	void			ResetClassParams (BOOL fileReset);
};

static RDExportClassDesc RDExportDesc;
ClassDesc* GetRDExportDesc() {return &RDExportDesc;}

//TODO: Should implement this method to reset the plugin params when Max is reset
void RDExportClassDesc::ResetClassParams (BOOL fileReset) 
{

}
