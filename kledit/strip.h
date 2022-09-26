/*
 * Strip.h
 * Stripification classes
 */

#ifndef _STRIP_H
#define _STRIP_H

 /********************************************************
 *** THIS FILE *MUST* BE KEPT UP-TO-DATE WITH THE MAX ***
 *** PLUGIN OR ELSE ALL HELL WILL BREAK LOOSE!        ***
 ********************************************************/
class VertexLibrary;

// Structures output
struct StripEntry
{
	Uint32		vertIndex;			// Index into model's vertex list x 16
	Uint32		uv;					// Per-vertex UV value (packed)
};

struct PosAndCol
{
	Point3 p;
	Uint32 col;
	PosAndCol () { }
	PosAndCol (Point3 p, Uint32 col)
	{
		this->p = p;
		this->col = col;
	}
	bool operator == (const PosAndCol &other) const
	{
		return (p==other.p && col==other.col);
	}
};

struct StripHeader
{
	Uint32		length;
	Uint32		material;
};
struct StripModel
{
	Point3		low, hi;
	Uint32		visGroup;
	StripEntry	*list;		// == 0 on save
	int			nVerts;
	void		*vertex;			/* The array of vertices */
};


/*
 * An EndPoint holds the colour, uv and position of a point
 */
struct RDVertex;
struct EndPoint
{
	Point3		position, normal;
	float		u, v;
	Uint32		colour;

	inline bool operator == (const EndPoint &a) const
	{
		return (position == a.position &&
				u == a.u &&
				v == a.v &&
				normal == a.normal &&
				colour == a.colour);
	}
	void Setup (RDVertex *vert);
};

class RDPoly;
class StripPoly;
struct BBoxBounds;
typedef Tab<StripPoly *> StripPolyList;
struct PointAndNormal;

class RDStripVertex;
typedef Tab<RDStripVertex *> RDStrip;
class StripList
{
private:
	Tab<RDStrip *>					strips;		// The strips
	Tab<RDVertex *>					invisVerts;	// All the vertices we ignored, to allow the vertexLib to be up-to-date
	int nPolys;
	
public:
	StripList(RDPoly *list, const Library<RDMaterial> *matLib = NULL);
	// Take a list of RDPolys and stripify them
	~StripList();				// Deletion

	void AddToLib (VertexLibrary &vertLib);
	void Export (FILE *out, BBoxBounds bounds, VertexLibrary &vertLib, Tab<RedDog::VisBox> &visBoxes, Tab<IShadBox *> &shadBoxes);

	void ObjectOutput (Library<PointAndNormal> &vertLib, FILE *out);
	void OutputUVs (FILE *out);
	int CountForObj (void);
	int CountForObjPolys (void);
	int CountStrips (void) { return strips.Count(); }

	static void EndOfModel (FILE *f);
};

class StripPoly
{
private:
	StripPoly			*prev, *next;	// Previous and next polygon in the bucket list
	RDPoly				*parent;		// Pointer to parent for collision polynumber updating purposes
	EndPoint			v[3];			// The three vertices
	StripPoly			*adj[3];		/* The three adjacent polygons (NULL if none) 
										 * such that v[0] -> v[1] 's other side is adj[0] */
	int					adjEdge[3];		// The edge indices on the other edge
	int					bucket;
	int					timeStamp;		// The timestamp of the last time this was updated
	int					activeVert;		// The vertex which is 'active'
	static int			curTime;

	// Decrement the bucket count
	void StripPoly::DecrementBucket (StripPoly *buck[]);
	// Unlink a poly
	StripPoly *Unlink (StripPoly *);
public:
	StripPoly (RDPoly *poly);			// Construct from a RDPoly
	~StripPoly () {}					// Destruct

	inline StripPoly *getNext() const { return next; }
	// Check the adjacencies between this and an other polygon, updating both
	void CheckAdjacent (StripPoly *other);
	// Add a StripPoly to this linked list
	StripPoly *Add (StripPoly *, int bucket);
	// Number of adjacent polygons
	int numAdj() const;
	// Remove self from list, updating buckets as necessary
	void Remove (StripPoly *bucket[]);
	// Do the business
	int Stripify (int edge, StripPolyList *output = NULL);
	// Set the number (for collis purps)
	void SetNum (int num);
	// Get the material
	int getMat() const;
	// Add vert to a vertLib
	void AddLib (VertexLibrary &vertLib);
	// Output a vert
	void Output (FILE *file, VertexLibrary &vertLib, int vertOff = 0);
	// Output a vert for an object
	void ObjOutput (FILE *file, Library<PointAndNormal> &vertLib, int vertOff = 0);
	// Output a UV set
	void OutputUV (FILE *file, int vertOff = 0);
	// Output a plane equation
	void OutputPlane (FILE *file);
	// Is this the same as...
	inline bool Equals (RDPoly *pol) const { return (pol == parent); }
};

#endif
