/*
 * MatLib.cpp
 * The material library
 */

#include "StdAfx.h"
#include "util.h"
#include "MatLib.h"
#include "Material.h"

// fix up a filename
static void fixUp (char *fn)
{
	char *str;
	do {
		str = strchr (fn, '/');
		if (!str)
			break;

		memmove (str, str+1, strlen(str+1)+1);
	} while (1);
}

 /*
  * Class RDMaterial Methods
  */
// Fairly bog-standard default constructor
RDMaterial::RDMaterial()
{
	materialName[0] = '\0';
	textureFilename[0] = '\0';
	filterFilename[0] = '\0';
	diffuse = Colour (1.f, 1.f, 1.f);
	specular = Colour (0.8f, 0.8f, 0.8f);
	specAmount = 20;
	specExponent = 3;
	opacity = 1.f;
	collisionable = true;
	additive = subtractive = false;
	filter = BILINEAR;
	flags = 0;
	matType = ROCK | ALLCOLLIDE;
	filterAmt = 0.3f;
	mipmapAdjust = 1.f;
	visMask = 1;
}
// Create a material based on a Mtl
RDMaterial::RDMaterial (Mtl &m) : uvTransform (TRUE)
{
	matType = ROCK | ALLCOLLIDE;
	mipmapAdjust = 1.f;
	visMask = 1;
	// Check the material type
	Class_ID foo = m.ClassID();
	// First and foremost, is it an MGMaterial??
	if (foo == MGMATERIAL_CLASS_ID) {
		MGMaterial *mg = (MGMaterial *)&m;
		*this = mg->getMat();
		

		// Read the diffuse texture map, if any
			Texmap *t= mg->GetSubTexmap(0);
			if (t != NULL) {
				// Check for the texture being a bitmap image
				if (t->ClassID() == Class_ID(BMTEX_CLASS_ID, 0)) {
					
					BitmapTex *bmt = (BitmapTex *) t;
					StdUVGen *uvGen = bmt->GetUVGen();
					uvGen->GetUVTransform (uvTransform);
										
					safeStrcpy (textureFilename, bmt->GetMapName(), MAX_FNAME_SIZE);
					fixUp (textureFilename);
					flags |= RDMF_TEXMAPPED;
					if (!(uvGen->GetTextureTiling() & (U_WRAP)))
						flags |= RDMF_CLAMP_X;
					if (!(uvGen->GetTextureTiling() & (V_WRAP)))
						flags |= RDMF_CLAMP_Y;
					
					// Check the alpha source
					switch (bmt->GetAlphaSource()) {
					case ALPHA_FILE:	// Uses image alpha as transparency
						break;
					case ALPHA_RGB:		// Uses image alpha as decal
						flags |= RDMF_DECAL;
						break;
					case ALPHA_NONE:
						flags |= RDMF_NOALPHA;
						break;
					}
					
				}
			}
			// Read the filter map, if any
			t = mg->GetSubTexmap(1);
			if (t != NULL && filterAmt > 0.f) {
				// Check for the texture being a bitmap image
				if (t->ClassID() == Class_ID(BMTEX_CLASS_ID, 0)) {
					BitmapTex *bmt = (BitmapTex *) t;
					
					safeStrcpy (filterFilename, bmt->GetMapName(), MAX_FNAME_SIZE);
					fixUp (filterFilename);
					flags |= RDMF_FILTERED;
				}
			}
	} else {
		diffuse		= m.GetDiffuse();
		specular	= m.GetSpecular();
		specAmount	= m.GetShinStr();
		specExponent= m.GetShininess();
		opacity		= 1.0f;
		filterAmt	= 0.0f;
		collisionable = true;
		StdMat *stdMat = NULL;
		
		safeStrcpy (materialName, m.GetName(), MAX_NAME_SIZE);
		
		// Clear texturing state
		textureFilename[0] = filterFilename[0] = '\0';
		flags = 0;
		filter = BILINEAR;
		additive = subtractive = false;
		
		if (foo == Class_ID (0x459a16ee, 0x27977f9c)) {
			TSTR foo = m.GetSubMtlSlotName(0);
			Mtl *m2 = m.GetSubMtl(0);
			if (m2->ClassID() ==  Class_ID (DMTL_CLASS_ID, 0)) {
				stdMat = (StdMat *)m2;
				// xxx might want to reinit diffuse et al here....
			}
		} else if (m.ClassID() == Class_ID (DMTL_CLASS_ID, 0)) {
			stdMat = (StdMat *)&m;
		}
		
		if (stdMat) {
			// Get the opacity
			opacity = stdMat->GetOpacity(0);
			
			// Non-collisionable?
			if (stdMat->GetIOR(0) < 0.1f)
				collisionable = false;
			
			// See if we're two sided
			if (stdMat->GetTwoSided())
				flags |= RDMF_TWOSIDED;
			// See if we want supersampling
			if (stdMat->Requirements(-1) & MTLREQ_SUPERSAMPLE)
				flags |= RDMF_ANISOTROPIC;
			// See if it's flat shaded
			if (stdMat->GetShading() == SHADE_CONST)
				flags |= RDMF_FLAT_SHADE;
			
			// See if the map has a reflection map
			if (stdMat->MapEnabled(ID_RL))
				flags |= RDMF_ENVIRONMENT;
			
			// No lighting if we have any self illumination
			if (stdMat->GetSelfIllum(0))
				flags |= RDMF_NOLIGHTING;
			
			// See if this is an additive texture
			if (stdMat->GetTransparencyType() == TRANSP_ADDITIVE)
				additive = true;
			
			// Read the diffuse texture map, if any
			Texmap *t= stdMat->GetSubTexmap(ID_DI);
			if (t != NULL && stdMat->MapEnabled (ID_DI)) {
				
				// Check for the texture being a bitmap image
				if (t->ClassID() == Class_ID(BMTEX_CLASS_ID, 0)) {
					
					BitmapTex *bmt = (BitmapTex *) t;
					StdUVGen *uvGen = bmt->GetUVGen();
					uvGen->GetUVTransform (uvTransform);
					
					safeStrcpy (textureFilename, bmt->GetMapName(), MAX_FNAME_SIZE);
					fixUp (textureFilename);
					flags |= RDMF_TEXMAPPED;
					if (!(uvGen->GetTextureTiling() & (U_WRAP)))
						flags |= RDMF_CLAMP_X;
					if (!(uvGen->GetTextureTiling() & (V_WRAP)))
						flags |= RDMF_CLAMP_Y;
					
					// Check the alpha source
					switch (bmt->GetAlphaSource()) {
					case ALPHA_FILE:	// Uses image alpha as transparency
						break;
					case ALPHA_RGB:		// Uses image alpha as decal
						flags |= RDMF_DECAL;
						break;
					case ALPHA_NONE:
						flags |= RDMF_NOALPHA;
						break;
					}
					
					// Check the filter type
					switch (bmt->GetFilterType()) {
					default:
						break;
					case FILTER_NADA:
						filter = POINT;
						break;
					case FILTER_SAT:
						filter = TRILINEAR;
						break;
					}
					
				}
			}
			// Read the filter map, if any
			t = stdMat->GetSubTexmap(ID_FI);
			filterAmt = stdMat->GetTexmapAmt (ID_FI, TimeValue(0));
			if (t != NULL && stdMat->MapEnabled(ID_FI)) {
				// Check for the texture being a bitmap image
				if (t->ClassID() == Class_ID(BMTEX_CLASS_ID, 0)) {
					BitmapTex *bmt = (BitmapTex *) t;
					
					safeStrcpy (filterFilename, bmt->GetMapName(), MAX_FNAME_SIZE);
					fixUp (filterFilename);
					flags |= RDMF_FILTERED;
				}
			}
	}
	}
}
// Are two RDMaterials the same?
int RDMaterial::operator == (const RDMaterial &other) const
{
	// I deliberately ignore the material names

	// Check the flags first
	if (flags != other.flags || opacity != other.opacity || specAmount != other.specAmount 
		|| specExponent != other.specExponent || filterAmt != other.filterAmt || matType != other.matType ||
		additive != other.additive || collisionable != other.collisionable || filter != other.filter ||
		subtractive != other.subtractive ||
		mipmapAdjust != other.mipmapAdjust ||
		visMask != other.visMask ||
		palette != other.palette)
		return 0;

	// Check the texture filenames next
	if (strcmp (textureFilename, other.textureFilename))
		return 0;

	// Check the filter filenames next
	if (strcmp (filterFilename, other.filterFilename))
		return 0;

	// Check the matrices
	Point3 a, b;
	a.x = b.x = 10.f;
	a.y = b.y = 23.f;
	a.z = b.z = -12.f;
	a = a * uvTransform;
	b = b * other.uvTransform;
	if (Length(a - b) > 0.1f)
		return 0;
	
	// Check the colours
	if (diffuse  != other.diffuse ||
		specular != other.specular)
		return 0;
	// Must be the same then
	return 1;
}

RedDog::Material RDMaterial::getMaterial()
{
	RedDog::Material mat;
	mat.diffuse.argb.a		= (char) (opacity * 255.0);
	mat.diffuse.argb.r		= (char) (diffuse.r * 255.0);
	mat.diffuse.argb.g		= (char) (diffuse.g * 255.0);
	mat.diffuse.argb.b		= (char) (diffuse.b * 255.0);
	mat.specular.argb.a		= (char) (opacity * 255.0);
	mat.specular.argb.r		= (char) (specular.r * 255.0);
	mat.specular.argb.g		= (char) (specular.g * 255.0);
	mat.specular.argb.b		= (char) (specular.b * 255.0);
//	mat.specAmount			= specAmount;
	mat.hideID				= visMask;
	mat.exponent			= specExponent;
	mat.flags				= 0;
	mat.pasted.flags		= 0;
	mat.pasted.surf.GBIX	= 0;

	mat.pasted.transparency	= filterAmt;

	if (flags & RDMF_TEXMAPPED) {
		switch (filter) {
		case POINT:
			mat.flags |= MF_TEXTURE_NO_FILTER;
			break;
		case BILINEAR:
			mat.flags |= MF_TEXTURE_BILINEAR;
			break;
		case TRILINEAR:
			// Trilinear is a two-pass operation - we used the pasted slot 
			// to do the second pass
			mat.flags |= MF_TEXTURE_TRILINEAR;
			flags |= RDMF_FILTERED;
			mat.pasted.transparency = 1.f;
			mat.pasted.flags |= MF_TRANSPARENCY_ADDITIVE;
			strcpy (filterFilename, textureFilename);
			break;
		}		
	} else {
		mat.flags |= MF_NO_TEXTURE;
	}

	if (opacity <= 0.99f)
		mat.flags |= MF_ALPHA;
	if (specAmount > 0.1f)
		mat.flags |= MF_SPECULAR;
	if (flags & RDMF_ENVIRONMENT)
		mat.flags |= MF_ENVIRONMENT;
	if (flags & RDMF_BUMPMAP)
		mat.flags |= MF_BUMP_MAPPED;
	if (flags & RDMF_ANISOTROPIC)
		mat.flags |= MF_ANISOTROPIC, mat.pasted.flags |= MF_ANISOTROPIC;
	if (flags & RDMF_TWOSIDED)
		mat.flags |= MF_TWO_SIDED, mat.pasted.flags |= MF_TWO_SIDED;
	if (flags & RDMF_PUNCH) {
		mat.flags |= MF_TRANSPARENCY_PUNCHTHRU;
		mat.diffuse.argb.a = 0xff;
	}
	if (flags & RDMF_CLAMP_X)
		mat.flags |= MF_CLAMP_U;
	if (flags & RDMF_CLAMP_Y)
		mat.flags |= MF_CLAMP_V;
	if (flags & RDMF_NOLIGHTING)
		mat.flags |= MF_NO_LIGHTING, mat.pasted.flags |= MF_NO_LIGHTING;
	if (flags & RDMF_FLAT_SHADE)
		mat.flags |= MF_FLAT_SHADED;
	if (flags & RDMF_NOALPHA)
		mat.flags |= MF_IGNORE_TEXTURE_ALPHA;
	if (flags & RDMF_DECAL)
		mat.flags |= MF_DECAL;

	if (flags & RDMF_FILTERED) {
		mat.flags |= MF_TEXTURE_PASTE;
		switch (filter) {
		case POINT:
			mat.pasted.flags |= MF_TEXTURE_NO_FILTER;
			break;
		case BILINEAR:
		default:
			mat.pasted.flags |= MF_TEXTURE_BILINEAR;
			break;
		case TRILINEAR:
			// This is the second pass
			mat.pasted.flags |= MF_TEXTURE_TRILINEAR | MF_SECOND_PASS;
			break;
		}		
		if (additive)
			mat.pasted.flags |= MF_TRANSPARENCY_ADDITIVE;
		if (subtractive)
			mat.pasted.flags |= MF_TRANSPARENCY_SUBTRACTIVE;
		if (flags & RDMF_FROMSCREEN) {
			mat.pasted.flags |= MF_TRANSPARENCY_FROM_SCREEN;
			mat.flags &= ~MF_ALPHA;
		}
	} else {
		if (additive)
			mat.flags |= MF_TRANSPARENCY_ADDITIVE;
		if (subtractive)
			mat.flags |= MF_TRANSPARENCY_SUBTRACTIVE;
		if (flags & RDMF_FROMSCREEN)
			mat.flags |= MF_TRANSPARENCY_FROM_SCREEN;
	}

	int i;
	i = (int) (mipmapAdjust * 4.f);
	if (i < 1) i = 1;
	if (1 > 0xf) i = 0xf;
	mat.flags |= MF_MIPMAP_SET(i);
	mat.pasted.flags |= MF_MIPMAP_SET(i);

	i = RANGE (0, palette, 3);
	mat.flags |= MF_PALETTE_SET(i);
	mat.pasted.flags |= MF_PALETTE_SET(i);

	return mat;
}
