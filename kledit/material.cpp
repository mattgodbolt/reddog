/*
 * MattG's Material
 */

#include "stdafx.h"
#include <stdmat.h>
#include "camcontrol.h"

#define PB_INDEX_COLL			0
#define PB_INDEX_TWOSIDED		1
#define PB_INDEX_TRANS			2
#define PB_INDEX_ADDITIVE		3
#define PB_INDEX_ANISOTROPIC	4
#define PB_INDEX_UNLIT			5
#define PB_INDEX_FILTERING		6
#define PB_INDEX_DIFFUSE		7
#define PB_INDEX_SPECULAR		8
#define PB_INDEX_SHININESS		9
#define PB_INDEX_SHINSTR		10
#define PB_INDEX_OPACITY		11
#define PB_INDEX_PASTED			12
#define PB_INDEX_PHYSICS		13
#define PB_INDEX_EMAP			14
#define PB_INDEX_PUNCH			15
#define PB_INDEX_ADJUST			16
#define PB_INDEX_SUBTRACTIVE	17
#define PB_INDEX_BUMP			18
#define PB_INDEX_WRAPFIX		19
#define PB_INDEX_FROMSCREEN		20
#define PB_INDEX_TRANSTYPE		21
#define PB_INDEX_PALETTE		22
#define PB_INDEX_VIS			23
#define	PB_INDEX_ENEMYCOLL		24
#define PB_INDEX_CAMERACOLL		25
#define	PB_INDEX_BULLETCOLL		26
#define	PB_INDEX_PLAYERCOLL		27
#define PB_INDEX_WRAP_U			28
#define PB_INDEX_WRAP_V			29

#define PB_INDEX_MAX			30

static ParamBlockDescID pdesc0[] = {
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_COLL }, // collisionable
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_TWOSIDED }, // twosided
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_TRANS }, // transparent
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_ADDITIVE }, // additive
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_ANISOTROPIC }, // anisotropic
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_UNLIT }, // unlit
	{ TYPE_INT,		NULL,	FALSE, PB_INDEX_FILTERING }, // filter mode
	{ TYPE_RGBA,	NULL,	FALSE, PB_INDEX_DIFFUSE }, // diffuse
	{ TYPE_RGBA,	NULL,	FALSE, PB_INDEX_SPECULAR }, // specular
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_SHININESS }, // shininess
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_SHINSTR }, // shinstr
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_OPACITY }, // opacity
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_PASTED }, // pasted
	{ TYPE_INT,		NULL,	FALSE, PB_INDEX_PHYSICS }, // physics props
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_EMAP }, // environment map flag
};
static ParamBlockDescID pdesc1[] = {
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_COLL }, // collisionable
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_TWOSIDED }, // twosided
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_TRANS }, // transparent
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_ADDITIVE }, // additive
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_ANISOTROPIC }, // anisotropic
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_UNLIT }, // unlit
	{ TYPE_INT,		NULL,	FALSE, PB_INDEX_FILTERING }, // filter mode
	{ TYPE_RGBA,	NULL,	FALSE, PB_INDEX_DIFFUSE }, // diffuse
	{ TYPE_RGBA,	NULL,	FALSE, PB_INDEX_SPECULAR }, // specular
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_SHININESS }, // shininess
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_SHINSTR }, // shinstr
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_OPACITY }, // opacity
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_PASTED }, // pasted
	{ TYPE_INT,		NULL,	FALSE, PB_INDEX_PHYSICS }, // physics props
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_EMAP }, // environment map flag
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_PUNCH } // punch-thru flag
};
static ParamBlockDescID pdesc2[] = {
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_COLL }, // collisionable
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_TWOSIDED }, // twosided
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_TRANS }, // transparent
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_ADDITIVE }, // additive
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_ANISOTROPIC }, // anisotropic
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_UNLIT }, // unlit
	{ TYPE_INT,		NULL,	FALSE, PB_INDEX_FILTERING }, // filter mode
	{ TYPE_RGBA,	NULL,	FALSE, PB_INDEX_DIFFUSE }, // diffuse
	{ TYPE_RGBA,	NULL,	FALSE, PB_INDEX_SPECULAR }, // specular
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_SHININESS }, // shininess
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_SHINSTR }, // shinstr
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_OPACITY }, // opacity
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_PASTED }, // pasted
	{ TYPE_INT,		NULL,	FALSE, PB_INDEX_PHYSICS }, // physics props
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_EMAP }, // environment map flag
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_PUNCH }, // punch-thru flag
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_ADJUST } // mipmap adjust
};
static ParamBlockDescID pdesc3[] = {
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_COLL }, // collisionable
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_TWOSIDED }, // twosided
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_TRANS }, // transparent
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_ADDITIVE }, // additive
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_ANISOTROPIC }, // anisotropic
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_UNLIT }, // unlit
	{ TYPE_INT,		NULL,	FALSE, PB_INDEX_FILTERING }, // filter mode
	{ TYPE_RGBA,	NULL,	FALSE, PB_INDEX_DIFFUSE }, // diffuse
	{ TYPE_RGBA,	NULL,	FALSE, PB_INDEX_SPECULAR }, // specular
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_SHININESS }, // shininess
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_SHINSTR }, // shinstr
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_OPACITY }, // opacity
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_PASTED }, // pasted
	{ TYPE_INT,		NULL,	FALSE, PB_INDEX_PHYSICS }, // physics props
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_EMAP }, // environment map flag
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_PUNCH }, // punch-thru flag
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_ADJUST }, // mipmap adjust
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_SUBTRACTIVE }, // subtractive
};
static ParamBlockDescID pdesc4[] = {
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_COLL }, // collisionable
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_TWOSIDED }, // twosided
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_TRANS }, // transparent
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_ADDITIVE }, // additive
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_ANISOTROPIC }, // anisotropic
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_UNLIT }, // unlit
	{ TYPE_INT,		NULL,	FALSE, PB_INDEX_FILTERING }, // filter mode
	{ TYPE_RGBA,	NULL,	FALSE, PB_INDEX_DIFFUSE }, // diffuse
	{ TYPE_RGBA,	NULL,	FALSE, PB_INDEX_SPECULAR }, // specular
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_SHININESS }, // shininess
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_SHINSTR }, // shinstr
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_OPACITY }, // opacity
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_PASTED }, // pasted
	{ TYPE_INT,		NULL,	FALSE, PB_INDEX_PHYSICS }, // physics props
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_EMAP }, // environment map flag
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_PUNCH }, // punch-thru flag
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_ADJUST }, // mipmap adjust
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_SUBTRACTIVE }, // subtractive
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_BUMP }, // environment map flag
};
static ParamBlockDescID pdesc5[] = {
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_COLL }, // collisionable
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_TWOSIDED }, // twosided
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_TRANS }, // transparent
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_ADDITIVE }, // additive
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_ANISOTROPIC }, // anisotropic
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_UNLIT }, // unlit
	{ TYPE_INT,		NULL,	FALSE, PB_INDEX_FILTERING }, // filter mode
	{ TYPE_RGBA,	NULL,	FALSE, PB_INDEX_DIFFUSE }, // diffuse
	{ TYPE_RGBA,	NULL,	FALSE, PB_INDEX_SPECULAR }, // specular
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_SHININESS }, // shininess
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_SHINSTR }, // shinstr
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_OPACITY }, // opacity
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_PASTED }, // pasted
	{ TYPE_INT,		NULL,	FALSE, PB_INDEX_PHYSICS }, // physics props
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_EMAP }, // environment map flag
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_PUNCH }, // punch-thru flag
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_ADJUST }, // mipmap adjust
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_SUBTRACTIVE }, // subtractive
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_BUMP }, // environment map flag
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_WRAPFIX }, // texture wrap fix
};
static ParamBlockDescID pdesc6[] = {
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_COLL }, // collisionable
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_TWOSIDED }, // twosided
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_TRANS }, // transparent
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_ADDITIVE }, // additive
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_ANISOTROPIC }, // anisotropic
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_UNLIT }, // unlit
	{ TYPE_INT,		NULL,	FALSE, PB_INDEX_FILTERING }, // filter mode
	{ TYPE_RGBA,	NULL,	FALSE, PB_INDEX_DIFFUSE }, // diffuse
	{ TYPE_RGBA,	NULL,	FALSE, PB_INDEX_SPECULAR }, // specular
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_SHININESS }, // shininess
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_SHINSTR }, // shinstr
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_OPACITY }, // opacity
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_PASTED }, // pasted
	{ TYPE_INT,		NULL,	FALSE, PB_INDEX_PHYSICS }, // physics props
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_EMAP }, // environment map flag
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_PUNCH }, // punch-thru flag
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_ADJUST }, // mipmap adjust
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_SUBTRACTIVE }, // subtractive
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_BUMP }, // environment map flag
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_WRAPFIX }, // texture wrap fix
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_FROMSCREEN }, // fromscreen transp
	{ TYPE_INT,		NULL,	FALSE, PB_INDEX_TRANSTYPE }, // trans type
};
static ParamBlockDescID pdesc7[] = {
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_COLL }, // collisionable
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_TWOSIDED }, // twosided
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_TRANS }, // transparent
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_ADDITIVE }, // additive
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_ANISOTROPIC }, // anisotropic
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_UNLIT }, // unlit
	{ TYPE_INT,		NULL,	FALSE, PB_INDEX_FILTERING }, // filter mode
	{ TYPE_RGBA,	NULL,	FALSE, PB_INDEX_DIFFUSE }, // diffuse
	{ TYPE_RGBA,	NULL,	FALSE, PB_INDEX_SPECULAR }, // specular
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_SHININESS }, // shininess
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_SHINSTR }, // shinstr
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_OPACITY }, // opacity
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_PASTED }, // pasted
	{ TYPE_INT,		NULL,	FALSE, PB_INDEX_PHYSICS }, // physics props
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_EMAP }, // environment map flag
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_PUNCH }, // punch-thru flag
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_ADJUST }, // mipmap adjust
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_SUBTRACTIVE }, // subtractive
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_BUMP }, // environment map flag
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_WRAPFIX }, // texture wrap fix
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_FROMSCREEN }, // fromscreen transp
	{ TYPE_INT,		NULL,	FALSE, PB_INDEX_TRANSTYPE }, // trans type
	{ TYPE_INT,		NULL,	FALSE, PB_INDEX_PALETTE }, // palette number
};

static ParamBlockDescID pdesc8[] = {  
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_COLL }, // collisionable
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_TWOSIDED }, // twosided
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_TRANS }, // transparent
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_ADDITIVE }, // additive
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_ANISOTROPIC }, // anisotropic
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_UNLIT }, // unlit
	{ TYPE_INT,		NULL,	FALSE, PB_INDEX_FILTERING }, // filter mode
	{ TYPE_RGBA,	NULL,	FALSE, PB_INDEX_DIFFUSE }, // diffuse
	{ TYPE_RGBA,	NULL,	FALSE, PB_INDEX_SPECULAR }, // specular
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_SHININESS }, // shininess
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_SHINSTR }, // shinstr
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_OPACITY }, // opacity
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_PASTED }, // pasted
	{ TYPE_INT,		NULL,	FALSE, PB_INDEX_PHYSICS }, // physics props
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_EMAP }, // environment map flag
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_PUNCH }, // punch-thru flag
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_ADJUST }, // mipmap adjust
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_SUBTRACTIVE }, // subtractive
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_BUMP }, // environment map flag
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_WRAPFIX }, // texture wrap fix
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_FROMSCREEN }, // fromscreen transp
	{ TYPE_INT,		NULL,	FALSE, PB_INDEX_TRANSTYPE }, // trans type
	{ TYPE_INT,		NULL,	FALSE, PB_INDEX_PALETTE }, // palette number
	{ TYPE_INT,		NULL,	FALSE, PB_INDEX_VIS }, // visibility index
};
static ParamBlockDescID pdesc9[] = {  
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_COLL }, // collisionable
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_TWOSIDED }, // twosided
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_TRANS }, // transparent
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_ADDITIVE }, // additive
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_ANISOTROPIC }, // anisotropic
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_UNLIT }, // unlit
	{ TYPE_INT,		NULL,	FALSE, PB_INDEX_FILTERING }, // filter mode
	{ TYPE_RGBA,	NULL,	FALSE, PB_INDEX_DIFFUSE }, // diffuse
	{ TYPE_RGBA,	NULL,	FALSE, PB_INDEX_SPECULAR }, // specular
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_SHININESS }, // shininess
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_SHINSTR }, // shinstr
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_OPACITY }, // opacity
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_PASTED }, // pasted
	{ TYPE_INT,		NULL,	FALSE, PB_INDEX_PHYSICS }, // physics props
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_EMAP }, // environment map flag
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_PUNCH }, // punch-thru flag
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_ADJUST }, // mipmap adjust
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_SUBTRACTIVE }, // subtractive
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_BUMP }, // environment map flag
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_WRAPFIX }, // texture wrap fix
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_FROMSCREEN }, // fromscreen transp
	{ TYPE_INT,		NULL,	FALSE, PB_INDEX_TRANSTYPE }, // trans type
	{ TYPE_INT,		NULL,	FALSE, PB_INDEX_PALETTE }, // palette number
	{ TYPE_INT,		NULL,	FALSE, PB_INDEX_VIS }, // visibility index
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_ENEMYCOLL }, // ENEMY COLLISION
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_CAMERACOLL }, // CAMERA COLLISION
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_BULLETCOLL }, // BULLET COLLISION
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_PLAYERCOLL }, // PLAYER COLLISION
};
static ParamBlockDescID pdesc10[] = {  
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_COLL }, // collisionable
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_TWOSIDED }, // twosided
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_TRANS }, // transparent
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_ADDITIVE }, // additive
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_ANISOTROPIC }, // anisotropic
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_UNLIT }, // unlit
	{ TYPE_INT,		NULL,	FALSE, PB_INDEX_FILTERING }, // filter mode
	{ TYPE_RGBA,	NULL,	FALSE, PB_INDEX_DIFFUSE }, // diffuse
	{ TYPE_RGBA,	NULL,	FALSE, PB_INDEX_SPECULAR }, // specular
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_SHININESS }, // shininess
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_SHINSTR }, // shinstr
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_OPACITY }, // opacity
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_PASTED }, // pasted
	{ TYPE_INT,		NULL,	FALSE, PB_INDEX_PHYSICS }, // physics props
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_EMAP }, // environment map flag
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_PUNCH }, // punch-thru flag
	{ TYPE_FLOAT,	NULL,	FALSE, PB_INDEX_ADJUST }, // mipmap adjust
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_SUBTRACTIVE }, // subtractive
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_BUMP }, // environment map flag
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_WRAPFIX }, // texture wrap fix
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_FROMSCREEN }, // fromscreen transp
	{ TYPE_INT,		NULL,	FALSE, PB_INDEX_TRANSTYPE }, // trans type
	{ TYPE_INT,		NULL,	FALSE, PB_INDEX_PALETTE }, // palette number
	{ TYPE_INT,		NULL,	FALSE, PB_INDEX_VIS }, // visibility index
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_ENEMYCOLL }, // ENEMY COLLISION
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_CAMERACOLL }, // CAMERA COLLISION
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_BULLETCOLL }, // BULLET COLLISION
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_PLAYERCOLL }, // PLAYER COLLISION
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_WRAP_U }, // texture wrap fix in U
	{ TYPE_BOOL,	NULL,	FALSE, PB_INDEX_WRAP_V }, // texture wrap fix in V
};


#define PBLOCK_LENGTH			PB_INDEX_MAX
#define CURRENT_VERSION			11
#define CURRENT_DESC			pdesc10
static int radButs[] = { IDC_NOFILT, IDC_BILINEAR, IDC_TRILINEAR };
static int transType[] = { IDC_NOTRANS, IDC_ADDITIVE, IDC_SUBTRACTIVE, IDC_PUNCHTHRU, IDC_SCREEN };

static char *physTypes[] = { "Rock", "Grass", "Metal", "Mud", "Sand", "LoopTheLoop", "Ice", "Water", "Snow", "RealGravity", "WaterSlideWater", "SpiderMan", "KotH", "Danger", "NODECAL", "WALLOFDEATH" };

static int CheckBox[] =
{
	IDC_CHECK1,
	IDC_CHECK2,
	IDC_CHECK3,
	IDC_CHECK4,
	IDC_CHECK5,
	IDC_CHECK6,
	IDC_CHECK7,
	IDC_CHECK17,
	IDC_CHECK9,
	IDC_CHECK10,
	IDC_CHECK11,
	IDC_CHECK12,
	IDC_CHECK13,
	IDC_CHECK14,
	IDC_CHECK15,
	IDC_CHECK16,
	IDC_CHECK18,
	IDC_CHECK19,
	IDC_CHECK20,
	IDC_CHECK21,
	IDC_CHECK22,
	IDC_CHECK23,
	IDC_CHECK24,
	IDC_CHECK25,
	IDC_CHECK26,
	IDC_CHECK27,
	IDC_CHECK28,
	IDC_CHECK29,
	IDC_CHECK30,
	IDC_CHECK31,
	IDC_CHECK32,
	IDC_CHECK33
};

static ParamUIDesc uidesc[] =
{
	ParamUIDesc (PB_INDEX_COLL,			TYPE_SINGLECHEKBOX, IDC_COLLISION),
	ParamUIDesc (PB_INDEX_TWOSIDED,		TYPE_SINGLECHEKBOX, IDC_TWOSIDED),
	ParamUIDesc (PB_INDEX_ANISOTROPIC,	TYPE_SINGLECHEKBOX, IDC_ANISOTROPIC),
	ParamUIDesc (PB_INDEX_UNLIT,		TYPE_SINGLECHEKBOX, IDC_UNLIT),
	ParamUIDesc (PB_INDEX_FILTERING,	TYPE_RADIO, radButs, asize(radButs)),
	ParamUIDesc (PB_INDEX_DIFFUSE,		TYPE_COLORSWATCH,	IDC_DIFFUSE),
	ParamUIDesc (PB_INDEX_SPECULAR,		TYPE_COLORSWATCH,	IDC_SPECULAR),
	ParamUIDesc (PB_INDEX_OPACITY,		EDITTYPE_POS_FLOAT, IDC_OP_EDIT, IDC_OP_SPIN, 0, 1, 0.01f),
	ParamUIDesc (PB_INDEX_PASTED,		EDITTYPE_POS_FLOAT, IDC_AMT_PA,	 IDC_SPIN_PA, 0, 1, 0.01f),
	ParamUIDesc (PB_INDEX_EMAP,			TYPE_SINGLECHEKBOX, IDC_EMAP),
	ParamUIDesc (PB_INDEX_ADJUST,		EDITTYPE_POS_FLOAT, IDC_AMT_ADJUST,	 IDC_SPIN_ADJUST, 0.25f, 3.75f, 0.25f),
	ParamUIDesc (PB_INDEX_BUMP,			TYPE_SINGLECHEKBOX, IDC_BUMPMAP),
	ParamUIDesc (PB_INDEX_WRAP_U,		TYPE_SINGLECHEKBOX, IDC_WRAP_U),
	ParamUIDesc (PB_INDEX_WRAP_V,		TYPE_SINGLECHEKBOX, IDC_WRAP_V),
	ParamUIDesc (PB_INDEX_TRANSTYPE,	TYPE_RADIO, transType, asize(transType)),
	ParamUIDesc (PB_INDEX_PALETTE,		EDITTYPE_POS_INT,	IDC_PAL_ADJUST,	 IDC_SPIN_PAL, 0, 3, 1),
	ParamUIDesc (PB_INDEX_ENEMYCOLL,   	TYPE_SINGLECHEKBOX, IDC_ENEMYCOLLISION),
	ParamUIDesc (PB_INDEX_CAMERACOLL,   	TYPE_SINGLECHEKBOX, IDC_CAMERACOLLISION),
	ParamUIDesc (PB_INDEX_BULLETCOLL,   	TYPE_SINGLECHEKBOX, IDC_BULLETCOLLISION),
	ParamUIDesc (PB_INDEX_PLAYERCOLL,   	TYPE_SINGLECHEKBOX, IDC_PLAYERCOLLISION),
}; 
#define UIDESC_LEN 20

#define NUMOLDVERSIONS (CURRENT_VERSION-1)
static ParamVersionDesc versions[NUMOLDVERSIONS] = {
	ParamVersionDesc (pdesc0, 15, 1),
	ParamVersionDesc (pdesc1, 16, 2),
	ParamVersionDesc (pdesc2, 17, 3),
	ParamVersionDesc (pdesc3, 18, 4),
	ParamVersionDesc (pdesc4, 19, 5),
	ParamVersionDesc (pdesc5, 20, 6),
	ParamVersionDesc (pdesc6, 22, 7),
	ParamVersionDesc (pdesc7, 23, 8),  
	ParamVersionDesc (pdesc8, 28, 9),  
	ParamVersionDesc (pdesc9, 28, 10),  
};
static ParamVersionDesc curVersion = ParamVersionDesc (CURRENT_DESC, PBLOCK_LENGTH, CURRENT_VERSION);


void MGMaterialDlg::SetThing (ReferenceTarget *target)
{
	assert (target->ClassID() == MGMATERIAL_CLASS_ID);
	assert (target->SuperClassID()==MATERIAL_CLASS_ID);

	if (mat)
		mat->paramDlg = NULL;
	mat = (MGMaterial *)target;
	if (mat)
		mat->paramDlg = this;
	LoadDialog (TRUE);
}

class MaterialClassDesc : public ClassDesc
{
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new MGMaterial(loading);}
	const TCHAR *	ClassName() { return GetString(IDS_MATERIAL); }
	SClass_ID		SuperClassID() { return MATERIAL_CLASS_ID; }
	Class_ID		ClassID() { return MGMATERIAL_CLASS_ID; }
	const TCHAR* 	Category() { return _T("");}
};

MaterialClassDesc materialClassDesc;

ClassDesc *GetMaterialDesc()
{
	return &materialClassDesc;
}

void MGMaterialDlg::LoadDialog (BOOL draw)
{
	if (mat && mat->pblock) {
		stdMap->SetParamBlock (mat->pblock);
	}
	UpdateSubMtlNames();
}

class MGMaterialDlgProc : public ParamMapUserDlgProc
{
private:
	MGMaterialDlg *dlg;
public:
	MGMaterialDlgProc (MGMaterialDlg *d)
	{
		dlg = d;
	}
	BOOL DlgProc (TimeValue t, IParamMap *map, HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
	{
		int res;
		dlg->SetActive(true);
		res = dlg->WndProc (wnd, msg, wp, lp);
		dlg->SetActive(false);
		return res;
	}
	void DeleteThis() { delete this; }
};

MGMaterialDlg::MGMaterialDlg (HWND w, IMtlParams *params, MGMaterial *m)
{
	dadMgr.Init(this);
	dadMgrConv.Init(this);
	ip = params; mtlWindow = w;
	mat = m;
	isActive = false;
	texBut = pasteBut = camBut = NULL;
	matType = NULL;
	stdMap = CreateMParamMap (uidesc, UIDESC_LEN,
		m->pblock, ip, hInstance, MAKEINTRESOURCE(IDD_MATERIAL), GetString (IDS_BASIC),
		0);
	stdMap->SetUserDlgProc (new MGMaterialDlgProc(this));
	UpdateSubMtlNames();
}

void MGMaterialDlg::ActivateDlg(BOOL onOff)
{ 
	// Update the swatches
	if (mat) {
#if 0
		((IColorSwatch *)(mat->pblock->GetController(PB_INDEX_DIFFUSE)))->Activate(onOff);
		((IColorSwatch *)(mat->pblock->GetController(PB_INDEX_SPECULAR)))->Activate(onOff);
#endif
	}
}








void PrepareStratBox(INode *node,HWND listBox, HTREEITEM after)
{
	TV_INSERTSTRUCT insert;
	int i,numchild;
	INode *oldnode=node;
	//char buf[64];
		
	numchild = node->NumberOfChildren();

  	Object *object = node->EvalWorldState(GetCOREInterface()->GetTime()).obj;

	if (object)
  	if ((object->ClassID() == camcontrols_CLASS_ID))
  	{
	  //	RDObject *rd = (RDObject*)object;

	  //	if (rd->Type == CAMCONTROL)
	  //	{

			HTREEITEM tree;
			insert.hParent = after;
			insert.hInsertAfter = TVI_SORT;
			insert.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
			insert.item.pszText = node->GetName();
			insert.item.lParam = (LPARAM) after;
			insert.item.iImage = 2;
			insert.item.iSelectedImage = 1;
			after = 
				tree = TreeView_InsertItem (listBox, &insert);
	
	   //	}
	} 

	if (!numchild)
		return;

	for (i=0;i<numchild;i++)
	{
		node = oldnode->GetChildNode(i);
		PrepareStratBox(node,listBox,after);
	}
}


// Recursively gets the filename of a listbox item
static void GetItemName (HWND treeItem, char *buffer, HTREEITEM tree)
{
	TV_ITEM newInfo;
   	char textBuf[256];
	// Get the information from this tree node
	memset (&newInfo, 0, sizeof (newInfo));
	newInfo.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM;
	newInfo.hItem = tree;
	newInfo.pszText = &textBuf[0];
	newInfo.cchTextMax = sizeof (textBuf);
	TreeView_GetItem (treeItem, &newInfo);
	
	strcat (buffer, textBuf);

}

static char *selectedModelName = NULL;			// a pointer to the currently selected model


static BOOL CALLBACK dialogFunc (HWND dBox, UINT msg, WPARAM wp, LPARAM lp)
{
	static HWND treeItem;
	static HIMAGELIST imageList;
	switch (msg) {
	case WM_INITDIALOG:
		
		treeItem = GetDlgItem (dBox, IDC_OBJECT_TREE);
	  	PrepareStratBox (GetCOREInterface()->GetRootNode(),treeItem, NULL);
		imageList = ImageList_LoadBitmap (hInstance, MAKEINTRESOURCE (IDB_IMAGELIST),
			16, 0, RGB (0, 0, 255));
		TreeView_SetImageList (treeItem, imageList, TVSIL_NORMAL);
		EnableWindow (GetDlgItem (dBox, IDOK), FALSE);

		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wp)) {
		case IDOK:
			EndDialog (dBox, TRUE);
			return FALSE;
		case IDCANCEL:
			EndDialog (dBox, FALSE);
			return FALSE;
		}
	case WM_NOTIFY:
		LPNMHDR pnmh = (LPNMHDR) lp; 
		switch (pnmh->code) {
		case NM_DBLCLK:
	   //	case TVN_SELCHANGED:
			char textBuf[MAX_PATH];
			textBuf[0]='\0';
			if (pnmh->code == TVN_SELCHANGED)
			{
				// Get a pointer to the tree event block
				NM_TREEVIEW FAR *pnmtv = (NM_TREEVIEW FAR *) pnmh;
				// Check for a valid selection
				GetItemName (treeItem, &textBuf[0], pnmtv->itemNew.hItem);
			} else
			{
				HTREEITEM tree;
				tree = TreeView_GetSelection (treeItem);
				GetItemName (treeItem, &textBuf[0], tree);
			}
			FindNamedNode(GetCOREInterface()->GetRootNode(),textBuf);
		  /*	if (selectedModelName)
				free (selectedModelName);
			selectedModelName = strdup (textBuf);*/
			EndDialog (dBox, TRUE);
			return TRUE;

		}
	}
	return FALSE;
}



void UpdateDialogue (HWND hWnd, int val)
{
  	return;
	if (val & ENEMYCOLLISION)
		CheckDlgButton(hWnd, IDC_ENEMYCOLLISION, 1);
	else
		CheckDlgButton(hWnd, IDC_ENEMYCOLLISION, 0);

	if (val & BULLETCOLLISION)
		CheckDlgButton(hWnd, IDC_BULLETCOLLISION, 1);
	else
		CheckDlgButton(hWnd, IDC_BULLETCOLLISION, 0);

	if (val & PLAYERCOLLISION)
		CheckDlgButton(hWnd, IDC_PLAYERCOLLISION, 1);
	else
		CheckDlgButton(hWnd, IDC_PLAYERCOLLISION, 0);

	if (val & CAMERACOLLISION)
		CheckDlgButton(hWnd, IDC_CAMERACOLLISION, 1);
	else
		CheckDlgButton(hWnd, IDC_CAMERACOLLISION, 0);

//	if (val & SPECIALCOLLISION)
//		CheckDlgButton(hWnd, IDC_SPECIALCOLLISION, 1);
//	else
//		CheckDlgButton(hWnd, IDC_SPECIALCOLLISION, 0);



}


BOOL MGMaterialDlg::WndProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BOOL result;
  //	RedDog::Uint16 val;
	int val;
	switch (msg) {
	case WM_INITDIALOG:
		texBut = GetICustButton(GetDlgItem(hWnd,IDC_MAP_DI));
		texBut->SetDADMgr(&dadMgr);
		convBut = GetICustButton(GetDlgItem(hWnd,IDC_CONV));
		convBut->SetDADMgr(&dadMgrConv);
		pasteBut = GetICustButton(GetDlgItem(hWnd,IDC_MAP_PA));
		pasteBut->SetDADMgr(&dadMgr);
		camBut = GetICustButton(GetDlgItem(hWnd,IDC_MATCAM));
		{
			int i;
			Interval ivalid = FOREVER;
			for (i=0; i < asize (physTypes); ++i)
				SendMessage (GetDlgItem(hWnd, IDC_SURFACE), CB_ADDSTRING,
				0, (LPARAM)physTypes[i]);
			mat->pblock->GetValue (PB_INDEX_PHYSICS, 0, i, ivalid);
			i = GET_SURF_TYPE(i);
		   	matType = GetDlgItem(hWnd, IDC_SURFACE);
			SendMessage (matType, CB_SETCURSEL,
				(WPARAM)i, 0);
		}
		
		UpdateSubMtlNames();

	   	mat->CamNode = (INode*)mat->GetReference(REF_CAMERA);
	   	if (mat->CamNode)
	   		SetDlgItemText (hWnd, IDC_MATCAM, mat->CamNode->GetName());
	    val = (int)mat->material.getCollFlags();
		UpdateDialogue(hWnd,val);
		return TRUE;
	case WM_COMMAND:
	   	mat->CamNode = (INode*)mat->GetReference(REF_CAMERA);
	   	if (mat->CamNode)
	   		SetDlgItemText (hWnd, IDC_MATCAM, mat->CamNode->GetName());

		switch (LOWORD(wParam)) {
		case IDC_MAP_DI:
			PostMessage(mtlWindow, WM_TEXMAP_BUTTON, 
				0 ,(LPARAM)mat);
			break;
		case IDC_MAP_PA:
			PostMessage(mtlWindow, WM_TEXMAP_BUTTON, 
				1 ,(LPARAM)mat);
			break;
		case IDC_SURFACE:
			if (HIWORD(wParam)==CBN_SELCHANGE) {
				int handle = SendMessage (GetDlgItem(hWnd, IDC_SURFACE), CB_GETCURSEL, 0, 0);
				mat->pblock->SetValue (PB_INDEX_PHYSICS, 0, handle);
				UpdateSubMtlNames();
			}
			break;
		case IDC_NOFILT:
		case IDC_BILINEAR:
		case IDC_TRILINEAR:
			// Ought to disable pasted map here...
			break;
		case IDC_MATCAM:
			result = DialogBoxParam (hInstance, MAKEINTRESOURCE (IDD_STRATLIST),
			hWnd, dialogFunc, NULL);


			if (result)
			{
				if (mat->GetReference(REF_CAMERA))
					mat->ReplaceReference(REF_CAMERA,NamedNode,1);
				else
			   		mat->MakeRefByID(FOREVER,REF_CAMERA,NamedNode);
				UpdateSubMtlNames();

				//NOW DISPLAY THE CAMERA DETAILS

				theHold.Begin();
			   	GetCOREInterface()->SelectNode(NamedNode);
				GetCOREInterface()->ExecuteMAXCommand(MAXCOM_ZOOMEXT_SEL);
				GetCOREInterface()->ExecuteMAXCommand(MAXCOM_MODIFY_MODE);
				TSTR undostr; undostr.printf("Select");
				theHold.Accept(undostr); 
				
			}
			break;
		case IDC_CAMCLEAR:
			mat->CamNode = NULL;
			UpdateSubMtlNames();
			break;
#if 0
		case IDC_CAMERACOLLISION:

			val = (int)mat->material.getCollFlags();
			
			if (val & CAMERACOLLISION)
		   		val = val & ~CAMERACOLLISION;
		   	else
				val |= CAMERACOLLISION;

			mat->material.setCollFlags((RedDog::Uint16)val);
			break;
		case IDC_ENEMYCOLLISION:
		    val = (int)mat->material.getCollFlags();
			if (val & ENEMYCOLLISION)
				val = val & ~ENEMYCOLLISION;
			else
				val |= ENEMYCOLLISION;


			mat->material.setCollFlags((RedDog::Uint16)val);
		   
		//   mat->material.setCollFlags(val);
			break;
		case IDC_BULLETCOLLISION:
		    val = (int)mat->material.getCollFlags();
			if (val & BULLETCOLLISION)
				val = val & ~BULLETCOLLISION;
			else
				val |= BULLETCOLLISION;

			mat->material.setCollFlags((RedDog::Uint16)val);

		  // mat->material.setCollFlags(val);
			break;
		case IDC_PLAYERCOLLISION:
		   	 val = (int)mat->material.getCollFlags();
			if (val & PLAYERCOLLISION)
				val = val & ~PLAYERCOLLISION;
			else
				val |= PLAYERCOLLISION;

			mat->material.setCollFlags((RedDog::Uint16)val);

		case IDC_SPECIALCOLLISION:
		   	 val = (int)mat->material.getCollFlags();
			if (val & SPECIALCOLLISION)
				val = val & ~SPECIALCOLLISION;
			else
				val |= SPECIALCOLLISION;

			mat->material.setCollFlags((RedDog::Uint16)val);



		 //  mat->material.setCollFlags(val);
			break;
#endif
		default:
			int i;
			int id = 0;
			for (i = 0; i < 32; ++i) {
				if (IsDlgButtonChecked (hWnd, CheckBox[i])) {
					id |= (1<<i);
				}
			}
			if (id == 0)
				id = 1;
			mat->material.SetVisMask (id);
			mat->pblock->SetValue (PB_INDEX_VIS, 0, id);

//			char buf[1024];
//			sprintf (buf, "It's 0x%x", id);
//			MessageBox (NULL, buf, "test", MB_OK);
			break;
		}
		    val = (int)mat->material.getCollFlags();
		UpdateDialogue(hWnd,val);
		return TRUE;
	}
	return FALSE;
}

void MGMaterialDlg::UpdateSubMtlNames()
{
	if (stdMap && stdMap->GetHWnd()) {
		int i, j=0;
		Uint32 mask = mat->material.GetVisMask();
		for (i = 0; i < 32; ++i) {
			CheckDlgButton (stdMap->GetHWnd(), CheckBox[i], (Uint32(mask & (1<<i))) ? TRUE : FALSE);
		}
	}
	if (texBut && pasteBut) {
		if (mat && mat->texture) {
			texBut->SetText (mat->texture->GetName());
		} else {
			texBut->SetText (GetString (IDS_NONE));
		}
		if (mat && mat->pasted) {
			pasteBut->SetText (mat->pasted->GetName());
		} else {
			pasteBut->SetText (GetString (IDS_NONE));
		}
		if (mat && mat->CamNode) {
			camBut->SetText (mat->CamNode->GetName());
		} else {
			camBut->SetText (GetString (IDS_NONE));
		}
	}
	if (convBut)
		convBut->SetText (GetString (IDS_CONV));
	if (mat && matType) {
		int i;
		Interval ivalid = FOREVER;
		mat->pblock->GetValue (PB_INDEX_PHYSICS, 0, i, ivalid);
		i = GET_SURF_TYPE(i) ;
		SendMessage (matType, CB_SETCURSEL,
			(WPARAM)i, 0);
	}
}

void MGMaterial::UpdateRDMaterial()
{
	if (pblock && !resetting && pblock && pblock->GetVersion() == CURRENT_VERSION) {
		int coll, fType, mType, tType;
		int enemycoll;
		int playercoll;
		int cameracoll;
		int bulletcoll;

		Interval ivalid = FOREVER;
		pblock->GetValue (PB_INDEX_DIFFUSE, 0, material.diffuse, ivalid);
		pblock->GetValue (PB_INDEX_SPECULAR, 0, material.specular, ivalid);
		pblock->GetValue (PB_INDEX_COLL, 0, coll, ivalid);
		pblock->GetValue (PB_INDEX_FILTERING, 0, fType, ivalid);
		pblock->GetValue (PB_INDEX_SHININESS, 0, material.specExponent, ivalid);
		pblock->GetValue (PB_INDEX_SHINSTR, 0, material.specAmount, ivalid);
		pblock->GetValue (PB_INDEX_OPACITY, 0, material.opacity, ivalid);
		pblock->GetValue (PB_INDEX_PASTED, 0, material.filterAmt, ivalid);
		pblock->GetValue (PB_INDEX_ADJUST, 0, material.mipmapAdjust, ivalid);
		pblock->GetValue (PB_INDEX_PALETTE, 0, material.palette, ivalid);
		pblock->GetValue (PB_INDEX_VIS, TimeValue(0), material.visMask, ivalid);
	   
	  //	UPDATE GET VALUE ?
		if (material.visMask == 0)
			material.visMask = 1;

		// Transparency type
		pblock->GetValue (PB_INDEX_TRANSTYPE, 0, tType, ivalid);
		material.additive = (tType == 1) ? true:false;
		material.subtractive = (tType == 2) ? true:false;
		if (tType == 3)	{
			material.flags |= RDMF_PUNCH;
			material.opacity = 1.f;
		} else
			material.flags &= ~RDMF_PUNCH;
		if (tType == 4)
			material.flags |= RDMF_FROMSCREEN;
		else
			material.flags &= ~RDMF_FROMSCREEN;

		
		//GET THE PHYSICS FLAGS IN
		pblock->GetValue (PB_INDEX_PHYSICS, 0, mType, ivalid);
		
		//GET COLLISION WAREZ IN
		pblock->GetValue (PB_INDEX_ENEMYCOLL, 0, enemycoll, ivalid);
		pblock->GetValue (PB_INDEX_CAMERACOLL, 0, cameracoll, ivalid);
		pblock->GetValue (PB_INDEX_BULLETCOLL, 0, bulletcoll, ivalid);
		pblock->GetValue (PB_INDEX_PLAYERCOLL, 0, playercoll, ivalid);





		material.collisionable = coll?true:false;
		material.filter = (RDMaterial::FilterType) fType;
 //		material.matType = (RDMaterial::MatType) mType;
		material.matType = mType&31; /*TO ENSURE ALL TOP BITS ARE CLEARED*/
		material.matType |= (enemycoll <<5);
		material.matType |= (cameracoll <<6);
		material.matType |= (bulletcoll <<7);
		material.matType |= (playercoll <<8);

		/*if none are set, someink is wrong so ensure all are set*/

		if (!(material.matType & 480))
		{
			material.matType += 480;
			pblock->SetValue (PB_INDEX_ENEMYCOLL, 0, 1);
			pblock->SetValue (PB_INDEX_CAMERACOLL, 0, 1);
			pblock->SetValue (PB_INDEX_BULLETCOLL, 0, 1);
			pblock->SetValue (PB_INDEX_PLAYERCOLL, 0, 1);


		}


		//IF THE MATERIAL HAS AN ATTACHED CAMERA REC, OR IN ITS RECORD NUMBER + AN ID OF CAMERA
		if (CamNode)
		{
			camcontrol *obj = (camcontrol*)CamNode->EvalWorldState(GetCOREInterface()->GetTime()).obj;

			int record = (obj->GetMyCamIndex())<<10;

   			material.matType |= (CAMERA + record);


		}
		else
		{
			//SEE IF IT WAS DEFINED AS A CAMERA SLOPE

			/*if (camslope)
			   material.matType |= CAMERA;
			else
			  */

			//ENSURE IT HAS NO RECORD OF A CAMERA BEING ATTACHED
			//IE:ONLY ITS LOWEST 9 BITS ARE VALID
			material.matType &= 511;

			

		}


		int aniso, unlit, twos, emap, bump, wrapX, wrapY;
		pblock->GetValue (PB_INDEX_ANISOTROPIC, 0, aniso, ivalid);
		pblock->GetValue (PB_INDEX_UNLIT, 0, unlit, ivalid);
		pblock->GetValue (PB_INDEX_TWOSIDED, 0, twos, ivalid);
		pblock->GetValue (PB_INDEX_EMAP, 0, emap, ivalid);
		pblock->GetValue (PB_INDEX_BUMP, 0, bump, ivalid);
		pblock->GetValue (PB_INDEX_WRAP_U, 0, wrapX, ivalid);
		pblock->GetValue (PB_INDEX_WRAP_V, 0, wrapY, ivalid);
		if (aniso)
			material.flags |= RDMF_ANISOTROPIC;
		else
			material.flags &= ~RDMF_ANISOTROPIC;
		if (unlit)
			material.flags |= RDMF_NOLIGHTING;
		else
			material.flags &= ~RDMF_NOLIGHTING;
		if (twos)
			material.flags |= RDMF_TWOSIDED;
		else
			material.flags &= ~RDMF_TWOSIDED;
		if (emap)
			material.flags |= RDMF_ENVIRONMENT;
		else
			material.flags &= ~RDMF_ENVIRONMENT;
		if (bump)
			material.flags |= RDMF_BUMPMAP;
		else
			material.flags &= ~RDMF_BUMPMAP;
		if (wrapX)
			material.flags |= RDMF_CLAMP_X;
		else
			material.flags &= ~RDMF_CLAMP_X;
		if (wrapY)
			material.flags |= RDMF_CLAMP_Y;
		else
			material.flags &= ~RDMF_CLAMP_Y;
	}
}

void MGMaterial::Reset()
{
	resetting = true;
	ReplaceReference (REF_PARAMBLOCK, CreateParameterBlock (CURRENT_DESC, PBLOCK_LENGTH, CURRENT_VERSION));
	assert (pblock);
	ReplaceReference(REF_TEXTURE,NULL);
	ReplaceReference(REF_PASTED,NULL);
	ReplaceReference(REF_CAMERA,NULL);
	ReplaceReference(REF_SUBMATERIAL, NULL);
	pblock->SetValue (PB_INDEX_COLL, 0, material.collisionable);
	pblock->SetValue (PB_INDEX_TWOSIDED, 0, material.flags & RDMF_TWOSIDED);
	pblock->SetValue (PB_INDEX_PUNCH, 0, 0);
	pblock->SetValue (PB_INDEX_ADDITIVE, 0, material.additive);
	pblock->SetValue (PB_INDEX_SUBTRACTIVE, 0, material.subtractive);
	pblock->SetValue (PB_INDEX_ANISOTROPIC, 0, material.flags & RDMF_ANISOTROPIC);
	pblock->SetValue (PB_INDEX_UNLIT, 0, material.flags & RDMF_NOLIGHTING);
	pblock->SetValue (PB_INDEX_FILTERING, 0, material.filter);
	pblock->SetValue (PB_INDEX_DIFFUSE, 0, material.diffuse);
	pblock->SetValue (PB_INDEX_SPECULAR, 0, material.specular);
	pblock->SetValue (PB_INDEX_SHININESS, 0, material.specExponent);
	pblock->SetValue (PB_INDEX_SHINSTR, 0, material.specAmount);
	pblock->SetValue (PB_INDEX_OPACITY, 0, material.opacity);
	pblock->SetValue (PB_INDEX_PASTED, 0, material.filterAmt);
	pblock->SetValue (PB_INDEX_PHYSICS, 0, material.matType);
	pblock->SetValue (PB_INDEX_ADJUST, 0, material.mipmapAdjust);
	pblock->SetValue (PB_INDEX_EMAP, 0, material.flags & RDMF_ENVIRONMENT);
	pblock->SetValue (PB_INDEX_BUMP, 0, material.flags & RDMF_BUMPMAP);
	pblock->SetValue (PB_INDEX_WRAP_U, 0, material.flags & RDMF_CLAMP_X);
	pblock->SetValue (PB_INDEX_WRAP_V, 0, material.flags & RDMF_CLAMP_Y);
	pblock->SetValue (PB_INDEX_FROMSCREEN, 0, material.flags & RDMF_FROMSCREEN);
	pblock->SetValue (PB_INDEX_PALETTE, 0, material.palette);
	pblock->SetValue (PB_INDEX_VIS, 0, material.visMask);
   
	pblock->SetValue (PB_INDEX_ENEMYCOLL, 0, material.matType & ENEMYCOLLISION);
	pblock->SetValue (PB_INDEX_CAMERACOLL, 0, material.matType & CAMERACOLLISION);
	pblock->SetValue (PB_INDEX_BULLETCOLL, 0, material.matType & BULLETCOLLISION);
	pblock->SetValue (PB_INDEX_PLAYERCOLL, 0, material.matType & PLAYERCOLLISION);
//	pblock->SetValue (PB_INDEX_SPECIALCOLL, 0, material.matType & SPECIALCOLLISION);





	
	
	//	UPDATE HERE SOMEHOW
	
	resetting = false;
}

void MGMaterial::setMat(Mtl *m)
{
	if (m==NULL)
		return;
	material = RDMaterial(*m);	// Convert base stuff across
	TSTR str = m->GetName();
	SetName(str);
	Reset();
	// Set up the textures
	if (m->ClassID() == Class_ID (DMTL_CLASS_ID, 0)) {
		StdMat *stdMat = (StdMat *)m;
		Texmap *t = stdMat->GetSubTexmap(ID_DI);
		if (t != NULL && stdMat->MapEnabled (ID_DI)) {
			ReplaceReference (REF_TEXTURE, t);
			SetActiveTexmap (t);
		}
		t = stdMat->GetSubTexmap(ID_FI);
		if (t != NULL && stdMat->MapEnabled (ID_FI)) {
			ReplaceReference (REF_PASTED, t);
		}
	}
}

class MGMaterialFixer : public PostLoadCallback
{
	MGMaterial *that;
public:
	MGMaterialFixer (MGMaterial *t)
	{ that = t; }
	void proc (ILoad *load)
	{
		int tType, add, sub, punch;
		Interval ivalid = FOREVER;
		(void)load;
		// Already new format?
		if (that->pblock->GetVersion() < 8) {
			that->pblock->GetValue (PB_INDEX_TRANSTYPE, 0, tType, ivalid);
			that->pblock->GetValue (PB_INDEX_ADDITIVE, 0, add, ivalid);
			that->pblock->GetValue (PB_INDEX_SUBTRACTIVE, 0, sub, ivalid);
			that->pblock->GetValue (PB_INDEX_PUNCH, 0, punch, ivalid);
			if (add)
				tType = 1;
			else if (sub)
				tType = 2;
			else if (punch)
				tType = 3;
			that->pblock->SetValue (PB_INDEX_TRANSTYPE, 0, tType);
		}
		if (that->pblock->GetVersion() < 9) {
			that->pblock->SetValue (PB_INDEX_VIS, 0, (int)1);
			that->material.SetVisMask(1);
		}
		int wrap;
		that->pblock->GetValue (PB_INDEX_WRAPFIX, 0, wrap, ivalid);
		if (wrap) {
			that->pblock->SetValue (PB_INDEX_WRAP_U, 0, (int)1);
			that->pblock->SetValue (PB_INDEX_WRAP_V, 0, (int)1);
			that->pblock->SetValue (PB_INDEX_WRAPFIX, 0, (int)0);
		}

		delete this;
	}
};

IOResult MGMaterial::Load(ILoad *load)
{
	/* Load parent's stuff */
	MtlBase::Load(load);
	load->RegisterPostLoadCallback (new ParamBlockPLCB (versions, NUMOLDVERSIONS, &curVersion, this, 0));
	load->RegisterPostLoadCallback (new MGMaterialFixer(this));
	return IO_OK;
}

MGMaterialDlg::~MGMaterialDlg()
{
	mat->paramDlg = NULL;

	if (texBut) ReleaseICustButton (texBut);
	if (pasteBut) ReleaseICustButton (pasteBut);
	if (camBut) ReleaseICustButton (camBut);
	if (convBut) ReleaseICustButton (convBut);
	if (stdMap) DestroyMParamMap(stdMap); 
	matType = NULL;
}

void MGMaterial::Shade (ShadeContext &sc)
{
	Colour modulus = Colour(1,1,1), lighting = Colour(0,0,0);
	Colour add = Colour(0,0,0);
	Point3 R = sc.ReflectVector();
	float aMul = 1.f;
	float spec = material.specExponent * 2.f;
	
	if (texture && sc.doMaps) {
		AColor col = texture->EvalColor (sc);
		modulus.r = col.r;
		modulus.g = col.g;
		modulus.b = col.b;
		aMul = col.a;
	}
	if (pasted && sc.doMaps) {
		AColor col = pasted->EvalColor (sc);
		modulus.r = (1.f - material.filterAmt) * modulus.r + material.filterAmt * col.r;
		modulus.g = (1.f - material.filterAmt) * modulus.g + material.filterAmt * col.g;
		modulus.b = (1.f - material.filterAmt) * modulus.b + material.filterAmt * col.b;
	}
	if (material.flags & RDMF_NOLIGHTING) {
		lighting = Colour (1,1,1);
	} else {
		for (int i = 0; i < sc.nLights; ++i) {
			LightDesc *l = sc.Light(i);
			Colour c;
			float NL, diffCoef;
			Point3 L;
			if (l->Illuminate (sc, sc.Normal(), c, L, NL, diffCoef)) {
				if (NL <= 0.0f)
					continue;
				if (l->affectDiffuse)
					lighting += diffCoef * c;
				if (l->affectSpecular && material.specAmount > 0.f) {
					float sp = DotProd(L,R);
					if (sp>0.0f) {
						sp = sp / (spec - sp*spec + sp);
						add += (sp*material.specAmount*(1.f/100.f)) * c;
					}
				}
				
			}
		}
	}
	sc.out.c = (material.diffuse * lighting) * modulus + (add * material.specular);
	if (sc.out.c.r > 1.f)
		sc.out.c.r = 1.f;
	if (sc.out.c.g > 1.f)
		sc.out.c.g = 1.f;
	if (sc.out.c.b > 1.f)
		sc.out.c.b = 1.f;
	sc.out.t = Colour(material.opacity*aMul, material.opacity*aMul, material.opacity*aMul);
	sc.out.ior = 1.f;
	sc.out.gbufId = 0;
	if (material.flags & MF_TRANSPARENCY_PUNCHTHRU)
		sc.out.t = Colour(1.f,1.f,1.f);
}

