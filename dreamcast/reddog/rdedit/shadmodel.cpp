/*
 * Shadow model exporter
 */

#include "stdafx.h"
#include "ObjFlags.h"
#include "PLUGAPI.h"
#include "MeshAdj.h"
#include <stddef.h>

class ShadMunger : public MeshSupport
{
private:
	Tab<Mesh *>	meshes;
	void RecurseAcc (INode *node)
	{
		int i;
		// Retrieve the TriObject from the node
		int deleteIt;
		Object *obj = node->EvalWorldState(GetCOREInterface()->GetTime()).obj;
		if (obj && !((obj->ClassID() == RDOBJECT_CLASS_ID)
			||
			(obj->ClassID() == Class_ID(CIRCLE_CLASS_ID,0))
			||
			(obj->SuperClassID() == SHAPE_CLASS_ID))) {

			TriObject *triObject = GetTriObjectFromNode (GetCOREInterface(), node, deleteIt);
			
			// Use the TriObject if available
			if (triObject) {
				// Add this mesh to the object
				Matrix3 transform = node->GetObjectTM(0);
				
				AddMesh (node->GetMtl(), triObject->mesh, &transform, node->GetName());

				// Take a copy of the mesh, if it had any data in it
				if (accMesh.numVerts) {
					accMesh.RemoveDegenerateFaces();
					Mesh *copy = new Mesh(accMesh);
					meshes.Append(1, &copy);
				}
				accMesh.FreeAll();
				
				// Delete it when done...
				if (deleteIt) triObject->DeleteMe();
			}		
		}
		for (i = 0; i < node->NumberOfChildren(); ++i)
			RecurseAcc (node->GetChildNode(i));
	}
	AdjEdgeList *edgeList;
public:
	ShadMunger ()
	{
		RecurseAcc (GetCOREInterface()->GetRootNode());
		edgeList = NULL;
	}
	~ShadMunger() 
	{
		for (int i=0; i < meshes.Count(); ++i)
			delete meshes[i];
	}
	void Prepare()
	{
		edgeList = NULL;
	}
	void Output(FILE *f)
	{
		long position;
		Library<Point3> vertLib;
		RedDog::ShadModel sModel;
		sModel.nHulls = meshes.Count();
		position = ftell (f) + offsetof(RedDog::ShadModel, nVertices);
		fwrite (&sModel, sizeof (sModel), 1, f);
		for (int i = 0; i < sModel.nHulls; ++i) {
			accMesh = *meshes[i];
			if (edgeList)
				delete edgeList;
			edgeList = new AdjEdgeList (accMesh);
			// Write out the header
			RedDog::ShadHull sModel;
			sModel.nEdges = edgeList->edges.Count();
			sModel.nFaces = accMesh.numFaces;
			fwrite (&sModel, sizeof (sModel), 1, f);
			
			// Write out the faces
			for (int face = 0; face < sModel.nFaces; ++face) {
				RedDog::ShadFace sFace;
				sFace.culled = FALSE;
				sFace.edges[0] = (RedDog::ShadEdge *)edgeList->FindEdge (accMesh.faces[face].v[0], accMesh.faces[face].v[1]);
				sFace.edges[1] = (RedDog::ShadEdge *)edgeList->FindEdge (accMesh.faces[face].v[1], accMesh.faces[face].v[2]);
				sFace.edges[2] = (RedDog::ShadEdge *)edgeList->FindEdge (accMesh.faces[face].v[2], accMesh.faces[face].v[0]);
				Point3 p[3];
				for (int i = 0; i < 3; ++i) {					 
					sFace.p[i] = vertLib.AddObject(&accMesh.verts[accMesh.faces[face].v[i]]);
				}
				Point3 xP = Normalize (CrossProd(accMesh.verts[accMesh.faces[face].v[1]] - accMesh.verts[accMesh.faces[face].v[0]], accMesh.verts[accMesh.faces[face].v[2]] - accMesh.verts[accMesh.faces[face].v[0]]));
				sFace.faceNormal.x = xP.x;
				sFace.faceNormal.y = xP.y;
				sFace.faceNormal.z = xP.z;
				
				fwrite (&sFace, sizeof (sFace), 1, f);
			}
			// Write out the edges
			for (int edge = 0; edge < sModel.nEdges; ++edge) {
				RedDog::ShadEdge sEdge;
				sEdge.face[0] = (RedDog::ShadFace *)edgeList->edges[edge].f[0];
				sEdge.face[1] = (RedDog::ShadFace *)edgeList->edges[edge].f[1];
				
				for (int i = 0; i < 3; ++i) {
					// Get the vertex number of face 0 vertex i
					DWORD f0Num = accMesh.faces[edgeList->edges[edge].f[0]].v[i];
					if (f0Num == edgeList->edges[edge].v[0])
						break;
				}
				assert (i != 3);
				sEdge.v0 = i;
				// Next vertex
				for (i = 0; i < 3; ++i) {
					// Get the vertex number of face 0 vertex i
					DWORD f0Num = accMesh.faces[edgeList->edges[edge].f[0]].v[i];
					if (f0Num == edgeList->edges[edge].v[1])
						break;
				}
				assert (i != 3);
				sEdge.v1 = i;
				
				fwrite (&sEdge, sizeof (sEdge), 1, f);
			}
		}
		// Now write out the vertex library
		fwrite (vertLib.Addr(0), sizeof (Point3), vertLib.Count(), f);
		// Kludge the number of vertices back into the doobry
		fseek (f, position, SEEK_SET);
		int nVerts = vertLib.Count();
		fwrite (&nVerts, 4, 1, f);
	}
};

class ShadExporter : public SceneExport {
public:
	/*
     * MAX API
	 */
	// Returns the number of supported extensions
	virtual int ExtCount() { return 1; }
	// Returns the file extension for a given number
	virtual const TCHAR *Ext(int i) { assert(i==0); return _T("RDS"); };
	// Returns the short and long descriptions, and other strings
	virtual const TCHAR *LongDesc() { return _T("Argonaut Software Red Dog shadow"); }
	virtual const TCHAR *ShortDesc() { return _T("Red Dog shadow"); }
	virtual const TCHAR *AuthorName() { return _T("Argonaut Software"); }
	virtual const TCHAR *CopyrightMessage() { return _T("(c) 1998 Argonaut Software Ltd"); }
	virtual const TCHAR *OtherMessage1() { return _T(""); }
	virtual const TCHAR *OtherMessage2() { return _T(""); }
	// Returns the version of the plugin
	virtual unsigned int Version() { return 1; }
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
		assert (f);

		ShadMunger munge;
		munge.Prepare();
		munge.Output (f);

		fclose (f);
		return TRUE;
	}
	
};

/*
 * Red Dog object class description
 */

class ShadClassDesc:public ClassDesc {
	public:
	int 			IsPublic() {return 1;}
	void *			Create (BOOL loading = FALSE) {return new ShadExporter;}
	const TCHAR *	ClassName() {return "RDSExport";}
	SClass_ID		SuperClassID() {return SCENE_EXPORT_CLASS_ID;}
	Class_ID		ClassID() {return Class_ID(0x7fd60079, 0x65fc081c);}
	const TCHAR* 	Category() {return GetString(IDS_CATEGORY);}
	void			ResetClassParams (BOOL fileReset) { }
};

static ShadClassDesc ShadDesc;
ClassDesc* GetShadDesc() {return &ShadDesc;}
