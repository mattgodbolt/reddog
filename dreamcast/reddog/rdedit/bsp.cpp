/*
 * BSP.cpp
 * Generate a BSP tree from an MNMesh
 */

#include "BSP.h"
#include "RDMap.h"

#include "vertcomp.h"

// Class variables for BSPTree
int BSPTree::polysDone = 0;
int BSPTree::polys = 0;
// Class variables used to hold the emitted vertices
Library<NinjaPoint3>			BSPTree::vertLib;
Library<NinjaColour>			BSPTree::uvLib;

/*
 * BSPPoly class methods
 */

/*
 * Classify a plane as either completely to one side or the other,
 * straddling, or coplanar
 */
BSPPoly::Side BSPPoly::Where (const BSPPlane &plane) const
{
	int vert;
	int degree = Degree();
	int flag = 0;
	float distance;

	// For each point find the signed distance to the plane
	for (vert = 0; vert < degree; ++vert) {
		// Get the current point
		const BSPVertex *v = GetVertex (vert);

		// Calculate the signed distance
		distance = plane.aa * v->x + plane.bb * v->y + plane.cc * v->z + plane.dd;

		// Classify the point
		if (distance >= COPLANAR_TOLERANCE) {
			// Point is on positive side of plane
			flag |= 1;
		} else if (distance <= -COPLANAR_TOLERANCE) {
			// Point is on negative side of plane
			flag |= 2;
		} else {
			// Point is coplanar - do nothing
		}

		// Break early if we know poly straddles plane
		if (flag==3)
			break;
	}

	// Return the classification
	switch (flag) {
	case 0:
		// All points were within the tolerance of being coplanar
		return COPLANAR;
	case 1:
		// All points were either coplanar, or positive
		return POSITIVE;
	case 2:
		// All points were either coplanar, or negative
		return NEGATIVE;
	default:
		// Points on either side
		return STRADDLES;
	}
}

// Sorts a list of vertices by distance order **using nextCreated**
static void SortByDistance (BSPVertex *&list)
{
	BSPVertex *sortedList = NULL;
	while (list) {
		BSPVertex *next = list->nextCreated;
		BSPVertex *v;

		if (sortedList == NULL) {
			sortedList = list;
			list->nextCreated = NULL;
		} else if (list->dist < sortedList->dist) {
			list->nextCreated = sortedList;
			sortedList = list;
		} else for (v = sortedList; v; v = v->nextCreated) {
			if (v->nextCreated == NULL) {
				v->nextCreated = list;
				list->nextCreated = NULL;
			} else if (list->dist < v->nextCreated->dist) {
				list->nextCreated = v->nextCreated;
				v->nextCreated = list;
				break;
			}
		}

		list = next;
	}

	list = sortedList;
}

/*
 * Splits the polygon using the given plane, returning the negative half,
 * and updating 'this' to be the positive half
 */
void BSPPoly::Split (const BSPPlane &plane, BSPPoly *&posList, BSPPoly *&negList)
{
	BSPVertex *v;
	BSPVertex *createdList;

	// Prepare the vertices
	for (v = vertices; v; v = v->next) {
		v->link = NULL;
		v->visited = FALSE;

		v->dist = plane.aa * v->x + plane.bb * v->y + plane.cc * v->z + plane.dd;
		v->inside = (v->dist < 0.0) ? FALSE : TRUE;

	}

	createdList = NULL;

	// Generate intersection data
	v = vertices;
	do {
		BSPVertex *n = v->next;
		if (n==NULL)
			n = vertices;			// Loop vertices around

		// Does the edge between v and n cross the plane?
		if (v->inside != n->inside) {
			Point3 p0(v->x, v->y, v->z);
			Point3 p1(n->x, n->y, n->z);

			Point3 dp = p1 - p0;

			// Calculate the distance along 'dp' that the intersection lies
			double distance = - v->dist / (plane.aa * dp.x + plane.bb * dp.y + plane.cc * dp.z);
			
			// Generate the point of intersection
			Point3 newPoint = Point3 (p0.x + dp.x * distance,
				p0.y + dp.y * distance,
				p0.z + dp.z * distance);
			
			// Calculate the UV values
			Point2 uv0 = Point2 (v->u, v->v);
			Point2 uv1 = Point2 (n->u, n->v);
			
			Point2 dp2 = uv1 - uv0;
			
			// Distance needs to be normalised to ensure 0 < u,v < 1
			distance /= Length (dp);
			Point2 newUV = Point2 (uv0.x + dp2.x * distance, uv0.x + dp2.x * distance);
			
			// Generate a new BSPVertex
			BSPVertex *newVertex = new BSPVertex (newPoint, newUV.x, newUV.y);

			// Insert the BSPVertex between v and n
			// Don't use N here unless you want circularly linked polygons
			newVertex->next = v->next;
			v->next = newVertex;

			// Also pop the vertex on the createdList
			newVertex->nextCreated = createdList;
			createdList = newVertex;
		}

		v = n;
	} while (v != vertices);

	// Make sure we have at least two created vertices
	assert (createdList != NULL);
	assert (createdList->nextCreated != NULL);

	// Sort the vertices in order of a line passing through the first pair of created vertices
	Point3 a(createdList->x, createdList->y, createdList->z);
	Point3 b(createdList->nextCreated->x, createdList->nextCreated->y, createdList->nextCreated->z);
	Point3 ab = b - a;

	for (v = createdList; v ; v=v->nextCreated) {
		v->dist = DotProd (ab,  (Point3 (v->x, v->y, v->z) - a));
	}

	// The actual sort is an insertion sort
	SortByDistance (createdList);

	// Set up the link fields
	for (v = createdList; v; v = v->nextCreated->nextCreated) {
		// Make sure we have a pair value
		assert (v->nextCreated != NULL);

		v->link = v->nextCreated;
		v->nextCreated->link = v;
	}

	BSPVertex *u, *newVert;
	BSPPoly *newPoly;

	// The actual polygon output bits
	for (;;) {
		// Find an unvisited vertex
		for (v = vertices; v; v = v->next)
			if (v->visited == FALSE)
				break;

		if (v==NULL)	// Finished!
			break;

		// Make a new polygon ready for the vertices
		u = v;
		newPoly = new BSPPoly (material);

		// Output the vertices, taking care to create copies of the vertices
		do {
			v->visited = TRUE;
			// Take a copy
			newVert = new BSPVertex(Point3 (v->x, v->y, v->z), v->u, v->v);

			newPoly->AppendVertex (newVert);
			if (v->link) {
				v = v->link;
				v->visited = TRUE;
				newVert = new BSPVertex(Point3 (v->x, v->y, v->z), v->u, v->v);
				newPoly->AppendVertex (newVert);
			}

			v = v->next;

			if (v==NULL)			// Loop round the poly
				v = vertices;

		} while (v != u);

		// Now add the polygon to the appropriate list
		if (v->inside)
			posList = posList->Link (newPoly);
		else
			negList = negList->Link (newPoly);
	}

	// This poly should now be deleted, to free up all the vertex information
}

// Generate a plane from a poly
BSPPlane BSPPoly::GetPlane (int num) const
{
	BSPPlane retVal;
	const BSPVertex *v0, *v1;
	int degree = Degree();

	num = num % degree;

	v0 = GetVertex(num);
	v1 = GetVertex((num+1) % degree);

	retVal.aa = v0->y - v1->y;
	retVal.bb = v1->x - v0->x;
	retVal.cc = 0.0f;

	double div;
	div = 1.0 / sqrt (retVal.aa*retVal.aa + retVal.bb*retVal.bb + retVal.cc*retVal.cc);
	
	retVal.aa = (float) (retVal.aa * div);
	retVal.bb = (float) (retVal.bb * div);
	retVal.cc = (float) (retVal.cc * div);
	
	retVal.dd = - (retVal.aa * v0->x + retVal.bb * v0->y + retVal.cc * v0->z);
	
	return retVal;
}

// Is the polygon convex?
BOOL BSPPoly::isConvex () const
{
	int degree = Degree();
	// Triangles are convex
	if (degree==3)
		return TRUE;

	// Make sure no edge intersects the polygon
	int edge;
	BSPPlane plane;

	for (edge = 0; edge < degree; ++edge) {
		plane = GetPlane (edge);
		if (Where (plane) == STRADDLES)
			return FALSE;
	}

	return TRUE;
}

/*
 * BSPTree class
 */

// Class function used to start the whole process off
BSPTree *BSPTree::MakeBSPTree (Mesh &mesh)
{
	/*
	 * Generate a list of polygons to be split
	 */
	int f;
	BSPPoly *polyList = NULL;
	BSPVertex *vertices;

	// Estimate number of polys in finished tree
	polysDone = 0;
	polys = (int) (mesh.numFaces * 1.5f);

	for (f = 0; f < mesh.numFaces; ++f) {
		Face &face = mesh.faces[f];

		// Make a list of the vertices, in order
		vertices = new BSPVertex (mesh.verts[face.v[0]]);
		vertices->next = new BSPVertex (mesh.verts[face.v[1]]);
		vertices->next->next = new BSPVertex (mesh.verts[face.v[2]]);

		// Fill in any texture co-ordinates
		if (mesh.tvFace) {
			TVFace &tface = mesh.tvFace[f];
			vertices->u = mesh.tVerts[tface.t[0]].x;
			vertices->v = mesh.tVerts[tface.t[0]].y;
			vertices->next->u = mesh.tVerts[tface.t[1]].x;
			vertices->next->v = mesh.tVerts[tface.t[1]].y;
			vertices->next->next->u = mesh.tVerts[tface.t[2]].x;
			vertices->next->next->v = mesh.tVerts[tface.t[2]].y;
		}

		polyList = new BSPPoly (face.getMatID(), polyList, vertices);

	}

	/*
	 * Now construct a BSP tree using the polygon list
	 */
	BSPTree *retVal = new BSPTree (polyList);

	// Ensure exit conditions are held
	assert (polyList == NULL);

	// Create a vertex library
	retVal->AddVertices();

	// Sort the vertices
	retVal->SortVertices();

	return retVal;
}

// The constructor
BSPTree::BSPTree (BSPPoly *&polygonList)
{
	// Lists of polygons on either side
	BSPPoly *posSide, *negSide;

	// Choose a partitioning plane, if we can
	if (polygonList->Count() < LEAF_CUTOFF || !ChoosePlane (polygonList) || Canceled()) {
		// We couldn't partition the plane - so we're a node
		// Or, there are too few polys to bother with

		polysDone+= polygonList->Count();
		setProgressText (NULL, (int) ((100.0*polysDone) / polys));

		positive = negative = NULL;
		polygons = polygonList;
		polygonList = NULL;
		return;
	}

	// Partition the polygons, breaking as necessary
	PartitionPolys (polygonList, posSide, negSide);
	polygons = NULL;
	assert (polygonList == NULL);

	/*
	 * There should be at least one poly on either side, otherwise the !ChoosePlane if
	 * above would have succeeded and we'd be a node by now
	 */
	assert (posSide != NULL);
	assert (negSide != NULL);

	// Recurse on
	positive = new BSPTree (posSide);
	negative = new BSPTree (negSide);
	
	assert (posSide == NULL);
	assert (negSide == NULL);

	assert (positive != NULL);
	assert (negative != NULL);
}

// The plane chooser
BOOL BSPTree::ChoosePlane (BSPPoly *list)
{
	BSPPlane	bestPlane;					// The best plane so far
	int			bestScore = 10000;			// The best score so far (highest is worst)
	BSPPoly		*curPoly;
	int		curDegree;
	int			countDown;
	static int	num = 0;
	int score = 0;
	int posScore = 0, negScore = 0;
	BSPPoly *p;
	BSPPoly::Side side;

	assert (list != NULL);

	// Try up to fifty polys
	countDown = (int) ((float)list->Count() / 2.0);

	// Here we go
	for (curPoly = list, curDegree = curPoly->Degree(); curPoly; /* get next plane */) {
		
		// Generate the values for the plane
		if (num >= curDegree) {
			curPoly = curPoly->getNext();
			if (curPoly == NULL)
				break;
			curDegree = curPoly->Degree();
			num = 0;
		} 
		plane = curPoly->GetPlane (num++);

		// Go through each poly and see if it intersects, or is invalid
		score = posScore = negScore = 0;

		for (p = list; p; p = p->getNext()) {
			side = p->Where(plane);
			// We must reject immediately if the side is coplanar
			if (side==BSPPoly::COPLANAR)
				break;

			// Otherwise, update scores appropriately
			switch (side) {
			case BSPPoly::STRADDLES:
				score++;
				break;
			case BSPPoly::POSITIVE:
				posScore++;
				break;
			case BSPPoly::NEGATIVE:
				negScore++;
				break;
			}
		}

		// Trivial reject test
		if (side==BSPPoly::COPLANAR || negScore==0 || posScore==0)
			continue;

		// Fiddle the score to make it a bit nonlinear
		score+=(score>>4);

		// Fiddle the score to try and generate balanced trees
		score+=(ABS(negScore-posScore)) >> 1;
		if (score > 10000)
			score = 9999;

		// Trivial acceptance test
		if (score < 16) {
			bestPlane = plane;
			break;
		}

		// Check to see if this one is better than any of the others
		if (score < bestScore) {
			bestScore = score;
			bestPlane = plane;
		}

		if (bestScore==10000)		// Exhaust all polys looking for a single valid one
			continue;

		if (--countDown==0)
			break;					// OK, give up

	}
	
	// If we couldn't choose a plane, then we are finished
	if (bestScore==10000)
		return FALSE;

	// OK, so we chose this plane
	plane = bestPlane;
	return TRUE;
}

/*
 * Partition the given list of polygons
 */
void BSPTree::PartitionPolys (BSPPoly *&src, BSPPoly *&posList, BSPPoly *&negList)
{
	// Entry condition check
	assert (src != NULL);

	posList = negList = NULL;

	while (src) {
		// Pop off the first polyon
		BSPPoly *curPoly = src;
		src = src->Unlink (src);

		// Prevent disasters on deletion
		curPoly->Orphan();

		// Classify it and add it to the appropriate list
		switch (curPoly->Where (plane)) {
		case BSPPoly::COPLANAR:
			// We should never get here!
			assert (0);
			break;
		case BSPPoly::POSITIVE:
			posList = posList->Link (curPoly);
			break;
		case BSPPoly::NEGATIVE:
			negList = negList->Link (curPoly);
			break;
		default:
			// Polygon is straddled, so break it in to bits
			curPoly->Split (plane, posList, negList);
			delete curPoly;	// Delete the remnants of the polygon
			break;
		}
	}
}

// Add the component vertices to the vertex library
void BSPTree::AddVertices()
{
	BSPPoly *poly;
	if (polygons==NULL) {
		if (positive)
			positive->AddVertices();
		if (negative)
			negative->AddVertices();
		return;
	}
	for (poly = polygons; poly; poly = poly->getNext())
		poly->AddVertices(vertLib, uvLib);
	int numUVs = uvLib.Count();
}

// Sorts the vertex library ready for Delta compression
void BSPTree::SortVertices()
{
	int numVerts = vertLib.Count();
	int totNumVerts = numVerts;
	int vert = 0;
	double totDist = 0.0f;
	// Allocate the final list
	NinjaPoint3 *finList = new NinjaPoint3[numVerts];

	// Copy the first point in, and start from there
	NinjaPoint3 point = vertLib[0];
	vertLib.Delete (0, 1);
	finList[vert++] = point;
	numVerts--;

	setProgressText ("Sorting vertices", 0);

	// Output all other vertices
	while (numVerts) {
		double dist, bestDist;
		int v, bestV;

		// Set up initial state
		bestDist = (1000000.0 * 1000000.0);
		bestV = 0;

		// Find the nearest neighbour
		for (v = 0; v < numVerts; ++v) {
			NinjaPoint3 &pt = vertLib[v];
			dist = (point.x - pt.x) * (point.x - pt.x) +
				(point.y - pt.y) * (point.y - pt.y) +
				(point.z - pt.z) * (point.z - pt.z);
			if (dist < bestDist) {
				bestV = v;
				bestDist = dist;
			}
		}

		// Output that neighbour next, and repeat
		totDist+=sqrt (bestDist);
		point = vertLib[bestV];
		vertLib.Delete (bestV, 1);
		finList[vert++] = point;
		if ((vert & 0xff) == 0xff) {
			setProgressText (NULL, (int) ((100.0*vert) / totNumVerts));
		}
		numVerts--;
	}

	double avDist = totDist / totNumVerts;

	// Now update the vertex library
	vertLib.ZeroCount();
	vertLib.Append (vert, finList);

	delete [] finList;
}

void BSPTree::OutputFaces (ostream &o)
{
	BSPPoly *poly;
	if (polygons==NULL) {
		if (positive)
			positive->OutputFaces(o);
		if (negative)
			negative->OutputFaces(o);
		return;
	}
	for (poly = polygons; poly; poly = poly->getNext())
		poly->OutputVertices(o, vertLib, uvLib);
}


static int indent = 0;
static void in(ostream &o) { for (int i=0; i < indent; ++i) o << " "; }
static int count = 0, nodes = 0, leaves = 0, treed = 0;
ostream &operator << (ostream &o, BSPTree *tree)
{

	if (indent == 0) {
		FILE *f = fopen ("C:\\foo.rdm", "wb");
		CompressVertices (f, tree->vertLib.Count(), tree->vertLib.Addr(0));
		fclose (f);
		// Output the ascii representation
		o << tree->vertLib.Count() << endl;
		o << tree->numFaces() << endl;
		int vertex, numVertex = tree->vertLib.Count();
		for (vertex = 0; vertex < numVertex; ++vertex) {
			o << tree->vertLib[vertex].x << " " <<
				tree->vertLib[vertex].z << " " <<
				-tree->vertLib[vertex].y << endl;
		}
		tree->OutputFaces(o);
	}

	indent++;

	in(o);
	if (tree->isLeaf()) {
		o << "BSP Leaf with " << tree->polygons->Count() << " polys" << endl;
		count+= tree->polygons->Count();
		leaves++;
	} else {
		o << "Positive side:" << endl << tree->positive;
		in(o);
		o << "Negative side:" << endl << tree->negative;
		nodes++;
	}

	if (indent > treed)
		treed = indent;

	if (--indent==0) {
		o << endl << "Total polys coded:  " << count << endl;
		o << "Total nodes:        " << nodes << endl;
		o << "Total leaves:	      " << leaves << endl;
		o << "Max tree depth:     " << treed << endl;
		o << "Average polys/leaf: " << ((float)count / (float)leaves) << endl;
		count = nodes = leaves = treed = 0;
	}
	return o;
}
