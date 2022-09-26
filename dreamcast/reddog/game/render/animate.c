/*
 * Animate.c
 * Animation and object heirarchy functions
 */

#define OLD_VERSIONS 1
#include "..\RedDog.h"
#include "..\Coll.h"
#include "Animate.h"

static Object	*SubObjectArray[MAX_SUB_OBJECTS];
static Uint16	SubObjectCount = 0;

static Uint32 curVersion = 0x000;
static Bool		bStore = TRUE;

static void PNodeObjFree (Object *obj, Deallocator *dealloc, void *ptr)
{
	int child;

	/* Free collision information */
	if (obj->ncp) {
		dealloc (ptr, obj->ocpt);
		dealloc (ptr, obj->cp);
	}

	/* Free the model information */
	if (obj->model)
		ModelRelease (obj->model, dealloc, ptr);
	/* Free children */

	
	if (obj->no_child)
	{
		for (child = 0; child < obj->no_child; ++child)
		{
			PNodeObjFree (&obj->child[child], dealloc, ptr);
		}		

		if (obj->child)
			dealloc (ptr, obj->child);
	}		
}


void PNodeFree (PNode *node, Deallocator *dealloc, void *ptr)
{
	dAssert (node != NULL, "NULL node");
	dAssert (dealloc != NULL, "NULL deallocator");

	/* Free the object */
/*	if (node->obj) { */
	PNodeObjFree (node->obj, dealloc, ptr);
	dealloc (ptr, node->obj);
/*	} */

	/* Free the object ID list */
	if (node->objId)
		dealloc (ptr, node->objId);

	/* Free the name */
	if (node->name)
		dealloc (ptr, node->name);

	/* Free the animations */
	if (node->anim) {
		/*AnimFree (node->anim, dealloc, ptr);*/
		dealloc (ptr, node->anim);
	}

	node->anim=NULL;

	/* Free the animation block */
/*	if (node->anim) */
/*		dealloc (ptr, node->anim); */

}


