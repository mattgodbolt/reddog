/*
 * Map.c
 * Map functions
 */

#include "..\RedDog.h"
#include "Internal.h"


static CollNode *RecurseLoadCollision (Stream *s)
{
	CollNode *node = CollisionAlloc (sizeof(CollNode));
	sRead (node, sizeof (CollNode), s);
	if (node->left) {
		node->left = RecurseLoadCollision(s);
		node->right = RecurseLoadCollision(s);
	} else {
		CollLeaf *leaf = CollisionAlloc (sizeof(CollLeaf));
		node->right = (CollNode *)leaf;
		sRead (leaf, sizeof (CollLeaf), s);
		leaf->polyNumber = CollisionAlloc (sizeof(Uint16) * leaf->noPoly);
		sRead (leaf->polyNumber, (sizeof(Uint16) * leaf->noPoly), s);
	}
	return node;
}

// Fix up the pointers in a hierarchy
static void RecurseFixUpHierarchy (BoxTree *tree, Map *map)
{
	// Node or leaf?
	if (tree->leaf.tag == NULL) {
		// Leaf:
		tree->leaf.model = &map->box[(int)tree->leaf.model];
	} else {
		// Node:
		int i;
		for (i = 0; i < 4; ++i) {
			if ((int)tree->node.child[i] == -1) {
				tree->node.child[i] = NULL;
			} else {
				tree->node.child[i] = &map->hierarchy[(int)tree->node.child[i]];
			}
			if (tree->node.child[i])
				RecurseFixUpHierarchy (tree->node.child[i], map);
		}
	}
}

/*
 * Load a map in, using the MapHeap
 */
Map *MapLoad (Stream *s)
{
	Map *retVal;
	int i;
	int loop,loop2;

	dAssert (s != NULL, "Bad stream passed to MapLoad");

	dLog ("* Loading new map :\n");

	/* Set the material type accordingly */
	rLoadContext = &scapeContext;

	/* First read in the toplevel structure */
	retVal = MapAlloc (sizeof (Map));
	retVal->header.tag = 0;
	sRead (retVal, sizeof (Map), s);

	/* Check this is a map */
	dAssert (retVal->header.tag == MAPHEADER_TAG, "Invalid map file");
	dLog ("* Map has %d materials and %d boxes\n", retVal->numMaterials, retVal->numBoxes);

	/* Read in the material array */
	retVal->material = MapAlloc (sizeof (Material) * retVal->numMaterials);
	for (i = 0; i < retVal->numMaterials; ++i) {
		rLoadMaterial (&retVal->material[i], s);
	}

#if 0
	/* Read in the vertex array */
	i = (int)retVal->v;
	dLog ("Map has %d vertices\n", i / sizeof (StripPos));
	/* Align the vertex data */
	retVal->v = MapAlloc (i + 4);
	if ((int)retVal->v & 7)
		retVal->v = (StripPos *)((char *)retVal->v + 4);
	sRead (retVal->v, i, s);
#endif

	/* Read in the collision polygons */
	i = (int)retVal->collPoly;
	dLog ("Map has %d vertices\n", i);
	retVal->collPoly = CollisionAlloc (i * sizeof (CollPoly));
	sRead (retVal->collPoly, i * sizeof (CollPoly), s);

	for (loop=0;loop<i;loop++) {
		dAssert ((retVal->collPoly[loop].flag & ALLCOLLIDE) != 0, "Dodgy coll poly");
	}


	/* Read in the box array */
	retVal->box = MapAlloc (sizeof (ScapeModel) * retVal->numBoxes);
	for (i = 0; i < retVal->numBoxes; ++i) {
		/* Read in the model */
		ScapeModel *m = rLoadScapeModel (&retVal->box[i], s, (Allocator *)SHeapAlloc, MapHeap);
		// Go through and check all the materials
		{
			Uint32 matFlags = 0, hideId = 0, numStrip;
			StripHeader *h = (StripHeader *)m->strip;
			numStrip = h->nStrip;
			while (numStrip) {
				matFlags |= retVal->material[h->material].flags;
				if (retVal->material[h->material].hideID != 1)
					hideId++;
				h+= 1 + numStrip;
				numStrip = h->nStrip;
			}
			if (((matFlags & (MF_ENVIRONMENT|MF_TEXTURE_PASTE|MF_ALPHA|MF_TEXTURE_ANIMATED|MF_TRANSPARENCY_PUNCHTHRU|MF_TRANSPARENCY_SUBTRACTIVE|MF_TRANSPARENCY_FROM_SCREEN|MF_SECOND_PASS)) == 0) && hideId == 0)
				m->strip = (StripEntry *)((int)m->strip & 0x7fffffff);
		}
	}

	/* Read in the collision information */
#if NEWCOLL
	if (retVal->topNode) {
		retVal->topNode = RecurseLoadCollision (s);
	}
#else
	if (retVal->grid) {
		int x, y;
		
		dLog ("Map has a %d x %d collision grid\n", retVal->xNum, retVal->yNum);
		
		retVal->grid = (GridBox *)CollisionAlloc (sizeof (GridBox) * retVal->xNum * retVal->yNum);

		sRead (retVal->grid, sizeof (GridBox) * retVal->xNum * retVal->yNum, s);

		for (y = 0; y < retVal->yNum; ++y) {
			for (x = 0; x < retVal->xNum; ++x) {
				if (retVal->grid[x + retVal->xNum * y].polygon) {
					Uint32 num;
					num = retVal->grid[x + retVal->xNum * y].nPolys;
					retVal->grid[x + retVal->xNum * y].polygon = (GridPoly *)CollisionAlloc (sizeof (GridPoly) * num);
					sRead (retVal->grid[x + retVal->xNum * y].polygon, sizeof (GridPoly) * num, s);
				}
			}
		}
	}
#endif
	/* Read in the visibility boxes */
	if (retVal->numVisBoxes) {
		dLog ("Map has %d visibility boxes\n", retVal->numVisBoxes);
		retVal->visBox = MapAlloc (sizeof (VisBox) * retVal->numVisBoxes);
		sRead (retVal->visBox, sizeof (VisBox) * retVal->numVisBoxes, s);
	}
	
	/* Read in the new collision madness */
	sRead (&i, 4, s);
	retVal->collLookup = MapAlloc (i);
	sRead (retVal->collLookup, i, s);
	// Sort out the madness
	for (loop = 0; loop < (i/4); ++loop) {
		Sint16 *p = (Sint16 *)(retVal->collLookup + loop);
		Sint16 box, vert;
		box = *p++;
		vert = *p++;
		retVal->collLookup[loop] = &retVal->box[box].vertex[vert];
	}

	/* Read in the hierarchy */
	if (retVal->hierarchy) {
		int nBytes = (int)retVal->hierarchy * sizeof (BoxTree);
		retVal->hierarchy = MapAlloc (nBytes);
		sRead (retVal->hierarchy, nBytes, s);
		RecurseFixUpHierarchy (retVal->hierarchy, retVal);
	}

	/* Skip to next chunk */
	sSkip (s);

	
	return retVal;
}

/*
 * Reset the map
 */
void MapReset (Map *map)
{
	Uint32 i;
	dLog ("Resetting map");
}