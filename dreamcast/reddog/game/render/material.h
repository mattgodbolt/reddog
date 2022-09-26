/*
 * Render/Material.h
 * Handles materials and textures
 */

#ifndef _MATERIAL_H
#define _MATERIAL_H

/* The maximum number of textures loaded at once ever */
#define MAX_TEXTURES					768



#define GET_SURF_TYPE(a) ((a) & 31)
#define SET_SURF_TYPE(a,b) (((a) & ~31) | ((b) & 31))


/* 6th bit - 9th coll type	 */

#define ALLCOLLIDE			(1<<5) + (1<<6) + (1<<7) + (1<<8) + (1<<9)
#define ENEMYCOLLISION		(1<<5)
#define	CAMERACOLLISION		(1<<6)
#define	BULLETCOLLISION		(1<<7)
#define	PLAYERCOLLISION		(1<<8)
#define	SPECIALCOLLISION	(1<<9)


/*CAMERA FIELD IS THE 10th Bit in .. */
#define CAMERA (1<<10)


/* Material flags */
#define MF_TWO_SIDED					(1<<0)			/* Material is two sided */
#define MF_ALPHA						(1<<1)			/* Material may be transparent */
#define MF_NO_TEXTURE					(0<<2)			/* Material has no texture */
#define MF_TEXTURE_NO_FILTER			(1<<2)			/* Material has an unfiltered texture */
#define MF_TEXTURE_BILINEAR				(2<<2)			/* Material has a bilinear filtered texture */
#define MF_TEXTURE_TRILINEAR			(3<<2)			/* Material has a trilinear filtered texture */
#define MF_TEXTURE_FILTER_MASK			(3<<2)			/* Mask to get filter */
#define MF_ENVIRONMENT					(1<<4)			/* Material is environment mapped */
#define MF_ANISOTROPIC					(1<<5)			/* Material is anisotropically filtered (textured only) */
#define MF_NO_LIGHTING					(1<<6)			/* Material should not be lit */
#define MF_FLAT_SHADED					(1<<7)			/* Material should be flat shaded */
#define MF_DECAL						(1<<8)			/* Material's texture alpha is for decals */
#define MF_SPECULAR						(1<<9)			/* Material receives specular highlights */
#define MF_IGNORE_TEXTURE_ALPHA			(1<<10)			/* Material ignores texture's alpha */
#define MF_TEXTURE_PASTE				(1<<11)			/* Material has a 'pasted' texture */
#define MF_TRANSPARENCY_ADDITIVE		(1<<12)			/* Material should be 'added' to the buffer */
#define MF_TEXTURE_ANIMATED				(1<<13)			/* Texture is animated and needs updating */
#define MF_CLAMP_U						(1<<14)			/* Texture should be clamped in U */
#define MF_TRANSPARENCY_PUNCHTHRU		(1<<15)			/* Alpha transparency should be placed in the punch pipe */
#define MF_BUMP_MAPPED					(1<<16)			/* Material is bumpmapped */
#define MF_WHITE						(1<<17)			/* Material had a white diffuse colour and can used optimised
														 * renderers */
#define MF_TRANSPARENCY_SUBTRACTIVE		(1<<18)			/* Material should have subtractive transparency */
#define MF_TRANSPARENCY_FROM_SCREEN		(1<<19)			/* Material should pick up its alpha from the screen */
#define MF_SECOND_PASS					(1<<20)			/* This is the second pass of a two-pass operation:
														 * eg: Trilinear */
#define MF_FRONT_END					(1<<21)			/* A front-end material eg HUD */
#define MF_CLAMP_V						(1<<22)			/* Clamp V only */
#define MF_PALETTE_MASK					(3<<26)			/* Mask to get palette number */
#define MF_PALETTE_SET(a)				((a)<<26)
#define MF_PALETTE_GET(a)				(((Uint32)(a)>>26) & 0x3)
#define MF_MIPMAP_ADJUST_MASK			(0xf<<28)		/* Top 4 bits are the mipmap adjust */
#define MF_MIPMAP_SET(a)				((a)<<28)
#define MF_MIPMAP_GET(a)				((Uint32)(a)>>28)

/* Lowest level processed material */
typedef struct tagMaterialInfo
{
	Uint32					GLOBALPARAMBUFFER;		/*  Grobal (sic) Parameter Buffer	*/
	Uint32					ISPPARAMBUFFER;			/*  ISP Parameter Buffer			*/
	Uint32					TSPPARAMBUFFER;			/*	TSP Parameter Buffer			*/
	Uint32					TexturePARAMBUFFER;		/*	Texture Parameter Buffer		*/
	struct tagRasteriser	*renderer;				/*	The renderer for this material	*/
} MaterialInfo;

/* A material */
typedef struct {
	Colour			diffuse;			/* The diffuse colour */
	Colour			specular;			/* The specular colour */
	float			exponent;			/* The exponent of the specular colour - ie higher number (eg 60) tighter spotlight */
	Uint32			hideID;				/* Bitmask of hidden stuff for landscape hiding */
    Uint32			flags;				/* flags */
	union {
		KMSURFACEDESC	*surfaceDesc;	/* Pointer to the texture surface of the texture for this material */
		Uint32			GBIX;			/* GBIX of the texture */
	} surf;
	MaterialInfo		info;			/* The lowest level material information */
	struct {
		Uint32				flags;			/* The flags for this material */
		/*
		 * Note that only some flags have an effect on a pasted texture :
		 * At this stage the UV values, and lighting values have already been worked out
		 * and so MF_ENVIRONMENT, MG_TWO_SIDED, MF_NO_LIGHTING and MF_TEXTURE_PASTE have no effect
		 */
		float				transparency;	/* How transparent this texture is (opacity values are *'d by this) */
		union {
			KMSURFACEDESC	*surfaceDesc;	/* Pointer to the texture surface of the texture for this material */
			Uint32			GBIX;			/* GBIX of the texture */
		} surf;
		MaterialInfo		info;			/* The lowest level material information */
	} pasted;								/* The pasted texture identification */
} Material;

#endif
