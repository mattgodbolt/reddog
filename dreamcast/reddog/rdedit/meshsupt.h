/*
 * MeshSupt.h
 * Mesh support class
 */

#ifndef _MESHSUPT_H
#define _MESHSUPT_H

#include "util.h"
#include "MatLib.h"
class RDObject;
#include "Library.h"
#include "ApplyVC\EvalCol.h"

#define MAX_MTLS 2048
class RDPoly;
struct BBoxBounds;

class MeshSupport
{
protected:
	// The mesh used to accumulate all the landscape pieces
	Mesh			accMesh;

	// The table of 3dsMax UV transformations
	Matrix3					uvTLib[MAX_MTLS];
	// The library of materials used in this map
	Library<RDMaterial>		matLib;
	// The library of objects used in this map
//	Library<RDObject>		objLib;
public:
	/*
	 * Constructor(s)
	 */
	MeshSupport () { }
	virtual ~MeshSupport() { }

	/*
	 * Methods
	 */
	// Add a mesh (which has the given material)
	void AddMesh (Mtl *mat, const Mesh &mesh, Matrix3 *matrix, char *name, ColorTab *tab = NULL);
	// Add an object
	void AddObject (RDObject *obj);
	// Prepares the internal representation
	virtual void Prepare () = 0;
	// Outputs the converted mesh
	virtual void Output (FILE *f) = 0;
	// Prepare a mesh
	static RDPoly *MeshPrep (Mesh &accMesh, Matrix3 *uvTLib, BBoxBounds &bounds, Library<RDMaterial> matLib);

};

#endif
