/*
 * Model-type renderer
 */

#define SIDES 3

#ifdef TEXTURED

#define VERT(pcw) \
	vtx = &v[*strip++]; \
	vPull = &vtx->x; \
	((PKMVERTEX5)(pkmCurrentPtr))->ParamControlWord	= pcw;					\
	((PKMVERTEX5)(pkmCurrentPtr))->fX				= *vPull++;				\
	((PKMVERTEX5)(pkmCurrentPtr))->fY				= *vPull++;				\
	((PKMVERTEX5)(pkmCurrentPtr))->u.fZ				= *vPull++;				\
	((PKMVERTEX5)(pkmCurrentPtr))->fU				= *uv++;				\
	((PKMVERTEX5)(pkmCurrentPtr))->fV				= *uv++;				\
	prefetch((void *)pkmCurrentPtr);										\
	rApplyLight ((LightingValue *)&((PKMVERTEX5)(pkmCurrentPtr))->fBaseAlpha, &vtx->lighting.diffuse, &context->colour); \
	rApplyLight ((LightingValue *)&((PKMVERTEX5)(pkmCurrentPtr))->fOffsetAlpha, &vtx->lighting.specular, &context->specColour); \
	prefetch((void *)(pkmCurrentPtr + 8));									\
	pkmCurrentPtr += (sizeof(KMVERTEX5) / sizeof(KMDWORD))

#define _EMIT(pcw,VERT) \
	vtx = &v[strip[VERT]]; \
	vPull = &vtx->x; \
	((PKMVERTEX5)(pkmCurrentPtr))->ParamControlWord	= pcw;					\
	((PKMVERTEX5)(pkmCurrentPtr))->fX				= *vPull++;				\
	((PKMVERTEX5)(pkmCurrentPtr))->fY				= *vPull++;				\
	((PKMVERTEX5)(pkmCurrentPtr))->u.fZ				= *vPull++;				\
	((PKMVERTEX5)(pkmCurrentPtr))->fU				= uv[VERT*2];			\
	((PKMVERTEX5)(pkmCurrentPtr))->fV				= uv[VERT*2+1];			\
	prefetch((void *)pkmCurrentPtr);										\
	rApplyLight ((LightingValue *)&((PKMVERTEX5)(pkmCurrentPtr))->fBaseAlpha, &vtx->lighting.diffuse, &context->colour); \
	rApplyLight ((LightingValue *)&((PKMVERTEX5)(pkmCurrentPtr))->fOffsetAlpha, &vtx->lighting.specular, &context->specColour); \
	prefetch((void *)(pkmCurrentPtr + 8));									\
	pkmCurrentPtr += (sizeof(KMVERTEX5) / sizeof(KMDWORD))

#define EMIT(VERT)			_EMIT(KM_VERTEXPARAM_NORMAL,VERT)
#define EMIT_FINAL(VERT)	_EMIT(KM_VERTEXPARAM_ENDOFSTRIP,VERT)

#define EMIT_EDGE(V0,V1) \
	vtx = &v[strip[V0]]; \
	vtx2 = &v[strip[V1]]; \
	alpha = vtx2->vZ / (vtx2->vZ - vtx->vZ); \
	((PKMVERTEX5)(pkmCurrentPtr))->ParamControlWord	= KM_VERTEXPARAM_NORMAL;\
	X = X_MID + rWnearVal * LERP(vtx2->vX, vtx->vX, alpha); \
	Y = Y_MID + rWnearVal * LERP(vtx2->vY, vtx->vY, alpha); \
	((PKMVERTEX5)(pkmCurrentPtr))->fX				= X;					\
	((PKMVERTEX5)(pkmCurrentPtr))->fY				= Y;					\
	((PKMVERTEX5)(pkmCurrentPtr))->u.fZ				= rWnearVal;			\
	((PKMVERTEX5)(pkmCurrentPtr))->fU				= LERP(uv[V1*2],uv[V0*2],alpha);			\
	((PKMVERTEX5)(pkmCurrentPtr))->fV				= LERP(uv[V1*2+1],uv[V0*2+1],alpha);		\
	prefetch((void *)pkmCurrentPtr);										\
	rApplyLight ((LightingValue *)&((PKMVERTEX5)(pkmCurrentPtr))->fBaseAlpha, &vtx->lighting.diffuse, &context->colour); \
	rApplyLight ((LightingValue *)&((PKMVERTEX5)(pkmCurrentPtr))->fOffsetAlpha, &vtx->lighting.specular, &context->specColour); \
	prefetch((void *)(pkmCurrentPtr + 8));									\
	pkmCurrentPtr += (sizeof(KMVERTEX5) / sizeof(KMDWORD))

#else

#define VERT(pcw) \
	vtx = &v[*strip++]; \
	vPull = &vtx->x; \
	rApplyLight ((LightingValue *)&((PKMVERTEX1)(pkmCurrentPtr))->fBaseAlpha, &vtx->lighting.diffuse, &context->colour); \
	((PKMVERTEX1)(pkmCurrentPtr))->ParamControlWord	= pcw;					\
	((PKMVERTEX1)(pkmCurrentPtr))->fX				= *vPull++;				\
	((PKMVERTEX1)(pkmCurrentPtr))->fY				= *vPull++;				\
	((PKMVERTEX1)(pkmCurrentPtr))->u.fZ				= *vPull++;				\
	prefetch((void *)pkmCurrentPtr);										\
	pkmCurrentPtr += (sizeof(KMVERTEX1) / sizeof(KMDWORD));					

#define _EMIT(pcw,VERT) \
	vtx = &v[strip[VERT]]; \
	vPull = &vtx->x; \
	((PKMVERTEX1)(pkmCurrentPtr))->ParamControlWord	= pcw;					\
	((PKMVERTEX1)(pkmCurrentPtr))->fX				= *vPull++;				\
	((PKMVERTEX1)(pkmCurrentPtr))->fY				= *vPull++;				\
	((PKMVERTEX1)(pkmCurrentPtr))->u.fZ				= *vPull++;				\
	prefetch((void *)pkmCurrentPtr);										\
	rApplyLight ((LightingValue *)&((PKMVERTEX5)(pkmCurrentPtr))->fBaseAlpha, &vtx->lighting.diffuse, &context->colour); \
	prefetch((void *)(pkmCurrentPtr + 8));									\
	pkmCurrentPtr += (sizeof(KMVERTEX1) / sizeof(KMDWORD))

#define EMIT(VERT)			_EMIT(KM_VERTEXPARAM_NORMAL,VERT)
#define EMIT_FINAL(VERT)	_EMIT(KM_VERTEXPARAM_ENDOFSTRIP,VERT)

#define EMIT_EDGE(V0,V1) \
	vtx = &v[strip[V0]]; \
	vtx2 = &v[strip[V1]]; \
	alpha = vtx2->vZ / (vtx2->vZ - vtx->vZ); \
	((PKMVERTEX1)(pkmCurrentPtr))->ParamControlWord	= KM_VERTEXPARAM_NORMAL;\
	X = X_MID + rWnearVal * LERP(vtx2->vX, vtx->vX, alpha); \
	Y = Y_MID + rWnearVal * LERP(vtx2->vY, vtx->vY, alpha); \
	((PKMVERTEX1)(pkmCurrentPtr))->fX				= X;					\
	((PKMVERTEX1)(pkmCurrentPtr))->fY				= Y;					\
	((PKMVERTEX1)(pkmCurrentPtr))->u.fZ				= rWnearVal;			\
	prefetch((void *)pkmCurrentPtr);										\
	rApplyLight ((LightingValue *)&((PKMVERTEX1)(pkmCurrentPtr))->fBaseAlpha, &vtx->lighting.diffuse, &context->colour); \
	prefetch((void *)(pkmCurrentPtr + 8));									\
	pkmCurrentPtr += (sizeof(KMVERTEX1) / sizeof(KMDWORD))

#endif

void REND_NAME (ModelContext *context, Uint32 num)
{
#if OLD_RENDER
	int strips = num;
	register VertRef *strip = context->strip;
#ifdef GENERIC
	Uint32 flags = context->mat->flags;
	extern Uint32 genCalls;
#endif
#ifdef TEXTURED
	float *uv = (float *)context->uvArray;
#endif
	register PrepVtx *v = context->vertArray;
	PrepVtx *vtx;
	register float *vPull;
	int done = 0;
#ifdef CLIPPED
	float alpha, X, Y;
	PrepVtx *vtx2;
#endif
	
#ifdef GENERIC
	genCalls++;
#endif

	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip (&vertBuf);
#ifndef TEXTURED
	context->uvArray += SIDES * num;
#endif

	do {
#ifdef CLIPPED
		Outcode code = (v[strip[0]].outcode | v[strip[1]].outcode | v[strip[2]].outcode);

		if (code & OC_OFF_NEAR) {
			switch (((v[strip[0]].outcode & OC_OFF_NEAR) ? 0:1) |
				((v[strip[1]].outcode & OC_OFF_NEAR) ? 0:2) |
				((v[strip[2]].outcode & OC_OFF_NEAR) ? 0:4)) {
			case 0:
				break;
			case 1:
				EMIT_EDGE(0, 1);
				EMIT_EDGE(2, 0);
				EMIT_FINAL(0);
				break;
			case 2:
				EMIT_EDGE(1, 2);
				EMIT_EDGE(0, 1);
				EMIT_FINAL(1);
				break;
			case 3:
				EMIT_EDGE(1, 2);
				EMIT_EDGE(2, 0);
				EMIT(1);
				EMIT_FINAL(0);
				break;
			case 4:
				EMIT_EDGE(1, 2);
				EMIT_EDGE(0, 2);
				EMIT_FINAL(2);
				break;
			case 5:
				EMIT_EDGE(0, 1);
				EMIT_EDGE(1, 2);
				EMIT(0);
				EMIT_FINAL(2);
				break;
			case 6:
				EMIT_EDGE(2, 0);
				EMIT_EDGE(0, 1);
				EMIT(2);
				EMIT_FINAL(1);
				break;
			case 7:
				EMIT(0);
				EMIT(1);
				EMIT_FINAL(2);
				break;
			}			
# ifdef TEXTURED
			uv += (SIDES*2);
# endif
			strip += SIDES;
			
		} 
		else
#endif /* CLIPPED */
		{
			VERT (KM_VERTEXPARAM_NORMAL);
			VERT (KM_VERTEXPARAM_NORMAL);
			VERT (KM_VERTEXPARAM_ENDOFSTRIP);
			done++;
		}
	} while (--num != 0);
	
	context->strip = strip;
#ifdef TEXTURED
	context->uvArray = (UV *)uv;
#endif
	kmxxReleaseCurrentPtr (&vertBuf);
#endif
}

/* Undefine some macros for convienience */
#undef REND_NAME
#undef VERT
#undef TEXTURED
#undef SIDES
#undef CLIPPED

