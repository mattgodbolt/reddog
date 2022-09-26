#ifndef _MATLIB_H
#define _MATLIB_H

#include "util.h"
#include "RedDog.h"

class RDMaterial
{
private:
	char		textureFilename[MAX_FNAME_SIZE];
	char		filterFilename[MAX_FNAME_SIZE];
	char		materialName[MAX_NAME_SIZE];
	Colour		diffuse, specular;
	int			palette;
	float		specAmount, specExponent, filterAmt;
	bool		additive, collisionable, subtractive;
	enum FilterType
	{ POINT, BILINEAR, TRILINEAR }
				filter;

	Uint16	matType;
	#define	MatType matType
	float		opacity;
	float		mipmapAdjust;
	int			flags;
#define RDMF_TEXMAPPED			0x0001
#define RDMF_FILTERED			0x0002
#define RDMF_ENVIRONMENT		0x0004
#define RDMF_ANISOTROPIC		0x0008
#define RDMF_NOLIGHTING			0x0010
#define RDMF_TWOSIDED			0x0020
#define RDMF_FLAT_SHADE			0x0040
#define RDMF_DECAL				0x0080
#define RDMF_NOALPHA			0x0100
#define RDMF_CLAMP_X			0x0200
#define RDMF_PUNCH				0x0400
#define RDMF_BUMPMAP			0x0800
#define RDMF_FROMSCREEN			0x1000
#define RDMF_CLAMP_Y			0x2000
	int			visMask;
	friend class MGMaterial;
public:
	/*
	 * Constructor(s)
	 */
	RDMaterial ();
	RDMaterial (Mtl &m);
	virtual ~RDMaterial() {};
	// Compare two RDMaterials
	int operator == (const RDMaterial &other) const;
	// Get the filename
	inline const char *getName() const { return (const char *) &materialName[0]; }
	// Get the texture
	inline const char *getTexName() const { return (const char *) &textureFilename[0]; }
	// Get the filter
	inline const char *getFilterName() const { return (const char *) &filterFilename[0]; }

	// Is this material a noncollisionable material
	inline bool isnonCollisionable() const { return !collisionable; }
	inline bool isInvisible() const { return opacity < 0.01f; }
	inline Uint32 getOpacity() const { if (flags & RDMF_PUNCH) return 0xff000000; else return (((int)(opacity * 0xff)) << 24); }
	inline bool isTwoSided() const { return (flags & RDMF_TWOSIDED)?true:false; }

	inline Uint32 GetVisMask() const { return visMask; }
	inline void SetVisMask(Uint32 foo) { visMask = foo; }

	RedDog::Uint16 getCollFlags()
	{
		/*QUICK CHECK TO ENSURE NO POLY COLL*/
	  	if (!(matType & 480))
			matType = matType + 480;

		return matType;
	}

	void setCollFlags(Uint16 val)
	{
		matType = val;

	}

	inline unsigned int GetModelFlags() const {
		unsigned int ret = 0;
		if (flags & RDMF_BUMPMAP)
			ret |= MODELFLAG_BUMPMAP;
		if (flags & RDMF_ENVIRONMENT)
			ret |= MODELFLAG_ENVIRONMENTMAP;
		return ret;
	}

	// Is it suitable for stripping?
	inline bool canBeStripped() {
		if (!(flags & RDMF_TEXMAPPED) || (flags & RDMF_ENVIRONMENT))
			return false;
		else
			return true;
	}

	RedDog::Material getMaterial();
	Matrix3 uvTransform;
};

#endif
