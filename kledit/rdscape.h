#ifndef _RDScape_H
#define _RDScape_H

#include <fstream.h>
#include <max.h>

#define RDM_VERSION 100

#define RDEXPORT_CLASS_ID	Class_ID(0x61dc6f59, 0x612f5466)

#include "util.h"
#include "MatLib.h"
#include "RDObject.h"
#include "Library.h"
#include "RedDog.h"
#include "BBox.h"
#include "MeshSupt.h"
#include "shadbox.h"

BOOL Canceled();
void setProgressText (char *text, int pc = 0);
void setProgressName (char *name);

class RDScape : public MeshSupport
{
private:
	// Instance vars:
	Interface		*iface;
	BBoxLandscape	*box;
	Tab<RedDog::VisBox>		visibilityBoxes;
public:
	/*
	 * Constructor(s)
	 */
	RDScape (Interface *i) {iface = i; box = NULL; }
	virtual ~RDScape() { if (box) delete box; }

	/*
	 * Methods
	 */
	// Prepares the internal representation
	void Prepare ();
	// Outputs the converted mesh
	void Output (FILE *f);
	// Counts the triangles in the current 3DS stored map
	int getTriCount ();
	// Add a visibility box
	void AddBox (RedDog::VisBox *box)
	{
		visibilityBoxes.Append (1, box, 32);
	}
};

// Method used to get the Export class description
extern ClassDesc* GetRDExportDesc();

#endif
