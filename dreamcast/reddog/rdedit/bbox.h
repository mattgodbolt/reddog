/*
 * BBox.h
 * Bounding box representation of the map
 */

#ifndef _BBOX_H
#define _BBOX_H

#include "util.h"
#include "simpobj.h"
#include "MatLib.h"
#include "RedDog.h"
#include "Library.h"
#include "ShadBox.h"

#include "Strip.h"

#include "VertLib.h"

/*
 * A vertex in bounding box representation
 */
struct RDVertex
{
	RDVertex	*next;					// The next vertex within a polygon
	float		x, y, z, u, v;			// The position, and UV value of this point
	float		nx, ny, nz;				// The vertex normal
	Uint32		colour;

	RDVertex (Point3 &p, Point3 &normal, float u=0.0, float v=0.0, Uint32 col = 0xffffffff)
	{
		x = p.x; y = p.y; z = p.z;
		nx = normal.x; ny = normal.y; nz = normal.z;
		this->u = u; this->v = v;
		next = NULL;
		colour = col;
	}
	~RDVertex () { if (next) delete next; }
	operator Point3 () { return Point3(x,y,z); }
};

/*
 * A bounding box's co-ordinates
 */
struct BBoxBounds
{
	float		lowX, lowY, lowZ;		// The co-ordinates of the far, left, bottom point of the box
	float		hiX, hiY, hiZ;			// The co-ordinates of the near, right, top point of the box7
	BBoxBounds (float lowX, float lowY, float lowZ, float hiX, float hiY, float hiZ)
	{
		this->lowX = lowX; this->lowY = lowY; this->lowZ = lowZ;
		this->hiX = hiX; this->hiY = hiY; this->hiZ = hiZ;
	}

	// Centre of the box
	inline Point3	Centre () const
	{
		return Point3 ((hiX + lowX) / 2.0, (hiY + lowY) / 2.0, (hiZ + lowZ) / 2.0);
	}

	inline float LongestEdge () const
	{
		return MAX(hiX - lowX, hiY - lowY);
	}

	// Merge another box with this one
	void Merge (BBoxBounds &box)
	{
		if (box.hiX > hiX)
			hiX = box.hiX;
		if (box.lowX > hiX)
			hiX = box.lowX;
		if (box.hiX < lowX)
			lowX = box.hiX;
		if (box.lowX < lowX)
			lowX = box.lowX;

		if (box.hiY > hiY)
			hiY = box.hiY;
		if (box.lowY > hiY)
			hiY = box.lowY;
		if (box.hiY < lowY)
			lowY = box.hiY;
		if (box.lowY < lowY)
			lowY = box.lowY;

		if (box.hiZ > hiZ)
			hiZ = box.hiZ;
		if (box.lowZ > hiZ)
			hiZ = box.lowZ;
		if (box.hiZ < lowZ)
			lowZ = box.hiZ;
		if (box.lowZ < lowZ)
			lowZ = box.lowZ;

	}
	// Return the volume enclosed
	inline float Volume () const
	{
		return float((hiX-lowX) * (hiY - lowY) * (hiZ - lowZ));
	}
	inline float SqrRadius() const
	{
		return LengthSquared (Centre() - Point3(lowX, lowY, lowZ));
	}
};


/* 
 * A poly, as stored in the bounding box
 */
class RDPoly
{
private:
	RDPoly	*next;						// The next polygon in this bounding box
	RDPoly	*prev;						// The previous polygon
	int			material;				// The material of this polygon
	RDVertex	*vertices;				// The list of vertices;
	float		nx, ny, nz;				// The normal to this polygon
	bool		bInvisible;				// Is this polygon invisible (ie collision poly only)
	int			boxNumber;				// The BBox number this polygon lives in
	int			polyNumber;				// The BBox poly number
	float		leftMost, rightMost, upMost, downMost;
	friend class RDSave;
	friend class BBox;
	friend class BBoxLandscape;
	friend class StripPoly;
public:
	RDPoly (int material, RDVertex *vertices = NULL, RDPoly *next = NULL,
		float nx = 0.0, float ny = 0.0, float nz = 0.0, bool invis = FALSE)
	{ this->material = material; this->vertices = vertices; this->next = next; 
	  bInvisible = invis;
	  this->nx = nx; this->ny = ny; this->nz = nz;
	  if (next)
		  next->prev = this;
	  prev = NULL;
	  boxNumber = -1;
	}
	~RDPoly ()
	{ if (vertices) delete vertices; if (next) delete next; }

	// Return an invisible, reversed version of this polygon for collision only
	RDPoly *GetReversedVersion(void)
	{
		RDPoly *retVal = new RDPoly (material);
		*retVal = *this;
		retVal->bInvisible = true;
		// Reverse the verts
		retVal->vertices = new RDVertex (
			Point3(vertices->next->next->x, vertices->next->next->y, vertices->next->next->z), 
			Point3(vertices->next->next->nx, vertices->next->next->ny, vertices->next->next->nz), 
			vertices->next->next->u, vertices->next->next->v,
			vertices->next->next->colour);
		retVal->vertices->next = new RDVertex (
			Point3(vertices->next->x, vertices->next->y, vertices->next->z), 
			Point3(vertices->next->nx, vertices->next->ny, vertices->next->nz), 
			vertices->next->u, vertices->next->v,
			vertices->next->colour);
		retVal->vertices->next->next = new RDVertex (
			Point3(vertices->x, vertices->y, vertices->z), 
			Point3(vertices->nx, vertices->ny, vertices->nz), 
			vertices->u, vertices->v,
			vertices->colour);
		retVal->prev = retVal->next = NULL;
		return retVal;
	}

	// Return the normal
	inline Point3 Normal() const { return Point3 (nx, ny, nz); }

	// Return the material
	inline Uint16 getMaterial() const { return material; }

	inline int isInvisible() const { return bInvisible; }

	RDVertex *getVertex(int n)
	{
		RDVertex *vert;
		vert = vertices;
		while (n--)
			vert = vert->next;
		return vert;
	}

	float getHighestPoint (void)
	{
		float pt = -10000.f;
		RDVertex *vert;
		for (vert = vertices; vert; vert = vert->next)
			if (vert->z > pt)
				pt = vert->z;
		return pt;
	}

	void PrepCol()
	{
		leftMost = rightMost = vertices->x;
		upMost = downMost = vertices->y;
		for (RDVertex *v = vertices->next; v; v = v->next) {
			if (v->x < leftMost)
				leftMost = v->x;
			if (v->x > rightMost)
				rightMost = v->x;
			if (v->y > upMost)
				upMost = v->y;
			if (v->y < downMost)
				downMost = v->y;
		}
	}

	inline float LeftMost() const
	{
		return leftMost;
	}
	inline float RightMost() const
	{
		return rightMost;
	}
	inline float UpMost() const
	{
		return upMost;
	}
	inline float DownMost() const
	{
		return downMost;
	}

	// Get a bounding box for this polygon
	BBoxBounds	getBounds () const
	{
		float lowX, lowY, lowZ;
		float hiX, hiY, hiZ;
		const RDVertex *v;

		if (vertices == NULL)
			return BBoxBounds (0, 0, 0, 0, 0, 0);

		lowX = hiX = vertices->x;
		lowY = hiY = vertices->y;
		lowZ = hiZ = vertices->z;

		// Check the other vertices
		for (v = vertices->next; v; v = v->next) {
			if (v->x < lowX)
				lowX = v->x;
			else if (v->x > hiX)
				hiX = v->x;
			if (v->y < lowY)
				lowY = v->y;
			else if (v->y > hiY)
				hiY = v->y;
			if (v->z < lowZ)
				lowZ = v->z;
			else if (v->z > hiZ)
				hiZ = v->z;
		}

		return BBoxBounds (lowX, lowY, lowZ, hiX, hiY, hiZ);
	}

	// Is this poly completely within this bounding box?
	int isCompletelyInside (const BBoxBounds &boundary) const
	{
		const RDVertex *v;
		for (v = vertices; v; v = v->next) {
			if (!(v->x >= boundary.lowX && v->x <= boundary.hiX &&
				v->y >= boundary.lowY && v->y <= boundary.hiY &&
				v->z >= boundary.lowZ && v->z <= boundary.hiZ))
			{
				// A point outside the box
				return 0;
			}
		}
		return 1;
	}

	// Is this poly partially within this bounding box?
	int isPartiallyInside (const BBoxBounds &boundary) const
	{
		const RDVertex *v;
		bool inside, outside;
		inside = outside = FALSE;

		for (v = vertices; v; v = v->next) {
			if (!(v->x >= boundary.lowX && v->x <= boundary.hiX &&
				v->y >= boundary.lowY && v->y <= boundary.hiY &&
				v->z >= boundary.lowZ && v->z <= boundary.hiZ))
			{
				// A point outside the box
				outside = true;
			} else {
				inside = true;
			}
		}
		return inside && outside;
	}

	bool CollidesWith (const BBoxBounds &boundary) const;

	inline RDPoly *getNext() const { return next; }

	// How many polys in this list?
	int Count() const
	{
		int retVal = 0;
		const RDPoly *p;
		for (p = this; p; p = p->next)
			retVal++;
		return retVal;
	}

	/*
	 * Unlink a poly from the list starting at 'this'
	 * Returns the new head of the linked list, so use as
	 * list = list->Unlink (poly)
	 */
	RDPoly *Unlink (RDPoly *poly)
	{
		assert (poly != NULL);
		if (poly == this) {
			assert (poly->prev == NULL);
			RDPoly *retVal = poly->next;
			if (poly->next)
				poly->next->prev = NULL;
			poly->next = NULL;
			return retVal;
		} else {
			if (poly->next)
				poly->next->prev = poly->prev;
			if (poly->prev)
				poly->prev->next = poly->next;
			poly->next = poly->prev = NULL;
			return this;
		}
	}

	/*
	 * Links a poly onto the head of a linked list
	 * Use as list = list->Link (poly)
	 */
	RDPoly *Link (RDPoly *poly)
	{
		assert (poly != NULL);
		assert (poly->next == NULL);
		assert (poly->prev == NULL);
		if (this)
			this->prev = poly;
		poly->next = this;
		return poly;
	}

	RDPoly *ForceLink (RDPoly *poly)
	{
		assert (poly != NULL);
		if (this)
			this->prev = poly;
		poly->next = this;
		return poly;
	}

	int Degree () const
	{
		int ret;
		const RDVertex *v;
		for (ret = 0, v = vertices; v; v=v->next)
			ret++;
		return ret;
	}
	void AppendVerts(Tab<RDVertex *> &invisVerts) const {
		for (RDVertex *vert = vertices; vert; vert = vert->next)
			invisVerts.Append (1, &vert);
	}

};

/*
 * A bounding box
 * Bounding boxes are volumes bounded by two points, 
 * and are considered as axis-aligned boxes
 */
class BBox
{
private:
	BBox		*next;					// The next box in the landscape
	RDPoly 		*polygons;
	bool		needsBounds;
	BBoxBounds	boundary;
	StripList	*stripList;				// The list of strips
	Tab<RDPoly *>	polyList;			// The list of polys
	friend class BBoxLandscape;
public:
	VertexLibrary vertLib;				// The vertex library
	BBox () : boundary(0,0,0,0,0,0)
	{
		next = NULL;
		polygons = NULL;
		needsBounds = true;
		polyList.SetCount(0);
	}
	~BBox () 
	{ 
		if (polygons) delete polygons;
		if (next) delete next; 
	}
	void SetNext (BBox *n) { next = n; }
	inline const BBox *getNext() const		{ return next; }
	int Count(int foo=0) const 
	{
		if (polygons) {
			if (!next)
				return (foo+1);
			return next->Count(foo+1);			
		} else {
			if (!next)
				return foo;
			return next->Count(foo);
		}
	}
	void ExportMap (FILE *f, int num, Tab<RedDog::VisBox> &visBoxes, Tab<IShadBox *> &shadowBox);

	int AddPoly (RDPoly *poly)
	{
		needsBounds = true;
		return polyList.Append (1, &poly, polyList.Count());
	}
	int AddPoly (Tab<RDPoly *> &pList)
	{
		needsBounds = true;
		return polyList.Append (pList.Count(), &pList[0], polyList.Count());
	}
	void UpdateBoundary()
	{
		if (needsBounds) {
			int i, max;
			if (polyList.Count() != 0) {
				boundary = polyList[0]->getBounds();
				for (i = 0, max = polyList.Count(); i < max; ++i)
					boundary.Merge (polyList[i]->getBounds());
			}
		}
		needsBounds = false;
	}
	BBoxBounds getBoundary()  const
	{
		assert (!needsBounds);
		return boundary;
	}
};

/*
 * The entire landscape stored as a list of bounding boxed
 */
class BoxTree;
class BBoxLandscape
{
private:
	BBox			*boxList;			// List of bounding boxes
	Tab<IShadBox *>	shadBox;
	float			minX, minY;
	float			maxX, maxY;
	float			areaDone, areaTot;
	/*
	 * Partition a set of polygons along an axis
	 */
	typedef Tab<RDPoly *> PolyTab;
	void PartitionX (const PolyTab &source, PolyTab &left, PolyTab &right, float pos) const;
	void PartitionY (const PolyTab &source, PolyTab &left, PolyTab &right, float pos) const;
	RedDog::CollNode *Partition (PolyTab &parent, BBoxBounds &bounds, int depth);
	void DeleteTree (RedDog::CollNode *node);
	void RecurseWriteNode (RedDog::CollNode *node, FILE *f);
	BoxTree	*MakeHierarchy();

	RedDog::CollNode *topNode;
	// List of ghost polygons that need freeing
	PolyTab			ghostList;
	// The root list of collisionable polygons
	PolyTab			rootList;
public:
	BBoxLandscape (Mesh &mesh, Matrix3 *uvTLib, Library<RDMaterial> &matLib, Tab<IShadBox *> shadowBox);
	// Build a box representation
#if NEWCOLL
	~BBoxLandscape () 
	{ 
		if (boxList) delete boxList; if (topNode) DeleteTree (topNode); 
		for (int i = 0; i < ghostList.Count(); ++i)
			delete ghostList[i];
	}
#else
	~BBoxLandscape () { if (boxList) delete boxList; if (collGrid) delete [] collGrid; }
#endif
	void ExportMap (FILE *f, Library<RDMaterial> &matLib, Tab<RedDog::VisBox> &visBoxes);
};

class BoxTree
{
private:
	bool		isLeaf;
	int			modelNum;
	BBoxBounds	boundary;
	BoxTree		*child[4];
	int			boxNumber;
public:
	BoxTree(const BBox *model, int num) : boundary(model->getBoundary())
	{
		modelNum = num;
		isLeaf = true;
		for (int i = 0; i < 4; ++i)
			child[i] = NULL;
	}
	BoxTree(BoxTree *a, BoxTree *b, BoxTree *c = NULL, BoxTree *d = NULL) : boundary(a->boundary)
	{
		boundary.Merge(b->boundary);
		if (c)
			boundary.Merge(c->boundary);
		if (d)
			boundary.Merge(d->boundary);
		isLeaf = false;
		modelNum = -1;
		child[0] = a;
		child[1] = b;
		child[2] = c;
		child[3] = d;
	}
	~BoxTree()
	{
		if (!isLeaf) {
			for (int i = 0; i < 4; ++i)
				delete child[i];
		}
	}
	Point3 midPoint() const
	{
		return boundary.Centre();
	}
	
	void Number(int *n)
	{
		boxNumber = *n;
		(*n)++;
		if (!isLeaf) {
			for (int i = 0; i < 4; ++i) {
				if (child[i])
					child[i]->Number(n);
			}
		}
	}
	void WriteOut (FILE *f);
};

#endif
