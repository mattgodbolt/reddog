/*
 * Stripification
 */

#include "StdAfx.h"
#include "IStrip.h"
#include "VertLib.h"

int stripPolys, stripStrip, numStrip;

typedef IStripPoly<RDStripVertex> RDStripPoly;
typedef Stripifier<RDStripVertex> RDStripifier;

/*
 * End of model
 */
void StripList::EndOfModel (FILE *f)
{
	StripHeader h;
	h.length = h.material = 0;
	fwrite (&h, 1, sizeof (StripHeader), f);
}

/*
 * Stripification
 * If matLib is non-NULL: (Object exporting)
 * Go through a strip list and remove any polygons not appropriate
 * to stripification :
 * # Single unattached triangles
 * # Environment mapped polygons
 */
// My internal vertex representation
class RDStripVertex
{
private:
	RDVertex *v;
	int material;
public:
	RDStripVertex() : v(NULL) {}
	RDStripVertex (RDVertex *vert, int mat)
	{ v = vert;	material = mat; }
	RDStripVertex(const RDStripVertex &r)
	{
		v = r.v;
		material = r.material;
	}
	// Equivalence and non-equivalance operators
	bool operator == (const RDStripVertex &p) const
	{
		return (v->x == p.v->x &&
			 v->y == p.v->y &&
			 v->z == p.v->z &&
			 v->u == p.v->u &&
			 v->v == p.v->v &&
			 v->nx == p.v->nx &&
			 v->ny == p.v->ny &&
			 v->nz == p.v->nz &&
			 v->colour == p.v->colour &&
			 material == p.material);
	}
	bool operator != (const RDStripVertex &p) const
	{
		return (v->x != p.v->x ||
			 v->y != p.v->y ||
			 v->z != p.v->z ||
			 v->u != p.v->u ||
			 v->v != p.v->v ||
			 v->nx != p.v->nx ||
			 v->ny != p.v->ny ||
			 v->nz != p.v->nz ||
			 v->colour != p.v->colour ||
			 material != p.material);
	}
	Position getPos() const { return Position(v->x, v->y, v->z); }
	friend bool operator < (const RDStripVertex &a, const RDStripVertex &b)
	{
		return a.v < b.v;
	}
	friend bool operator > (const RDStripVertex &a, const RDStripVertex &b)
	{
		return a.v > b.v;
	}
	// Other functions
	int getMat() const { return material; }
	void AddLib (VertexLibrary &vertLib)
	{
		Point3 pt;
		pt.x = v->x;
		pt.y = v->y;
		pt.z = v->z;
		PosAndCol p (pt, v->colour);
		vertLib.Add (p);
	}
	void Output (FILE *f, VertexLibrary &vertLib)
	{
		StripEntry ent;
		Point3 newP;
		
		newP.x = v->x;
		newP.y = v->y;
		newP.z = v->z;
		PosAndCol pAndC(newP, v->colour);
		ent.vertIndex = (vertLib.Find (pAndC)) << 2;
		
		// Mangle the UVs together
#define PACKUV(fU, fV)  ((*((int *) &fU) & 0xFFFF0000) | (*((int *) &fV) >> 16))
		ent.uv = PACKUV(v->u, v->v);
		fwrite (&ent, sizeof (ent), 1, f);
	}
	void OutputUV (FILE *f)
	{
		fwrite(&v->u, 4, 1, f);
		fwrite(&v->v, 4, 1, f);
	}
	void ObjOutput (FILE *f, Library<PointAndNormal> &vertLib)
	{
		PointAndNormal pAndN;
		pAndN.point.x = v->x;
		pAndN.point.y = v->y;
		pAndN.point.z = v->z;
		pAndN.normal.x = v->nx;
		pAndN.normal.y = v->ny;
		pAndN.normal.z = v->nz;
		Uint16 vertIndex = vertLib.FindObject (&pAndN);
		
		fwrite (&vertIndex, sizeof (vertIndex), 1, f);
	}
};

static int __cdecl stripComparer (const void *elem1, const void *elem2)
{
	int mat1, mat2;
	RDStrip *s1 = *(RDStrip **)elem1;
	RDStrip *s2 = *(RDStrip **)elem2;
	mat1 = (*s1)[0]->getMat();
	mat2 = (*s2)[0]->getMat();
	return mat1 == mat2? 0 : mat1 < mat2 ? -1 : 1;
}

// Output class
class MyOutput : public IOutput<RDStripVertex>
{
	Tab<RDStrip *> &s;
	RDStrip *curStrip;
public:
	MyOutput (Tab<RDStrip *> &strip) : s(strip)	{}
	void BeginStrip (const RDStripVertex &r)
	{
		curStrip = new RDStrip;
		s.Append (1, &curStrip);
	}
	void MidStrip (const RDStripVertex &r)
	{
		RDStripVertex *foo = new RDStripVertex(r);
		curStrip->Append(1, &foo);
	}
};


StripList::StripList (RDPoly *polygons, const Library<RDMaterial> *matLib /* = NULL */)
{
	// Various checks
	assert (sizeof (PosAndCol) == 16);
	assert (sizeof (StripEntry) == 8);
	assert (sizeof (StripEntry) == sizeof (StripHeader));

	/*
	 * Go through all the polygons, and create a corresponding RDStripPoly
	 */
	vector <RDStripPoly> polyList;
	for (RDPoly *p = polygons; p; p = p->getNext()) {
		if (p->isInvisible()) {
			p->AppendVerts (invisVerts);
			continue;
		}
		RDStripVertex	a(p->getVertex(0), p->getMaterial()), 
						b(p->getVertex(1), p->getMaterial()),
						c(p->getVertex(2), p->getMaterial());
		RDStripPoly newPoly(a, b, c);
		polyList.push_back(newPoly);
	}
	if (!polyList.empty()) {
		RDStripifier stripper(polyList);
		stripper.Stripify();
		
		int a, b, c;
		stripper.getMetrics(a, b, c, nPolys);
		stripPolys += nPolys;
		stripStrip += b;
		numStrip += a;
		
		// Now to 'output' the strip information
		MyOutput outputter(strips);
		stripper.Output(&outputter);
		// Sort the strip lists based on their material
		strips.Sort (stripComparer);
	} else {
		strips.SetCount(0);
	}
		
}

// Destructor
StripList::~StripList()
{
	for (int strip = 0; strip < strips.Count(); ++strip) {
		for (int poly = 0; poly < strips[strip]->Count(); ++poly)
			delete (*(strips[strip]))[poly];
		delete strips[strip];
	}

}

/*
 * StripPolys
 */

int StripPoly::curTime = 23456;

// Construct
StripPoly::StripPoly(RDPoly *poly)
{
	// Note our parent
	parent = poly;
	// Set up out three verts
	v[0].Setup (poly->vertices);
	v[1].Setup (poly->vertices->next);
	v[2].Setup (poly->vertices->next->next);

	// Linked list initialisation
	prev = next = NULL;

	// Adjacencies (sp?)
	adj[0] = adj[1] = adj[2] = NULL;
	adjEdge[0] = adjEdge[1] = adjEdge[2] = 0;

	timeStamp = 0;
}

// Check the adjacencies
void StripPoly::CheckAdjacent (StripPoly *other)
{
	int myEdge, otherEdge;
	for (myEdge = 0; myEdge < 3; ++myEdge) {
		for (otherEdge = 0; otherEdge < 3; ++otherEdge) {
			if (v[myEdge] == other->v[(otherEdge+1)%3] &&
				v[(myEdge+1)%3] == other->v[otherEdge]) {
				// Check to see that the other edge isn't pointing somewhere else
				if (other->adj[otherEdge] && other->adj[otherEdge] != this) {
					// Ignore this edge, as it is degenerate
				} else {
					adj[myEdge] = other;
					other->adj[otherEdge] = this;
					adjEdge[myEdge] = otherEdge;
					other->adjEdge[otherEdge] = myEdge;
				}
			}
		}
	}
}

// Add to a bucket
StripPoly *StripPoly::Add (StripPoly *poly, int bucket)
{
	poly->bucket = bucket;
	if (this) {
		this->prev = poly;
		poly->next = this;
		poly->prev = NULL;
		return poly;
	} else {
		poly->next = poly->prev = NULL;
		return poly;
	}
}

// Number of adjs
int StripPoly::numAdj() const
{
	int i;
	int retVal = 0;
	for (i = 0; i < 3; ++i)
		if (adj[i])
			retVal++;
	return retVal;
}

// Remove self from list, updating buckets and adjacencies as necessary
void StripPoly::Remove (StripPoly *buck[])
{
	if (bucket != -1) {
		buck[bucket] = buck[bucket]->Unlink (this);
		for (int edge = 0; edge < 3; ++edge) {
			if (adj[edge]) {
				adj[edge]->adj[adjEdge[edge]] = NULL;
				adj[edge]->DecrementBucket (buck);
			}
		}
		bucket = -1;
	}
}

void StripPoly::DecrementBucket (StripPoly *buck[])
{
	if (bucket != -1) {
		buck[bucket] = buck[bucket]->Unlink (this);
		bucket--;
		if (bucket != -1)
			buck[bucket] = buck[bucket]->Add (this, bucket);
	}
}

StripPoly *StripPoly::Unlink (StripPoly *unlink)
{
	if (unlink == this) {
		assert (prev == NULL);
		StripPoly *retVal = next;
		next = prev = NULL;
		if (retVal)
			retVal->prev = NULL;
		return retVal;
	} else {
		if (unlink->prev)
			unlink->prev->next = unlink->next;
		if (unlink->next)
			unlink->next->prev = unlink->prev;
		unlink->prev = unlink->next = NULL;
		return this;
	}
}

void StripPoly::SetNum (int i)
{
	parent->polyNumber = i;
}

/*
 * The real workhorse of stripification
 */
int StripPoly::Stripify (int edge, StripPolyList *output /* = NULL */)
{
	int origEdge = edge;
	int nextEdgeDelta = 2;		// Amount to add to this edge to move to 'next'
	int numInStrip = 0;
	StripPoly *poly = this;
	int curMat = parent->material;

	curTime++; // Increment the 'current time'

	for (;;) {
		// Is this a different material?
		if (poly->parent->material != curMat)
			break;
		// Check the timestamp - has this polygon already been accounted for?
		if (poly->timeStamp == curTime)
			break;	// Has already been done
		// Timestamp this polygon
		poly->timeStamp = curTime;
		// Set the active vertex
		poly->activeVert = (nextEdgeDelta == 2) ? ((edge+1) % 3) : edge;

		// Output this polygon
		if (output)
			output->Append (1, &poly);
		numInStrip++;

		// Is there another polygon on t'other side of this edge?
		if (poly->adj[edge] == NULL)
			break;	// No more stripification possible
		// Update edge and poly to point to the corresponding edge on the adjacent poly
		int newEdge = poly->adjEdge[edge];
		poly = poly->adj[edge];
		edge = newEdge;
		// Move round to the 'next' edge in the strip process
		edge = (edge + nextEdgeDelta) % 3;
		// Zigzap the strips
		nextEdgeDelta = 3 - nextEdgeDelta;
	}
#if 0
	int lastEdge;
	// Now try going backwards, adding polys to the front of the queue
	poly = this;
	nextEdgeDelta = 1;
	edge = (origEdge + 2) % 3;
	if (poly->adj[edge] != NULL) {
		int newEdge = poly->adjEdge[edge];
		poly = poly->adj[edge];
		lastEdge = edge;
		edge = newEdge;
		edge = (edge + nextEdgeDelta) % 3;
		nextEdgeDelta = 3 - nextEdgeDelta;
		for (;;) {
			// Is this a different material?
			if (poly->parent->material != curMat)
				break;
			// Check the timestamp - has this polygon already been accounted for?
			if (poly->timeStamp == curTime)
				break;	// Has already been done
			// Timestamp this polygon
			poly->timeStamp = curTime;
			// Set the active vertex
			poly->activeVert = (nextEdgeDelta == 2) ? ((edge+1) % 3) : edge;
			
			// Output this polygon
			if (output)
				output->Insert (0, 1, &poly);
			numInStrip++;
			
			// Is there another polygon on t'other side of this edge?
			if (poly->adj[edge] == NULL)
				break;	// No more stripification possible
			// Update edge and poly to point to the corresponding edge on the adjacent poly
			int newEdge = poly->adjEdge[edge];
			lastEdge = edge;
			poly = poly->adj[edge];
			edge = newEdge;
			// Move round to the 'next' edge in the strip process
			edge = (edge + nextEdgeDelta) % 3;
			// Zigzap the strips
			nextEdgeDelta = 3 - nextEdgeDelta;
		}
		// Re-order the polygons
		if (output) {
			int i, max;
			edge = lastEdge;
			nextEdgeDelta = 2;
			max = output->Count();
			for (i = 1; i < max; ++i) {
				StripPoly *poly = (*output)[i];
				poly->activeVert = (nextEdgeDelta == 2) ? ((edge+1) % 3) : edge;
				// Update edge and poly to point to the corresponding edge on the adjacent poly
				int newEdge = poly->adjEdge[edge];
				poly = poly->adj[edge];
				edge = newEdge;
				// Move round to the 'next' edge in the strip process
				edge = (edge + nextEdgeDelta) % 3;
				// Zigzap the strips
				nextEdgeDelta = 3 - nextEdgeDelta;
			}
		}
	}
#endif
	return numInStrip;
}

void StripList::AddToLib (VertexLibrary &vertLib)
{
	int strip;
	// Create a library for the vertices
	for (strip = 0; strip < strips.Count(); ++strip) {
		RDStrip *s = strips[strip];
		for (int i = 0; i < s->Count(); ++i) {
			(*s)[i]->AddLib (vertLib);
		}
	}
	// Now bung in all the invible verts
	int nVerts, vert;
	for (vert = 0, nVerts = invisVerts.Count(); vert < nVerts; ++vert) {
		Point3 pt;
		pt.x = invisVerts[vert]->x;
		pt.y = invisVerts[vert]->y;
		pt.z = invisVerts[vert]->z;
		PosAndCol p (pt, invisVerts[vert]->colour);
		vertLib.Add (p);
	}
}

// Visibility checks
static inline bool Within (const Matrix3 &mat, const Point3 &p)
{
	Point3 pInMat = p * mat;
	if ((pInMat.x >= 0.f && pInMat.x <= 1.f) &&
		(pInMat.y >= 0.f && pInMat.y <= 1.f) &&
		(pInMat.z >= 0.f && pInMat.z <= 1.f))
		return true;
	else
		return false;
}

static inline bool Within2 (const Matrix3 &mat, const Point3 &p, const BBoxBounds &bounds)
{
	Point3 pInMat = p * mat;
	if ((pInMat.x >= bounds.lowX && pInMat.x <= bounds.hiX) &&
		(pInMat.y >= bounds.lowY && pInMat.y <= bounds.hiY) &&
		(pInMat.z >= bounds.lowZ && pInMat.z <= bounds.hiZ))
		return true;
	else
		return false;
}

#define OC_O_TOP	1
#define OC_O_BOT	2
#define OC_O_NEA	4
#define OC_O_FAR	8
#define OC_O_LEF	16
#define OC_O_RIG	32
#define OC_N_TOP	64
#define OC_N_BOT	128
#define OC_N_NEA	256
#define OC_N_FAR	512
#define OC_N_LEF	1024
#define OC_N_RIG	2048

#define OC_NOT		(OC_N_TOP | OC_N_BOT | OC_N_NEA | OC_N_FAR | OC_N_LEF | OC_N_RIG)
#define OC_ALL		(OC_O_TOP | OC_O_BOT | OC_O_NEA | OC_O_FAR | OC_O_LEF | OC_O_RIG)

static Uint32 OutcodePoint (const Matrix3 &mat, const Point3 &pWorld)
{
	Uint32 outcode = OC_NOT;
	Point3 p = pWorld * mat;

	if (p.x < 0.f)
		outcode ^= (OC_N_LEF | OC_O_LEF);
	if (p.x > 1.f)
		outcode ^= (OC_N_RIG | OC_O_RIG);

	if (p.y < 0.f)
		outcode ^= (OC_N_BOT | OC_O_BOT);
	if (p.y > 1.f)
		outcode ^= (OC_N_TOP | OC_O_TOP);

	if (p.z < 0.f)
		outcode ^= (OC_N_NEA | OC_O_NEA);
	if (p.z > 1.f)
		outcode ^= (OC_N_FAR | OC_O_FAR);

	return outcode;
}

static bool IsIncluded (RedDog::VisBox &visBox, BBoxBounds &bounds)
{
	Matrix3 mat;
	Uint32 outcode;
	// Get a Max matrix for the world to box space
	mat.SetRow(0, Point3(visBox.worldToBoxSpace.m[0][0], visBox.worldToBoxSpace.m[0][1], visBox.worldToBoxSpace.m[0][2])); 
	mat.SetRow(1, Point3(visBox.worldToBoxSpace.m[1][0], visBox.worldToBoxSpace.m[1][1], visBox.worldToBoxSpace.m[1][2])); 
	mat.SetRow(2, Point3(visBox.worldToBoxSpace.m[2][0], visBox.worldToBoxSpace.m[2][1], visBox.worldToBoxSpace.m[2][2])); 
	mat.SetRow(3, Point3(visBox.worldToBoxSpace.m[3][0], visBox.worldToBoxSpace.m[3][1], visBox.worldToBoxSpace.m[3][2]));

	// New system using outcodes for each point
	outcode =  OutcodePoint (mat, Point3 (bounds.lowX, bounds.lowY, bounds.lowZ));
	outcode |= OutcodePoint (mat, Point3 (bounds.lowX, bounds.lowY, bounds.hiZ ));
	outcode |= OutcodePoint (mat, Point3 (bounds.lowX, bounds.hiY , bounds.lowZ));
	outcode |= OutcodePoint (mat, Point3 (bounds.lowX, bounds.hiY , bounds.hiZ ));
	outcode |= OutcodePoint (mat, Point3 (bounds.hiX , bounds.lowY, bounds.lowZ));
	outcode |= OutcodePoint (mat, Point3 (bounds.hiX , bounds.lowY, bounds.hiZ ));
	outcode |= OutcodePoint (mat, Point3 (bounds.hiX , bounds.hiY , bounds.lowZ));
	outcode |= OutcodePoint (mat, Point3 (bounds.hiX , bounds.hiY , bounds.hiZ ));

	// Check for being completely onscreen
	if ((outcode & OC_ALL) == 0)
		return true;

	// Is it totally off the left hand side?
	if ((outcode & OC_N_LEF) != OC_N_LEF)
		return false;

	// Is it totally off the right hand side?
	if ((outcode & OC_N_RIG) != OC_N_RIG)
		return false;

	// Is it totally off the top?
	if ((outcode & OC_N_TOP) != OC_N_TOP)
		return false;

	// Is it totally off the bottom?
	if ((outcode & OC_N_BOT) != OC_N_BOT)
		return false;

	// Is it totally off the far plane?
	if ((outcode & OC_N_FAR) != OC_N_FAR)
		return false;

	// Is it totally off the near plane?
	if ((outcode & OC_N_NEA) != OC_N_NEA)
		return false;

	// Then it must be partially onscreen
	return true;

/*
	// Quick check - are any of the points of the model bounds inside the visbox?
	if (Within(mat, Point3 (bounds.lowX, bounds.lowY, bounds.lowZ)) ||
		Within(mat, Point3 (bounds.lowX, bounds.lowY, bounds.hiZ )) ||
		Within(mat, Point3 (bounds.lowX, bounds.hiY , bounds.lowZ)) ||
		Within(mat, Point3 (bounds.lowX, bounds.hiY , bounds.hiZ )) ||
		Within(mat, Point3 (bounds.hiX , bounds.lowY, bounds.lowZ)) ||
		Within(mat, Point3 (bounds.hiX , bounds.lowY, bounds.hiZ )) ||
		Within(mat, Point3 (bounds.hiX , bounds.hiY , bounds.lowZ)) ||
		Within(mat, Point3 (bounds.hiX , bounds.hiY , bounds.hiZ )))
		return true;

	// Now for the reverse check
	mat = Inverse (mat);
	if (Within2(mat, Point3 (0, 0, 0), bounds) ||
		Within2(mat, Point3 (0, 0, 1), bounds) ||
		Within2(mat, Point3 (0, 1, 0), bounds) ||
		Within2(mat, Point3 (0, 1, 1), bounds) ||
		Within2(mat, Point3 (1, 0, 0), bounds) ||
		Within2(mat, Point3 (1, 0, 1), bounds) ||
		Within2(mat, Point3 (1, 1, 0), bounds) ||
		Within2(mat, Point3 (1, 1, 1), bounds))
		return true;

	return false;
	*/
}

/*
 * Export the strip info
 */
void StripList::Export (FILE *out, BBoxBounds bounds, VertexLibrary &vertLib, Tab<RedDog::VisBox> &visBoxes, Tab<IShadBox *> &shadBoxes)
{
	int strip;
	int numStrips;

	// Count the number of strips
	numStrips = 0;
	for (strip = 0; strip < strips.Count(); ++strip)
		numStrips += strips[strip]->Count();

	StripModel model;
	assert (sizeof (StripHeader) == sizeof (StripEntry));
	model.low.x		= bounds.lowX;
	model.low.y		= bounds.lowY;
	model.low.z		= bounds.lowZ;
	model.hi.x		= bounds.hiX;
	model.hi.y		= bounds.hiY;
	model.hi.z		= bounds.hiZ;
	model.list		= (StripEntry *)
		((numStrips + strips.Count()) * sizeof (StripEntry) + sizeof (StripEntry));
	model.visGroup	= 0;
	model.vertex	= (RedDog::StripPos *)vertLib.Size();
	model.nVerts	= vertLib.Size() / sizeof (PosAndCol);
	for (int box = 0; box < visBoxes.Count(); ++box) {
		if (IsIncluded (visBoxes[box], bounds))
			model.visGroup |= 1<<(visBoxes[box].ID);
	}
	if (model.visGroup == 0)
	{
		
		char buf[256];
		if (ALLMAPMODE < GODMODEINTERMEDIATE)
		{	
			sprintf (buf, "Box @[%.2f,%.2f,%.2f]-[%.2f,%.2f,%.2f] is outside all visboxes",
				bounds.lowX, bounds.lowY, bounds.lowZ,
				bounds.hiX, bounds.hiY, bounds.hiZ);
			MessageBox (NULL, buf, "VisBox message", 0);
		}
		else
			fprintf (godlog, "Box @[%.2f,%.2f,%.2f]-[%.2f,%.2f,%.2f] is outside all visboxes\n",
				bounds.lowX, bounds.lowY, bounds.lowZ,
				bounds.hiX, bounds.hiY, bounds.hiZ);
	}
	fwrite (&model, sizeof (StripModel), 1, out);
	
 	int bytes = 0;
	for (strip = 0; strip < strips.Count(); ++strip) {
		RDStrip *s = strips[strip];
		assert (s);

		StripHeader h;
		h.length = s->Count();
		h.material = (*s)[0]->getMat();
		fwrite (&h, sizeof (h), 1, out);
		bytes += sizeof (h);

		// Loop throughout the strip
		for (int p = 0; p < s->Count(); ++p) {
			(*s)[p]->Output (out, vertLib);
			bytes += sizeof (StripEntry);
		}
	}

	EndOfModel (out);
	bytes += sizeof (StripEntry) + vertLib.Size();

	vertLib.Output (out, shadBoxes);
}

void EndPoint::Setup (RDVertex *vert)
{
	position.x	= vert->x;
	position.y	= vert->y;
	position.z	= vert->z;
	normal.x	= vert->nx;
	normal.y	= vert->ny;
	normal.z	= vert->nz;
	u			= vert->u;
	v			= vert->v;
	colour		= vert->colour;
}

void StripPoly::AddLib (VertexLibrary &vertLib)
{
	for (int i =0 ; i < 3; ++i) {
		Point3 pt;
		pt.x = v[i].position.x;
		pt.y = v[i].position.y;
		pt.z = v[i].position.z;
		PosAndCol p (pt, v[i].colour);
		vertLib.Add (p);
	}
}

void StripPoly::Output (FILE *f, VertexLibrary &vertLib, int vertOff)
{
	int vert = (activeVert+vertOff) % 3;
	StripEntry ent;
	Point3 newP;

	newP.x = v[vert].position.x;
	newP.y = v[vert].position.y;
	newP.z = v[vert].position.z;
	PosAndCol pAndC(newP, v[vert].colour);
	ent.vertIndex = (vertLib.Find (pAndC)) << 4;

	// Mangle the UVs together
#define PACKUV(fU, fV)  ((*((int *) &fU) & 0xFFFF0000) | (*((int *) &fV) >> 16))
	ent.uv = PACKUV(v[vert].u, v[vert].v);
	fwrite (&ent, sizeof (ent), 1, f);
}

int StripPoly::getMat() const
{ return parent->material; }

void StripList::OutputUVs (FILE *out)
{
	int strip;
	for (strip = 0; strip < strips.Count(); ++strip) {
		RDStrip *s = strips[strip];
		assert (s);

		// Output the first two vertices
		RDStripVertex *poly = (*s)[0];
		// Loop throughout the strip
		for (int p = 0; p < s->Count(); ++p) {
			(*s)[p]->OutputUV (out);
		}
	}
}
void StripList::ObjectOutput (Library<PointAndNormal> &vertLib, FILE *out)
{
	int strip;
	Uint16 mat, nStrip;
	for (strip = 0; strip < strips.Count(); ++strip) {
		RDStrip *s = strips[strip];
		assert (s);

		nStrip = s->Count();
		mat = (*s)[0]->getMat();
		fwrite (&mat, sizeof (mat), 1, out);
		fwrite (&nStrip, sizeof (nStrip), 1, out);

		// Loop throughout the strip
		for (int p = 0; p < s->Count(); ++p) {
			(*s)[p]->ObjOutput (out, vertLib);
		}
	}

	// Terminate it
	Uint32 zero = 0;
	fwrite (&zero, 1, 4, out);
}

void StripPoly::ObjOutput (FILE *f, Library<PointAndNormal> &vertLib, int vertOff/* = 0*/)
{
	int vert = (activeVert+vertOff) % 3;
	
	PointAndNormal pAndN;
	pAndN.point.x = v[vert].position.x;
	pAndN.point.y = v[vert].position.y;
	pAndN.point.z = v[vert].position.z;
	pAndN.normal.x = v[vert].normal.x;
	pAndN.normal.y = v[vert].normal.y;
	pAndN.normal.z = v[vert].normal.z;
	Uint16 vertIndex = vertLib.FindObject (&pAndN);
	
	fwrite (&vertIndex, sizeof (vertIndex), 1, f);
}

void StripPoly::OutputUV (FILE *f, int vertOff /*= 0*/)
{
	int vert = (activeVert+vertOff) % 3;
	RedDog::UV uv;

	uv.u = v[vert].u; uv.v = v[vert].v;
	
	fwrite (&uv, sizeof (uv), 1, f);
}

static ::Point3 GetNormal (Point3 v1, Point3 v2, Point3 v3)
{
	::Point3 a = v3 - v2;
	::Point3 b = v1 - v2;
	return Normalize (a ^ b);
}

void StripPoly::OutputPlane (FILE *f)
{
	::Point3 normal;
	float dist;

	normal = ::GetNormal (Point3 (v[0].position.x, v[0].position.y, v[0].position.z),
						  Point3 (v[1].position.x, v[1].position.y, v[1].position.z),
						  Point3 (v[2].position.x, v[2].position.y, v[2].position.z));
	
	dist =	normal.x * v[0].position.x +
		normal.y * v[0].position.y +
		normal.z * v[0].position.z;

	fwrite (&normal, 1, sizeof (normal), f);
	fwrite (&dist, 1, sizeof (dist), f);
}

int StripList::CountForObj (void)
{
	int retVal = 2; // Account for terminator

	int strip;
	for (strip = 0; strip < strips.Count(); ++strip) {
		RDStrip *s = strips[strip];
		assert (s);

		retVal += s->Count() + 2;  // Uint16 mat, Uint16 nStrip, strip...
	}
	return retVal;
}
int StripList::CountForObjPolys (void)
{
	int retVal = 0;

	int strip;
	for (strip = 0; strip < strips.Count(); ++strip) {
		RDStrip *s = strips[strip];
		assert (s);

		retVal += s->Count() - 2;
	}
	return retVal;
}