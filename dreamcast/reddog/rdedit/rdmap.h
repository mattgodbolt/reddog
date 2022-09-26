#ifndef _RDMAP_H
#define _RDMAP_H

#include <fstream.h>
#include <max.h>

#define RDM_VERSION 100

#define RDEXPORT_CLASS_ID	Class_ID(0x61dc6f59, 0x612f5466)

#include "util.h"
#include "MatLib.h"
#include "RDObject.h"
#include "Library.h"
#include "Ninja.h"
#include "BBox.h"

BOOL Canceled();
void setProgressText (char *text, int pc = 0);

class RDMap
{
private:
	// Class vars:
	// The theshold angle
	static float	threshold;

	// An interface to MAX
	Interface		*iface;

	// The mesh used to accumulate all the landscape pieces
	Mesh			accMesh;
	BBoxLandscape	*box;

	// The library of materials used in this map
	Library<RDMaterial>		matLib;
	// The library of objects used in this map
	Library<RDObject>		objLib;
public:
	/*
	 * Constructor(s)
	 */
	RDMap (Interface *i) {box = NULL; iface = i; }
	virtual ~RDMap() { if (box) delete box; }

	/*
	 * Methods
	 */
	// Add a mesh to the map (which has the given material)
	void AddMesh (Mtl *mat, const Mesh &mesh, Matrix3 *matrix);
	// Add an object to the map
	void AddObject (RDObject *obj);
	// Prepares the internal representation
	void Prepare ();
	// Outputs the converted mesh
	void Output (FILE *f);
	// Counts the triangles in the current 3DS stored map
	int getTriCount ();
	// Get the current angle to prepare at
	inline float getAngle () { return threshold; }
	inline void setAngle (float angle) { threshold = angle; }
};

// Method used to get the Export class description
extern ClassDesc* GetRDExportDesc();

#endif
