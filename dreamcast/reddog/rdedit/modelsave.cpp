// Save a red dog model format model

#include "stdafx.h"
#include <stddef.h>

// The root directory on the network (with trailing backslash)
#if LOCAL_COPY
// Local stuff resides from the .EXE downwards
#define REDDOG_ROOT
#else
#define REDDOG_ROOT				"P:\\GAME\\"
#endif

// Where Red Dog Objects are kept
#define	REDDOG_OBJECTS			REDDOG_ROOT "OBJECTS\\"
// Where landscapes are kept
#define REDDOG_LANDSCAPES		REDDOG_ROOT "SCAPES\\"
// Where texture maps are kept
#define REDDOG_TEXMAPS			REDDOG_ROOT "TEXMAPS\\"
// Where the original textures are kept
#define REDDOG_ORIG_TEXMAPS		"P:\\TEXTURES\\"
#define REDDOG_ORIG_TEXMAPS_2	"\\\\SANDY\\REDDOG\\TEXTURES\\"

using namespace RedDog;

#define NEAR_TOLER (0.001f)
#define NEAR_UV_TOLER (0.01f)
#define NORM_NEAR_TOLER (0.99996f)	// acs(0.5) - half a degree between polygons
#define MIN_POLY_SIZE (0.0008f)		// 0.8mm is probably underkill
#ifndef PI
#define PI 3.14159265355f
#endif
// The edge of a polygon, used in Quadification
struct RDEdge
{
	RDEdge	*next, *prev;
	::Point3 a, b;
	::Point3 na, nb;
	float ua, ub, va, vb;
	RDEdge (RDVertex *first, RDVertex *v, bool sense)
	{
		RDVertex *v2;
		if (sense) {
			v2 = v->next ? v->next : first;
		} else {
			RDVertex *vert = first;
			v2 = NULL;
			for (; vert->next; vert = vert->next) {
				if (vert->next == v)
					v2 = vert;
			}
			if (v2 == NULL)
				v2 = vert;
		}				

		a = ::Point3 (v->x, v->y, v->z);
		b = ::Point3 (v2->x, v2->y, v2->z);
		na = ::Point3 (v->nx, v->ny, v->nz);
		nb = ::Point3 (v2->nx, v2->ny, v2->nz);
		ua = v->u; va = v->v;
		ub = v2->u; vb = v2->v;
		next = prev = NULL;
	}
	~RDEdge () 
	{ 
		if (next)
			delete next;
	}
	RDEdge *Unlink (RDEdge *e)
	{
		if (e==this) {
			RDEdge *rV = e->next;
			if (e->next)
				e->next->prev = NULL;
			e->next = e->prev = NULL;
			return rV;
		} else {
			if (e->prev)
				e->prev->next = e->next;
			if (e->next)
				e->next->prev = e->prev;
		}
		e->next = e->prev = NULL;
		return this;
	}
	void Append (RDEdge *e)
	{
		RDEdge *ptr;
		for (ptr = this; ptr->next; ptr = ptr->next)
			;
		ptr->next = e;
		e->prev = ptr;
		e->next = NULL;
	}
	bool operator == (const RDEdge &e) const
	{
		if (Length (a - e.a) < NEAR_TOLER &&
			Length (b - e.b) < NEAR_TOLER &&
			DotProd (na, e.na) > NORM_NEAR_TOLER &&
			DotProd (nb, e.nb) > NORM_NEAR_TOLER &&
			ABS (ua - e.ua) < NEAR_TOLER &&
			ABS (ub - e.ub) < NEAR_TOLER &&
			ABS (va - e.va) < NEAR_TOLER &&
			ABS (vb - e.vb) < NEAR_TOLER)
			return true;
		else
			return false;
	}
};

// New version - tris, then quads
int RDSave::StripOutput (RDPoly *polygons, VertexLib &library, RedDog::Point3 centre, FILE *f, StripList *list)
{
	return 0; // XXX tris
#if 0
	int retVal = 0;
	RedDog::Uint16 curMaterial;
	int doing = 0;

	for (doing = 3; doing <= 4; ++doing) {
		curMaterial = -1;
		for (RDPoly *p = polygons; p; p = p->next) {
			if (list && list->Find(p))
				continue;
			if (p->Degree() != doing)
				continue;
			if (p->getMaterial() != curMaterial) {
				VertRef matChange, count;
				matChange = p->getMaterial();
				if (f)
					fwrite (&matChange, 1, sizeof (matChange), f);
				retVal++;
				curMaterial = p->getMaterial();
				// Now to count the number of similar-typed same-materialed polygons
				count = 0;
				for (RDPoly *c = p; c; c = c->next) {
					if (c->getMaterial() != curMaterial)
						break;
					if (list && list->Find(c))
						continue;
					if (c->Degree() != p->Degree())
						continue;
					count++;
				}
				if (f)
					fwrite (&count, 1, sizeof (count), f);
				retVal++;
			}

			if (doing == 3) {
				VertRef ref;
				PointAndNormal pAndN;

#define DO_VERT(v) \
	pAndN.point.x	= v->x - centre.x; \
	pAndN.point.y	= v->y - centre.y; \
	pAndN.point.z	= v->z - centre.z; \
	pAndN.normal.x	= v->nx; \
	pAndN.normal.y	= v->ny; \
	pAndN.normal.z	= v->nz; \
	\
	ref = library.FindObject (&pAndN); \
	if (f) \
		fwrite (&ref, 1, sizeof (ref), f); \
	retVal++

				DO_VERT (p->vertices);
				DO_VERT (p->vertices->next);
				DO_VERT (p->vertices->next->next);
			} else {
				VertRef ref;
				PointAndNormal pAndN;

				DO_VERT (p->vertices);
				DO_VERT (p->vertices->next);
				DO_VERT (p->vertices->next->next);
				DO_VERT (p->vertices->next->next->next);
#undef DO_VERT
			}
		}
	}
	return retVal;
#endif
}

static ::Point3 GetNormal (RDVertex *vertices, RDVertex *start = NULL)
{
	RDVertex *v1, *v2, *v3;
	v1 = vertices;
	v2 = v1->next ? v1->next : start;
	v3 = v2->next ? v2->next : start;
	::Point3 a(
		v3->x - v2->x,
		v3->y - v2->y,
		v3->z - v2->z);
	::Point3 b(
		v1->x - v2->x,
		v1->y - v2->y,
		v1->z - v2->z);
	return Normalize (a ^ b);
}

static ::Point3 GetNormal (RDVertex *v1, RDVertex *v2, RDVertex *v3)
{
	::Point3 a(
		v3->x - v2->x,
		v3->y - v2->y,
		v3->z - v2->z);
	::Point3 b(
		v1->x - v2->x,
		v1->y - v2->y,
		v1->z - v2->z);
	return Normalize (a ^ b);
}

RDPoly *RDSave::RemoveSingular (RDPoly *polygons)
{
	RDPoly *poly, *nextPoly;
	/*
	 * First pass - remove singular polygons
	 */
#define vdist(va,vb) ((va->x - vb->x)*(va->x - vb->x) + \
						 (va->y - vb->y)*(va->y - vb->y) + \
						 (va->z - vb->z)*(va->z - vb->z))
	for (poly = polygons; poly; poly = nextPoly) {
		nextPoly = poly->next;
		float minDist = 100000.f;
		// Madness check
		if (poly->Degree() < 3) {
			polygons = polygons->Unlink (poly);
			delete poly;
		} else {
			minDist = vdist (poly->vertices, poly->vertices->next);
			if (vdist (poly->vertices->next, poly->vertices->next->next) < minDist)
				minDist = vdist (poly->vertices->next, poly->vertices->next->next);
			if (vdist (poly->vertices, poly->vertices->next->next) < minDist)
				minDist = vdist (poly->vertices, poly->vertices->next->next);
			if (minDist < (MIN_POLY_SIZE*MIN_POLY_SIZE)) {
				polygons = polygons->Unlink (poly);
				delete poly;
			}
		}
	}	
	return polygons;
}

RDPoly *RDSave::Sort (RDPoly *polygons)
{
	/*
	 * Second pass :
	 * Sort the polygons into ascending material ID number
	 * Bubble sort - XXX 
	 */
	bool finished;
	do {
		finished = TRUE;
		for (RDPoly *p = polygons; p && p->next; p = p->next) {
			if (p->getMaterial() > p->next->getMaterial()) {
				finished = FALSE;
				RDPoly *next = p->next;
				if (p->prev) {
					p->prev->next = next;
					next->prev = p->prev;
				} else {
					polygons = next;
					next->prev = NULL;
				}
				if (next->next) {
					next->next->prev = p;
				}
				p->next = next->next;
				
				next->next	= p;
				p->prev		= next;
				p			= next;
			}
		}
	} while (!finished);
	
	return polygons;
}

/*
 * Calculates the bumpmap information
 */
void OutputBumpInfo (FILE *f, RDPoly *polygons, const VertexLib &verts)
{
	int nVerts = verts.Count();
	int vert;
	RDPoly *p;
	RDVertex *v0, *v1, *v2;
	PointAndNormal pos;
	::Point3 p0, p1, p2, normal;

	for (vert = 0; vert < nVerts; ++vert) {
		pos = verts[vert];
		normal = ::Point3 (pos.normal.x, pos.normal.y, pos.normal.z);
		// For each vertex, find a polygon which includes this vertex
		for (p = polygons; p; p = p->getNext()) {
			v0 = p->getVertex(0);
			v1 = p->getVertex(1);
			v2 = p->getVertex(2);
			if ((v0->x == pos.point.x &&
				 v0->y == pos.point.y &&
				 v0->z == pos.point.z &&
				 v0->nx == pos.normal.x &&
				 v0->ny == pos.normal.y &&
				 v0->nz == pos.normal.z) ||
				(v1->x == pos.point.x &&
				 v1->y == pos.point.y &&
				 v1->z == pos.point.z &&
				 v1->nx == pos.normal.x &&
				 v1->ny == pos.normal.y &&
				 v1->nz == pos.normal.z) ||
				(v2->x == pos.point.x &&
				 v2->y == pos.point.y &&
				 v2->z == pos.point.z &&
				 v2->nx == pos.normal.x &&
				 v2->ny == pos.normal.y &&
				 v2->nz == pos.normal.z))
				 break;
		}
		assert (p != NULL);
		p0 = ::Point3(v0->x, v0->y, v0->z);
		p1 = ::Point3(v1->x, v1->y, v1->z);
		p2 = ::Point3(v2->x, v2->y, v2->z);
		// Now to get a base vector (p0 to p1)
		::Point3 baseVector = Normalize (p1 - p0);
		// Get the vector in UV space (from p0 to p1)
		::Point3 uvVector = Normalize (::Point3(v1->u - v0->u, v1->v - v0->v, 0.f));

		::Point3 biNormal = CrossProd (normal, baseVector);
		::Point3 tangent  = CrossProd (normal, biNormal);

		/*
		 * In the new coordinate system, Angle is the angle
		 * that the UV system is rotated wrt the X/Y system
		 * (Or at least the baseVector system)
		 */
		float Angle = (float) atan2 (uvVector.y, uvVector.x) / (2*PI);
		if (Angle < 0) Angle += 1.f;

		/*
		 * Now to output that data
		 */
		fwrite (&tangent,	sizeof (float), 3, f);
		fwrite (&biNormal,	sizeof (float), 3, f);
		fwrite (&Angle,		sizeof (float), 1, f);
	}
}

void RDSave::SaveRedDogModel (const BBoxBounds &boundary, RDPoly *polygons, FILE *f, 
							  RedDog::Point3 centre, const Library<RDMaterial> *matLib, 
							  bool Stripify, RedDog::Uint32 ModelFlags)
{
	Model			model;
	ModelHeader		header;
	VertexLib		library;
	StripList		*strip = NULL;

	// Output the header
	using RedDog::Uint32;
	header.tag		= MODELHEADER_TAG;
	header.version	= MODELHEADER_VERSION;
	fwrite (&header, sizeof (ModelHeader), 1, f);

	// Clear the model structure
	memset (&model, 0, sizeof (model));

	// Fill in the model structure
	model.bounds.low.x	= boundary.lowX - centre.x;
	model.bounds.low.y	= boundary.lowY - centre.y;
	model.bounds.low.z	= boundary.lowZ - centre.z;
	model.bounds.hi.x	= boundary.hiX - centre.x;
	model.bounds.hi.y	= boundary.hiY - centre.y;
	model.bounds.hi.z	= boundary.hiZ - centre.z;

	// Suss out any material-related model flags
	for (int i = 0; i < matLib->Count(); ++i) {
		ModelFlags |= (*matLib)[i].GetModelFlags();
	}

	model.modelFlags = ModelFlags;

	if (Stripify) {
		// Reset the triangle and quad counts
		model.nTris = model.nQuads = 0;
		// Stripify the polygons which can be stripified
		strip = new StripList (polygons, matLib);
	}

	// Add all of the vertices into the vertex library, and keep track of the edges
	for (RDPoly *p = polygons; p; p = p->next) {
		PointAndNormal pAndN;

		for (RDVertex *v = p->vertices; v; v = v->next) {
			pAndN.point.x	= v->x - centre.x;
			pAndN.point.y	= v->y - centre.y;
			pAndN.point.z	= v->z - centre.z;
			pAndN.normal.x	= v->nx;
			pAndN.normal.y	= v->ny;
			pAndN.normal.z	= v->nz;

			library.AddObject (&pAndN);
		}
		if (strip) // XXX - triangles here?
			continue;

		model.nTris++;
	}
	
	// Get the vertex count
	model.nVerts			= library.Count();
	if (matLib)
		model.nMats			= matLib->Count();
	else
		model.nMats			= 0;

	// Fill in the pointers
	if (matLib)
		model.material		= (RedDog::Material *)1;
	else
		model.material		= NULL;
	model.plane				= (Plane *)0;

	model.vertexNormal		= (RedDog::Vector3 *)1;
	model.uv				= (UV *)1;
	model.strip				= (VertRef *)StripOutput (polygons, library, centre, NULL, strip);

	// Check to see if we need to do bumpmapping
	if (model.modelFlags & MODELFLAG_BUMPMAP)
		model.bump			= (BumpMapInfo *)1;

	// Add in the stripificationalismisticallness
	if (Stripify && strip) {
		model.strip = (VertRef *)((int)model.strip + strip->CountForObj());
		model.nStripPolys = strip->CountForObjPolys();
		model.nStrips = strip->CountStrips();
	}

	// Output the model
	fwrite (&model, 1, sizeof (RedDog::Model), f);

	// Output materials, if needed
	if (matLib)
		SaveMaterials (matLib, f);

	// Output the vertices
	for (int v = 0; v < model.nVerts; ++v) {
		RedDog::Point3 p;
		p = library[v].point;
		fwrite (&p, 1, sizeof (RedDog::Point3), f);
	}

	// Output the vertex normals
	for (v = 0; v < model.nVerts; ++v) {
		RedDog::Vector3 n;
		n = library[v].normal;
		fwrite (&n, 1, sizeof (RedDog::Point3), f);
	}
	
	// Output the UV values 
	// Remember they're in tri/quad order now!
#if 0 /// XXX tris
	for (count = 3; count <= 4; ++count) {
		for (p = polygons; p; p = p->next) {
			if (strip && strip->Find(p))
				continue; // Ignore stripified polygons
			if (p->Degree() != count)
				continue;
			for (RDVertex *v = p->vertices; v; v = v->next) {
				UV uv;
				uv.u = v->u; uv.v = v->v;
				fwrite (&uv, 1, sizeof (UV), f);
			}
		}
	}
#endif
	// Stripified UVs
	if (strip) {
		strip->OutputUVs(f);
	}
	
	// Calculate and output the bumpmap info, if any
	if (model.bump) {
		OutputBumpInfo (f, polygons, library);
	}

	// Output the 'strip'
	(void)StripOutput (polygons, library, centre, f, strip);
	// Output any stripped polygons
	if (strip)
		strip->ObjectOutput (library,f);

	return;
}

void RDSave::SaveMaterials (const Library<RDMaterial> *matLib, FILE *f)
{
	int material, numMaterials = matLib->Count();
	for (material = 0; material < numMaterials; ++material) {
		const char *matName, *filterName;
		char buf[256], buf2[256];
		RedDog::Material mat = (*matLib)[material].getMaterial();

		matName = (*matLib)[material].getTexName();
		if (matName[0] == '\0') {
			mat.flags &= ~MF_TEXTURE_FILTER_MASK;
			mat.flags |= MF_NO_TEXTURE;
			mat.surf.GBIX = 0;
		} else {

			if (!strnicmp (REDDOG_ORIG_TEXMAPS, matName, strlen (REDDOG_ORIG_TEXMAPS))) {
				strcpy (buf, matName + strlen (REDDOG_ORIG_TEXMAPS));
				buf[strlen(buf)-4] = '\0';
			} else if (!strnicmp (REDDOG_ORIG_TEXMAPS_2, matName, strlen (REDDOG_ORIG_TEXMAPS_2))) {
				strcpy (buf, matName + strlen (REDDOG_ORIG_TEXMAPS_2));
				buf[strlen(buf)-4] = '\0';
			} else {
				OutputDebugString ("Unknown texture :");
				OutputDebugString (buf);
				OutputDebugString ("\n");
				strcpy (buf, matName);
			}

			mat.surf.GBIX = strlen (buf) + 1;
		}

		filterName = (*matLib)[material].getFilterName();
		if (filterName[0] == '\0') {
			mat.flags &= ~MF_TEXTURE_PASTE;
			mat.pasted.surf.GBIX = 0;
		} else {
			if (!strnicmp (REDDOG_ORIG_TEXMAPS, filterName, strlen (REDDOG_ORIG_TEXMAPS))) {
				strcpy (buf2, filterName + strlen (REDDOG_ORIG_TEXMAPS));
				buf2[strlen(buf2)-4] = '\0';
			} else if (!strnicmp (REDDOG_ORIG_TEXMAPS_2, filterName, strlen (REDDOG_ORIG_TEXMAPS_2))) {
				strcpy (buf2, filterName + strlen (REDDOG_ORIG_TEXMAPS_2));
				buf2[strlen(buf2)-4] = '\0';
			} else {
				OutputDebugString ("Unknown texture :");
				OutputDebugString (buf);
				OutputDebugString ("\n");
				strcpy (buf2, filterName);
			}

			mat.pasted.surf.GBIX = strlen (buf2) + 1;
		}
		fwrite (&mat, sizeof (mat), 1, f);
		if (mat.surf.GBIX) {
			fwrite (&buf[0], 1, mat.surf.GBIX, f);
		}
		if (mat.pasted.surf.GBIX) {
			fwrite (&buf2[0], 1, mat.pasted.surf.GBIX, f);
		}
	}
}
