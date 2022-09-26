/*
 * BBox.cpp
 * Build a bounding box representation of a landscape
 */

#include "StdAfx.h"
#include <limits.h>

static int numStrips;
static int numSint16s;

#define ERROR_FIX	2.0f


float MIN_BBOX_POLYS		= 40;
float MAX_BBOX_SIZE		= 10.f,
      MAX_POLY_DENSITY	= 0.3f,
	  BASE_CARVE_AMOUNT_X = 60.f,
	  BASE_CARVE_AMOUNT_Y = 60.f;
/*
#define MIN_BBOX_POLYS		200
#define MAX_BBOX_SIZE		15.f
#define MAX_POLY_DENSITY	0.3f 
#define BASE_CARVE_AMOUNT	20.f

#define BBOX_SIDE_LENGTH	20.f
*/

#define CUTOFF_DEPTH	16		// The maximum tree depth
#define CUTOFF_SIZE		10.f	// Terminate when longest edge is less than this big
#define CUTOFF_POLYS	10		// Terminate when this many polys in a box

/*
 * Partition a set of polygons along an axis
 */
void BBoxLandscape::PartitionX (const PolyTab &source, PolyTab &left, PolyTab &right, float pos) const
{
	RDPoly **sourcePtr = source.Addr(0);
	int i, nMax;
	float min, max;
	max = pos + ERROR_FIX;
	min = pos - ERROR_FIX;
	nMax = source.Count();
	for (i = 0 ; i < nMax; ++i) {
		RDPoly *poly = *sourcePtr++;
		if (poly->LeftMost() <= max)
			left.Append (1, &poly, left.Count());
		if (poly->RightMost() >= min)
			right.Append (1, &poly, right.Count());
	}
}

void BBoxLandscape::PartitionY (const PolyTab &source, PolyTab &left, PolyTab &right, float pos) const
{
	RDPoly **sourcePtr = source.Addr(0);
	int i, nMax;
	float min, max;
	max = pos + ERROR_FIX;
	min = pos - ERROR_FIX;
	nMax = source.Count();
	for (i = 0 ; i < nMax; ++i) {
		RDPoly *poly = *sourcePtr++;
		if (poly->UpMost() >= min)
			left.Append (1, &poly, left.Count());
		if (poly->DownMost() <= max)
			right.Append (1, &poly, right.Count());
	}
}

RedDog::CollNode *BBoxLandscape::Partition (PolyTab &parent, BBoxBounds &bounds, int depth)
{
	/*
	 * Check for a Cancel
	 */
	if (GetCOREInterface()->GetCancel())
		return NULL;
	/*
	 * Check to see if we've reached a terminal node
	 */
	if (parent.Count() < CUTOFF_POLYS ||
		depth > CUTOFF_DEPTH ||
		MAX((bounds.hiX - bounds.lowX),(bounds.hiY - bounds.lowY)) < CUTOFF_SIZE) {
		RedDog::CollNode *retVal = new RedDog::CollNode;
		RedDog::CollLeaf *leaf = new RedDog::CollLeaf;
		retVal->left = NULL;
		retVal->right = (RedDog::CollNode *)leaf;
		leaf->noPoly = parent.Count();
		float height = -10000.f;
		if (leaf->noPoly) {
			Uint16 *list = new Uint16[leaf->noPoly];
			for (int i = 0; i < leaf->noPoly; ++i) {
				list[i] = (Uint16) parent[i]->polyNumber;
				float highestPoint = parent[i]->getHighestPoint();
				if (highestPoint > height)
					height = highestPoint;
			}
			leaf->polyNumber = list;
		} else {
			leaf->polyNumber = NULL;
		}
		// Get 16bit FP version
		unsigned short h16 = (unsigned short) ((*(unsigned long *)&height)>>16);
		// Ensure 16bit version is greater than FP version
		for (;;) {
			unsigned int h32;
			float actualHeight;
			h32 = h16 << 16;
			actualHeight = *(float *)&h32;
			if (actualHeight >= height)
				break;
			h16++;
		}
		leaf->height = h16;
		areaDone += (bounds.hiX - bounds.lowX) * (bounds.hiY - bounds.lowY);
		setProgressText (NULL, (int)(100.f * areaDone / areaTot));
		if (depth != 0)			// Don't zero the root list
			parent.SetCount(0);
		return retVal;
	} else {
		/* 
		 * Partition the set of polys
		 */
		float ySize, xSize;
		PolyTab left, right;
		left.Resize(int(parent.Count() / 1.5f));
		right.Resize(int(parent.Count() / 1.5f));
		ySize = bounds.hiY - bounds.lowY;
		xSize = bounds.hiX - bounds.lowX;
		if (fabs (xSize - ySize) < 0.01f) {
			MessageBox (NULL, "Error: perfectly square region found - collision will not work - see MattG or Sav!", "Error", MB_OK);
		}
		if (xSize > ySize) {
			float partition = (bounds.hiX + bounds.lowX) * 0.5f;
			PartitionX (parent, left, right, partition);
			if (depth != 0)			// Don't zero the root list
				parent.SetCount(0);
			/* And recurse down */
			RedDog::CollNode *retVal = new RedDog::CollNode;
			BBoxBounds leftBounds (bounds.lowX, bounds.lowY, 0, partition, bounds.hiY, 0),
					rightBounds (partition, bounds.lowY, 0, bounds.hiX, bounds.hiY, 0);
			retVal->left = Partition (left, leftBounds, depth+1);
			retVal->right = Partition (right, rightBounds, depth+1);

			return retVal;
		} else {
			float partition = (bounds.hiY + bounds.lowY) * 0.5f;
			PartitionY (parent, left, right, partition);
			if (depth != 0)			// Don't zero the root list
				parent.SetCount(0);
			/* And recurse down */
			RedDog::CollNode *retVal = new RedDog::CollNode;
			BBoxBounds leftBounds (bounds.lowX, partition, 0, bounds.hiX, bounds.hiY, 0),
					rightBounds (bounds.lowX, bounds.lowY, 0, bounds.hiX, partition, 0);
			retVal->left = Partition (left, leftBounds, depth+1);
			retVal->right = Partition (right, rightBounds, depth+1);

			return retVal;
		}
	}
}

void BBoxLandscape::DeleteTree (RedDog::CollNode *node)
{
	if (node->left == NULL) {
		RedDog::CollLeaf *leaf = (RedDog::CollLeaf *)node->right;
		delete [] (RDPoly **)leaf->polyNumber;
		delete leaf;
		delete node;
	} else {
		delete node->left;
		delete node->right;
		delete node;
	}
}


/*
 * Generate the map hierarchy
 */

/*
 * Constructor for BBoxLandscape, taking parameters
 * of the accumulated mesh it is to represent
 */


static int numPolys, polysDone;

static int numMats;

static Interface *maxIp;
static int curPc = -1, nBox = 0;

BBoxLandscape::BBoxLandscape (Mesh &mesh, Matrix3 *uvTLib, Library<RDMaterial> &matLib, Tab<IShadBox *> shadowBox)
{
	BBoxBounds dummy(0,0,0,0,0,0);
	int boxNumber = 0;
	BBox *prevBox = NULL;
	int num = 0;

	setProgressName ("Preparing mesh");
	RDPoly *polyList = MeshSupport::MeshPrep (mesh, uvTLib, dummy, matLib);

	if (GetCOREInterface()->GetCancel() || polyList == NULL)
		return;

	// Get rid of singular polygons
	polyList = RDSave::RemoveSingular (polyList);
	
	numPolys = polyList->Count(); polysDone = 0;

	if (GetCOREInterface()->GetCancel())
		return;

	boxList = NULL;
	
	// Generate the box representation and collision bounding box data
	mesh.buildBoundingBox ();
	Box3 bounds = mesh.getBoundingBox ();

	setProgressName ("Processing collision data");
	minX = bounds.pmin.x;
	minY = bounds.pmin.y;
	maxX = bounds.pmax.x;
	maxY = bounds.pmax.y;

	rootList.SetCount(0);

	areaDone = 0;
	areaTot = (maxX-minX) * (maxY-minY);
	setProgressText ("Collision pass");
	/* 
	 * New method for generating collision information 
	 * Generate the root list of all polys
	 */
	if (GetCOREInterface()->GetCancel())
		return;

	int nColPolys = 0, nDS = 0;
	for (RDPoly *poly = polyList; poly; poly = poly->getNext()) {
		if (!matLib[poly->getMaterial()].isnonCollisionable()) {
			nColPolys++;
			poly->PrepCol();
			poly->polyNumber = rootList.Count();
			rootList.Append (1, &poly, rootList.Count());
		}
	}

	if (rootList.Count() >= 65536) {
		char buf[1024];
		sprintf (buf, "Too many collisionable polygons (>65536).\rSome parts of the collision may not work properly.\r"
					  "You have %d polys, of which %d are two-sided.", nColPolys, nDS);
		MessageBox (NULL, buf, "Collision warning", MB_OK);
	}

	/*
	 * Now recursively repartition the polygons into two halves
	 */
	topNode = Partition(rootList, BBoxBounds (bounds.pmin.x, bounds.pmin.y, 0, bounds.pmax.x, bounds.pmax.y, 0), 0);
	if (GetCOREInterface()->GetCancel())
		return;
	/*
	 * Based on the leaves, generate the boxlist
	 */
	float xbox, ybox;
	/* (-o-) */
	xbox = (maxX - minX) / BASE_CARVE_AMOUNT_X;
	ybox = (maxY - minY) / BASE_CARVE_AMOUNT_Y;
	setProgressName ("BBox generation");
	for (float y = minY; y < maxY; y+= ybox) {
		/*
		 * Get a list of all polygons that are active, and discard those already
		 * completely processed
		 */
		Tab<RDPoly *> activePolyTab;
		RDPoly *nextPoly;
		for (RDPoly *poly = polyList; poly; poly = nextPoly) {
			float	ymin = y,
					ymax = y + ybox;
			nextPoly = poly->getNext();
			BBoxBounds bounds = poly->getBounds();
			// Is the polygon invisible?
			if (bounds.lowY <= ymax) {
				// The polygon is within this line
				activePolyTab.Append (1, &poly, activePolyTab.Count());
			}
		}
		if (GetCOREInterface()->GetCancel())
			return;
		for (float x = minX; x < maxX; x+= xbox) {
			BBoxBounds boxBound (
				x,			y,		-1000000.f,
				x+xbox,		y+ybox,	 1000000.f);
	
			float hiZ = -10000, lowZ = 10000;
			BBox *thisBox = NULL;

			for (int p = 0; p < activePolyTab.Count(); ++p) {
				poly = activePolyTab[p];

				BBoxBounds bounds = poly->getBounds();

				// Does this polygon collide with this box?
				if (!poly->CollidesWith (boxBound))
					continue;

				// Do we need to start a new box?
				if (thisBox == NULL) {
					thisBox = new BBox ();
					if (boxList == NULL) {
						boxList = prevBox = thisBox;
					} else {
						if (prevBox)
							prevBox->SetNext (thisBox);
						prevBox = thisBox;
					}
				}
				(void)thisBox->AddPoly (poly);
				polyList = polyList->Unlink (poly);
				activePolyTab.Delete(p, 1);
				--p;
				
			}
			if (thisBox != NULL)
				thisBox = NULL, boxNumber++;
		}
		
		setProgressText (NULL, (int)((100.0 * (y-minY))/(maxY-minY)));
		if (GetCOREInterface()->GetCancel())
			break;
	}
#if 0
	// Now coalesce small boxes with nearby boxes
	// and break up big boxes
	setProgressName ("Amalgamating boxes");
	int foo = 0;
	bool done = false;
	while (!done) {
		if (GetCOREInterface()->GetCancel())
			break;
		BBox *box;
		done = true;
		for (box = boxList; box; box = box->next) {
			if (foo++ == 20) {
				foo = 0;
				num = boxList->Count();
				setProgressText ("", (int)(100.f * (float)(num - box->Count())/(float)num));
			}
			float polyDensity;
			box->UpdateBoundary();
			// Is this box too big?
			// Check the polygon density
			polyDensity = box->polyList.Count() / (PI * box->boundary.SqrRadius());
			if (polyDensity > MAX_POLY_DENSITY || box->boundary.LongestEdge() > MAX_BBOX_SIZE)
			{
				continue;
			}
			// Has this box got less than average polygons?
			if (box->polyList.Count() >= MIN_BBOX_POLYS || box->polyList.Count() == 0)
				continue;
			// Now try to find a nearby box with which this box can be merged with
			BBox *box2, *box2next;
			for (box2 = box->next; box2; box2 = box2next) {
				box2next = box2->next;
				if (box2->polyList.Count() == 0)
					continue;
				float amount = 1.4142f * (box->boundary.SqrRadius() + box2->boundary.SqrRadius());
				box2->UpdateBoundary();
				// Check that the amalgamated bbox isn't too big in the XY direction
				BBoxBounds amalgam = box->boundary;
				amalgam.Merge (box2->boundary);
				if ((amalgam.hiX - amalgam.lowX) > MAX_BBOX_SIZE ||
					(amalgam.hiY - amalgam.lowY) > MAX_BBOX_SIZE)
					continue;
				Point3 p = box2->boundary.Centre() - box->boundary.Centre();
				if (LengthSquared(p) < amount) {
					box->AddPoly (box2->polyList);
					// Remove box2 from the list
					if (box2 == box->next) {
						box->next = box2->next;
						box2->next = NULL;
						delete box2;
					} else {
						for (BBox *b = box; b->next; b = b->next) {
							if (b->next == box2) {
								b->next = box2->next;
								box2->next = NULL;
								delete box2;
								break;
							}
						}
					}
					done = false;
				}
				
			}
		}
				
	}
#endif

	if (GetCOREInterface()->GetCancel())
		return;
	
	num = 0;
	int maxNum = 0, fooNum = 0;
	for (BBox *bee = boxList; bee; bee = bee->next)
		fooNum++;

	setProgressName ("Stripifying boxes");
	for (BBox *box = boxList; box; box = box->next) {
		maxNum++;
		setProgressText (NULL, (int)((100.0 * maxNum)/((float)fooNum)));

		// Generate the boundary and create a linked list of polygons
		box->polygons = NULL;
		for (int i=box->polyList.Count()-1; i >= 0; --i) {
			box->polygons = box->polygons->ForceLink (box->polyList[i]);
		}
	
		if (box->polygons)
			box->polygons->prev = NULL;	// Boundary condition
		
		// Now perform sorting on these 
		box->polygons = RDSave::Sort (box->polygons);

		// Have we killed all the polys?
		if (box->polygons == NULL) {
			continue;
		}

		// Generate the bounding box of the polygons
		box->UpdateBoundary();

		// Stripify the polygons (which renumbers all the polygons anyway)
		box->stripList = new StripList (box->polygons);

		// Now generate the boundary, and fill in the boxNumber details 
		box->boundary = box->polygons->getBounds();
		for (RDPoly *p = box->polygons; p; p = p->next) {
			box->boundary.Merge (p->getBounds());
		}

		num++;
	}

	shadBox = shadowBox;
}

void BBoxLandscape::ExportMap (FILE *f, Library<RDMaterial> &matLib, Tab<RedDog::VisBox> &visBoxes)
{
	assert (f != NULL);

	// If there are no visBoxes, create an infinite one
	if (visBoxes.Count() == 0) {
		RedDog::VisBox box;
		memset (&box.worldToBoxSpace, 0, sizeof (RedDog::Matrix));
		box.ID = box.canAlsoSee = 0;
		visBoxes.Append (1, &box, 0);
	}

	// Output the Map header and numbers
	RedDog::Map map;
	map.header.tag			= MAPHEADER_TAG;
	map.header.version		= MAPHEADER_VERSION;
	map.box					= (RedDog::ScapeModel *)1;
	map.numBoxes			= boxList ? boxList->Count() : 0;
	map.numMaterials		= matLib.Count();
	map.xMin				= minX;
	map.yMin				= minY;
	map.xMax				= maxX;
	map.yMax				= maxY;
	map.topNode				= (RedDog::CollNode *)1;
	map.numVisBoxes			= visBoxes.Count();
	map.visBox				= (RedDog::VisBox *)1;
	map.shadBox				= (RedDog::ShadBox *)1;

	// Create the hierarchy
	BoxTree *tree = MakeHierarchy ();
	// Number all of the boxes
	int ID = 0;
	tree->Number(&ID);
	map.hierarchy			= (RedDog::BoxTree *)ID;

	map.collPoly			= (RedDog::CollPoly *)rootList.Count();

	fwrite (&map, sizeof (map), 1, f);

	curPc = -1; nBox = map.numBoxes;
	maxIp = GetCOREInterface();

	// Output materials here...
	RDSave::SaveMaterials (&matLib, f);

	// Create a collisionable vertex library
	VertexLibrary vertLib;
	for (int pol = 0; pol < (int)map.collPoly; ++pol) {
		RDPoly *poly = rootList[pol];
		Point3 a, b, c;
		RedDog::CollPoly cPoly;
		a = Point3(*poly->vertices);
		b = Point3(*poly->vertices->next);
		c = Point3(*poly->vertices->next->next);
		cPoly.flag = matLib[poly->material].getCollFlags();
	 	int found;
		if (cPoly.flag)
			found=1;

		int v0, v1, v2;
		v0 = vertLib.Add(PosAndCol(a, poly->vertices->colour), true);
		v1 = vertLib.Add(PosAndCol(b, poly->vertices->next->colour), true);
		v2 = vertLib.Add(PosAndCol(c, poly->vertices->next->next->colour), true);

		cPoly.v[0] = v0;
		cPoly.v[1] = v1;
		cPoly.v[2] = v2;
		Point3 normal = Normalize(CrossProd (b-a, c-a));
		cPoly.n.x = normal.x;
		cPoly.n.y = normal.y;
		cPoly.n.z = normal.z;
		cPoly.D = -DotProd (normal, a);
		fwrite (&cPoly, 1, sizeof (cPoly), f);
	}
	vertLib.Prep();

	if (vertLib.Count() > 65535) {
		char buf[256];
		sprintf (buf, "Too many collisionable vertices: found %d - collision won't work!", vertLib.Count());
		MessageBox (NULL, buf, "Error!", 0);
	}

	// For each box, output that
	boxList->ExportMap (f, 0, visBoxes, shadBox);

	// Write out the collision data
	RecurseWriteNode (topNode, f);
	rootList.SetCount(0);

	// Write out the visibility boxes
	if (visBoxes.Count())
		fwrite (visBoxes.Addr(0), sizeof (RedDog::VisBox), visBoxes.Count(), f);

	// Now to write out the collisionable lookup, stored as Uint16 map box, Uint16 vert num
	// First write out number of bytes to read
	int i = vertLib.Count() * 4;
	fwrite (&i, 1, 4, f);
	
	for (i = 0; i < vertLib.Count(); ++i) {
		int vertNum = -1;
		int box;
		const BBox *b;
		PosAndCol *p = vertLib.Addr(i);
		for (box = 0, b = boxList; b; b = b->getNext(), box++) {
			vertNum = b->vertLib.Find(*p);
			if (vertNum != -1) {
				signed short Box, Vert;
				Box = (signed short) box;
				Vert = (signed short) vertNum;
				fwrite (&Box, 1, 2, f);
				fwrite (&Vert, 1, 2, f);
				break;
			}
			if (vertNum != -1)
				break;
		}
		if (vertNum == -1) {
			MessageBox (NULL, "Unable to find collisionable vertex in list - buggery", "arse", 0);
		}
	}
	

	// Write out the shadow boxes
#if 0
	if (shadBoxes.Count())
	fwrite (shadBoxes.Addr(0), sizeof (RedDog::ShadBox), shadBoxes.Count(), f);
#endif
	// Write out the hierarchy (and free it)
	tree->WriteOut (f);
}

void BBoxLandscape::RecurseWriteNode (RedDog::CollNode *node, FILE *f)
{
	fwrite (node, sizeof (RedDog::CollNode), 1, f);
	if (node->left == NULL) {
		RedDog::CollLeaf *leaf = (RedDog::CollLeaf *)node->right;
		fwrite (leaf, sizeof (RedDog::CollLeaf), 1, f);
		if (leaf->noPoly)
			fwrite (leaf->polyNumber, sizeof (Uint16), leaf->noPoly, f);
	} else {
		RecurseWriteNode (node->left, f);
		RecurseWriteNode (node->right, f);
	}
}

void BBox::ExportMap (FILE *f, int num, Tab<RedDog::VisBox> &visBoxes, Tab<IShadBox *> &shadBoxes)
{
	assert (f != NULL);

	int pc = int(100.f * float(num) / float(nBox));
	if (pc != curPc) {
		curPc = pc;
		maxIp->ProgressUpdate (pc);
	}

	// Ignore empty boxes
	if (polygons) {
		// Fill in the model structure
		stripList->AddToLib(vertLib);
		vertLib.Prep();
		stripList->Export (f, boundary, vertLib, visBoxes, shadBoxes);
		if (next)
			next->ExportMap (f, num+1, visBoxes, shadBoxes);
	} else {
		if (next)
			next->ExportMap (f, num, visBoxes, shadBoxes);
	}

}

/*
 * Does this poly belong in the given boundary collision box
 */
bool RDPoly::CollidesWith (const BBoxBounds &boundary) const
{
	RDVertex *v;

	int vertCount;
	int numVerts = Degree();

	vertCount = 0;
	// Are all polys above the box?
	for (v = vertices; v; v = v->next) {
		if (v->y > boundary.hiY)
			vertCount++;
	}
	if (vertCount == numVerts)
		return false;
	
	vertCount = 0;
	// Are all polys below the box?
	for (v = vertices; v; v = v->next) {
		if (v->y < boundary.lowY)
			vertCount++;
	}
	if (vertCount == numVerts)
		return false;
	
	vertCount = 0;
	// Are all polys left of the box?
	for (v = vertices; v; v = v->next) {
		if (v->x < boundary.lowX)
			vertCount++;
	}
	if (vertCount == numVerts)
		return false;
	
	vertCount = 0;
	// Are all polys right of the box?
	for (v = vertices; v; v = v->next) {
		if (v->x > boundary.hiX)
			vertCount++;
	}
	if (vertCount == numVerts)
		return false;
	
	return true;
}

BoxTree *BBoxLandscape::MakeHierarchy()
{
	// Firstly for each model create a boxtree leaf
	int i = 0;
	const BBox *box;
	Tab<BoxTree *>treeList;
	Tab<BoxTree *>topLevel;
	for (box = boxList; box; box = box->getNext()) {
		BoxTree *newBox = new BoxTree (box, i);
		i++;
		treeList.Append (1, &newBox, treeList.Count());
	}
	// Keep on coalescing bboxes
	do {
		while (treeList.Count() > 1) {
			// Take the first bbox and find the nearest 3 boxes
			float bestDist;
			BoxTree *newBox[3] = {NULL, NULL, NULL};
			int newBoxNum[3], startFrom;
			BoxTree *amalgam;
			int bestOther, nFound = 0;
			Point3 p, q;
			bestDist;
			p = treeList[0]->midPoint();
			startFrom = 1;
			do {
				bestOther = -1;
				bestDist = 9e9f;
				for (int i = startFrom; i < treeList.Count(); ++i) {
					q = treeList[i]->midPoint();
					float dist = Length (p - q);
					if (dist < bestDist) {
						bestDist = dist;
						bestOther = i;
					}
				}
				if (bestOther != -1) {
					newBox[nFound] = treeList[bestOther];
					newBoxNum[nFound] = bestOther;
					startFrom = bestOther + 1;
					nFound++;
				} else 
					break;
			} while (treeList.Count() > 1 && nFound < 3);
			amalgam = new BoxTree (treeList[0], newBox[0], newBox[1], newBox[2]);
			topLevel.Append (1, &amalgam, topLevel.Count());
			if (newBox[2])
				treeList.Delete(newBoxNum[2], 1);
			if (newBox[1])
				treeList.Delete(newBoxNum[1], 1);
			treeList.Delete(newBoxNum[0], 1);
			treeList.Delete(0, 1);
		}
		// Now merge topLevel and treeList
		treeList.Append (topLevel.Count(), topLevel.Addr(0), 0);
		topLevel.ZeroCount();
	} while (treeList.Count() != 1);
	return treeList[0];
}

void BoxTree::WriteOut(FILE *f)
{
	// Write out 'this'
	RedDog::BoxTree ent;
	if (isLeaf) {
		ent.leaf.tag = NULL;
		ent.leaf.model = (RedDog::ScapeModel *)modelNum;
		fwrite (&ent, 1, sizeof (RedDog::BoxTree), f);
	} else {
		ent.node.child[0] = (RedDog::BoxTree *)child[0]->boxNumber;
		ent.node.child[1] = (RedDog::BoxTree *)child[1]->boxNumber;
		ent.node.child[2] = child[2] ? (RedDog::BoxTree *)child[2]->boxNumber : (RedDog::BoxTree *)-1;
		ent.node.child[3] = child[3] ? (RedDog::BoxTree *)child[3]->boxNumber : (RedDog::BoxTree *)-1;
		ent.node.bounds.low.x = boundary.lowX;
		ent.node.bounds.low.y = boundary.lowY;
		ent.node.bounds.low.z = boundary.lowZ;
		ent.node.bounds.hi.x = boundary.hiX;
		ent.node.bounds.hi.y = boundary.hiY;
		ent.node.bounds.hi.z = boundary.hiZ;
		fwrite (&ent, 1, sizeof (RedDog::BoxTree), f);
		for (int i = 0; i < 4; ++i)
			if (child[i])
				child[i]->WriteOut(f);
	}
}
