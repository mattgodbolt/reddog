/*
 * Material and texture handling
 */

#include "..\RedDog.h"
#include "..\Loading.h"
#include "Internal.h"
#include <sg_cache.h>

/* The texture palette */
TexPaletteEntry texPalette[MAX_TEXTURES];
int AnimatedNum[MAX_TEXTURES];
int nTextures, nAnimatedTextures;
static int texHWM = 0;		/* The texture high-water mark */
static int texAnimHWM = 0;	/* Globally animated texture HWM */
static Bool fireUsed = FALSE;	/* Is the fire texture animation being used? */
static int nBytesSaved = 0;
volatile nTexturesToDMA = 0;

#define USE_DMA 0

/*
 * Initialise the texture subsystem
 */
void texInit (void)
{
	extern volatile int DisableTexDMAs;
	int texNum;
	KMSTATUS result;
	dAssert (texHWM <= nTextures, "Resetting texture number invalid");

#if defined(_DEBUG) && defined(TEXTURE_CHECK)
	{
		int i;
		dPrintf ("Unused textures: ");
		for (i = 0; i < texHWM; ++i) {
			if (texPalette[i].useCount == 0) {
				dPrintf ("%d", texPalette[i].GBIX);
			}
		}
	}
#endif

	// Prevent any more DMAs from happening
	DisableTexDMAs = TRUE;
	nTexturesToDMA = 0;
	nBytesSaved = 0;
	for (texNum = texHWM; texNum < nTextures; ++texNum) {
		if (!texPalette[texNum].animation) {
			result = kmFreeTexture (&texPalette[texNum].desc);
			dAssert (result == KMSTATUS_SUCCESS, "Texture free failed");
		} else {
			Uint32 frame;
			if (texPalette[texNum].animation->anim.type == TEXANIM_OFFSET || 
				texPalette[texNum].animation->anim.type == TEXANIM_GLOBAL) {
				// Free the only copy
				result = kmFreeTexture (&texPalette[texNum].desc);
				dAssert (result == KMSTATUS_SUCCESS, "Texture free failed");
			} else {
				for (frame = 0; frame < texPalette[texNum].animation->anim.nFrames; ++frame) {
					result = kmFreeTexture (&texPalette[texNum].animation->descArray[frame]);
					dAssert (result == KMSTATUS_SUCCESS, "Texture free failed");
				}
			}
			/* do nothing else - the mission heap must be reset to collect the animation data */
		}
	}
	nTextures = texHWM;
	nAnimatedTextures = texAnimHWM;
	DisableTexDMAs = FALSE;
}

/*
 * Totally reset the textures
 */
void texReset (void)
{
	texHWM = texAnimHWM = 0;
	fireUsed = FALSE;
	texInit();
}

/* Set the highwater mark */
void texSetHWM(void)
{
	texHWM = nTextures;
	texAnimHWM = nAnimatedTextures;
}

static char *DMABuffer = NULL;
static int DMASize = 0;

void texSetDMABuffer (char *buffer, int size)
{
	int b = (int)buffer;
	if (b) {
		b = (b+31) &~31;
		size = size - (b - (int)buffer);
	}
	DMABuffer = (char *)b;
	DMASize = size;
}

Uint32 texLoadSingleTexture (Stream *s)
{
	struct {
		Uint32 tag, tagSize;
	} tag;
	struct {
		Uint32 dataType;
		Uint16 x, y;
	} texHeader;
	Uint32 frame;
	int nBytes;
	char *data, *datBase;
	KMSTATUS ok;
	SerialisedTexAnim anim;
	Uint32 GBIX;
	int foundPVRT;
	
	dAssert (nTextures < MAX_TEXTURES, "Out of palette space!");
	
#ifdef _DEBUG
	texPalette[nTextures].useCount = 0;
#endif

	/* Read in the header information until we get the PVR */
	anim.type = 0;
	GBIX = 0;
	foundPVRT = 0;
	do {
		sRead (&tag, sizeof (tag), s);
		switch (tag.tag) {
		case 0x4d494e41:	/* ANIM */
			dAssert (tag.tagSize == sizeof (anim), "Corrupt texture");
			sRead (&anim, sizeof (anim), s);
			break;
		case 0x58494247:	/* GBIX */
			dAssert (tag.tagSize == 4 ||
				tag.tagSize == 8, "Corrupt texture");
			sRead (&GBIX, sizeof (GBIX), s);
			if (tag.tagSize == 8) {
				Uint32 foo;
				sRead (&foo, sizeof (foo), s);				
			}
			if (anim.type == TEXANIM_OFFSET)
				goto BlankTexture;
			break;
		case 0x54525650:	/* PVRT */
			foundPVRT++;
			break;
		default:
			dAssert (FALSE, "Corrupt texture");
			break;
		}
		
	} while (foundPVRT==0);
	
	/* Work out how much scratch space we need, and align it for DMA */
	nBytes = tag.tagSize - 8;
	if (DMABuffer) {
		dAssert (DMASize > nBytes, "Out of DMA space!");
		data = DMABuffer;
	} else {
		datBase = ScratchAlloc (nBytes + 32);
		data = (char *)(((int)datBase + 31) & ~31);
	}
	
	/* Read in the texture header */
	sRead (&texHeader, 8, s);
BlankTexture:
	switch (anim.type) {
	case 0: /* A non-animated texture */
		/* Read the texture itself */
		sRead (data, nBytes, s);
		
		/* Upload the texture into VRAM */
		ok = kmCreateTextureSurface (&texPalette[nTextures].desc, texHeader.x, texHeader.y, texHeader.dataType);
		dAssert (ok == KMSTATUS_SUCCESS, "Unable to create texture surface (out of VRAM?)");
		syCacheOCWB (data, nBytes);
		ok = kmLoadTexture (&texPalette[nTextures].desc, (KMDWORD *)data, FALSE, FALSE);
		dAssert (ok == KMSTATUS_SUCCESS, "Unable to upload texture");
		if (!DMABuffer)
			ScratchRewind (datBase);
		
		/* Note the GBIX and clear the animation field */
		texPalette[nTextures].GBIX = GBIX;
		texPalette[nTextures].animation = NULL;
		
		break;
	case TEXANIM_GLOBAL:
		{
			int nBytesAligned = (nBytes+31) &~31;
			char *dataWrite;
			// Note as a global animation needing processing
			AnimatedNum[nAnimatedTextures++] = nTextures;

			texPalette[nTextures].animation = (TexAnim *)MissionAlloc (sizeof (TexAnim));
			
			texPalette[nTextures].animation->anim = anim;
			texPalette[nTextures].animation->curFrame = 0;
			texPalette[nTextures].animation->curCount = anim.speed;
			texPalette[nTextures].animation->descArray = NULL;
			texPalette[nTextures].animation->nBytesPerTex = nBytesAligned;// 32 byte align
			texPalette[nTextures].animation->textureArray = MissionAlloc (nBytesAligned * anim.nFrames + 32);
			texPalette[nTextures].animation->textureArray = (void *)((((int)texPalette[nTextures].animation->textureArray) + 31) & ~31);

			// Woo - we've saved some VRAM
			nBytesSaved += nBytes * (anim.nFrames - 1);

			dataWrite = texPalette[nTextures].animation->textureArray;
			for (frame = 0; frame < anim.nFrames; ++frame) {
				sRead (dataWrite, nBytes, s);
				dataWrite += nBytesAligned;
			}

			/* Upload the first texture into VRAM */
			texPalette[nTextures].type = texHeader.dataType;
			ok = kmCreateTextureSurface (&texPalette[nTextures].desc, texHeader.x, texHeader.y, texHeader.dataType);
			dAssert (ok == KMSTATUS_SUCCESS, "Unable to create texture surface (out of VRAM?)");
			syCacheOCWB (texPalette[nTextures].animation->textureArray, nBytesAligned);
			ok = kmLoadTexture (&texPalette[nTextures].desc, (KMDWORD *)texPalette[nTextures].animation->textureArray, FALSE, FALSE);
			dAssert (ok == KMSTATUS_SUCCESS, "Unable to upload texture");

			if (!DMABuffer)
				ScratchRewind (datBase);
			
			texPalette[nTextures].GBIX = GBIX;
		}			
		break;

	case TEXANIM_STRAT: /* An animated texture */
		texPalette[nTextures].animation = (TexAnim *)MissionAlloc (sizeof (TexAnim));
		
		texPalette[nTextures].animation->anim = anim;
		texPalette[nTextures].animation->curFrame = 0;
		texPalette[nTextures].animation->curCount = anim.speed;
		texPalette[nTextures].animation->descArray = (KMSURFACEDESC *)MissionAlloc (sizeof (KMSURFACEDESC) * anim.nFrames);
		
		for (frame = 0; frame < anim.nFrames; ++frame) {
			KMSURFACEDESC *desc = &texPalette[nTextures].animation->descArray[frame];
			
			sRead (data, nBytes, s);
			syCacheOCWB (data, nBytes);
			
			ok = kmCreateTextureSurface (desc, texHeader.x, texHeader.y, texHeader.dataType);
			dAssert (ok == KMSTATUS_SUCCESS, "Unable to create texture surface (out of VRAM?)");
			ok = kmLoadTexture (desc, (KMDWORD *)data, FALSE, FALSE);
			dAssert (ok == KMSTATUS_SUCCESS, "Unable to upload texture");
		}
		
		if (!DMABuffer)
			ScratchRewind (datBase);
		
		texPalette[nTextures].GBIX = GBIX;
		texPalette[nTextures].desc = texPalette[nTextures].animation->descArray[0];
		break;
		
	case TEXANIM_OFFSET: /* Offset animation */
		AnimatedNum[nAnimatedTextures++] = nTextures;
		texPalette[nTextures].animation = (TexAnim *)MissionAlloc (sizeof (TexAnim));
		texPalette[nTextures].animation->anim = anim;

		texPalette[nTextures].GBIX = GBIX;
		break;
	}
	nTextures++;
	return texPalette[nTextures-1].GBIX;
}

/* Reload the texture palette from a stream */
void texLoad (Stream *s)
{
	int nToRead, tex, newTex;
	Uint32 frame;
	
	/* See how many of these badgers we have to put in */
	sRead (&nToRead, 4, s);
	dLog ("Texture palette contains %d textures\n", nToRead);
	sSkip (s);

	newTex = nTextures;
	for (tex = 0; tex < nToRead; ++tex) {
		texLoadSingleTexture (s);
		LoadPoll();
		/* Move on to next texture */
		sSkip (s);
	}
	// Now to resolve all the offset animations
	for (tex = newTex; tex < nTextures; ++tex) {
		if (texPalette[tex].animation &&
			texPalette[tex].animation->anim.type == TEXANIM_OFFSET) {
			// Find the root animation
			int texAnim, GBIX;
			// GBIX is the base GBIX
			GBIX = texPalette[tex].GBIX;
			for (texAnim = 0; texAnim < nTextures; ++texAnim) {
				if (texPalette[texAnim].GBIX == GBIX &&
					texPalette[texAnim].animation &&
					texPalette[texAnim].animation->anim.type == TEXANIM_GLOBAL)
					break;
			}
			dAssert (texAnim != nTextures, "Unable to find offset texture anim");
			if (texAnim == nTextures) {
				texPalette[tex].desc = texPalette[0].desc;
			} else {
				KMSTATUS ok;
				frame = texPalette[tex].animation->anim.speed;
				if (frame > texPalette[texAnim].animation->anim.nFrames)
					frame = texPalette[texAnim].animation->anim.nFrames;
				texPalette[tex].GBIX += texPalette[tex].animation->anim.speed;
				texPalette[tex].animation->anim = texPalette[texAnim].animation->anim;
				texPalette[tex].animation->anim.type = TEXANIM_OFFSET; 
				texPalette[tex].animation->curFrame = frame;
				texPalette[tex].animation->curCount = texPalette[texAnim].animation->anim.speed;
				texPalette[tex].animation->textureArray = texPalette[texAnim].animation->textureArray;
				texPalette[tex].animation->nBytesPerTex = texPalette[texAnim].animation->nBytesPerTex;

				/* Create the first texture in VRAM */
				ok = kmCreateTextureSurface (&texPalette[tex].desc, texPalette[texAnim].desc.u0.Width, 
					texPalette[texAnim].desc.u1.Height, 
					texPalette[texAnim].type);
				dAssert (ok == KMSTATUS_SUCCESS, "Unable to create texture surface (out of VRAM?)");
		
				// Upload the correct texture
				ok = kmLoadTexture (&texPalette[tex].desc, 
					(KMDWORD *)((int)texPalette[tex].animation->textureArray + texPalette[tex].animation->nBytesPerTex * frame),
					FALSE, FALSE);
				dAssert (ok == KMSTATUS_SUCCESS, "Unable to upload texture");
			}
		}

	}
	// Wait for the DMA to finish
	while (kmQueryFinishLastTextureDMA() != KMSTATUS_SUCCESS)
		;
}

/*
 * Find a texture in the palette
 */
int rLastFoundAnimated = 0;
KMSURFACEDESC *texFind (Uint32 GBIX)
{
	int i;

	if (GBIX == 0x13ef13e)
		fireUsed = TRUE;

	for (i = 0; i < nTextures; ++i) {
		if (texPalette[i].GBIX == GBIX) {			
			if (texPalette[i].animation)
				rLastFoundAnimated = 1;
			else
				rLastFoundAnimated = (GBIX == 0x013ef13e);
#ifdef _DEBUG
			texPalette[i].useCount++;
#endif
			return &texPalette[i].desc;
		}
	}

	
	dAssert (FALSE, "Unable to find texture in palette");
	return &texPalette[0].desc;
}

/*
 * Find a texture by material
 */
Uint32 texFindFromDesc (KMSURFACEDESC *desc)
{	int i;
	for (i = 0; i < nTextures; ++i) {
		if (&texPalette[i].desc == desc) {
			if (texPalette[i].animation)
				rLastFoundAnimated = 1;
			else
				rLastFoundAnimated = (texPalette[i].GBIX = 0x013ef13e);
#ifdef _DEBUG
			texPalette[i].useCount++;
#endif
			return texPalette[i].GBIX;
		}
	}
	dAssert (FALSE, "Unable to find texture in palette");
	return texPalette[0].GBIX;
}

// The DMA list of global animations
#define MAX_TEXTURES_TO_DMA		32
static struct {
	KMDWORD desc;
	KMDWORD *data;
	int size;
} textureToDMA[MAX_TEXTURES_TO_DMA];

/*
 * DoGlobalTexAnims - updates all global texture animations
 */
void DoGlobalTexAnims (void)
{
	int i, texture;
	extern volatile bool TexturesReadyForDMA;
	
	nTexturesToDMA = 0;

	for (i = 0; i < nAnimatedTextures; ++i) {
		TexAnim *anim;

		texture = AnimatedNum[i];
		anim = texPalette[texture].animation;
		dAssert (texPalette[texture].animation && 
			texPalette[texture].animation->anim.type == TEXANIM_GLOBAL ||
			texPalette[texture].animation->anim.type == TEXANIM_OFFSET,
			"Something nasty has happened in the global texanimation system");
		if (--anim->curCount <= 0) {
			KMSTATUS ok;
			anim->curCount = anim->anim.speed;
			anim->curFrame++;
			while (anim->curFrame >= anim->anim.nFrames)
				anim->curFrame -= anim->anim.nFrames;
#if USE_DMA
			textureToDMA[nTexturesToDMA].desc = (KMDWORD)&texPalette[texture].desc;
			textureToDMA[nTexturesToDMA].data = (KMDWORD *)((int)texPalette[texture].animation->textureArray + texPalette[texture].animation->nBytesPerTex * anim->curFrame);
#else
			textureToDMA[nTexturesToDMA].desc = (KMDWORD)((Uint32)texPalette[texture].desc.pSurface | 0xa4000000);
			textureToDMA[nTexturesToDMA].data = (KMDWORD *)((int)texPalette[texture].animation->textureArray + texPalette[texture].animation->nBytesPerTex * anim->curFrame);
			textureToDMA[nTexturesToDMA].size = texPalette[texture].desc.uSize.dwTextureSize;
#endif
			nTexturesToDMA++;
		}
	}

	TexturesReadyForDMA = true;
}

void DoGlobalTexAnimDMA(void)
{
#if USE_DMA
	int i;
	for (i = 0; i < nTexturesToDMA; ++i) {
		while (kmQueryFinishLastTextureDMA() != KMSTATUS_SUCCESS) {
			/* Do something rather cunning here... */
		}
		kmLoadTexture ((void*)textureToDMA[i].desc, textureToDMA[i].data, FALSE, FALSE);
	}
	nTexturesToDMA = 0;
#else
	int i;
	KMDWORD a, b;

	a = *((volatile PKMDWORD)0xFF000038);
	b = *((volatile PKMDWORD)0xFF00003C);
	for (i = 0; i < nTexturesToDMA; ++i) {
		prefetch (textureToDMA[i].data);
		*((volatile PKMDWORD)0xFF000038) = (KMDWORD)textureToDMA[i].desc>>24;
		*((volatile PKMDWORD)0xFF00003C) = (KMDWORD)textureToDMA[i].desc>>24;
		AlignedCopy8SQWithPref ((PKMDWORD)(((KMDWORD)textureToDMA[i].desc & 0x03FFFFFF) | 0xE0000000), textureToDMA[i].data, textureToDMA[i].size);
	}
	nTexturesToDMA = 0;
	*((volatile PKMDWORD)0xFF000038) = a;
	*((volatile PKMDWORD)0xFF00003C) = b;
#endif
}

/*
 * Get the texture animations of an object
 * Returns the number of tex anims
 * Writes Animations into anim, unless anim is NULL
 */
int GetTexAnims (const Object *obj, Animation *anim)
{
	int nAnims = 0, child;
	if (obj->model) {
		int material, nMats = obj->model->nMats;
		for (material = 0; material < nMats; ++material) {
			if (obj->model->material[material].surf.surfaceDesc) {
				/*
				 * This *must* be a pointer to the surfaceDesc part of our TexCache
				 */
				TexPaletteEntry *ent = (TexPaletteEntry *)
					((char *)obj->model->material[material].surf.surfaceDesc - offsetof(TexPaletteEntry, desc));

				if (ent->animation && ent->animation->anim.type == TEXANIM_STRAT) {
					if (anim) {
						anim[nAnims].type		= ANIM_TEXTURE;
						anim[nAnims].typeData	= (void *)ent;
						anim[nAnims].firstFrame	= 0;
						anim[nAnims].lastFrame	= ent->animation->anim.nFrames;
						anim[nAnims].frameDelay	= 1;
						strcpy (anim[nAnims].name, ent->animation->anim.stratAnim.name);
					}
					nAnims++;
				}
			}
		}
	}
	if (anim) {
		for (child = 0; child < obj->no_child; ++child)
			nAnims += GetTexAnims (&obj->child[child], &anim[nAnims]);
	} else {
		for (child = 0; child < obj->no_child; ++child)
			nAnims += GetTexAnims (&obj->child[child], NULL);
	}
	return nAnims;
}

