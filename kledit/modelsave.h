#ifndef _MODELSAVE_H
#define _MODELSAVE_H

#include "RedDog.h"

#include "BBox.h"
#include "Library.h"

struct PointAndNormal
{
	RedDog::Point3	point;
	RedDog::Vector3	normal;
	inline bool operator == (PointAndNormal &p)
	{
		return point.x==p.point.x && point.y==p.point.y && point.z==p.point.z &&
			normal.x==p.normal.x && normal.y==p.normal.y && normal.z==p.normal.z;
	}
};

typedef Library<PointAndNormal> VertexLib;

class RDSave
{
private:
	static int StripOutput (RDPoly *polygons, VertexLib &library, RedDog::Point3 centre, FILE *f, StripList *list);
public:
	static void RDSave::SaveRedDogModel (const BBoxBounds &boundary, RDPoly *polygons, FILE *f,
		RedDog::Point3 centre, const Library<RDMaterial> *matLib, bool Stripify, RedDog::Uint32 ModelFlags);
	static void RDSave::SaveMaterials (const Library<RDMaterial> *matLib, FILE *f);
	static RDPoly *RDSave::Sort (RDPoly *polygons);
	static RDPoly *RDSave::RemoveSingular (RDPoly *polygons);
	static void RDSave::SaveMap (const Library<RDMaterial> *matLib, FILE *f);

};
#endif
