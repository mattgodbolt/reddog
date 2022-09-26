#ifndef _RDStratMap_H
#define _RDStratMap_H

#include <fstream.h>
#include <max.h>

#define RDM_VERSION 100

#define RDSTRATEXPORT_CLASS_ID	Class_ID(0x79ba1050, 0x6fea27f9);

#include "util.h"
#include "MatLib.h"
#include "RDObject.h"
#include "levelflags.h"
#include "Library.h"
#include "RedDog.h"
#include "BBox.h"
#include "MeshSupt.h"
#include "object.h"
#include "polyshp.h"
#include "istdplug.h"

#define GODMODEINTERMEDIATE 2
#define GODMODEFINAL 3

extern FILE *godlog;


extern int ALLMAPMODE,GDSRUN,CODERUN;

// Method used to get the Export class description
extern ClassDesc* GetRDStratExportDesc();

#endif
