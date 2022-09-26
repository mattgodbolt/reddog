/*
 * IStrip.h
 * Interface to a strip 
 * (c) 1998 Argonaut Software Ltd
 * By Matthew Godbolt based on work by Francine Evans
 */

#ifndef _MG_ISTRIP_H
#define _MG_ISTRIP_H

#ifndef MATT_DEBUG
#define MATT_DEBUG 0
#endif

// MS-specific: disable annoying warning
#pragma warning(disable : 4786)

#include <math.h>
#include <vector>
#include <list>
#include <queue>
#include <cassert>
#include <algorithm>
#if MATT_DEBUG
#include <iostream>
#endif
// I'm lazy - pull std into the global namespace
using namespace std;

// A position in 3D space
// Used to check whether two polygons are coplanar
struct Position
{
	float x, y, z;
	
	// Construction
	Position() { x = y = z = 0.f; }
	Position (const Position &p) { x = p.x; y = p.y; z = p.z; }
	Position (float x, float y, float z) { this->x = x; this->y = y; this->z = z; }
	
	// Comparison
	inline bool operator == (const Position &p) const
	{ return (p.x == x && p.y == y && p.z == z); }
	inline bool operator != (const Position &p) const
	{ return (p.x != x || p.y != y || p.z != z); }
	
	// Maths
	Position operator - (const Position &p) const
	{ return Position(x - p.x, y - p.y, z - p.z); }
	Position operator ^ (const Position &p) const
	{ return Position(y * p.z - z * p.y, z * p.x - x * p.z, x * p.y - y * p.x); }
	inline float operator * (const Position &p) const
	{ return x*p.x + y*p.y + z*p.z; }
	inline float Length () const
	{ return sqrt(x*x+y*y+z*z); }
	inline bool CloseTo (const Position &p) const
	{ return (Position(*this - p).Length() < 0.0001f); }
	
	void operator += (const Position &p)
	{ x += p.x; y += p.y; z += p.z; }
	void Normalise()
	{
		float rLength = 1.f / Length();
		x *= rLength;
		y *= rLength;
		z *= rLength;
	}

#if MATT_DEBUG
	// Debug output
	friend ostream &operator << (ostream &o, const Position &p)
	{ o << "[" << p.x << ", " << p.y << ", " << p.z << "]"; return o; }
#endif

};

// An interface to a vertex in model format
// This should hold all information needed to output the vertex
// in full, e.g. x,y,z,colour,u,v
// Or just an integer offset into a vertex array
// This class can be substituted for other, completely different classes,
// which is the preferred way of customization.  This is done via templates,
// which allows the compiler more chance of optimizing than would be using
// virtual calls.
// This implementation has a position, a normal, and a UV value
class IStripVertex
{
private:
	Position pos;
	Position normal;
	float u, v;
public:
	// Constructors
	IStripVertex() {}
	IStripVertex(float x, float y, float z, float nX = 0, float nY = 0, float nZ = 1.f,
				float u = 0, float v = 0)
	{ this->pos = Position(x, y, z); this->normal = Position (nX, nY, nZ); 
	  this->u = u; this->v = v; }
	// Destructor
	~IStripVertex() {}
	// Equivalence and non-equivalance operators
	bool operator == (const IStripVertex &p) const
	{ return u == p.u && v == p.v && pos == p.pos && normal == p.normal; }
	bool operator != (const IStripVertex &p) const
	{ return u != p.u || v != p.v || pos != p.pos || normal != p.normal;}
	// Returns the position in 3d
	Position getPos() const { return pos; }
	// MS implementation of vector means we have to define < and >
	friend bool operator > (const IStripVertex &a, const IStripVertex &b);
	friend bool operator < (const IStripVertex &a, const IStripVertex &b);

#if MATT_DEBUG
	// Debug output
	friend ostream &operator << (ostream &o, const IStripVertex &c)
	{ o << c.pos; return o; }
#endif
};

// Output interface class
// You must derive a class from this and pass it to Stripifier::Output
// This approach has been used to allow any form of output to take place, and
// to prevent any C bods from having to write too much C++!  In an ideal world,
// I'd have mucked around with C++ streams and ued them, but this is an easier
// approach IMHO!
template <class Vertex>
class IOutput
{
public:
	// This is called prior to the first vertex in a strip
	virtual void BeginStrip (const Vertex &firstVertex) {}
	// This is called for every vertex in a strip
	virtual void MidStrip (const Vertex &vertex) = 0;
	// This is called after the last vertex
	virtual void EndStrip (void) {}
};

// A stripifiable polygon
template <class Vertex>
class IStripPoly
{
private:
	// A vector is used to hold the Strip vertex references
	vector<Vertex>		vertices;
	// Another vector holds the edge information
	vector<pair<IStripPoly<Vertex> *, int> >
						edges;
	// A position is used to hold the polygon normal
	Position			normal;
	
	// Polygons can be marked (for deleted, already processed, whatever)
	bool				marker;

	// Internal functions :
	// Generate the polygon normal
	void MakeNormal()
	{
		// First see if it is at least a triangle
		assert (nSides()>=3);
		// Special case for triangles
		if (nSides() == 3) {
			Position vector0, vector1;
			vector0 = vertices[1].getPos() - vertices[0].getPos();
			vector1 = vertices[2].getPos() - vertices[0].getPos();
			normal = vector0 ^ vector1;
			normal.Normalise();
		} else {
			// General case normal calculation using an average
			// Remember the number of sides - 2
			int numNormals = nSides(), i;
			// For each possible normal, add it into the current normal
			normal = Position(0,0,0);
			for (i = 0; i < numNormals; ++i) {
				int j = (i+1) % numNormals;
				int k = (i+2) % numNormals;
				Position vector0, vector1, thisNormal;
				vector0 = vertices[j].getPos() - vertices[i].getPos();
				vector1 = vertices[k].getPos() - vertices[i].getPos();
				thisNormal = vector0 ^ vector1;
				thisNormal.Normalise();
				normal+= thisNormal;
			}
			normal.Normalise();
		}
	}


public:
	// Default constructor
	IStripPoly() : marker(false) { }
	// Constructor - for triangles only, taking three vertices
	IStripPoly(const Vertex &a, const Vertex &b, const Vertex &c)
		: marker(false)
	{
		vertices.push_back(a);
		vertices.push_back(b);
		vertices.push_back(c);
		
		// Update the normal
		MakeNormal();
		
		// Set the edge list size
		edges.resize(3);
		for (int edge = 0; edge < 3; ++edge) {
			edges[edge].first	= NULL;
			edges[edge].second	= 0;
		}
	}
	// Destructor
	~IStripPoly() {}

	// Return the number of sides of this polygon
	int nSides() const { return vertices.size(); }
	// Return a reference to vertex 'n'
	const Vertex &getVertex(int n) const
	{ return vertices[n]; }
	IStripPoly<Vertex> *getEdge (int n) const
	{ return edges[n].first; }
	int getOtherEdge (int n) const
	{ return edges[n].second; }
	Position getNormal() const
	{ return normal; }
	// Returns the number of adjacencies
	int nAdjacencies()
	{
		int i, retVal = 0;
		for (i = 0; i < nSides(); ++i)
			if (edges[i].first != NULL)
				retVal++;
			return retVal;
	}

	// Marking methods
	bool isMarked() { return marker; }
	void Mark(bool b = true) { marker = b; }

	// Called to note any adjacencies between ourselves and another polygon
	// Function must update both polygon's edge lists
	// A is adjacent to B iff it shares two consecutive vertices with B in the
	// reverse order : the two polygons must have the same sense
	void NoteAdjacencies (IStripPoly<Vertex> &other)
	{
		// Check every edge of this polygon
		for (int myEdge = 0; myEdge < nSides(); ++myEdge) {
			// If there is already an adjacency for this edge, ignore it
			if (edges[myEdge].first != NULL)
				continue;
			int myNextEdge = (myEdge + 1) % nSides();
			// Check every edge of the other polygon
			for (int theirEdge = 0; theirEdge < other.nSides(); ++theirEdge) {
				int theirNextEdge = (theirEdge + 1) % nSides();
				// If there is already an adjacency for this edge, ignore it
				if (other.edges[theirEdge].first != NULL)
					continue;
				// Are the two edges the same with opposite sense?
				if (vertices[myEdge] == other.vertices[theirNextEdge] &&
					vertices[myNextEdge] == other.vertices[theirEdge]) {
					// Yes - these polygons share an edge
					edges[myEdge] = pair<IStripPoly<Vertex> *, int> (&other, theirEdge);
					other.edges[theirEdge] = pair<IStripPoly<Vertex> *, int> (this, myEdge);
				}
			}
		}
	}

	// Called when a face next to this one has been deleted to kill the
	// adjacency information from this poly to the other
	void DeleteAdjacency (int edge)
	{ 
		edges[edge].first = NULL; 
	}

	// The following functions are used during higher order polygon generation
	// Once two coplanar adjacent polygons are found, they are merged using the
	// merge constructor below.  This generates a new, merged polygon which is then
	// subject to a concavity check.  If it isn't concave, then no more processing
	// takes place.  Otherwise, one of the two original polygons is replaced with
	// the merged copy, the other deleted, and the the edges updated so that the newly
	// adjacent polygons' edge pointers are valid

	// Merge constructor - taking two IStripPolys produce a merged polygon without
	// updating the edges of the surrounding polygons
	IStripPoly(const IStripPoly<Vertex> &a, int aEdge, const IStripPoly<Vertex> &b, int bEdge)
	{
		int aSides = a.nSides(), bSides = b.nSides();
		int edge;
		// Go around a from edge (aEdge+1) until we get to aEdge (%aSides)
		for (edge = (aEdge+1) % aSides; edge != aEdge; edge = (edge + 1) % aSides) {
			vertices.push_back(a.getVertex(edge));
			edges.push_back(a.edges[edge]);
		}
		// Now continue around b from edge bEdge+1 till bEdge
		for (edge = (bEdge+1) % bSides; edge != bEdge; edge = (edge + 1) % bSides) {
			vertices.push_back(b.getVertex(edge));
			edges.push_back(b.edges[edge]);
		}
		// Update the normal
		MakeNormal();
	}

	// Returns whether or not a polygon is concave
	bool IsConcave()
	{
		// Early out
		if (nSides() == 3)
			return true;
		// For every remaining edge, check all vertices and if any are on the -ve side,
		// then this polygon in convex
		// XXX Check this
		for (int edge = 0; edge < nSides(); ++edge) {
			Position edgeStart = getVertex(edge).getPos();
			Position edgeVector = getVertex((edge+1)%nSides()).getPos() - edgeStart;
			for (int vertex = (edge+2) % nSides(); vertex != edge; vertex = (vertex + 1) % nSides()) {
				Position pos = getVertex(vertex).getPos() - edgeStart;
				if ((pos * edgeVector) < 0.f)
					return false;
			}
		}
		return true;
	}

	// Called to update the edges of the surrounding polygons after a successful merge
	void UpdateEdges()
	{
		int edge;
		for (edge = 0; edge < nSides(); ++edge) {
			pair<IStripPoly<Vertex> *, int> edgePol;
			edgePol = edges[edge];
			if (edgePol.first) {
				IStripPoly<Vertex> *p = edgePol.first;
				// Update the polygon pointer
				p->edges[edgePol.second].first	= this;
				// Update the edge number
				p->edges[edgePol.second].second	= edge;
			}
		}
	}

	// MS implementation of vector means we have to define < and >
	friend bool operator > (const IStripPoly &a, const IStripPoly &b)
	{
		return a.vertices > b.vertices;
	}
	
	friend bool operator < (const IStripPoly &a, const IStripPoly &b)
	{
		return a.vertices < b.vertices;
	}
	
	friend bool operator == (const IStripPoly &a, const IStripPoly &b)
	{
		return a.vertices == b.vertices && a.normal == b.normal;
	}
};

// A polygon strip
template <class Vertex>
class IStrip
{
private:
	// The stripified data
	vector<Vertex> verts;
	// Where the pre-stripified polygon data is stored before
	// it is compiled into strips
	list<Vertex> polys;

	// Whether we are operating in 'reverse' mode
	bool reversed;

	// An internal variable to hold metric information
	int nPolys;

	// Finds the vertex in [a,b,c] which isn't in [d,e,f] and optionally
	// returns the two other vertices from [a,b,c] in order
	const Vertex *FindDifferentVertex (const Vertex &a, const Vertex &b, const Vertex &c,
		const Vertex &d, const Vertex &e, const Vertex &f,
		const Vertex **other1 = NULL, const Vertex **other2 = NULL) const
	{
		if (a != d && a != e && a != f) {
			if (other1)
				*other1 = &b;
			if (other2)
				*other2 = &c;
			return &a;
		}
		if (b != d && b != e && b != f) {
#if 0
			// XXX same as stripe, but I'm not entirely convinced this is right
			if (other1)
				*other1 = &a;
			if (other2)
				*other2 = &c;
#else
			// Now fixed to remove another chunk of code lower down
			if (other1)
				*other1 = &c;
			if (other2)
				*other2 = &a;
#endif
			return &b;
		}
		if (c != d && c != e && c != f) {
			if (other1)
				*other1 = &a;
			if (other2)
				*other2 = &b;
			return &c;
		}
		// May get here if the two polys are degenerate
		if (other1)
			*other1 = &e;
		if (other1)
			*other2 = &f;
		return &d;
	}


	// Checks if the sense of [a,b,c] is reversed with respect to [d,e,f]
	bool Reversed (const Vertex &a, const Vertex &b, const Vertex &c,
		const Vertex &d, const Vertex &e, const Vertex &f)
	{
		// First find the vertex in [d,e,f] which corresponds to a
		if (a==d) {
			if (b==e && c==f)
				return false;
			if (b==f && c==e)
				return true;
		} else if (a==e) {
			if (b==f && c==d)
				return false;
			if (b==d && c==f)
				return true;
		} else if (a==f) {
			if (b==d && c==e)
				return true;
			if (b==e && c==d)
				return false;
		}
		// If this assertion triggers, then [a,b,c] is not the same triangle as [d,e,f] or [f,e,d] !
		assert (false);
		return false;
	}

public:
	// Constructor
	IStrip() { nPolys = 0; reversed = false;}
	// Have any vertices already been output?
	inline bool isFirst() const
	{ return polys.size() == 0; }
	// Add a polygon
	void Add(const Vertex &v0, const Vertex &v1, const Vertex &v2)
	{ 
		if (reversed) {
			polys.push_front (v2); polys.push_front (v1); polys.push_front (v0); 
		} else {
			polys.push_back (v0); polys.push_back (v1); polys.push_back (v2); 
		}
	}
	// Return the last edge or two same vertices at the beginning
	pair<Vertex, Vertex> getLastEdge()
	{
		if (isFirst()) {
			Vertex vert;
			return pair<Vertex, Vertex>(vert, vert);
		} else {
			list<Vertex>::reverse_iterator i = polys.rbegin();
			const Vertex &p0 = *i++;
			const Vertex &p1 = *i; 
			return pair<Vertex, Vertex>(p1, p0);
		}
	}
	// Compile information from polys into verts at the end of registered polygons
	void End(void)
	{
		bool reversed = false, quadFlip = false;

		// Copy the linked list into an array to speed up the algorithm below
		vector<Vertex> polyArray;
		polyArray.resize(polys.size());
		copy(polys.begin(), polys.end(), polyArray.begin());

		// Sanity check - only triangles should get into the polyArray array
		assert ((polyArray.size() % 3) == 0);
		// Firstly - is there only one triangle?
		if (polyArray.size() == 3) {
			// Everything should be OK at this point - a triangle is a perfectly good strip
			// Just put polyArray into verts
			verts = polyArray;
		} else {
			// The three vertices of the last polygon
			const Vertex *a, *b, *c;
			// The ordered triangle vertices
			const Vertex *vertex1, *vertex2, *other1, *other2;
			// The last two outputted vertices
			const Vertex *id[2];
			int index = 0;
			// Find the vertex in the first triangle which isn't in the second triangle
			vertex1 = FindDifferentVertex (polyArray[0], polyArray[1], polyArray[2],
				polyArray[3], polyArray[4], polyArray[5]);
			// Find the vertex in the second triangle which isn't in the first, and the other two
			// vertices in order
			vertex2 = FindDifferentVertex (polyArray[3], polyArray[4], polyArray[5],
				polyArray[0], polyArray[1], polyArray[2],
				&other1, &other2);
	
			// I'm not totally convinced the next step is necessary:
			// The order of the vertices should come from other1 and other2 above, especially
			// when the stripe typos are fixed!
#if 0
			// Perform another lookahead to determine the order of the first polygon's
			// second and third vertices
			if (polyArray.size() >= 6) {
				const Vertex *other3;
				// I'm sure Stripe has a typo in it - this is my interpretation of what should happen
				other3 = FindDifferentVertex (polyArray[3], polyArray[4], polyArray[5],
					polyArray[6], polyArray[7], polyArray[8]);
				if (other3 != other1) {
					swap (other1, other2);
				}
			}
#endif
			// Is the sense of this strip the opposite of the original?  If so, note it down
			// for later fixing
			if (Reversed(*vertex1, *other1, *other2, polyArray[0], polyArray[1], polyArray[2])) {
				// Can we reverse the order of the polygons to get a strip without a dummy
				// vertex at the beginning?
				if (((polyArray.size() / 3) % 1) == 1) {
					reversed = true;
				} else {
					// Special case for a quad: if we're going the wrong way around the quad it's easy to fix
					if (polyArray.size() == 6)
						quadFlip = true;
					else {
						// We can't reverse; so stick a dummy vertex at the front
						verts.push_back(*other1);
					}
				}
			}
			verts.push_back(*vertex1);
			verts.push_back(*other1);
			verts.push_back(*other2);
			// Set up the last two outputted vertices
			id[0] = other1;
			id[1] = other2;
			
			// Set up the last triangle vertices appropriately
			a = &polyArray[3];
			b = &polyArray[4];
			c = &polyArray[5];
			
			// Loop for the remaining triangles
			for (int poly = 6; poly < polyArray.size(); poly+=3) {
				// Check to see if we need to output a swap vertex
				if (polyArray[poly] == *id[index] || 
					polyArray[poly+1] == *id[index] || 
					polyArray[poly+2] == *id[index]) 
				{
					verts.push_back (*id[index]);
					index = !index;
				}
				// Output the next vertex
				verts.push_back (*vertex2);
				id[index] = vertex2;
				index = !index;
				
				vertex2 = FindDifferentVertex (polyArray[poly], polyArray[poly+1], polyArray[poly+2],
					*a, *b, *c);
				a = &polyArray[poly];
				b = &polyArray[poly+1];
				c = &polyArray[poly+2];
			}
			
			// Put the last vertex on
			verts.push_back (*vertex2);
		}

		// Do we need to reverse that madness?
		if (reversed) {
			reverse (verts.begin(), verts.end());
		}

		// Quad flip?
		if (quadFlip) {
			swap (verts[0], verts[3]);
		}

		// Keep track of how many polygons we coded for, then free up the memory in the polys array
		nPolys = polys.size() / 3;
		polys.empty();
	}
	// Return the metrics: the number of polygons in this strip,
	// the number of vertices in this strip, and the number of null vertex swaps
	void getMetrics(int &nPolys, int &nStrip, int &nSwaps)
	{ nPolys = this->nPolys; nSwaps = (verts.size() - 2) - nPolys; nStrip = verts.size(); }

	// Called to output the vertices at the end of the process
	void Output (IOutput<Vertex> *output)
	{
		// Check for an empty strip
		if (verts.size() == 0)
			return;
		output->BeginStrip (verts[0]);
		vector<Vertex>::iterator i;
		for (i = verts.begin(); i < verts.end(); ++i) 
		{
			output->MidStrip (*i);
		}
		output->EndStrip ();
	}

	// Is the edge v0,v1 or v1,v0 the same as the first edge?
	bool FirstEdge (const Vertex &v0, const Vertex &v1) const
	{
		list<Vertex>::const_iterator i = polys.begin();
		const Vertex &p0 = *i++;
		const Vertex &p1 = *i; 
		return ((p0 == v0 && p1 == v1) || (p0 == v1 && p1 == v0));
	}

	// Set the reverse flag to make new polygons pop on the front of the queue
	void Reverse()
	{ reversed = true; }

	// MS implementation of vector means we have to define < and >
	friend bool operator > (const IStrip &a, const IStrip &b)
	{
		return a.verts > b.verts;
	}
	friend bool operator < (const IStrip &a, const IStrip &b)
	{
		return a.verts < b.verts;
	}
	
	friend bool operator == (const IStrip &a, const IStrip &b){
		return a.verts == b.verts;
	}

#if MATT_DEBUG
	// Debug output
	friend ostream &operator << (ostream &o, const IStrip<Vertex> &v)
	{
		for (int i = 0 ; i < v.verts.size(); ++i) {
			o << v.verts[i];
			if (i != v.verts.size()-1)
				o << ", ";
		}
		o << endl;
		return o;
	}
#endif
};


// The stripifier itself
template <class Vertex,
		  class Polygon = IStripPoly <Vertex>,
		  class Strip = IStrip<Vertex> >
class Stripifier
{
private:
	// Convenience types
	typedef vector<Polygon>::iterator			PolyIter;
	typedef list<PolyIter>::iterator			PolyAdjIter;
	typedef vector<list<PolyIter> >::iterator	AdjIter;
	// The polygon refs (adjacency buckets) are stored as a vector of lists.  An STL priority_queue would
	// have been nice here, but we need direct access to the storage and the ability
	// to reorder the sets as polygons are coded for
	vector <list<PolyIter> > adjBuckets;
	// The polygon data itself is stored in a vector so that as each polygon
	// moves from one adjacency bucket to another its address and hence the edge
	// data remains static
	vector <Polygon> polys;
	// The array of finished strips
	vector <Strip> strips;
	// The current strip we're working on
	Strip  *currentStrip;

	// Internal routines :
	// The top-level function of the local algorithm - stripify a polygon
	virtual void StripifyPolygon (PolyAdjIter polyIter)
	{
		Polygon &poly = **polyIter;
		int nAdjs = poly.nAdjacencies();
		int nSides = poly.nSides();

		// Mark as coded for
		poly.Mark();
		
		// Special case for triangles
		if (nSides == 3) {
			// It's a triangle
			if (nAdjs == 0) {
				// It's a triangle with no neighbours - we don't need to worry at which
				// edge the strip comes out
				currentStrip->Add(poly.getVertex(0), poly.getVertex(1), poly.getVertex(2));
				// Remove the triangle
				adjBuckets[0].erase (polyIter);
			} else {
				// It's a triangle with neighbours:
				// We need to update the adjacency count of the surrounding polygons
				// We then choose the lowest adjacent number to make towards
				int targetEdge = UpdateAdjacencies (polyIter);
				if (targetEdge == -1) {
					currentStrip->Add(poly.getVertex(0), poly.getVertex(1), poly.getVertex(2));
				} else {
					// We need to output such that the edge 'targetEdge' is the last active
					int v1 = (targetEdge + 1) % 3, v2 = (targetEdge + 2) % 3;
					currentStrip->Add(poly.getVertex(v2), poly.getVertex(targetEdge), poly.getVertex(v1));
				}
				// Remove the triangle
				adjBuckets[nAdjs].erase (polyIter);
				// Stripify the next polygon
				if (targetEdge != -1) {
					// Get the polygon on the other edge
					Polygon *p = poly.getEdge(targetEdge);
					assert (p);
					int bucket = p->nAdjacencies();
					// Find the iterator of this polygon
					polyIter = find (adjBuckets[bucket].begin(), adjBuckets[bucket].end(), p);
					assert (polyIter != adjBuckets[bucket].end());
					// And stripify it
					StripifyPolygon (polyIter);
				}
			}
		} else {
			// It's a polygon - we need to triangulate it for the best possible strip
			// XXX
		}
	}

	// Called to update currentStrip at the beginning of a new strip
	virtual void StartNewStrip ()
	{
		// Add a new strip
		strips.resize(strips.size()+1);
		currentStrip = &strips[strips.size()-1];
	}

	// Update the number of adjacencies of the polygons around this polygon
	// prior to it being coded in a strip.  The return value is the edge number
	// which has the lowest adjacency.  In the event of a tie, a heuristic is used
	// which attempts to keep the number of swaps to a minimum
	// It returns -1 if there are no edges to move to
	virtual int UpdateAdjacencies (PolyAdjIter polyIter)
	{
		Polygon &poly = **polyIter;
		int nAdjs = poly.nAdjacencies();
		int nSides = poly.nSides();
		int lowestEdge = -1, lowestAdj = 10000;
		vector<int> ties;
		
		for (int i = 0 ; i < nSides; ++i) {
			Polygon *p = poly.getEdge(i);
			if (p) {
				// There is a polygon on the other side - update its edges table
				// and plonk it in the right list
				int nOtherAdjs = p->nAdjacencies();
				PolyAdjIter pIter = find (adjBuckets[nOtherAdjs].begin(), adjBuckets[nOtherAdjs].end(), p);
				assert (pIter != adjBuckets[nOtherAdjs].end());
				// Lowest we've seen?
				if (nOtherAdjs <= lowestAdj) {
					// Was there a tie?
					if (nOtherAdjs == lowestAdj) {
						// There was a tie - add this to the vector of ties
						ties.push_back(i);
					} else {
						// No tie - reset the tie vector
						ties.empty();
						ties.push_back(i);
					}
					lowestAdj = nOtherAdjs;
					lowestEdge = i;
				}
				// Remove the pointer back to this polygon
				assert (p->getEdge(poly.getOtherEdge(i)) == &poly);
				p->DeleteAdjacency (poly.getOtherEdge(i));
				// Remove from adjacency list and plonk in the next one down
				adjBuckets[nOtherAdjs].erase (pIter);
				adjBuckets[nOtherAdjs-1].push_back (p);
			}
		}
		// If there was a tie, break it
		if (ties.size() > 1) {
			// We attempt to return the most 'sequential' edge - that is an edge
			// which requires no swap codes
			// What kind of poly is this?
			if (nSides == 3) {
				// A triangle:
				if (currentStrip->isFirst()) {
					// This is the first edge, so any one'll do
					lowestEdge = ties[0];
				} else {
					pair<Vertex, Vertex> edge = currentStrip->getLastEdge();
					// Otherwise, work out which of the two edges we'd come out on anyway
					if ((edge.first == poly.getVertex(ties[0]) && edge.second == poly.getVertex((ties[0]+1) % 3)) ||
						(edge.second == poly.getVertex(ties[0]) && edge.first == poly.getVertex((ties[0]+1) % 3))) {
						return ties[0];
					} else {
						return ties[1];
					}
				}
			} else if (nSides == 4) {
				// A quad:
				// XXX
			} else {
				// An n-gon
				// XXX
			}
		}
		return lowestEdge;
	}

public:
	// Construct a stripifier from a set of polygons
	Stripifier (vector<Polygon> polyList)
	{
		// Go through and note all the adjacencies between polygons
		// Copy the polygon data
		polys = polyList;
		PolyIter i;
		// First of two passes:  remove the freaky polygons
		// that MAX and other tools generate under some circumstances
		for (i = polys.begin(); i < polys.end(); ++i) {
			int numSameVertices = 0, nSides = i->nSides();;
			for (int side = 0; side < (nSides-1); ++side) {
				for (int otherSide = side + 1; otherSide < nSides; ++otherSide) {
					if (i->getVertex(side) == i->getVertex(otherSide))
						numSameVertices ++;
				}
			}
			if (numSameVertices >= (nSides - 2))
				i->Mark();
		}
		// Go back and delete all the marked deleted polygons
		// We go backwards for speed
		vector<Polygon>::reverse_iterator r, rNext;
		r = polys.rbegin();
		do {
			rNext = r+1;
			if (r->isMarked()) {
				polys.erase(& *r);
			}
			r = rNext;
		} while (r != polys.rend());
		// Now to go through and generate the edge information
		for (i = polys.begin(); i < polys.end(); ++i) {
			PolyIter j;
			for (j = i+1; j < polys.end(); ++j) {
				(*i).NoteAdjacencies(*j);
			}
		}
		currentStrip = NULL;
	}

	virtual ~Stripifier() {}
	
	// Call this function to generate higher order polygons prior to stripification
	// If you know all your polygons are the highest order possible, then don't call this
	// routine.  If not, it is best to call it, as polygons with order > 3 are dynamically
	// retriangulated to maximise strip length
	virtual void GenerateHighOrderPolygons()
	{
		// For every polygon, check to see if any of its adjacent polygons have
		// the same, or very very similar normal.  If so, combine them
		PolyIter i;
		for (i = polys.begin(); i < polys.end(); ++i) {
			Polygon &thisPoly = *i;
			if (thisPoly.isMarked())
				continue;
			int edge, nEdges = thisPoly.nSides();
			Position normal = thisPoly.getNormal();
			bool done;
			do {
				done = true;
				for (edge = 0; edge < nEdges; ++edge) {
					Polygon *p;
					p = thisPoly.getEdge(edge);
					if (p) {
						// Check to see if the polygon at the other side to this polygon
						// is nearly coplanar
						if (normal.CloseTo(p->getNormal())) {
							// Sanity check - make sure the other polygon hasn't already been deleted
							assert (p->isMarked() == false);
							// The two faces are coplanar, and share a common edge
							// Now we have to see if the combined polygon is valid
							Polygon combinedPoly(*p, thisPoly.getOtherEdge(edge), thisPoly, edge);
							if (combinedPoly.IsConcave()) {
								// The combinedPoly is valid : we need to update the polygon 'thisPoly'
								// with combinedPoly, and likewise update all the 'edges' pointers back from other
								// polygons to point to 'thisPoly'
								// First, mark 'other' as deleted
								p->Mark();
								
								thisPoly = combinedPoly;
								thisPoly.UpdateEdges();
								
								done = false;
							}
						}
					}
				}
			} while (!done);
		}
		// Now go back and delete all the marked deleted polygons
		// We go backwards for speed
		vector<Polygon>::reverse_iterator r, rNext;
		r = polys.rbegin();
		do {
			rNext = r+1;
			if (r->isMarked()) {
				polys.erase(& *r);
			}
			r = rNext;
		} while (r != polys.rend());
	}

	// The workhorse - take the internal representation and generate triangle strips
	virtual void Stripify (bool globalAlgorithm = true)
	{
		// First stage:  build the polyNum table, which is an array of arrays of polygons
		// Array 'n' contains the polygons with adjacency 'n'
		PolyIter i;
		for (i = polys.begin(); i < polys.end(); ++i) {
			int nAdjs = i->nAdjacencies();
			// Ensure the vector is big enough
			if (adjBuckets.size() < (nAdjs+1))
				adjBuckets.resize (nAdjs+1);
			adjBuckets[nAdjs].push_front(i);
		}
		
		// Global algorithm - not implemented yet
		if (globalAlgorithm) {
			// Patchification and stripification of those patches goes here...XXX
		}
		
		// Modified SGI algorithm is used as the local algorithm :
		// The polygon with lowest adjacency is selected as the starting point
		bool done;
		do {
			AdjIter i;
			done = true;
			for (i = adjBuckets.begin(); i < adjBuckets.end(); ++i) {
				if (i->size() != 0) {
					// This is the beginning of a new strip, so mark it as such
					StartNewStrip();
					// Remember the polygon before it gets removed in the stripifier,
					// so we can extend the strip backwards if necessary
					Polygon &poly = **i[0].begin();
					// Call the stripification code on this polygon
					StripifyPolygon (i[0].begin());
					// If there were two or more adjacencies to the original polygon, then try and
					// extend the strip backwards
					if (i - adjBuckets.begin() >= 2) {
						// See if we can find an edge which has polygons adjacent, and which
						// shares a vertex with the first
						int edge;
						for (edge = 0; edge < poly.nSides(); ++edge) {
							int nextEdge = (edge+1) % poly.nSides();
							if (currentStrip->FirstEdge(poly.getVertex(edge), poly.getVertex(nextEdge)) &&
								poly.getEdge(edge)) {
								Polygon *other = poly.getEdge(edge);
								// Check that this polygon hasn't already been coded for
								if (other->isMarked())
									continue;
								int nAdj = other->nAdjacencies();
								// Find the iterator of the polygon
								PolyAdjIter p = find (adjBuckets[nAdj].begin(), adjBuckets[nAdj].end(), other);
								assert (p != adjBuckets[nAdj].end());
								currentStrip->Reverse();
								StripifyPolygon (p);
								break;
							}
						}
					}
					// Mark the end of a strip, which compiles the polygon information into
					// a set of strips
					currentStrip->End();
					// Break out of the loop to find the new lowest adjacency polygon
					done = false;
					break;
				}
			}
		} while (!done);
	}
	
	// Get the metrics - number of strips, number of strip vertices, number of null swaps, number of polygons
	virtual void getMetrics(int &nStrips, int &nVerts, int &nSwaps, int &nPolys)
	{
		vector<Strip>::iterator i;
		nVerts = nSwaps = nPolys = 0;
		nStrips = strips.size();
		for (i = strips.begin(); i < strips.end(); ++i) {
			int stripV, stripS, stripP;
			i->getMetrics (stripP, stripV, stripS);
			nVerts += stripV;
			nSwaps += stripS;
			nPolys += stripP;
		}
	}


	// Called to output the vertices at the end of the process
	virtual void Output (IOutput<Vertex> *output)
	{
		// Sanity check
		assert (output);
		vector<Strip>::iterator i;
		for (i = strips.begin(); i < strips.end(); ++i) {
			i->Output (output);
		}
	}


#if MATT_DEBUG
	// Debug output
	friend ostream &operator << (ostream &o, const Stripifier<Vertex, Polygon, Strip> &s)
	{
		for (int i = 0 ; i < s.strips.size(); ++i) {
			o << s.strips[i] << endl;
		}
		return o;
	}
#endif
};

/* notes :

	don't generate concave polygons:
	yet to proove this works

  Need to remove collinear vertices 

  Need to do polygon triangulation

  Need to extend strips both ways - doesn't seem to work at the moment

  Need to implement stripification algorithm:
  * Global alogrithm

*/
	
#endif
