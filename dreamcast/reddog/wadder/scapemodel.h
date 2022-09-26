/*
 * ScapeModel.h
 * The landscape model format
 */

#ifndef _SCAPEMODEL_H
#define _SCAPEMODEL_H

/*
 * An entry in the model's strip list
 */
typedef struct tagStripEntry
{
	Uint32		vertRef;	/* vertRef is vertex number x 4 */
	Uint32		uv;
} StripEntry;

/*
 * The header of a model's strip list
 */
typedef struct tagStripHeader
{
	Uint32	nStrip;				/* Number of StripEntries following this - 0 is EOM */
	Uint32	material;			/* The material offset */
} StripHeader;

typedef struct tagScapeModel
{
	BBox			bounds;				/* The axis-aligned bounding box of this model */
	Uint32			visGroup;			/* Which visibility groups this box is a member of */ 
	StripEntry		*strip;				/* The array of StripHeaders/StripEntries */
	int				nVerts;				/* The number of vertices */
#if defined(MAX_INCLUDED) || defined(WADDER_INCLUDED)
	void			*vertex;			/* The array of vertices */
#else
	StripPos		*vertex;			/* The array of vertices */
#endif
} ScapeModel;

/*
 * Loads a stripmodel
 * Memory allocated from the given allocation routines
 */
ScapeModel *rLoadScapeModel (ScapeModel *model, Stream *s, Allocator *alloc, void *ptr);

/*
 * Draw the whole scape
 */
void rDrawScape (struct _MAP *m, Point3 *cameraPosition);

#endif
