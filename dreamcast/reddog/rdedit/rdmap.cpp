/*
 * RDScape.cpp - the map file format
 */

#include <math.h>
#include "RDScape.h"
#include "RDEdit.h"

// Class variables
float RDScape::threshold = 1.0f;

// The progress box
static Interface *progress = NULL;

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
	if (text)
		progress->ProgressUpdate (pc, FALSE, _T(text));
	else
		progress->ProgressUpdate (pc);
}

// Add a mesh to the map;
void RDScape::AddMesh (Mtl *mat, const Mesh &mesh, Matrix3 *matrix)
{
	/*
	 * Find all the materials for this mesh, and reassign the material
	 * IDs to match the material library
	 */
	Mesh reassignedMesh(mesh);

	// Check to see if the matrix will invert the order of the clockwise
	int mirror = (matrix && matrix->GetRow(0).x * matrix->GetRow(1).y * matrix->GetRow(2).z > 0) ? 0 : 1;

	// If there is a material (ie all the time in properly built models)
	if (mat) {
		for (int face = 0; face < mesh.numFaces; ++face) {
			Face &f = reassignedMesh.faces[face];
			// Find the material associated with this face
			int matID = f.getMatID();
			Mtl *m = mat->GetSubMtl (matID);
			// If we couldn't find the submaterial, use the main material
			if (m == NULL)
				m = mat;
			// Find the slot number of this material
			RDMaterial rdMat(*m);
			int slot = matLib.AddObject (&rdMat);
			// Set the face's material ID to be the slot number
			f.setMatID (slot);
			// Paranoia check
			assert (slot == f.getMatID());
			// Now mirror the vertices if necessary
			if (mirror) {
				f.setVerts (f.getVert(2), f.getVert(1), f.getVert(0));
				if (reassignedMesh.tvFace) {
					TVFace &tFace = reassignedMesh.tvFace[face];
					tFace.setTVerts (tFace.getTVert(2), tFace.getTVert(1), tFace.getTVert(0));
				}
			}
		}
	}
	// Accumulate the meshes
	Mesh tempMesh(accMesh);
	CombineMeshes (accMesh, tempMesh, reassignedMesh, NULL, matrix, -1);
}

// Counts the tris in the accumulated mesh
int RDScape::getTriCount ()
{
	return accMesh.numFaces;
}

// Add an object to the map
void RDScape::AddObject (RDObject *object)
{
	int slot = objLib.AddObject (object);
}

static BOOL CALLBACK progressProc (HWND dBox, UINT msg, WPARAM wp, LPARAM lp)
{
	return FALSE;
}

// Prepares the map data ready for output
void RDScape::Prepare ()
{
	// Initialise the progress stuff
	iface->ProgressStart (_T("Building Box list"), TRUE, fn, NULL);
	progress = iface;

	setProgressText (NULL, 0);
	box = new BBoxLandscape (accMesh);
	//box->IterateShowBoxes (iface);

	progress->ProgressEnd();
	progress = NULL;
}

// Outputs the converted mesh
void RDScape::Output (FILE *f)
{
	box->ExportMap (f, matLib);
}

/*
 * The saving enumerator
 */
class RDExportEnumerator : public ITreeEnumProc {
private:
	// The interface to max
	Interface *maxInterface;
	// The map we're building
	RDScape &map;

public:
	RDExportEnumerator (IScene *scene, Interface *i, RDScape &m) : map(m)
	{
		assert (i != NULL);
		// Set up the variables
		maxInterface = i;
		scene->EnumTree (this);
	}
	
	// A function called once per object to save it
	int callback (INode *node)
	{
		Object *obj = node->EvalWorldState(maxInterface->GetTime()).obj;
		// Is it a Red Dog Object?
		if (obj->ClassID() == RDOBJECT_CLASS_ID) {

			map.AddObject ((RDObject *)node);
			return TREE_CONTINUE;
		} else {
			
			// Retrieve the TriObject from the node
			int deleteIt;
			TriObject *triObject = GetTriObjectFromNode (maxInterface, node, deleteIt);
			
			// Use the TriObject if available
			if (!triObject) return TREE_CONTINUE;
			
			// Add this mesh to the map
			Matrix3 transform = node->GetObjectTM(0);
			map.AddMesh (node->GetMtl(), triObject->mesh, &transform);
			
			// Delete it when done...
			if (deleteIt) triObject->DeleteMe();
			
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
	virtual int DoExport (const TCHAR *name, ExpInterface *ei, Interface *i, BOOL suppressPrompts /* = FALSE */)
	{
		FILE *f = fopen (name, "wb");
		if (f==NULL)
			return FALSE;
		// Enumerate through all the objects in the scene, and add them to a map
		RDScape map (i);
		RDExportEnumerator saveEnumerator(ei->theScene, i, map);

		// Prepare the map
		map.Prepare ();

		// Save the map
		map.Output (f);
		
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
