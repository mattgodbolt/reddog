/*
 * BSP.h
 * BSP generation
 */

#ifndef _BSP_H
#define _BSP_H

#include "util.h"
#include "Ninja.h"
#include "Library.h"

// Various classes used for intermediate storage
struct BSPPlane
{
	float aa,bb,cc,dd;
};

struct BSPVertex
{
	BSPVertex	*next;					// The next vertex within a polygon
	float		x, y, z, u, v;			// The position, and UV value of this point
	BOOL		visited;				// Used in the clipping process
	BOOL		inside;					// Used in the clipping process
	BSPVertex	*link;					// Used in the clipping process
	double		dist;					// Used in the clipping process
	BSPVertex	*nextCreated;			// Used in the clipping process
	BSPVertex (Point3 &p, float u=0.0, float v=0.0)
	{ 
		x = p.x; y = p.y; z = p.z;
		this->u = u; this->v = v;
		link = next = nextCreated = NULL;
		visited = FALSE;
	}
	~BSPVertex() { if (next) delete next; }
};

class BSPPoly
{
private:
	BSPPoly		*next;					// The next polygon in the list
	int			material;				// The material assigned to the polygon
	BSPVertex	*vertices;				// The list of vertices
public:
	/*
	 * Constructor
	 */
	BSPPoly (int material, BSPPoly *list = NULL, BSPVertex *vertices = NULL)
	{ 
		next = list;
		this->material = material;
		this->vertices = vertices;
	}

	// Destructor
	~BSPPoly () { if (vertices) delete vertices; if (next) delete next; }
	
	// Is the polygon convex?
	BOOL BSPPoly::isConvex () const;
	
	// Append a vertex to the polygon
	inline void AppendVertex (BSPVertex *v)
	{
		BSPVertex *foo;
		if (vertices == NULL) {
			vertices = v;
			vertices->next = NULL;
		} else {
			for (foo = vertices; foo->next; foo = foo->next)
				;
			foo->next = v;
			v->next = NULL;
		}
	}

	// Orphans a polygon
	inline void Orphan() { next = NULL; }

	// Count the number of polygons in a list
	inline int Count() const
	{
		const BSPPoly *p;
		int count = 0;
		for (p = this; p; p = p->next)
			count++;
		return count;
	}

	// Return the degree of the poly
	inline int Degree() const
	{
		int retval = 0;
		const BSPVertex *p;
		for (p=vertices; p; p=p->next)
			retval++;
		return retval;
	}

	// Return the given vertex
	inline const BSPVertex *GetVertex (int num) const
	{
		const BSPVertex *retVal;
		for (retVal = vertices; retVal; retVal = retVal->next)
			if (num-- == 0)
				break;
		return retVal;
	}
	inline BSPVertex *GetVertex (int num) 
	{
		BSPVertex *retVal;
		for (retVal = vertices; retVal; retVal = retVal->next)
			if (num-- == 0)
				break;
		return retVal;
	}

	// Return the next value
	inline BSPPoly *getNext() const { return next; }

	// Generate a plane
	BSPPlane GetPlane (int num) const;

	/*
	 * Unlink a poly from the list starting at 'this'
	 * Returns the new head of the linked list, so use as
	 * list = list->Unlink (poly)
	 */
	BSPPoly *Unlink (BSPPoly *poly)
	{
		assert (poly != NULL);
		// Check for removing 'this'
		if (poly == this) {
			BSPPoly *retVal = poly->next;
			poly->next = NULL;
			return retVal;
		} else {
			BSPPoly *p;
			for (p = this; p->next; p = p->next) {
				if (p->next == poly) {
					// We found it!
					p->next = poly->next;
					poly->next = NULL;
					break;
				}
			}
			return this;
		}
	}

	/*
	 * Links a poly onto the head of a linked list
	 * Use as list = list->Link (poly)
	 */
	BSPPoly *Link (BSPPoly *poly)
	{
		assert (poly != NULL);
		assert (poly->next == NULL);
		poly->next = this;
		return poly;
	}

	// Registers all the vertices
	void AddVertices (Library<NinjaPoint3> &vertLib, Library<NinjaColour> &uvLib)
	{
		BSPVertex *v;
		for (v = vertices; v ; v = v->next) {
			NinjaPoint3 point;
			NinjaColour uv;
			point = NinjaPoint3(v->x, v->y, v->z);
			uv = NinjaUV(v->u, v->v);
			vertLib.AddObject (&point);
			uvLib.AddObject (&uv);
		}
	}

	// Registers all the vertices
	void OutputVertices (ostream &o, Library<NinjaPoint3> &vertLib, Library<NinjaColour> &uvLib)
	{
		BSPVertex *v;
		o << Degree() << " ";
		for (v = vertices; v ; v = v->next) {
			NinjaColour uv = NinjaUV(v->u, v->v);
			o << vertLib.FindObject (&NinjaPoint3(v->x, v->y, v->z)) << " [" <<
				uvLib.FindObject (&uv) << "]";
			if (v->next)
				o << " ";
		}
		o << endl;
	}

	/*
	 * Split a polygon in two, adding vertices as necessary
	 * Pops the positive polys on the posList, and negative polygons
	 * on the negList
	 */
	void Split (const BSPPlane &plane, BSPPoly *&posList, BSPPoly *&negList);

	/*
	 * Check to see which side of a given plane a polygon is on
	 */
	enum Side { STRADDLES, POSITIVE, NEGATIVE, COPLANAR };
	Side Where (const BSPPlane &plane) const;
};

// A superclass for the BSP nodes and leaves
class BSPTree
{
private:
	static int	polysDone, polys;		// Used for updating progress
	BSPPlane	plane;					// The plane equation of partition
	BSPTree		*positive;				// The tree on the positive side of the partition
	BSPTree		*negative;				// The tree on the negative side
	BSPPoly		*polygons;				// The list of polygons in the leaf

	BOOL ChoosePlane (BSPPoly *polyList);
	void PartitionPolys (BSPPoly *&polyList,
						 BSPPoly *&posSide,
						 BSPPoly *&negSide);

	// Class variables used to hold the emitted vertices
	static Library<NinjaPoint3>			vertLib;
	static Library<NinjaColour>			uvLib;

public:
	/*
	 * Pass the constructor a reference to your list of polygons
	 * Note this list will be NULL after constructing the BSP Tree, and all
	 * polys will have been farmed out into the tree
	 * The node will split the list in two recursively
	 */
	BSPTree	(BSPPoly *&polygonList);
	
	// Destructor
	~BSPTree ()
	{
		if (positive) delete positive;
		if (negative) delete negative;
		if (polygons) delete polygons;
	}

	/*
	 * This class function is used to start the whole process off
	 * from a source MNMesh
	 */
	static BSPTree *MakeBSPTree (Mesh &mesh);

	// Is this a node or a leaf?
	BOOL isLeaf() const { return !(negative || positive); }
		
	// Count the number of faces
	int numFaces () const
	{
		if (isLeaf())
			return polygons->Count();
		return (positive->numFaces() + negative->numFaces());
	}

	// Add the component vertices to the vertex library
	void AddVertices();

	// Output the vertex numbers for the faces
	void OutputFaces (ostream &o);

	// Sorts the vertex library ready for Delta compression
	void SortVertices();

	// Output the tree!
	friend ostream &operator << (ostream &o, BSPTree *tree);
};

#endif
