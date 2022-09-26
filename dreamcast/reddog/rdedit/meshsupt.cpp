/*
 * Mesh support functions
 */

#include "stdafx.h"
#include "RDObject.h"
#include "Normals.h"
#include "RDStratMap.h"
#include <math.h>

// Faster, special-case combine meshes clone
void RDCombineMeshes (Mesh &mesh, const Mesh &mesh1, const Mesh &mesh2,
					  Matrix3 *tm2)
{
	Matrix3 mat2;
	if (tm2) 
		mat2 = *tm2;
	else
		mat2 = Matrix3(1);
		
	mesh.setNumVerts(mesh1.getNumVerts() + mesh2.getNumVerts());
	mesh.setNumFaces(mesh1.getNumFaces() + mesh2.getNumFaces());
	if (mesh1.getNumTVerts() || mesh2.getNumTVerts()) {
		mesh.setNumTVerts(mesh1.getNumTVerts() + mesh2.getNumTVerts());
		mesh.setNumTVFaces(mesh1.getNumFaces() + mesh2.getNumFaces());
	}
	if (mesh1.getNumVertCol() || mesh2.getNumVertCol()) {
		mesh.setNumVertCol(mesh1.getNumVertCol() + mesh2.getNumVertCol());
		mesh.setNumVCFaces(mesh1.getNumFaces() + mesh2.getNumFaces());
	}
	
	int nv=mesh1.getNumVerts();
	memcpy (mesh.verts, mesh1.verts, sizeof (Point3) * nv);
	for (int i=0; i<mesh2.getNumVerts(); i++) {
		mesh.verts[nv++] = mesh2.verts[i] * mat2;
	}
	
	int nf=mesh1.getNumFaces();
	memcpy (mesh.faces, mesh1.faces, sizeof (Face) * nf);
	for (i=0; i<mesh2.getNumFaces(); i++) {
		mesh.faces[nf] = mesh2.faces[i];
		for (int j=0; j<3; j++) 
			mesh.faces[nf].v[j] += mesh1.getNumVerts();
		nf++;
	}
	
	if (mesh1.getNumTVerts() || mesh2.getNumTVerts()) {
		int ntv=mesh1.getNumTVerts();
		memcpy (mesh.tVerts, mesh1.tVerts, sizeof (UVVert) * ntv);
		for (i=0; i<mesh2.getNumTVerts(); i++) {
			mesh.tVerts[ntv++] = mesh2.tVerts[i];
		}
		
		int ntf=0;
		for (i=0; i<mesh1.getNumFaces(); i++) {
			if (mesh1.tvFace)
				mesh.tvFace[ntf++] = mesh1.tvFace[i];
			else mesh.tvFace[ntf++] = TVFace(0,0,0);
		}
		for (i=0; i<mesh2.getNumFaces(); i++) {
			if (mesh2.tvFace)
				mesh.tvFace[ntf] = mesh2.tvFace[i];
			else mesh.tvFace[ntf] = TVFace(0,0,0);
			for (int j=0; j<3; j++) 
				mesh.tvFace[ntf].t[j] += mesh1.getNumTVerts();
			ntf++;
		}
	}
	if (mesh1.getNumVertCol() || mesh2.getNumVertCol()) {
		int ncv=mesh1.getNumVertCol();
		memcpy (mesh.vertCol, mesh1.vertCol, sizeof (VertColor) * ncv);
		for (i=0; i<mesh2.getNumVertCol(); i++) {
			mesh.vertCol[ncv++] = mesh2.vertCol[i];
		}
		
		int ncf=0;
		for (i=0; i<mesh1.getNumFaces(); i++) {
			if (mesh1.vcFace)
				mesh.vcFace[ncf++] = mesh1.vcFace[i];
			else mesh.vcFace[ncf++] = TVFace(0,0,0);
		}
		for (i=0; i<mesh2.getNumFaces(); i++) {
			if (mesh2.vcFace)
				mesh.vcFace[ncf] = mesh2.vcFace[i];
			else mesh.vcFace[ncf] = TVFace(0,0,0);
			for (int j=0; j<3; j++) 
				mesh.vcFace[ncf].t[j] += mesh1.getNumVertCol();
			ncf++;
		}
	}
}

// Add a mesh to the map;
void MeshSupport::AddMesh (Mtl *mat, const Mesh &mesh, Matrix3 *matrix, char *name, ColorTab *tab /*= NULL */)
{
	/*
	 * Find all the materials for this mesh, and reassign the material
	 * IDs to match the material library
	 */
	Mesh reassignedMesh(mesh);

	/*
	 * Set up the vertex colours from the ColorTab
	 */
	if (tab && tab->Count()) {
		reassignedMesh.setNumVertCol(reassignedMesh.numVerts);
		reassignedMesh.setNumVCFaces(reassignedMesh.numFaces);
		
		for (int i=0; i<reassignedMesh.numVerts; i++) {
			reassignedMesh.vertCol[i] = i<tab->Count() ?
				Point3((*tab)[i]->r, (*tab)[i]->g, (*tab)[i]->b) :
			Point3(1.0f, 1.0f, 1.0f);
		}
		
		for (i=0; i<reassignedMesh.numFaces; i++) {
			reassignedMesh.vcFace[i].t[0] = reassignedMesh.faces[i].v[0];
			reassignedMesh.vcFace[i].t[1] = reassignedMesh.faces[i].v[1];
			reassignedMesh.vcFace[i].t[2] = reassignedMesh.faces[i].v[2];
		}
	}

	int mirror = matrix->Parity();

	if (mat==NULL)
	{
		char buffer[1024];
		if (ALLMAPMODE < GODMODEINTERMEDIATE)
		{	
			sprintf (buffer, "Unable to convert - model '%s' has no material assigned to it", name);
			MessageBox (NULL, buffer, "Aak", MB_OK);
		}
		else
			fprintf (godlog, "Unable to convert - model '%s' has no material assigned to it\n", name);
		return;
	}

	// If there is a material (ie all the time in properly built models)
	if (mat) {
		for (int face = 0; face < mesh.numFaces; ++face) {
			Face &f = reassignedMesh.faces[face];
			// Find the material associated with this face
			int matID = f.getMatID();
			Mtl *m = NULL;
			if (mat->NumSubMtls() > 0 && mat->IsMultiMtl())
				m = mat->GetSubMtl (matID % mat->NumSubMtls());
			// If we couldn't find the submaterial, use the main material
			if (m == NULL)
				m = mat;
			// Find the slot number of this material
			RDMaterial rdMat(*m);
			int slot = matLib.AddObject (&rdMat);
			assert (slot < MAX_MTLS);
			// Set the face's material ID to be the slot number
			f.setMatID (slot);
			// Paranoia checks
			assert (slot == f.getMatID());
			uvTLib[slot] = rdMat.uvTransform;

			// Now mirror the vertices if necessary
			if (mirror) {
				f.setVerts (f.getVert(2), f.getVert(1), f.getVert(0));
				if (reassignedMesh.tvFace) {
					TVFace &tFace = reassignedMesh.tvFace[face];
					tFace.setTVerts (tFace.getTVert(2), tFace.getTVert(1), tFace.getTVert(0));
				}
				if (reassignedMesh.vcFace) {
					TVFace &tFace = reassignedMesh.vcFace[face];
					tFace.setTVerts (tFace.getTVert(2), tFace.getTVert(1), tFace.getTVert(0));
				}
			}

		}
	}
	// Accumulate the meshes
	Mesh tempMesh(accMesh);
	RDCombineMeshes (accMesh, tempMesh, reassignedMesh, matrix);
}


// Add an object to the map
void MeshSupport::AddObject (RDObject *object)
{
//	int slot = objLib.AddObject (object);
}

RDPoly *MeshSupport::MeshPrep (Mesh &accMesh, Matrix3 *uvTLib, BBoxBounds &bounds, Library<RDMaterial> matLib)
{
	RDPoly *polygons = NULL;
	// Compute the face and vertex normals
	Face *face;	
	Point3 *verts;
	Point3 v0, v1, v2;
	Tab<VNormal> vnorms;
	Tab<Point3> fnorms;
	
	face = accMesh.faces;	
	verts = accMesh.verts;
	vnorms.SetCount(accMesh.getNumVerts());
	fnorms.SetCount(accMesh.getNumFaces());
	
	// Compute face and vertex surface normals
	for (int i = 0; i < accMesh.getNumVerts(); i++) {
		vnorms[i] = VNormal();
	}
	for (i = 0; i < accMesh.getNumFaces(); i++, face++) {
		
		// Calculate the surface normal
		v0 = verts[face->v[0]];
		v1 = verts[face->v[1]];
		v2 = verts[face->v[2]];
		fnorms[i] = (v1-v0)^(v2-v1);
		for (int j=0; j<3; j++) {		
			vnorms[face->v[j]].AddNormal(fnorms[i],face->smGroup);
		}
		fnorms[i] = Normalize(fnorms[i]);
	}
	for (i=0; i < accMesh.getNumVerts(); i++) {
		vnorms[i].Normalize();
	}
	
	// Generate a list of polygons	
	int numFaces, f;
	numFaces = accMesh.numFaces;
	for (f = 0; f < numFaces; ++f) {
		RDVertex *vertices;
		Face &face = accMesh.faces[f];
		int smooth = face.smGroup;
		int mtlId = face.getMatID();
		
		// Make a list of the vertices, in order
		vertices = new RDVertex (accMesh.verts[face.v[0]], vnorms[face.v[0]].GetNormal(smooth));
		vertices->next = new RDVertex (accMesh.verts[face.v[1]], vnorms[face.v[1]].GetNormal(smooth));
		vertices->next->next = new RDVertex (accMesh.verts[face.v[2]], vnorms[face.v[2]].GetNormal(smooth));
		
		// Fill in any texture co-ordinates
		if (accMesh.tvFace) {
			TVFace &tface = accMesh.tvFace[f];

			Point3 p1, p2, p3, pp1, pp2, pp3;
			p1 = pp1 = Point3 (accMesh.tVerts[tface.t[0]].x, accMesh.tVerts[tface.t[0]].y, 0.f) * uvTLib[mtlId];
			p2 = pp2 = Point3 (accMesh.tVerts[tface.t[1]].x, accMesh.tVerts[tface.t[1]].y, 0.f) * uvTLib[mtlId];
			p3 = pp3 = Point3 (accMesh.tVerts[tface.t[2]].x, accMesh.tVerts[tface.t[2]].y, 0.f) * uvTLib[mtlId];

			// Find the lowest x and greatest y value
			float lowX, hiY, addX, addY;
			lowX = MIN (p1.x, MIN (p2.x, p3.x));
			hiY  = MAX (p1.y, MAX (p2.y, p3.y));

			// Find the lowest whole number to skew the UV values to [0,...]
			addX = (float) -floor (lowX);
			addY = (float) ceil (hiY);

			// On a clamped texture, don't skew at all
			vertices->u = addX + p1.x;
			vertices->v = addY - p1.y;

			vertices->next->u = addX + p2.x;
			vertices->next->v = addY - p2.y;

			vertices->next->next->u = addX + p3.x;
			vertices->next->next->v = addY - p3.y;
		}
		// Is there any colour information
		if (accMesh.numCVerts && accMesh.vertCol && accMesh.vcFace) {
			TVFace &tface = accMesh.vcFace[f];
			VertColor c1, c2, c3;
			Uint32 u1, u2, u3;
			Uint32 opacity;
			c1 = accMesh.vertCol[tface.t[0]];
			c2 = accMesh.vertCol[tface.t[1]];
			c3 = accMesh.vertCol[tface.t[2]];
			u1 = (int(c1.z * 255.f)) | ((int(c1.y * 255.f))<<8) | ((int(c1.x * 255.f))<<16);
			u2 = (int(c2.z * 255.f)) | ((int(c2.y * 255.f))<<8) | ((int(c2.x * 255.f))<<16);
			u3 = (int(c3.z * 255.f)) | ((int(c3.y * 255.f))<<8) | ((int(c3.x * 255.f))<<16);

			if (matLib.Count()) {
				opacity = matLib[mtlId % matLib.Count()].getOpacity();
			} else {
				opacity = 0xff000000;
			}

#if SHOWUPPOLYGONS
			u1 = 0xff0000;
			u2 = 0x00ff00;
			u3 = 0x0000ff;
#endif
			vertices->colour = opacity | u1;
			vertices->next->colour = opacity | u2;
			vertices->next->next->colour = opacity | u3;
		}
		
		bool invis = FALSE;
		if (matLib.Count() && matLib[mtlId % matLib.Count()].isInvisible())
			invis = TRUE;
		polygons = new RDPoly (mtlId, vertices, polygons, fnorms[f].x, fnorms[f].y, fnorms[f].z, invis);
		BBoxBounds b = polygons->getBounds();
		bounds.Merge (b);
	}

	return polygons;
}
