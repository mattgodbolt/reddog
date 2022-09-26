/*
 * Render/Text.c
 * Text rendering routines
 */


#include "..\RedDog.h"
#include "..\View.h"
#include <stdarg.h>
#include "Internal.h"
#include "player.h"
#include "..\Input.h"
// Oh dear the hacks are getting worse and worse...
#include "..\VMSStruct.h"
#include "..\LevelDepend.h"
#include "..\Layers.h"

extern int GameVMS, GameProfile, GameDifficulty;

extern Uint32 SavedPadPress, SavedPadPush;
extern float SavedJoyY;
extern bool BeingBriefed;

#define OVERLAY_MIN_X	32.f
#define OVERLAY_MIN_Y	64.f
#define OVERLAY_SIZE_X	100.f
#define OVERLAY_SIZE_Y	100.f

#define OVERLAY_MID_X	((OVERLAY_SIZE_X * 0.5f) + OVERLAY_MIN_X)
#define OVERLAY_MID_Y	((OVERLAY_SIZE_Y * 0.5f) + OVERLAY_MIN_Y)

#define BAR_MIN_X		64.f
#define BAR_MIN_Y		96.f
#define BAR_SIZE_X		((PHYS_SCR_X - 64.f) - BAR_MIN_X)
#define BAR_SIZE_Y		130.f
#define BAR_COLOUR		0x40efefff

#ifndef NO_TEXT
#define NO_TEXT 0
#endif

#define LOCKON_XOFFSET 128
#define LOCKON_YOFFSET 64
#define TARGET_XOFFSET 128
#define TARGET_YOFFSET 32


#define CHAR_SIZE 16.f

static Material	psTextMat = { 0, 0, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_CLAMP_U | MF_CLAMP_V };
static Material	cursMat = { 0, 0, 0.f, 0.f, MF_ALPHA | MF_TWO_SIDED };
static Material	hudMatInside	= { 0, 0, 0.f, 0.f, MF_TEXTURE_BILINEAR| MF_ALPHA };
static Material	hudMatBorder	= { 0, 0, 0.f, 0.f, MF_TEXTURE_BILINEAR| MF_ALPHA };
static Material	BossHealth	= { 0, 0, 0.f, 0.f, MF_TEXTURE_BILINEAR| MF_ALPHA };
//static Material	HydroBossInside	= { 0, 0, 0.f, 0.f, MF_TEXTURE_BILINEAR| MF_ALPHA };
//static Material	HydroBossOutside	= { 0, 0, 0.f, 0.f, MF_TEXTURE_BILINEAR| MF_ALPHA };
static Material	hudMatNormalAim	= { 0, 0, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA };
static Material	hudMatNormalAimFire	= { 0, 0, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA };
static Material	hudMatMissileAim1	= { 0, 0, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA };
static Material	hudMatMissileAim2	= { 0, 0, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA };
static Material	hudMatMissileAim3	= { 0, 0, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA };
static Material	healthCol[3]		= {
	{ 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA },
	{ 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA },
	{ 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA }
};
static Material	healthFrame[3]		= {
	{ 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA },
	{ 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA },
	{ 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA }
};
static Material	lifeMat				= { 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA };
static Material	shieldCol			= { 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA };
static Material	adrenalinCol		= { 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA };
static Material	shieldFrame			= { 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA };
static Material	powerUpMat			= { 0, 0, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_TWO_SIDED };
static Material	powerUpMatGlow		= { 0, 0, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_TWO_SIDED };
Material	mpHUDMat			= { 0, 0, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_TWO_SIDED };
static Material	numberMat			= { 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_BILINEAR | MF_ALPHA | MF_TWO_SIDED };
static Material	sparkTexture	= { 0xa5ffffff, 0xffffffff, 0.f, 0.f, MF_TWO_SIDED};
static Material	textureSPSecondaryWeapon1	= { 0xa5ffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA };
static Material	textureSPSecondaryWeapon2	= { 0xa5ffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA };
static Material	textureSPSecondaryWeapon3	= { 0xa5ffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA };
static Material	textureSPSecondaryWeapon4	= { 0xa5ffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA };
static Material	textureSPSecondaryWeapon5	= { 0xa5ffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA };
static Material	textureSPSecondaryWeapon6	= { 0xa5ffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA };
static Material	textureSPSecondaryWeapon8	= { 0xa5ffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA };
static Material	textureSPSecondaryWeapon9	= { 0xa5ffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA };
static Material	textureSPSecondaryWeapon10	= { 0xa5ffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA };
static Material	textureSPSecondaryWeapon11	= { 0xa5ffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA };
static Material	textureSPSecondaryWeapon12	= { 0xa5ffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA };
static Material	textureSPSecondaryWeapon13	= { 0xa5ffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA };
extern Material	textureSecondaryWeapon1;
extern Material	textureSecondaryWeapon2;
extern Material	textureSecondaryWeapon3;
extern Material	textureSecondaryWeapon4;
extern Material	textureSecondaryWeapon5;
extern Material	textureSecondaryWeapon6;
extern Material	textureSecondaryWeapon7;
extern Material	textureSecondaryWeapon9;
extern Material	textureSecondaryWeapon10;
extern Material	textureSecondaryWeapon11;
extern Material	textureSecondaryWeapon12;
extern Material	textureSecondaryWeapon13;
Material	textureShootMe	= { 0xa5ffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA };

extern KMDWORD usrGlobalWork[];
extern void DrawCursor (float x, float y);

static Material argoLogo				= { 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA };
Material	youWinMat					= { 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA };
Material	textureLockOnMessage		= { 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA };
Material	textureNoTargetMessage		= { 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA };
Material	textureGreenBar				= { 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA };
Material	textureTargettingMessage	= { 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA };
Material	textureOrangeBar			= { 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA };
Material	textureRedBar				= { 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA };
Material	textureCharging				= { 0xffffffff, 0xffffffff, 0.f, 0.f, MF_TEXTURE_NO_FILTER | MF_ALPHA };
Material	fadeMat						= { 0, 0, 0.f, 0.f, MF_ALPHA | MF_TWO_SIDED};
Material	addMat						= { 0, 0, 0.f, 0.f, MF_ALPHA | MF_TRANSPARENCY_ADDITIVE };
Material	opaqueNonTextured			= { 0, 0, 0.f, 0.f, 0};
extern Material	fragMats[FT_MAX];
extern Material flareMat, lineMat, bulletHoleMat[], skidMat, lensFlareMat[], sunMat, expMat;
extern Material lightningMat, bombMat, snowMaterial;
extern	float	health;

static float FrontNumber = 0;

static float psMultiplier = 1.f;
void psSetMultiplier (float scale)
{
	psMultiplier = 1.f / scale;
}
float psGetMultiplier (void)
{
	return psMultiplier;
}


/* Text initialisation */

static struct {
	Material		*mat;
	KMVERTEXCONTEXT	*bC;
	Uint32			GBIX;
} FixUpList[] = {
	{ &fragMats[0],			&sfxContext,		0x57b383e2 },
	{ &fragMats[1],			&sfxContext,		0x57b383e2 },
	{ &fragMats[2],			&sfxContext,		0x57b383e2 },
	{ &cursMat,				&sfxContext,		0x00000000 },
	{ &lineMat,				&sfxContext,		0x00000000 },
	{ &lightningMat,		&sfxContext,		0x00000000 },
	{ &fadeMat,				&sfxContext,		0x00000000 },
	{ &addMat,				&sfxContext,		0x00000000 },
	{ &opaqueNonTextured,	&sfxContext,		0x00000000 },
	{ &sparkTexture,		&sfxContext,		0x00000000 },

// Remember to add the dependency in Render\Render.mak
/*	{ &BossHealth,			&objContext,		
#include "p:\game\texmaps\HUD\NEWHUD\BossHealth.h"
	},*/
	{ &flareMat,			&objContext,		
#include "p:\game\texmaps\specialfx\flares\flare1.h"
	},
/*	{ &hudMatBorder,			&objContext,		
#include "p:\game\texmaps\HUD\PANEL_BORDER_NEW.h"
	},
	{ &hudMatInside,			&objContext,		
#include "p:\game\texmaps\HUD\PANEL_INSIDE_NEW.h"
	},*/
	{ &hudMatNormalAim,			&objContext,		
#include "p:\game\texmaps\HUD\NORMALAIM.h"
	},
	{ &hudMatNormalAimFire,			&objContext,		
#include "p:\game\texmaps\HUD\NORMALAIMFIRE.h"
	},
	{ &hudMatMissileAim1,			&objContext,		
#include "p:\game\texmaps\HUD\MISSILEAIM1.h"
	},
	{ &hudMatMissileAim2,			&objContext,		
#include "p:\game\texmaps\HUD\MISSILEAIM2.h"
	},
	{ &hudMatMissileAim3,			&objContext,		
#include "p:\game\texmaps\HUD\MISSILEAIM3.h"
	},

	// Decals
	{ &bulletHoleMat[0],	&objContext,
#include "p:\game\texmaps\specialfx\decals\GndHit01.h"
	},
	{ &bulletHoleMat[1],	&objContext,
#include "p:\game\texmaps\specialfx\decals\GunHit01.h"
	},
	{ &bulletHoleMat[2],	&objContext,
#include "p:\game\texmaps\specialfx\decals\GunHit02.h"
	},
	{ &bulletHoleMat[3],	&objContext,
#include "p:\game\texmaps\specialfx\decals\GunHit04.h"
	},
	// Skid mark
	{ &skidMat,				&scapeContext,
#include "p:\game\texmaps\specialfx\skidmark01\skidmark02.h"
	},
	// Lens flares
	{ &lensFlareMat[0],	&newObjContext,
#include "p:\game\texmaps\SpecialFX\LenzFlareComponents\Flz00.h"
	},
	{ &lensFlareMat[1],	&newObjContext,
#include "p:\game\texmaps\SpecialFX\LenzFlareComponents\Flz01.h"
	},
	{ &lensFlareMat[2],	&newObjContext,
#include "p:\game\texmaps\SpecialFX\LenzFlareComponents\Flz02.h"
	},
	{ &lensFlareMat[3],	&newObjContext,
#include "p:\game\texmaps\SpecialFX\LenzFlareComponents\Flz03.h"
	},
	{ &lensFlareMat[4],	&newObjContext,
#include "p:\game\texmaps\SpecialFX\LenzFlareComponents\Flz03a.h"
	},
	{ &lensFlareMat[5],	&newObjContext,
#include "p:\game\texmaps\SpecialFX\LenzFlareComponents\Flz04.h"
	},
	{ &lensFlareMat[6],	&newObjContext,
#include "p:\game\texmaps\SpecialFX\LenzFlareComponents\Flz05.h"
	},
	{ &lensFlareMat[7],	&newObjContext,
#include "p:\game\texmaps\SpecialFX\LenzFlareComponents\Flz06.h"
	},
	{ &sunMat,			&newObjContext,
#include "p:\game\texmaps\SpecialFX\LenzFlareComponents\SunFlare.h"
	},
	{ &expMat,			&newObjContext,
#include "p:\game\texmaps\SpecialFX\LenzFlareComponents\SunFlare.h"
	},
	{ &powerUpMat,	&objContext,		
#include "p:\game\texmaps\hud\chargeup1.h"
	},
	{ &powerUpMatGlow,	&objContext,		
#include "p:\game\texmaps\hud\hudcharge2.h"
	},
	{ &numberMat,			&newObjContext, 
#include "p:\game\texmaps\hud\newhud\numbers.h"
	},
	{ &healthCol[0],	&newObjContext, 
#include "p:\game\texmaps\hud\newHUD\healthcolour1.h"
	},
	{ &healthFrame[0],	&newObjContext, 
#include "p:\game\texmaps\hud\newHUD\healthFrame1.h"
	},
	{ &healthCol[1],	&newObjContext, 
#include "p:\game\texmaps\hud\newHUD\healthcolour2.h"
	},
	{ &healthFrame[1],	&newObjContext, 
#include "p:\game\texmaps\hud\newHUD\healthFrame2.h"
	},
	{ &healthCol[2],	&newObjContext, 
#include "p:\game\texmaps\hud\newHUD\healthcolour3.h"
	},
	{ &healthFrame[2],	&newObjContext, 
#include "p:\game\texmaps\hud\newHUD\healthFrame3.h"
	},
	{ &lifeMat,			&newObjContext, 
#include "p:\game\texmaps\hud\newHUD\minitank.h"
	},
	{ &shieldCol,			&sfxContext, 
#include "p:\game\texmaps\hud\newHUD\SheildColour.h"
	},
	{ &adrenalinCol,			&sfxContext, 
#include "p:\game\texmaps\hud\newHUD\AdrenalinColour.h"
	},
	{ &shieldFrame,			&newObjContext, 
#include "p:\game\texmaps\hud\newHUD\SheildFrame.h"
	},
	{ &textureSPSecondaryWeapon1, &objContext, 
#include "p:\game\texmaps\hud\SPsecondaryweapon1.h"
	},
	{ &textureSPSecondaryWeapon2, &objContext, 
#include "p:\game\texmaps\hud\SPsecondaryweapon2.h"
	},
	{ &textureSPSecondaryWeapon3, &objContext, 
#include "p:\game\texmaps\hud\SPsecondaryweapon3.h"
	},
	{ &textureSPSecondaryWeapon4, &objContext, 
#include "p:\game\texmaps\hud\SPsecondaryweapon4.h"
	},
	{ &textureSPSecondaryWeapon6, &objContext, 
#include "p:\game\texmaps\hud\SPsecondaryweapon6.h"
	},
	{ &textureSPSecondaryWeapon9, &objContext, 
#include "p:\game\texmaps\hud\SPsecondaryweapon9.h"
	},
	{ &textureSPSecondaryWeapon10, &objContext, 
#include "p:\game\texmaps\hud\SPsecondaryweapon10.h"
	},
	{ &textureSPSecondaryWeapon11, &objContext, 
#include "p:\game\texmaps\hud\SPsecondaryweapon11.h"
	},
	{ &textureSPSecondaryWeapon12, &objContext, 
#include "p:\game\texmaps\hud\SPsecondaryweapon12.h"
	},
	{ &textureSPSecondaryWeapon13, &objContext, 
#include "p:\game\texmaps\hud\SPsecondaryweapon13.h"
	},
	{ &textureShootMe, &newObjContext, 
#include "p:\game\texmaps\hud\shootMe.h"
	},
	{ &argoLogo, &newObjContext,
#include "p:\game\texmaps\hud\ArgoLogo.h"
	},
	{ &psTextMat, &sfxContext,
#include "p:\game\texmaps\frontend\abet(dave)1.h"
	},
	{ &snowMaterial, &sfxContext,
#include "p:\game\texmaps\misc\snow.h"
	},
};

void fixUpMaterial(Material *mat, KMVERTEXCONTEXT *baseContext, int num)
{
	mat->surf.GBIX = num;
//	mat->flags |= MF_FRONT_END;
	rFixUpMaterial (mat, baseContext);
}

void tInit (void)
{
	KMSTATUS ok;
	Stream *texStream;
	int i;

	texStream = sOpen ("SYSTEM.TXP");
	sSetBuf (texStream, GameHeap->end - 1024*1024, 1024*1024);
	texSetDMABuffer (GameHeap->end - 1500*1024 - 1024*1024, 1500*1024); 
	texInit();
	texLoad (texStream);
	sClose (texStream);
	rInitFire();
//	bootRomInit();
	texSetHWM();

	for (i = 0 ; i < asize(FixUpList); ++i)
		fixUpMaterial (FixUpList[i].mat, FixUpList[i].bC, FixUpList[i].GBIX);
}

typedef enum {
	ENGLISH,
	GERMAN,
	FRENCH,
	SPANISH,
	ITALIAN,
	MAX_LANG
} Language;

static Language GetLang(char *name)
{
#define FINDLANG(a) if (!strcmp(name, #a)) return a
	FINDLANG(ENGLISH);
	FINDLANG(GERMAN);
	FINDLANG(FRENCH);
	FINDLANG(SPANISH);
	FINDLANG(ITALIAN);
	dAssert (NULL, "Unknown language!");
	return ENGLISH;
}

typedef struct tagLangEntry
{
	struct tagLangEntry *next;
	Language	lang;
	char		*token;
	char		*translation;
} LangEntry;

// Faster lookup; keyed off 'a-z' lists
LangEntry *langEntry[26];
Language CurrentLanguage = GERMAN;

const char *LangLookup (const char *token)
{
	static char LangLookupBuf[1024];
	char TokenBuf[64], *tokPtr;
	char *tokScan;
	LangEntry *l;
	tokScan = token;
	tokPtr = TokenBuf;
	while (*tokScan && *tokScan != ' ')
		*tokPtr++ = *tokScan++;
	*tokPtr++ = 0;
	dAssert (*token >= 'A' && *token <= 'Z', "Invalid token");
	// Firstly look for the selected language
	if (CurrentLanguage != ENGLISH) {
		for (l = langEntry[*token - 'A']; l; l = l->next) {
			if (!strcmp (TokenBuf, l->token) && l->lang == CurrentLanguage) {
				strcpy (LangLookupBuf, l->translation);
				if (*tokScan) {
					strcat (LangLookupBuf, tokScan);
					return LangLookupBuf;
				} else {
					return l->translation;
				}
			}
		}
	} 
	for (l = langEntry[*token - 'A']; l; l = l->next) {
		if (!strcmp (TokenBuf, l->token)) {
			strcpy (LangLookupBuf, l->translation);
			if (*tokScan) {
				strcat (LangLookupBuf, tokScan);
				return LangLookupBuf;
			} else {
				return l->translation;
			}
		}
	}
	dAssert (FALSE, "Unable to find token");
	return "!!NO TEXT!!";
}

static char buffer[512];
static const char *GetStr (Stream *s)
{
	char *ptr = buffer;
	char c;

	sRead (ptr, 1, s);
	while (isspace(*ptr))
		sRead (ptr, 1, s);

	do {
		ptr++;
		sRead (ptr, 1, s);
		if (isspace(*ptr)) {
			*ptr = '\0';
			break;
		}
	} while (1);

	return buffer;
}
static const char *GetLine (Stream *s)
{
	char *ptr = buffer;
	char c;

	sRead (ptr, 1, s);
	while (isspace(*ptr))
		sRead (ptr, 1, s);

	do {
		ptr++;
		sRead (ptr, 1, s);
		if (*ptr=='\n' || *ptr=='\r') {
			*ptr = '\0';
			break;
		}
	} while (1);

	return buffer;
}
extern char *strdup (char *str);

void tLangInit (void)
{
	Stream *s;
	char *str;
	Sint32 Lang;
	Language CurLang = ENGLISH;

	syCfgGetLanguage (&Lang);
	switch (Lang) {
	default:
	case SYD_CFG_ENGLISH:
		CurrentLanguage = ENGLISH;
		break;
	case SYD_CFG_GERMAN:
		CurrentLanguage = GERMAN;
		break;
	case SYD_CFG_FRENCH:
		CurrentLanguage = FRENCH;
		break;
	case SYD_CFG_SPANISH:
		CurrentLanguage = SPANISH;
		break;
	case SYD_CFG_ITALIAN:
		CurrentLanguage = ITALIAN;
		break;
	}
	s = sOpen ("LANG.TXT");
	do {
		str = GetStr (s);
		if (!strcmp (str, "[END]"))
			break;
		if (str[0] == '#') {
			// Skip the comment
			GetLine(s);
		} else if (str[0] == '[') {
			// Language change
			str[strlen(str)-1] = '\0';
			CurLang = GetLang (str+1);
		} else {
			// English is *guaranteed* to be above all the others, so replace
			// the English where needs be
			if (CurrentLanguage == CurLang || CurLang == ENGLISH) {
				LangEntry *l;

				dAssert (*str >= 'A' && *str <= 'Z', "Invalid token");
				// Check for duplicates
				for (l = langEntry[*str-'A']; l; l = l->next) {
					if (!strcmp (l->token, str))
						break;
				}
				// Not there?
				if (l == NULL) {
					l = syMalloc (sizeof (LangEntry));
					dAssert (l, "Out of RAM");
					l->token = strdup (str);
					// Chain onto the appropriate list
					l->next = langEntry[*str-'A'];
					l->lang = CurLang;
					langEntry[*str-'A'] = l;
				} else {
					// Already there - free the previous
					syFree (l->translation);
				}
				str = GetLine (s);
				l->translation = strdup (str);
			} else {
				str = GetLine(s); // Ignore 
			}
		}
	} while (1);
	sClose (s);
}

/* Draw a string */
void tPrintf (float x, float y, char *fmt, ...)
{
	char buf[1024], *ptr;
	int nBytes;
	va_list args;

#if NO_TEXT
	return;
#endif
	va_start (args, fmt);
	(void)vsprintf (buf, fmt, args);
	va_end (args);

	if (buf[0] == '\0')
		return;

	x*=24+1;y*=32+1;
	psSetAlpha (1);
	psSetColour (0xffffff);
	psDrawString (buf, x, y);
}

/*
 * Draw a sprite
 */
static void rSpriteInternal (float x, float y, float z, float xS, float yS, float rZ)
{
	register float xScale = xS, yScale = yS;
	register float s, c;
	SinCosVal val;

	SinCos (rZ, &val);
	s = val.sin; c = val.cos;

	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip (&vertBuf);
#define setvertex(A,X,Y,U,V) \
	kmxxSetVertex_7 (A,	x + ((X)*c - (Y)*s), y + ((X)*s + (Y)*c), z, U, V, 1, 0)
	setvertex (KM_VERTEXPARAM_NORMAL,		-xScale, -yScale, 0, 0);
	setvertex (KM_VERTEXPARAM_NORMAL,		-xScale,  yScale, 0, 1);
	setvertex (KM_VERTEXPARAM_NORMAL,		 xScale, -yScale, 1, 0);
	setvertex (KM_VERTEXPARAM_ENDOFSTRIP,	 xScale,  yScale, 1, 1);
	kmxxReleaseCurrentPtr (&vertBuf);
}

void rDrawBumpMap (Object *obj, float k1, float k2, float k3, float q)
{
/*	while (obj && obj->model == NULL)
		obj = &obj->child[0];

	rSetMaterial (&obj->model->material[0]);
*/
	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip (&vertBuf);

	kmxxSetVertex_5 (KM_VERTEXPARAM_NORMAL,		200, 200, 1000, 0, 0, 1, 1, 1, 1, k1, k2, k3, q);
	kmxxSetVertex_5 (KM_VERTEXPARAM_NORMAL,		200, 400, 1000, 0, 1, 1, 1, 1, 1, k1, k2, k3, q);
	kmxxSetVertex_5 (KM_VERTEXPARAM_NORMAL,		600, 200, 1000, 1, 0, 1, 1, 1, 1, k1, k2, k3, q);
	kmxxSetVertex_5 (KM_VERTEXPARAM_ENDOFSTRIP, 600, 400, 1000, 1, 1, 1, 1, 1, 1, k1, k2, k3, q);

	kmxxReleaseCurrentPtr (&vertBuf);
}

void rSprite (Point3 *pos, float xS, float yS, float rotZ, Object *obj)
{
	Point p;
	float x, y, z;
	float xScale, yScale;

	/* Find the active material and set it */
	while (obj && obj->model == NULL)
		obj = &obj->child[0];

	dAssert (obj != NULL, "Unable to find a material");
	if (obj == NULL)
		return;

	*(Point3 *)&p = *pos;
	p.w = 1.f;
	/* Ensure modelToScreen matrix is loaded into XMTRX */
	mLoad (&mWorldToScreen);
	mMul (&modelToScreen, &mCurMatrix);
	mLoad (&modelToScreen);
	mPoint (&p, &p);

	if (p.z < 0 || p.z > p.w)
		return;

	if (rGlobalModelFlags & MODELFLAG_GLOBALTRANS)
		rSetMaterialTrans (&obj->model->material[0]);
	else
		rSetMaterial (&obj->model->material[0]);

	usrGlobalWork[2] = (usrGlobalWork[2] & ~0x00c00000) | 0x00800000;

	z = 1.f / p.w;
	x = X_MID + pViewSize.x * p.x * z;
	y = Y_MID + pViewSize.y * p.y * z;

	xScale = xS * z * mPersp.m[0][0] * pViewSize.x;
	yScale = yS * z * mPersp.m[1][1] * pViewSize.y;

	rSpriteInternal (x, y, z, xScale, yScale, rotZ);
}

void rSpriteMtl (Point3 *pos, float xS, float yS, float rotZ, Material *mat)
{
	Point p;
	float x, y, z;
	float xScale, yScale;

	*(Point3 *)&p = *pos;
	p.w = 1.f;
	/* Ensure modelToScreen matrix is loaded into XMTRX */
	mLoad (&mWorldToScreen);
	mMul (&modelToScreen, &mCurMatrix);
	mLoad (&modelToScreen);
	mPoint (&p, &p);

	if (p.z < 0)
		return;

	rSetMaterial (mat);

//	kmChangeContextFogMode (KM_NOFOG);
	usrGlobalWork[2] = (usrGlobalWork[2] & ~0x00c00000) | 0x00800000;

	z = 1.f / p.w;
	x = X_MID + pViewSize.x * p.x * z;
	y = Y_MID + pViewSize.y * p.y * z;

	xScale = xS * z * pViewSize.x * 2;
	yScale = yS * z * pViewSize.y * 2;

	rSpriteInternal (x, y, z, xScale, yScale, rotZ);
}

void rSpriteNoZScale (Point3 *pos, float xS, float yS, Object *obj)
{
	Point p;
	float x, y, z;
	float xScale, yScale;

	/* Find the active material and set it */
	while (obj && obj->model == NULL)
		obj = &obj->child[0];

	dAssert (obj != NULL, "Unable to find a material");
	if (obj == NULL)
		return;

	*(Point3 *)&p = *pos;
	p.w = 1.f;
	/* Ensure modelToScreen matrix is loaded into XMTRX */
	mLoad (&mWorldToScreen);
	mMul (&modelToScreen, &mCurMatrix);
	mLoad (&modelToScreen);
	mPoint (&p, &p);

	if (p.z < 0 || p.z > p.w)
		return;

	rSetMaterial (&obj->model->material[0]);
/*	kmChangeContextColorType (KM_PACKEDCOLOR); */
//	kmChangeContextFogMode (KM_NOFOG);
	usrGlobalWork[2] = (usrGlobalWork[2] & ~0x00c00000) | 0x00800000;

	z = 1.f / p.w;
	x = X_MID + X_MID * p.x * z;
	y = Y_MID + Y_MID * p.y * z;

	xScale = xS * mPersp.m[0][0] * X_MID;
	yScale = yS * mPersp.m[1][1] * Y_MID;

	rSpriteInternal (x, y, z, xScale, yScale, 0.f);
}

void rSprite2D (float x, float y, Object *obj)
{
	float size;

	/* Find the active material and set it */
	while (obj && obj->model == NULL)
		obj = &obj->child[0];
	
	dAssert (obj != NULL, "Unable to find a material");
	if (obj == NULL)
		return;

	rSetMaterial (&obj->model->material[0]);
//	kmChangeContextFogMode (KM_NOFOG);
	usrGlobalWork[2] = (usrGlobalWork[2] & ~0x00c00000) | 0x00800000;

	size = (float)obj->model->material[0].surf.surfaceDesc->u0.USize * 0.5f;
	rSpriteInternal (x, y, 10000.f, size, size, 0.f);
}

void rSprite2DMtl (float x, float y, Material *mat, float scale, float rZ)
{
	float size;
	
	rSetMaterial (mat);
//	kmChangeContextFogMode (KM_NOFOG);
	usrGlobalWork[2] = (usrGlobalWork[2] & ~0x00c00000) | 0x00800000;

	size = (float)mat->surf.surfaceDesc->u0.USize * 0.5f * scale;
	rSpriteInternal (x, y, 10000.f + FrontNumber++, size, size, rZ);
	if (FrontNumber > 200)
		FrontNumber = 0;
}

// Draw a hanging bar doobry
static void DrawHUDBar (float x, float y, float xScale, float yScale, float amt, float a, float r, float g, float b)
{
	float U;
	float T = a;
	if (amt < 0.01f)
		return;

	amt = MIN(amt, 1.f);

	amt = (amt* 170.f/180.f) + (5.f / 180.f);

	if (xScale < 0)
		U = -0.5f;
	else
		U = 0.5f;

	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip (&vertBuf);

	if (amt >= 0.75f) {
		amt = -tan((1.f - amt) * PI);
		kmxxSetVertex_5 (KM_VERTEXPARAM_NORMAL,		x + xScale * amt, y - yScale, 
			10, 0.5f + U * amt, 0, T, r, g, b, 0, 0, 0, 0);
		kmxxSetVertex_5 (KM_VERTEXPARAM_NORMAL,		x - xScale, y - yScale, 
			10, 0.5f - U, 0, T, r, g, b, 0, 0, 0, 0);
		kmxxSetVertex_5 (KM_VERTEXPARAM_NORMAL,		x         , y         , 
			10, 0.5f, 0.5f, T, r, g, b, 0, 0, 0, 0);
		kmxxSetVertex_5 (KM_VERTEXPARAM_NORMAL,		x - xScale, y + yScale, 
			10, 0.5f - U, 1, T, r, g, b, 0, 0, 0, 0);
		kmxxSetVertex_5 (KM_VERTEXPARAM_ENDOFSTRIP, x         , y + yScale, 
			10, 0.5f, 1.f, T, r, g, b, 0, 0, 0, 0);
	} else if (amt >= 0.5f) {
		amt = -tan((amt-0.5f) * PI);
		kmxxSetVertex_5 (KM_VERTEXPARAM_NORMAL,		x - xScale, y + yScale * amt, 
			10, 0.5f - U, 0.5f + 0.5f * amt, T, r, g, b, 0, 0, 0, 0);
		kmxxSetVertex_5 (KM_VERTEXPARAM_NORMAL,		x - xScale, y + yScale, 
			10, 0.5f - U, 1, T, r, g, b, 0, 0, 0, 0);
		kmxxSetVertex_5 (KM_VERTEXPARAM_NORMAL,		x         , y         , 
			10, 0.5f, 0.5f, T, r, g, b, 0, 0, 0, 0);
		kmxxSetVertex_5 (KM_VERTEXPARAM_ENDOFSTRIP, x         , y + yScale, 
			10, 0.5f, 1.f, T, r, g, b, 0, 0, 0, 0);
	} else if (amt >= 0.25f) {
		amt = tan((0.5f - amt) * PI);
		kmxxSetVertex_5 (KM_VERTEXPARAM_NORMAL,		x - xScale, y + yScale * amt, 
			10, 0.5f - U, 0.5f + 0.5f * amt, T, r, g, b, 0, 0, 0, 0);
		kmxxSetVertex_5 (KM_VERTEXPARAM_NORMAL,		x - xScale, y + yScale, 
			10, 0.5f - U, 1, T, r, g, b, 0, 0, 0, 0);
		kmxxSetVertex_5 (KM_VERTEXPARAM_NORMAL,		x         , y         , 
			10, 0.5f, 0.5f, T, r, g, b, 0, 0, 0, 0);
		kmxxSetVertex_5 (KM_VERTEXPARAM_ENDOFSTRIP, x         , y + yScale, 
			10, 0.5f, 1.f, T, r, g, b, 0, 0, 0, 0);
	} else {
		amt = tan(amt * PI);
		kmxxSetVertex_5 (KM_VERTEXPARAM_NORMAL,		x         , y         , 
			10, 0.5f, 0.5f, T, r, g, b, 0, 0, 0, 0);
		kmxxSetVertex_5 (KM_VERTEXPARAM_NORMAL,		x - xScale * amt, y + yScale, 
			10, 0.5f - U * amt, 1, T, r, g, b, 0, 0, 0, 0);
		kmxxSetVertex_5 (KM_VERTEXPARAM_ENDOFSTRIP, x         , y + yScale, 
			10, 0.5f, 1, T, r, g, b, 0, 0, 0, 0);
	}

	kmxxReleaseCurrentPtr (&vertBuf);
}

#define AMMO_V (117.f / 256.f)
#define AMMO_X 10.f
#define AMMO_Y 15.f
static float tNumCharZ = W_LAYER_AMMO;
static PKMDWORD tNumberChar (register PKMDWORD pkmCurrentPtr, float x, float y, int num, float xSize, float ySize)
{
	float xx, wid;
	float xScale;
	static const struct {
		float u, v;
		float uEnd;
	} Num[] = {
		{ 1.f,		8.f,		20.f },		// 0
		{ 22.f,		8.f,		32.f },		// 1
		{ 37.f,		8.f,		56.f },		// 2
		{ 58.f,		8.f,		76.f },		// 3
		{ 81.f,		8.f,		99.f },		// 4
		{ 100.f,	8.f,		119.f},		// 5
		{ 1.f,		60.f,		20.f },		// 6
		{ 22.f,		60.f,		42.f },		// 7
		{ 44.f,		60.f,		64.f },		// 8
		{ 65.f,		60.f,		84.f },		// 9
		{ 86.f,		60.f,		92.f },		// :
		{ 95.f,		60.f,		100.f },	// .
		{ 104.f,	60.f,		121.f }		// -
	};
	float u, uEnd, v, vEnd;

	xScale = MIN(1, xSize/24.f);

	// Centre within xSize
	wid = (Num[num].uEnd - Num[num].u) * xScale;
	xx = x + (xSize-wid)*0.5f;
	u = Num[num].u * (1.f / 128.f);
	v = Num[num].v * (1.f / 128.f);
	uEnd = Num[num].uEnd * (1.f / 128.f);
	vEnd = (Num[num].v+35) * (1.f / 128.f);
	kmxxSetVertex_7 (KM_VERTEXPARAM_NORMAL,		xx, y,					tNumCharZ, u,	v, 1, 0);
	kmxxSetVertex_7 (KM_VERTEXPARAM_NORMAL,		xx, y + ySize,			tNumCharZ, u,	vEnd, 1, 0);
	kmxxSetVertex_7 (KM_VERTEXPARAM_NORMAL,		xx + wid, y,			tNumCharZ, uEnd, v, 1, 0);
	kmxxSetVertex_7 (KM_VERTEXPARAM_ENDOFSTRIP, xx + wid, y + ySize,	tNumCharZ, uEnd, vEnd, 1, 0);
	return pkmCurrentPtr;
}

void tAmmo (float x, float y, int amt)
{
	// Set the material
	numberMat.diffuse.colour = 0xff00ff00;
	rSetMaterial (&numberMat);
	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip (&vertBuf);

	// Here we go:
	do {
		pkmCurrentPtr = tNumberChar (pkmCurrentPtr, x, y, amt % 10, AMMO_X, AMMO_Y);
		x -= AMMO_X;
		amt/= 10;
	} while (amt != 0);

	kmxxReleaseCurrentPtr (&vertBuf);
}

static struct {
	float r, g, b;
} Adren[] = 
{
	{ 1, 1, 0 },
	{ 1, 156.f/255.f, 0 },
	{ 1, 0, 0 },
	{ 1, 1, 1 },
};

void rAim (float x, float y, int type, int pn)
{
	float xScale = 32.f * X_SCALE, yScale = 32.f, nb, Scale;
	int i;

	if (player[pn].Player == NULL ||
		player[pn].Player->health <= 0.f)
		return;

	Scale = Multiplayer? 0.5f : 1.f;
	xScale *= Scale, yScale *= Scale;

	i = RANGE (0, (player[pn].PlayerWeaponPower), asize(Adren));

	switch(type)
	{
	case 0:
		rSetMaterial (&hudMatNormalAim);
		break;
	case 1:
		rSetMaterial (&hudMatNormalAimFire);
		break;
	case 2:
		rSetMaterial (&hudMatMissileAim1);
		break;
	case 3:
		rSetMaterial (&hudMatMissileAim2);
		break;
	case 4:
		rSetMaterial (&hudMatMissileAim3);
		break;
	}

	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip (&vertBuf);

	kmxxSetVertex_5 (KM_VERTEXPARAM_NORMAL,		x - xScale, y - yScale, 4.99, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0);
	kmxxSetVertex_5 (KM_VERTEXPARAM_NORMAL,		x - xScale, y + yScale, 4.99, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0);
	kmxxSetVertex_5 (KM_VERTEXPARAM_NORMAL,		x + xScale, y - yScale, 4.99, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0);
	kmxxSetVertex_5 (KM_VERTEXPARAM_ENDOFSTRIP, x + xScale, y + yScale, 4.99, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0);

	kmxxReleaseCurrentPtr (&vertBuf);

	if (player[pn].PlayerSecondaryWeapon.amount) {
		int amt = player[pn].PlayerSecondaryWeapon.amount;
		if (player[pn].PlayerSecondaryWeapon.type == BOMB &&
			player[pn].PlayerArmageddon == 1)
			amt--;
		if (!Multiplayer) {
			tAmmo (x - 40 * Scale, y - 30 * Scale, amt);
		}
	}
}


/*
 * Draw and open the text bar and overlays
 */
struct {
	Uint32		GBIX;
	Material	mat;
} overlay[] = 
{
	{ 
#include "p:\game\texmaps\reddog\static1.h"
		,{ 0, 0, 0.f, 0.f, MF_TEXTURE_BILINEAR| MF_ALPHA } },
	{ 
#include "p:\game\texmaps\hud\Commander.h"
		,{ 0, 0, 0.f, 0.f, MF_TEXTURE_BILINEAR| MF_ALPHA } },
	{ 
#include "p:\game\texmaps\hud\DropshipPilot.h"
		,{ 0, 0, 0.f, 0.f, MF_TEXTURE_BILINEAR| MF_ALPHA } },
	{ 
#include "p:\game\texmaps\hud\Hero.h"
		,{ 0, 0, 0.f, 0.f, MF_TEXTURE_BILINEAR| MF_ALPHA } },
	{ 
#include "p:\game\texmaps\hud\IntelligenceOfficer04.h"
		,{ 0, 0, 0.f, 0.f, MF_TEXTURE_BILINEAR| MF_ALPHA } },

};
static KMSURFACEDESC *FindMat(Object *o)
{
	if (o && o->model)
		return o->model->material[0].surf.surfaceDesc;
	else if (o && o->no_child)
		return FindMat(o->child);
	return NULL;
}
void rInitOverlays(void)
{
	int i;
	for (i = 0; i < 1; ++i) {
		overlay[i].mat.surf.GBIX = overlay[i].GBIX;
		rFixUpMaterial (&overlay[i].mat, &objContext);
	}
	for (;i < asize(overlay); ++i) {
		if (GameHeads[i]) {
			overlay[i].mat.surf.GBIX = texFindFromDesc(FindMat(GameHeads[i-1]->obj));
			rFixUpMaterial (&overlay[i].mat, &objContext);
		}
	}
}

//#define BAR_COLOUR		0xffffffff

// Display a score or time at a given position
void tScoreInternal (float x, float y, char *score, Uint32 colour, float z)
{
	float u, v;
	float oldZ = tNumCharZ;

	tNumCharZ = z;
	numberMat.diffuse.colour = colour;
	rSetMaterial (&numberMat);

	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip (&vertBuf);

	while (*score) {
		if (*score >= '0' && *score <= '9')
			pkmCurrentPtr = tNumberChar (pkmCurrentPtr, x, y, *score - '0', 24, 30);
		else if (*score == ':')
			pkmCurrentPtr = tNumberChar (pkmCurrentPtr, x, y, 10, 24, 35);
		else if (*score == '.')
			pkmCurrentPtr = tNumberChar (pkmCurrentPtr, x, y, 11, 24, 35);
		else if (*score == '-')
			pkmCurrentPtr = tNumberChar (pkmCurrentPtr, x, y, 12, 24, 35);
		x += 24.f;
		score++;
	}
	kmxxReleaseCurrentPtr (&vertBuf);
	tNumCharZ = oldZ;
}
void tScore (float x, float y, char *score, Uint32 colour)
{
	tScoreInternal (x, y, score, colour, W_LAYER_AMMO);
}
void tScoreTime (float x, float y, char *score, Uint32 colour)
{
	tScoreInternal (x, y, score, colour, W_LAYER_TIME);
}

#define HEALTH_WIDTH	256
#define HEALTH_HEIGHT_M	0.35
#define HEALTH_HEIGHT	(256 * HEALTH_HEIGHT_M)
#define HEALTH_CAP_U	(8.f / 64.f)
#define HEALTH_CAP_X	8
#define HEALTH_X		((8-12) * X_SCALE)
#define HEALTH_Y		(32-14)

void rKmxxSetVertex_3(int pn)
{
	Uint32	col = 0xffff0000;

	float amt, topV, topY, topTop, topTopV;
	float shieldX, shieldY, shieldHeight, shieldWidth;
	float left, right;
	Uint32	colTop, colBottom;
	char textBuffer[8];

	#define SHIELD_SIZE 256.f
	#define SHIELD_TOP_V (85.f / SHIELD_SIZE)
	#define SHIELD_BOT_V (220.f / SHIELD_SIZE)
	#define SHIELD_LEFT_U (18.f / SHIELD_SIZE)
	#define SHIELD_RIGHT_U (170.f / SHIELD_SIZE)
	#define SHIELD_HEIGHT (220-85)
	#define SHIELD_WIDTH (170-18)
	#define SHIELD_V_SUB ((220.f - 212.f) / SHIELD_SIZE)
	#define SHIELD_V_SCALE ((212.f - 98.f) / SHIELD_SIZE)

	shieldX = (7) * X_SCALE;
	shieldY = PHYS_SCR_Y - 30 - SHIELD_HEIGHT;
	shieldHeight = SHIELD_HEIGHT;
	shieldWidth = SHIELD_WIDTH;
	left = SHIELD_LEFT_U;
	right = SHIELD_RIGHT_U;

	amt = player[pn].PlayerShieldEnergy;
	amt = RANGE(0,amt,1);

	topV = SHIELD_BOT_V - SHIELD_V_SUB - amt * SHIELD_V_SCALE;
	topY = shieldY + shieldHeight - SHIELD_SIZE * (SHIELD_V_SUB + amt * SHIELD_V_SCALE) * (Multiplayer?0.5f:1.f);

	rSetMaterial (&shieldCol);
	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip (&vertBuf);

	kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		shieldX, topY, 10, left, topV, 0xffffffff, 0);
	kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		shieldX, shieldY + shieldHeight, 10, left, SHIELD_BOT_V, 0xffffffff, 0);
	kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		shieldX + shieldWidth, topY, 10, right, topV, 0xffffffff, 0);
	kmxxSetVertex_3 (KM_VERTEXPARAM_ENDOFSTRIP, shieldX + shieldWidth, shieldY + shieldHeight, 10, right, SHIELD_BOT_V, 0xffffffff, 0);
	
	kmxxReleaseCurrentPtr (&vertBuf);	

	sprintf(textBuffer, "%02d", (int)(player[pn].PlayerWeaponEnergy * 20.0));
	tScore (570, 290, textBuffer, 0xff00ff00);

	if (player[pn].PlayerWeaponCharge >= 1.0)
	{
		colTop = 0xff00ff00;
		colBottom = 0xff0000ff;
		amt = RANGE(0, player[pn].PlayerWeaponCharge - 1.0, 1);		
	}
	else if (player[pn].PlayerWeaponCharge >= 0.0f)
	{
		colTop = 0x00000000;
		colBottom = 0xff00ff00;
		amt = RANGE(0, player[pn].PlayerWeaponCharge, 1);
	}
			
	
	shieldX = PHYS_SCR_X - shieldX - SHIELD_WIDTH;
	
	
		
	topV = SHIELD_BOT_V - SHIELD_V_SUB - amt * SHIELD_V_SCALE;
	topY = shieldY + shieldHeight - SHIELD_SIZE * (SHIELD_V_SUB + amt * SHIELD_V_SCALE);
	topTop = shieldY + shieldHeight - SHIELD_SIZE * (SHIELD_V_SUB + SHIELD_V_SCALE);
	topTopV = SHIELD_BOT_V - SHIELD_V_SUB - SHIELD_V_SCALE;
	
	rSetMaterial (&adrenalinCol);
	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip (&vertBuf);

	kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		shieldX				 , topTop, 10 , 1.f - SHIELD_RIGHT_U, topTopV, colTop, 0);
	kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		shieldX				 , topY  , 10 , 1.f - SHIELD_RIGHT_U, topV	 , colTop, 0);
	kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		shieldX + shieldWidth, topTop, 10 , 1.f - SHIELD_LEFT_U , topTopV, colTop, 0);
	kmxxSetVertex_3 (KM_VERTEXPARAM_ENDOFSTRIP, shieldX + shieldWidth, topY	 , 10 , 1.f - SHIELD_LEFT_U , topV	 , colTop, 0);

	kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		shieldX				 , topY					 , 10, 1.f - SHIELD_RIGHT_U, topV		 , colBottom, 0);
	kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		shieldX				 , shieldY + shieldHeight, 10, 1.f - SHIELD_RIGHT_U, SHIELD_BOT_V, colBottom, 0);
	kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		shieldX + shieldWidth, topY					 , 10, 1.f - SHIELD_LEFT_U , topV		 , colBottom, 0);
	kmxxSetVertex_3 (KM_VERTEXPARAM_ENDOFSTRIP, shieldX + shieldWidth, shieldY + shieldHeight, 10, 1.f - SHIELD_LEFT_U , SHIELD_BOT_V, colBottom, 0);
	
	kmxxReleaseCurrentPtr (&vertBuf);
}

void rSecondaryWeapon(int a, float x0, float y0, float x1, float y1)
{

	float farX, farY, xSize, ySize;

	farX = x1; 
	farY = y0;
	xSize = x1 - x0; 
	ySize = y0 - y1;

	dAssert (Multiplayer, "Only works in multiplayer now");
	
	switch(a)
	{
	case 0:
		return;
	case 1:
		rSetMaterial (&textureSecondaryWeapon1);
		break;
	case 2:
		rSetMaterial (&textureSecondaryWeapon2);
		break;
	case 3:
		rSetMaterial (&textureSecondaryWeapon3);
		break;
		/*			case 4:
		rSetMaterial (&textureSecondaryWeapon4);
		break;*/
	case 5:
		rSetMaterial (&textureSecondaryWeapon5);
		break;
	case 6:
		rSetMaterial (&textureSecondaryWeapon6);
		break;
	case 7:
		rSetMaterial (&textureSecondaryWeapon7);
		break;
		/*			case 9:
		rSetMaterial (&textureSecondaryWeapon9);*/
		break;
	case 10:
		rSetMaterial (&textureSecondaryWeapon10);
		break;
	case 11:
		rSetMaterial (&textureSecondaryWeapon11);
		break;
	case 12:
		rSetMaterial (&textureSecondaryWeapon12);
		break;
	case 13:
		rSetMaterial (&textureSecondaryWeapon13);
		break;
	default:
		return;
	}	

	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip (&vertBuf);

	kmxxSetVertex_5 (KM_VERTEXPARAM_NORMAL,		farX - xSize, farY - ySize, W_LAYER_AMMO, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0);
	kmxxSetVertex_5 (KM_VERTEXPARAM_NORMAL,		farX - xSize, farY, W_LAYER_AMMO, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0);
	kmxxSetVertex_5 (KM_VERTEXPARAM_NORMAL,		farX, farY - ySize, W_LAYER_AMMO, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0);
	kmxxSetVertex_5 (KM_VERTEXPARAM_ENDOFSTRIP, farX, farY, W_LAYER_AMMO, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0);

	kmxxReleaseCurrentPtr (&vertBuf);
}



#define YOUWIN_WIDTH	200
#define YOUWIN_HEIGHT	80

void rYouWin (void)
{
	float x1, x2, y1, y2;

	x1 = currentCamera->screenMidX - (YOUWIN_WIDTH/2);
	x2 = x1 + YOUWIN_WIDTH;
	y1 = currentCamera->screenMidY - (YOUWIN_HEIGHT/2);
	y2 = y1 + YOUWIN_HEIGHT;

	rSetMaterial (&youWinMat);
	
	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip (&vertBuf);

	kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		x1, y1, W_LAYER_HUD_LINE,	0, 0, -1, 0);
	kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		x1, y2, W_LAYER_HUD_LINE,	0, 1, -1, 0);
	kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		x2, y1, W_LAYER_HUD_LINE,	1, 0, -1, 0);
	kmxxSetVertex_3 (KM_VERTEXPARAM_ENDOFSTRIP,	x2, y2, W_LAYER_HUD_LINE,	1, 1, -1, 0);

	kmxxReleaseCurrentPtr (&vertBuf);
}

void rLockOn(int pn, int time)
{
	#define CHARGE_WAIT 50.0
	float ox, oy, xSize, ySize, tempf, alpha, myScale;
	int col;
	
	if (Winner >= 0)
		return;

	alpha = 1.0f;

	tempf = ((float)time) / LOCKON_WAIT;

	if (NumberOfPlayers > 2)
		myScale = 0.75f;
	else
		myScale = 0.75f;

	alpha *= 255.0f;
	col = alpha;
	col <<= 24;
	col |= 0xffffff;

	xSize = 128 * X_SCALE * myScale;
	ySize = 32 * myScale;

	ox = currentCamera->screenMidX - (xSize * 0.5f);
	oy = currentCamera->screenMidY - (currentCamera->screenY * 0.5f) + 32 + (48 * myScale);
	

	if (((time < LOCKON_WAIT) && (time >= 0) && (time % 30 < 15)) ||
		(time == -1) || (time > LOCKON_WAIT))
	{
		rSetMaterial (&textureLockOnMessage);
		
		kmxxGetCurrentPtr (&vertBuf);
		kmxxStartVertexStrip (&vertBuf);

		kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		ox, oy, 10, 0, 0, col, 0);
		kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		ox, oy + ySize, 10, 0, 1, col, 0);
		kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		ox + xSize, oy, 10, 1, 0, col, 0);
		kmxxSetVertex_3 (KM_VERTEXPARAM_ENDOFSTRIP, ox + xSize, oy + ySize, 10, 1, 1, col, 0);
		
		kmxxReleaseCurrentPtr (&vertBuf);
	}

	if (time == -1)
		xSize = 128.0 * X_SCALE * myScale;
	else
		xSize = (((float)time) / LOCKON_WAIT) * 128.0 * X_SCALE * myScale;

	ySize = 16 * myScale;
	oy += 32 * myScale;

	rSetMaterial (&textureRedBar);
	
	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip (&vertBuf);

	kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		ox, oy, 10,
		0, 0, col, 0);
	kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		ox, oy + ySize, 10, 
		0, 1, col, 0);
	kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		ox + xSize, oy, 10,
		tempf, 0, col, 0);
	kmxxSetVertex_3 (KM_VERTEXPARAM_ENDOFSTRIP, ox + xSize, oy + ySize, 10,
		tempf, 1, col, 0);

	
	kmxxReleaseCurrentPtr (&vertBuf);
}

void rTargetting(int pn, int time)
{

	#define CHARGE_WAIT 50.0
	float ox, oy, xSize, ySize, tempf, alpha, myScale;
	int col;
	
	if (Winner >= 0)
		return;
	alpha = 1.0f;

	if (NumberOfPlayers > 2)
		myScale = 0.75f;
	else
		myScale = 0.75f;

	alpha *= 255.0f;
	col = alpha;
	col <<= 24;
	col |= 0xffffff;

	tempf = ((float)time) / LOCKON_WAIT;

	xSize = 128 * X_SCALE * myScale;
	ySize = 32 * myScale;

	ox = currentCamera->screenMidX - (xSize * 0.5f);
	oy = currentCamera->screenMidY - (currentCamera->screenY * 0.5f) + 32;
	

	rSetMaterial (&textureTargettingMessage);
	
	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip (&vertBuf);

	kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		ox, oy, 10, 0, 0, col, 0);
	kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		ox, oy + ySize, 10, 0, 1, col, 0);
	kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		ox + xSize, oy, 10, 1, 0, col, 0);
	kmxxSetVertex_3 (KM_VERTEXPARAM_ENDOFSTRIP, ox + xSize, oy + ySize, 10, 1, 1, col, 0);
	
	kmxxReleaseCurrentPtr (&vertBuf);

	xSize = (((float)time) / LOCKON_WAIT) * 128.0 * X_SCALE * myScale;

	ySize = 16 * myScale;
	oy += 32 * myScale;

	rSetMaterial (&textureOrangeBar);
	
	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip (&vertBuf);

	kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		ox, oy, 10,
		0, 0, col, 0);
	kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		ox, oy + ySize, 10, 
		0, 1, col, 0);
	kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		ox + xSize, oy, 10,
		tempf, 0, col, 0);
	kmxxSetVertex_3 (KM_VERTEXPARAM_ENDOFSTRIP, ox + xSize, oy + ySize, 10,
		tempf, 1, col, 0);

	
	kmxxReleaseCurrentPtr (&vertBuf);
}

void rCharging(int pn, int time)
{

	#define CHARGE_WAIT 50.0
	float ox, oy, xSize, ySize, tempf, alpha, myScale;
	int col;
	if (Winner >= 0)
		return;
	if (time > 50)
		alpha = 1.0f - (time - 50) * 0.1f;
	else
		alpha = 1.0f;

	if (NumberOfPlayers > 2)
		myScale = 0.75f;
	else
		myScale = 0.75f;

	alpha *= 255.0f;
	col = alpha;
	col <<= 24;
	col |= 0xffffff;

	if (time <= 50.0f)
		tempf = ((float)time) / CHARGE_WAIT;
	else
		tempf = 50.0f / CHARGE_WAIT;

	xSize = 128 * X_SCALE * myScale;
	ySize = 32 * myScale;

	ox = currentCamera->screenMidX - (xSize * 0.5f);
	oy = currentCamera->screenMidY - (currentCamera->screenY * 0.5f) + 32;
	

	rSetMaterial (&textureCharging);
	
	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip (&vertBuf);

	kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		ox, oy, 10, 0, 0, col, 0);
	kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		ox, oy + ySize, 10, 0, 1, col, 0);
	kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		ox + xSize, oy, 10, 1, 0, col, 0);
	kmxxSetVertex_3 (KM_VERTEXPARAM_ENDOFSTRIP, ox + xSize, oy + ySize, 10, 1, 1, col, 0);
	
	kmxxReleaseCurrentPtr (&vertBuf);

	if (time <= 50.0)
		xSize = (((float)time) / CHARGE_WAIT) * 128.0 * X_SCALE * myScale;
	else
		xSize = (((float)50.0f) / CHARGE_WAIT) * 128.0 * X_SCALE * myScale;

	ySize = 16 * myScale;
	oy += 32 * myScale;

	rSetMaterial (&textureGreenBar);
	
	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip (&vertBuf);

	kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		ox, oy, 10,
		0, 0, col, 0);
	kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		ox, oy + ySize, 10, 
		0, 1, col, 0);
	kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		ox + xSize, oy, 10,
		tempf, 0, col, 0);
	kmxxSetVertex_3 (KM_VERTEXPARAM_ENDOFSTRIP, ox + xSize, oy + ySize, 10,
		tempf, 1, col, 0);

	
	kmxxReleaseCurrentPtr (&vertBuf);
}


void rNoTarget(int pn)
{
	#define CHARGE_WAIT 50.0
	float ox, oy, xSize, ySize, tempf, alpha, myScale;
	int col;
	if (Winner >= 0)
		return;
	alpha = 1.0f;

	if (NumberOfPlayers > 2)
		myScale = 0.75f;
	else
		myScale = 0.75f;

	alpha *= 255.0f;
	col = alpha;
	col <<= 24;
	col |= 0xffffff;

	xSize = 128 * X_SCALE * myScale;
	ySize = 32 * myScale;

	ox = currentCamera->screenMidX - (xSize * 0.5f);
	oy = currentCamera->screenMidY - (currentCamera->screenY * 0.5f) + 32;
	

	rSetMaterial (&textureNoTargetMessage);
	
	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip (&vertBuf);

	kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		ox, oy, 10, 0, 0, col, 0);
	kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		ox, oy + ySize, 10, 0, 1, col, 0);
	kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		ox + xSize, oy, 10, 1, 0, col, 0);
	kmxxSetVertex_3 (KM_VERTEXPARAM_ENDOFSTRIP, ox + xSize, oy + ySize, 10, 1, 1, col, 0);
	
	kmxxReleaseCurrentPtr (&vertBuf);
}

void rSparks(void)
{
	SPARK	*spark;
	register float x, y, rW;
	register float x2, y2, rW2;
	Point p, p2;
	Point3 *sp;
	int	col;
	float	midX, midY;

	mLoadWithXYScale (&mWorldToScreen, pViewSize.x, pViewSize.y);
	midX = pViewMidPoint.x;
	midY = pViewMidPoint.y;

	spark = firstSpark;

	
	rSetMaterial (&sparkTexture);
	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip (&vertBuf);
	while(spark)
	{
		if ((spark->type == 6) && (spark->time > 115))
		{
			spark = spark->next;
			continue; 

		}

		sp = &spark->pos;
		mPointXYZ (&p, sp->x, sp->y, sp->z);
		sp = &spark->oldPos;
		mPointXYZ (&p2, sp->x, sp->y, sp->z);

		if ((p.w <= 0) || (p2.w <= 0))
		{
			spark = spark->next;
			continue; 
		}

		rW = 1.f / p.w;
		x = p.x * rW + midX;
		y = p.y * rW + midY;

		rW2 = 1.f / p2.w;
		x2 = p2.x * rW2 + midX;
		y2 = p2.y * rW2 + midY;
		//col = 0xffff|(((20 - spark->time) * 12)<<8);

		
		switch(spark->type)
		{
			case 1:
				if(spark->col.colour == 0)
				{
					kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x, y, rW, 0xffffffff);
					kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x2 , y2, rW2, 0xffffffff);
					kmxxSetVertex_0 (KM_VERTEXPARAM_ENDOFSTRIP, x + 2.0, y - 2.0, rW, 0xffffffff);
					spark->type = 3;
				}
				else
				{
					kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x, y, rW, spark->col.colour);
					kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x2 , y2, rW2, spark->col.colour);
					kmxxSetVertex_0 (KM_VERTEXPARAM_ENDOFSTRIP, x + 2.0, y - 2.0, rW, spark->col.colour);
				}
				break;
			case 2:
				kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x, y, rW, 0xffffffff);
				kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x2 , y2, rW2, 0xffffffff);
				kmxxSetVertex_0 (KM_VERTEXPARAM_ENDOFSTRIP, x + 2.0, y - 2.0, rW, 0xffffffff);
				spark->type = 4;
				break;
			case 3:
				kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x, y, rW, 0xffff0000);
				kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x2 , y2, rW2, 0xfffffe00);
				kmxxSetVertex_0 (KM_VERTEXPARAM_ENDOFSTRIP, x + 2.0, y - 2.0, rW, 0xffff0000);
				break;
			case 4:
				kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x, y, rW, 0xff0000ff);
				kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x2 , y2, rW2, 0xffffffff);
				kmxxSetVertex_0 (KM_VERTEXPARAM_ENDOFSTRIP, x + 2.0, y - 2.0, rW, 0xff0000ff);
				break;
			case 5:
				kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x, y, rW, 0xffffffff);
				kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x2 , y2, rW2, 0xffffffff);
				kmxxSetVertex_0 (KM_VERTEXPARAM_ENDOFSTRIP, x + 2.0, y - 2.0, rW, 0xffffffff);
				break;
			case 6:	 //SPARKHOOVER
			case 7:	 //SPARKWHIRL
			case 8:	 //SPARKCOLWHIRL
			case 9:
			case 10:
			case 11: //SPARKCHARGEUP
			case 12: //SPARKPUFF
				kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x, y, rW, spark->col.colour);
				kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,		x2 , y2, rW2, spark->col.colour);
				kmxxSetVertex_0 (KM_VERTEXPARAM_ENDOFSTRIP, x + 2.0, y - 2.0, rW, spark->col.colour);
				break;
			default:
				break;

		}
		spark = spark->next;
	}
	kmxxReleaseCurrentPtr (&vertBuf);
}



void tLogo (void)
{
	float x, y, size;

	size = 64.f;
	x = currentCamera->screenMidX - (currentCamera->screenX / 2) + 8;
	y = currentCamera->screenMidY + (currentCamera->screenY / 2) - 32 - size;

	rSetMaterial (&argoLogo);

	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip (&vertBuf);
	kmxxSetVertex_7 (KM_VERTEXPARAM_NORMAL,		x, y, 10, 0, 0, 1, 0);
	kmxxSetVertex_7 (KM_VERTEXPARAM_NORMAL,		x, y + size, 10, 0, 1, 1, 0);
	kmxxSetVertex_7 (KM_VERTEXPARAM_NORMAL,		x + size, y, 10, 1, 0, 1, 0);
	kmxxSetVertex_7 (KM_VERTEXPARAM_ENDOFSTRIP, x + size, y + size, 10, 1, 1, 1, 0);
	kmxxReleaseCurrentPtr (&vertBuf);
}

/***************************************************************************
 * The all-new all-singing all-dancing proportionally spaced text routines *
 ***************************************************************************/

/*
 * The list of letters
 */
static char psLetters[] =	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz?!+-:.%;()<>\\/\"'*,#0123456789"
							"‰ÎÔˆ¸ﬂÁ‡ËÏÚ˘·ÈÌÛ˙‚ÍÓÙ˚„ÒıÊƒÀœ÷‹ﬂ«¿»Ã“Ÿ¡…Õ”⁄¬ Œ‘€√—’∆°ø^@";
#define NUM_LETTERS		(sizeof (psLetters)-2)
typedef struct {
	Sint16	start;
	Sint8	length;
	Sint8	row;
} PSLetter;
#define L(a,b) { a, ((b)-(a)), 0 },
PSLetter psLetterPos[] =
{
	L(0, 20)		// A
	L(21, 40)		// B
	L(43, 60)		// C
	L(61, 82)		// D
	L(85, 105)		// E
	L(106, 125)		// F
	L(126, 146)		// G
	L(146, 167)		// H
	L(171, 187)		// I
	L(192, 210)		// J
	L(210, 231)		// K
	L(232, 252)		// L
	L(252, 272)		// M
	L(272, 295)		// N
	L(298, 315)		// O
	L(319, 337)		// P
	L(339, 358)		// Q
	L(359, 379)		// R
	L(382, 399)		// S
	L(404, 421)		// T
	L(425, 443)		// U
	L(445, 464)		// V
	L(465, 485)		// W
	L(487, 506)		// X
	L(509, 527)		// Y
	L(531, 547)		// Z
	L(549, 564)		// a
	L(565, 579)		// b
	L(583, 595)		// c
	L(597, 614)		// d
	L(613, 631)		// e
	L(633, 646)		// f
	L(649, 665)		// g
	L(665, 681)		// h
	L(685, 698)		// i
	L(701, 714)		// j
	L(716, 732)		// k
	L(734, 750)		// l
	L(751, 768)		// m
	L(768, 785)		// n
	L(787, 801)		// o
	L(804, 818)		// p
	L(820, 834)		// q
	L(835, 851)		// r
	L(854, 866)		// s
	L(872, 885)		// t
	L(889, 903)		// u
	L(905, 920)		// v
	L(920, 936)		// w
	L(937, 952)		// x
	L(955, 969)		// y
	L(972, 985)		// z
	L(990, 1002)	// ?
	L(1010, 1015)	// !
	// End of row one
#undef L
#define L(a,b) { a, ((b)-(a)), 1 },
	L(2, 18)		// +
	L(21, 34)		// -
	L(41, 45)		// :
	L(59, 63)		// .
	L(70, 88)		// %
	L(90, 97)		// ;
	L(107, 115)		// (
	L(125, 132)		// )
	L(140, 151)		// <
	L(156, 168)		// >
	L(173, 186)		// backslash
	L(191, 203)		// /
	L(226, 234)		// "
	L(210, 214)		// '
	L(240, 255)		// *
	L(261, 266)		// ,
	L(269, 292)		// cursor (#)
	L(295, 312)		// 0
	L(315, 334)		// 1
	L(337, 355)		// 2
	L(360, 378)		// 3
	L(380, 398)		// 4
	L(401, 419)		// 5
	L(423, 440)		// 6
	L(444, 460)		// 7
	L(465, 480)		// 8
	L(487, 504)		// 9

	// Accented chars: umlauts
	L(511, 525)		// a
	L(528, 541)		// e
	L(545, 558)		// i
	L(562, 575)		// o
	L(577, 592)		// u

	L(594, 612)		// sharp S
	L(614, 620)		// c cedilla

	// Grave chars:
	L(629, 642)		// a
	L(647, 659)		// e
	L(664, 676)		// i
	L(681, 694)		// o
	L(695, 710)		// u

	// Acute chars
	L(713, 726)		// a
	L(731, 743)		// e
	L(748, 760)		// i
	L(763, 778)		// o
	L(780, 795)		// u
	
	// Hatted letters
	L(798, 812)		// a
	L(815, 828)		// e
	L(834, 845)		// i
	L(850, 863)		// o
	L(864, 879)		// u

	// Tilde'd letters
	L(881, 897)		// a
	L(899, 915)		// n
	L(919, 933)		// o

	// AE!
	L(969, 995)		// AE!

	// CAPITALS are the same as lower case innit
	// Accented chars: umlauts
	L(511, 525)		// a
	L(528, 541)		// e
	L(545, 558)		// i
	L(562, 575)		// o
	L(577, 592)		// u

	L(594, 612)		// sharp S
	L(614, 620)		// c cedilla

	// Grave chars:
	L(629, 642)		// a
	L(647, 659)		// e
	L(664, 676)		// i
	L(681, 694)		// o
	L(695, 710)		// u

	// Acute chars
	L(713, 726)		// a
	L(731, 743)		// e
	L(748, 760)		// i
	L(763, 778)		// o
	L(780, 795)		// u
	
	// Hatted letters
	L(798, 812)		// a
	L(815, 828)		// e
	L(834, 845)		// i
	L(850, 863)		// o
	L(864, 879)		// u

	// Tilde'd letters
	L(881, 897)		// a
	L(899, 915)		// n
	L(919, 933)		// o

	// AE!
	L(969, 995)		// AE!

	// Finally some spanish chars
	L(997, 1004)	// °
	L(1008,1020)	// ø

	// I lied about finally - cos here's the up down chars
	L(935, 950)		// up   (^)
	L(951, 966)		// down (@)
};
#undef L

struct {
	float top, left, right, bottom;
} TextWindow = { 0, 0, PHYS_SCR_X, PHYS_SCR_Y};

// The text matrix
static Matrix32 psTextMatrix = { 1, 0, 0, 1, 0, 0 };

// Text colours and transparency values
static Uint32 psTextColour = 0xff329900, OldTextColour;

// Alpha blending for fades off the top and bottom
static float psTop = -1e6f;
static float psBot = 1e6f;
static float psAlphaScale = 0.f;

// Text z position
static float psPri = 1000.f;

// The gap between letters 
#define INTER_LETTER_GAP	2
// The height of all letters
#define LETTER_HEIGHT		32
// Physical dimensions of the letters .PVR
#define PSTEXT_X			1024
#define PSTEXT_Y			64
// How wide a space is
#define SPACE_WIDTH			6

/*
 * Find the character number of a letter, returns ? if not found
 */
static int psGetCharNum (char c)
{
	char *findIt;
	// Special case speed mastery
	if (c >= 'A' && c <= 'Z')
		return (c - 'A');
	if (c >= 'a' && c <= 'z')
		return 26+(c - 'a');

	// Otherwise
	findIt = strchr (psLetters + (26*2), c);
	if (findIt == NULL)
		findIt = strchr (psLetters + (26*2), '?');
	dAssert (findIt != NULL, "Unable to find '?' in alphabet");
	return (findIt - psLetters);
}

/*
 * Return the length of a string in pixels
 */
float psStrWidth (const char *string)
{
	float width = 0.f;
	// Expanden sie erste!
	if (string[0] == '`')
		string = LangLookup(string+1);
	while (*string) {
		// Deal with escaped codes
		if (*string == '~') {
			string++;
			while (*string != '~' && *string)
				string++;
			dAssert (*string == '~', "Missing tilde in string");
			if (*string == '~')
				string++;
			continue;
		}
		// Special case for space
		if (*string == ' ')
			width += SPACE_WIDTH + INTER_LETTER_GAP;
		else
			width += psLetterPos[psGetCharNum(*string)].length + INTER_LETTER_GAP;
		string++;
	}
	return MAX(0, width-INTER_LETTER_GAP);
}

/*
 * Set the text colour and alpha amount
 */
static void psSetColour (Uint32 colour)
{
	psTextColour = (psTextColour & 0xff000000) | (colour & 0xffffff);
}
static void psSetAlpha (float amount)
{
	psTextColour = (psTextColour & 0xffffff) | ((int)(amount * 255.f) << 24);
}

/*
 * Set the text window
 * -1 as a paramater means camera render extent (includes safe frame)
 * Text is not clipped to this region, it merely set the extent for center/left/right justify
 * and the origin is set to the top left
 */
static void psSetWindow (float top, float left, float right, float bottom)
{
	TextWindow.top = ((top == -1) ? (currentCamera->screenMidY - currentCamera->screenY * 0.5f): top) + 32.f;
	TextWindow.bottom = ((bottom == -1) ? (currentCamera->screenMidY + currentCamera->screenY * 0.5f) - 32.f: bottom);
	TextWindow.left = ((left == -1) ? (currentCamera->screenMidX - currentCamera->screenX * 0.5f): left) + 8.f;
	TextWindow.right = ((right == -1) ? (currentCamera->screenMidX + currentCamera->screenX * 0.5f) - 8.f : right);
}
// Get the window
void psGetWindow (float window[4])
{
	window[0] = TextWindow.top;
	window[1] = TextWindow.left;
	window[2] = TextWindow.right;
	window[3] = TextWindow.bottom;
}

/*
 * Set/get the text matrix
 */
void psSetMatrix (Matrix32 *m)
{
	psTextMatrix = *m;
}
Matrix32 *psGetMatrix(void)
{
	return &psTextMatrix;
}

/*
 * Sets the text priority
 */
void psSetPriority (float priority)
{
	psPri = priority + 1000.f;
}

/*
 * Draw a character
 */
static PKMDWORD psDrawChar (register PKMDWORD pkmCurrentPtr, int charNum, float x, float y)
{
	float minU, maxU, len;
	float minV, maxV;
	Point2 p[4];
	int i;

	minU = psLetterPos[charNum].start * (1.f / (float)PSTEXT_X) + (0.5f / PSTEXT_X);
	maxU = (psLetterPos[charNum].start + psLetterPos[charNum].length) * (1.f / (float)PSTEXT_X)  + (0.5f / PSTEXT_X);
	minV = (psLetterPos[charNum].row * LETTER_HEIGHT) / (float)PSTEXT_Y;
	maxV = (float)LETTER_HEIGHT / (float)PSTEXT_Y + minV;
	len = (float) psLetterPos[charNum].length;

	// Set up the four points in letter space
	p[0].x = x;			p[0].y = y;
	p[1].x = x;			p[1].y = y + LETTER_HEIGHT;
	p[2].x = x + len;	p[2].y = y;
	p[3].x = x + len;	p[3].y = y + LETTER_HEIGHT;
	for (i = 0; i < 4; ++i) {
		mPoint2Multiply (p + i, p + i, &psTextMatrix);
		p[i].x += TextWindow.left;
		p[i].y += TextWindow.top;
	}

	// Apply the fading
	if (psAlphaScale) {
		Colour col[4];
		int ignore = 0;
		for (i = 0; i < 4; ++i) {
			col[i].colour = psTextColour;
			if (p[i].y < psTop) {
				float dist = psTop - p[i].y;
				float amt = (1.f - dist * psAlphaScale);
				col[i].argb.a = (char) ((float)col[i].argb.a * RANGE(0, amt, 1));
				if (amt < 0) {
					p[i].y = psTop - 1.f / psAlphaScale;
				}
			}
			if (p[i].y > psBot) {
				float dist = p[i].y - psBot;
				float amt = (1.f - dist * psAlphaScale);
				col[i].argb.a = (char) ((float)col[i].argb.a * RANGE(0, amt, 1));
				if (amt < 0) {
					p[i].y = psBot + 1.f / psAlphaScale;
				}
			}
			if (col[i].argb.a == 0)
				ignore++;
		}
		if (ignore != 4) {
			kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		p[0].x, p[0].y, psPri, minU, minV, col[0].colour, 0);
			kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		p[1].x, p[1].y, psPri, minU, maxV, col[1].colour, 0);
			kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		p[2].x, p[2].y, psPri, maxU, minV, col[2].colour, 0);
			kmxxSetVertex_3 (KM_VERTEXPARAM_ENDOFSTRIP,	p[3].x, p[3].y, psPri, maxU, maxV, col[3].colour, 0);
		}

	} else {	
		kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		p[0].x, p[0].y, psPri, minU, minV, psTextColour, 0);
		kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		p[1].x, p[1].y, psPri, minU, maxV, psTextColour, 0);
		kmxxSetVertex_3 (KM_VERTEXPARAM_NORMAL,		p[2].x, p[2].y, psPri, maxU, minV, psTextColour, 0);
		kmxxSetVertex_3 (KM_VERTEXPARAM_ENDOFSTRIP,	p[3].x, p[3].y, psPri, maxU, maxV, psTextColour, 0);
	}
	return pkmCurrentPtr;
}

/*
 * Draw a string, no printf() action
 * x and y in window space
 */
void psDrawString (const char *string, float x, float y)
{
	// Check for token lookup
	if (*string == '`')
		string = LangLookup (string+1);

	// Strip leading spaces
	while (*string == ' ') {
		x += SPACE_WIDTH + INTER_LETTER_GAP;
		string++;
	}
	if (*string == '\0')
		return;

	rSetMaterial (&psTextMat);
	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip (&vertBuf);

	do {
		int charNum;
		// Cheeky space madness
		if (*string == ' ') {
			x += SPACE_WIDTH + INTER_LETTER_GAP;
		} else if (*string == '~') {
			int moreCodes = TRUE;
			// Control codes
			string++;
			do {
				switch (*string) {
				default:
					dAssert (FALSE, "unknown control code - skipping");
					string++;
					break;
				case '\0':
					dAssert (FALSE, "Missing tilde in string");
					moreCodes = FALSE;
					break;
				case '~':
					string++;
					moreCodes = FALSE;
					break;
				case 'x':
					string++;
					psSetColour (OldTextColour);
					break;
				case '!':
					string++;
					OldTextColour = psTextColour;
					psSetColour (ColLerp (0xffffff,0x202030, sin(currentFrame*0.05f)*0.5f +0.5f));
					break;
				case 'c':
					{
						int colour = 0;
						string++;
						while (isxdigit(*string)) {
							colour<<=4;
							colour |= (*string <= '9' && *string >= '0') ? (*string - '0') :
									(toupper(*string) - 'A' + 10);
							string++;
						}
						OldTextColour = psTextColour;
						psSetColour (colour);
					}
					break;
				}
			} while (moreCodes);
			continue;
		} else {
			charNum = psGetCharNum (*string);
			pkmCurrentPtr = psDrawChar (pkmCurrentPtr, charNum, x, y);
			x += psLetterPos[charNum].length + INTER_LETTER_GAP;
		}
		string++;
	} while (*string);

	kmxxReleaseCurrentPtr (&vertBuf);
}

/*
 * Draws a string for the front end
 */
float psDrawStringFE (const char *string, float x, float y, float nChars)
{
	int nChar;
	Uint32 col;

	// Check for token lookup
	if (*string == '`')
		string = LangLookup (string+1);

	// Strip leading spaces
	while (*string == ' ') {
		x += SPACE_WIDTH + INTER_LETTER_GAP;
		string++;
	}
	if (*string == '\0')
		return x;

	rSetMaterial (&psTextMat);
	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip (&vertBuf);

	nChar = 0;
	col = psTextColour;
	do {
		int charNum;
		// Cheeky space madness
		if (*string == ' ') {
			nChar++;
			if (nChar > nChars)
				break;
			x += SPACE_WIDTH + INTER_LETTER_GAP;
		} else if (*string == '~') {
			int moreCodes = TRUE;
			// Control codes
			string++;
			do {
				switch (*string) {
				default:
					dAssert (FALSE, "unknown control code - skipping");
					string++;
					break;
				case '\0':
					dAssert (FALSE, "Missing tilde in string");
					moreCodes = FALSE;
					break;
				case '~':
					string++;
					moreCodes = FALSE;
					break;
				case 'x':
					string++;
					psSetColour (OldTextColour);
					col = OldTextColour;
					break;
				case '!':
					string++;
					OldTextColour = psTextColour;
					psSetColour (ColLerp (0xffffff,0x202030, sin(currentFrame*0.05f)*0.5f +0.5f));
					col = psTextColour;
					break;
				case 'c':
					{
						int colour = 0;
						string++;
						while (isxdigit(*string)) {
							colour<<=4;
							colour |= (*string <= '9' && *string >= '0') ? (*string - '0') :
									(toupper(*string) - 'A' + 10);
							string++;
						}
						OldTextColour = psTextColour;
						psSetColour (colour);
						col = colour;
					}
					break;
				}
			} while (moreCodes);
			continue;
		} else {
			float flare;
			nChar++;
			if (nChar > nChars) {
				break;
			}
			flare = 1.f - ((nChars - nChar) * (1.f/3.f));
			flare = RANGE(0, flare, 1);
			psSetColour (ColLerp (col, 0xffffffff, flare));
			charNum = psGetCharNum (*string);
			pkmCurrentPtr = psDrawChar (pkmCurrentPtr, charNum, x, y);
			x += psLetterPos[charNum].length + INTER_LETTER_GAP;
		}
		string++;
	} while (*string);

	kmxxReleaseCurrentPtr (&vertBuf);

	psTextColour = col;

	if (nChars > 0 && nChars < nChar)
		DrawCursor (x, y);

	return x;
}


/*
 * Draw a string, centred within the current window at a given y position
 */
void psDrawCentred (const char *string, float y)
{
	float width;
	width = psStrWidth (string);
	psDrawString (string, ((TextWindow.right - TextWindow.left)*psMultiplier - width)*0.5f, y);
}

/*
 * Draw a string, centred within the current window at a given y position, in a front-end stylee
 */
float psDrawCentredFE (const char *string, float y, float nChars)
{
	float width;
	width = psStrWidth (string);
	return psDrawStringFE (string, ((TextWindow.right - TextWindow.left)*psMultiplier - width)*0.5f, y, nChars);
}

/*
 * Draw a title, centred within the current window at a given y position
 * Has a nice background too blending from col1 to col2 and back to col1
 */
#define BAR_HEIGHT 35
#define BAR_EXTRA_WIDTH 64

void psDrawTitle (const char *string, float y, Uint32 col1, Uint32 col2)
{
	float width;
	Point2 p[6];
	int i;
	float pri;
	float amt = (col2>>24) * 1.f / 256.f;
	float psAlpha = (float)(psTextColour>>24);

	col2 = (col2 & 0xffffff) | ((int)(amt*psAlpha)<<24);

	// Do the background bar
	width = psStrWidth (string);
	p[0].y = y + LETTER_HEIGHT/2 - BAR_HEIGHT/2;
	p[1].y = p[0].y;
	p[2].y = p[0].y;
	p[3].y = p[0].y + BAR_HEIGHT;
	p[4].y = p[0].y + BAR_HEIGHT;
	p[5].y = p[0].y + BAR_HEIGHT;

	p[0].x = p[3].x = ((TextWindow.right - TextWindow.left) - (width+BAR_EXTRA_WIDTH*2))*0.5f;
	p[1].x = p[4].x = (TextWindow.right - TextWindow.left) * 0.5f;
	p[2].x = p[5].x = ((TextWindow.right - TextWindow.left) + (width+BAR_EXTRA_WIDTH*2))*0.5f;

	for (i = 0; i < 6; ++i) {
		mPoint2Multiply (p+i,p+i, &psTextMatrix);
		p[i].x += TextWindow.left;
		p[i].y += TextWindow.top;
	}

	pri = psPri * 0.98f;

	rSetMaterial (&fadeMat);
	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip (&vertBuf);

	kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,			p[0].x, p[0].y, pri, col1);
	kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,			p[3].x, p[3].y, pri, col1);
	kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,			p[1].x, p[1].y, pri, col2);
	kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,			p[4].x, p[4].y, pri, col2);
	kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,			p[2].x, p[2].y, pri, col1);
	kmxxSetVertex_0 (KM_VERTEXPARAM_ENDOFSTRIP,		p[5].x, p[5].y, pri, col1);

	kmxxReleaseCurrentPtr (&vertBuf);

	// Draw the text next (no autosort remember!)
	psDrawString (string, ((TextWindow.right - TextWindow.left)*psMultiplier - width)*0.5f, y);
}

/*
 * Draw a string, right aligned within the current window at a given y position
 */
void psDrawRJustified (const char *string, float y)
{
	float width;
	width = psStrWidth (string);
	psDrawString (string, (TextWindow.right - TextWindow.left)*psMultiplier - width, y);
}

/*
 * Return a statically sprintf'ed buffer
 */
static char psBuf[256];
int psLastLength;
char *psFormat (const char *fmt, ...)
{
	const char *fmt2;
	va_list args;

	// Firstly, look up fmt
	if (*fmt == '`')
		fmt2 = LangLookup(fmt+1);
	else
		fmt2 = fmt;

	va_start (args, fmt);
	if ((psLastLength = vsprintf (psBuf, fmt2, args)) >= sizeof (psBuf))
		dAssert (FALSE, "Buffer overflow in psFormat()");
	va_end (args);

	return psBuf;
}
// Numeric only formatting (8 digits max)
char *psFormatNum (int num)
{
	char *endOfBuf;
	char *retVal;
	char *write;
	bool negative;

	if (num >= 0) {
		endOfBuf = (psBuf + 8);
		negative = false;
	} else {
		endOfBuf = (psBuf + 9);
		num = -num;
		negative = true;
	}

	retVal = endOfBuf - 1;
	write = endOfBuf - 1;

	*endOfBuf = '\0';
	while (write >= psBuf) {
		*write = '0' + (num % 10);
		if (num % 10)
			retVal = write;
		write--;
		if (num == 0)
			break;
		num/=10;
	}

	if (negative)
		*--retVal = '-';

	psLastLength = (endOfBuf) - retVal;
	return retVal;
}
// Equivalent to sprintf "%d:%02d"
// relies of psFormatNum doing its bizz
char *psFormatTime(Uint32 min, Uint32 sec)
{
	char *retVal;

	dAssert (sec < 60, "Out of range error");

	retVal = psFormatNum (min);

	psBuf[8]  = ':';
	psBuf[9]  = '0' + (sec / 10);
	psBuf[10] = '0' + (sec % 10);
	psBuf[11] = '\0';

	psLastLength += 3;
	return retVal;
}
// Equivalent to sprintf "%d:%02d:%02d"
// relies of psFormatNum doing its bizz
char *psFormatTimeMilli(Uint32 min, Uint32 sec, Uint32 ms)
{
	char *retVal;

	dAssert (sec < 60, "Out of range error");
	dAssert (ms < 100, "Out of range error");

	retVal = psFormatNum (min);

	psBuf[8]  = ':';
	psBuf[9]  = '0' + (sec / 10);
	psBuf[10] = '0' + (sec % 10);
	psBuf[11] = ':';
	psBuf[12] = '0' + (ms / 10);
	psBuf[13] = '0' + (ms % 10);
	psBuf[14] = '\0';

	psLastLength += 6;
	return retVal;
}

/*
 * Get the number of characters in the alphabet
 */
int psNumChars(void)
{
	return sizeof (psLetters) - 1; // NULL at the end!
}

/*
 * Get the character for a given number
 */
char psGetChar(int i)
{
	dAssert (i >=0 && i < sizeof(psLetters), "Ack");
	return psLetters[i];
}

/*
 * Word-wrapping action
 */
static float psWordWrap (const char *string, float x, float y, float width, void (*drawRoutine)(char *, float, float))
{
	char lineBuf[512];
	char wordBuf[128];
	float length, wordLength;

	lineBuf[0] = '\0';

	if (string[0] == '`')
		string = LangLookup (string+1);

	length = 0;
	// Read each word in turn, adding it to linebuf until it goes over width
	while (*string) {
		char *wPtr;
		
		// Find a word
		wPtr = wordBuf;
		while (*string != ' ' && *string) {
			*wPtr++ = *string++;
		}
		if (*string == ' ')
			string++;
		*wPtr++ = '\0';

		// Find word length
		wordLength = psStrWidth (wordBuf) + ((length == 0) ? 0 : (SPACE_WIDTH+INTER_LETTER_GAP*2));

		if ((wordLength + length) >= width) {
			// Flush the line out
			if (drawRoutine)
				drawRoutine (lineBuf, x, y);
			y += 36.f;
			length = 0;
			lineBuf[0] = '\0';
		}
		if (length)
			strcat (lineBuf, " ");
		strcat (lineBuf, wordBuf);
		length += wordLength;
	}
	if (length) {
		if (drawRoutine)
			drawRoutine (lineBuf, x, y);
		y += 36;
	}
	return y-1;
}
static void psDrawCentredKludge (const char *str, float x, float y)
{
	psDrawCentred (str, y);
}
float psCentreWordWrap (const char *string, float y, float width)
{
	return psWordWrap (string, 0, y, width, psDrawCentredKludge);
}
float psDrawWordWrap (const char *string, float x, float y, float width)
{
	return psWordWrap (string, x, y, width, psDrawString);
}

static float gNChars, gAdj;
static void psDrawCentredKludgeFE (const char *str, float x, float y)
{
	psDrawCentredFE (str, y, gNChars - gAdj);
	gAdj += strlen (str);
}
float psCentreWordWrapFE (const char *string, float x0, float y0, float x1, float y1, float scale, float nChars)
{
	Matrix32 pushed;
	float midX, midY, height, width, textHeight;
	float rScale = 1.f / scale;
	float savedL, savedR, savedPS;

	y0 -= 32; y1 -= 32;

	savedL = TextWindow.left;
	savedR = TextWindow.right;

	// Calculations
	midX = (x0+x1) * 0.5f;
	midY = (y0+y1) * 0.5f;
	width = (x1 - x0)*rScale;
	height = (y1 - y0);

	TextWindow.left = x0;
	TextWindow.right = x1;

	savedPS = psMultiplier;
	psSetMultiplier(scale);

	memcpy (&pushed, psGetMatrix(), sizeof (pushed));

	// Find the height of the text to centre the text
	gNChars = 1e6f;
	gAdj = 0;
	textHeight = psWordWrap (string, 0, 0, width, NULL);

		// Apply the scale about the midpoint
	mPreTranslate32 (psGetMatrix(), 0, midY + midY*scale-textHeight*scale/2);
	mPreScale32 (psGetMatrix(), scale, scale);
	mPreTranslate32 (psGetMatrix(), 0, -midY);

	// Draw the text
	gNChars = nChars;
	gAdj = 0;
	psWordWrap (string, 0, 0, width, psDrawCentredKludgeFE);

	TextWindow.left = savedL;
	TextWindow.right = savedR;
	psMultiplier = savedPS;

	// Reset the matrix
	memcpy (psGetMatrix(), &pushed, sizeof (pushed));

}

/*
 * New version of Script print, taking into account the new font system
 */
void ScriptPrint (float x, float y, char *fmt, float numchars)
{
	int i;
	float w[4];
	Point2 p[4];
	bool cursor = TRUE, flash = FALSE;

	psGetWindow (w);

	if (numchars > 255)
		numchars -= 256, flash = TRUE;
	if (numchars < 0)
		numchars = -numchars, cursor = FALSE;

	if ((flash && (currentFrame & 4)) || !cursor)
		return;

	psSetAlpha(1);
	psSetColour(0xffffff);
	psSetPriority(7000);
	mUnit32 (psGetMatrix());
	mPostScale32 (psGetMatrix(), 0.75f, 0.75f);
	psDrawWordWrap (fmt, x+4, y+4, 590);
}

#define B_SCALE			0.6f
#define B_BORDER		4
#define B_OUTER_BORDER	64
#define B_ONE_LINE		((32*B_SCALE)+B_BORDER)
#define B_TWO_LINES		((32*B_SCALE*2)+B_BORDER)
#define B_EDGE_BORDER_X	8
#define B_EDGE_BORDER_Y	8
#define B_BRIEFING_TOP	(B_ONE_LINE + locHeight + B_ONE_LINE/2 + objHeight + B_EDGE_BORDER_Y + B_OUTER_BORDER)
#define B_BRIEFING_BOT	(PHYS_SCR_Y - B_OUTER_BORDER - B_ONE_LINE)
#define B_BRIEFING_HEIGHT (B_BRIEFING_BOT - B_BRIEFING_TOP)
#define B_NAME_WIDTH_MIN	128.f
#define B_NAME_WIDTH	bNameWidth
#define B_TEXT_WIDTH	(PHYS_SCR_X - B_BORDER*4 - B_OUTER_BORDER*2 - B_NAME_WIDTH - B_EDGE_BORDER_X*2)
#define B_BRIEF_WIDTH	(PHYS_SCR_X - B_BORDER*4 - B_OUTER_BORDER*2 - B_EDGE_BORDER_X*2)
#define B_WAIT_SCROLL	(5*FRAMES_PER_SECOND)
// Ride on time:
#define BLACKBOX(A,B,C,D,P)\
	kmxxGetCurrentPtr (&vertBuf);\
	kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,			A,	B,	P, BKG_COL); \
	kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,			A,	D,	P, BKG_COL); \
	kmxxSetVertex_0 (KM_VERTEXPARAM_NORMAL,			C,	B,	P, BKG_COL); \
	kmxxSetVertex_0 (KM_VERTEXPARAM_ENDOFSTRIP,		C,	D,	P, BKG_COL); \
	kmxxReleaseCurrentPtr (&vertBuf); \
	rLine2D (A,B,C,B,P+1,hiCol,hiCol); \
	rLine2D (A,B,A,D,P+1,hiCol,hiCol); \
	rLine2D (C,B,C,D,P+1,loCol,loCol); \
	rLine2D (A,D,C,D,P+1,loCol,loCol)

extern int GetTimeRating (int level, float time);
extern int GetScoreRating (int level, int score);
extern int GetLivesRating (int level, int dam);
extern int GetBossTimeRating (int level, float time);
extern int GetGrading (int score);
extern short LevelNum;
void FinaliseGameScore(void)
{
	// XXX LEVEL DEPENDENCY
	switch (LevelNum) {
	case LEV_DEFENSIVE: // Survival special case 	
		score = (int)(100.f * (player[0].Player->health / player[0].maxHealth));
		break;
	case LEV_SHOOTING_RANGE:
		score = 80 - score;
		break;
	case LEV_SEARCH_AND_COLLECT:
		score = 22 - score;
		break;
	case LEV_FOOTSOLDIER_RAMPAGE:
		score = 150 - score;
		break;
	case LEV_HEAVY_DUTY_RAMPAGE:
		score = 30 - score;
		break;
	}
}
int DisplayBriefing (int scriptNum, float amount, int debrief)
{
	int nLines = GetScriptLines (scriptNum);
	int i, nBytes;
	Uint32 BKG_COL;
	Colour hiCol, loCol;
	float y, width, height;
	char textBuf[4096], *p;
	static int Score, Time, LivesLost, BTime;
	static float scrollAmt = 0, scrollSpeed = 0;
	static int waitframes;
	float bNameWidth;

	mUnit32 (psGetMatrix());

	if ((!MissionFailure) && (nLines < 0))
		return;

	if (amount < 0.05f) {
		Score = 0;
		Time = 0;
		LivesLost = 0;
		BTime = 0;
		scrollAmt = scrollSpeed = 0;
		waitframes = 0;
	}

	nBytes = GetScriptLine (scriptNum, 0, &p);
 	if ((MissionFailure) || (nBytes))
	{
		amount = RANGE(0, amount, 1);

		if (waitframes >= 0)
			waitframes++;

		rSetMaterial (&fadeMat);
		kmStartVertexStrip (&vertBuf);

		hiCol.colour = 0x404040;
		loCol.colour = 0;
		hiCol.argb.a = 128*amount;
		loCol.argb.a = 128*amount;

		BKG_COL = (((int)(128 * amount)) << 24) | 0x202020;

		// Draw the very background tile
		BLACKBOX(B_OUTER_BORDER,B_OUTER_BORDER,PHYS_SCR_X-B_OUTER_BORDER,PHYS_SCR_Y-B_OUTER_BORDER,6000);

		loCol.colour = 0x404040;
		hiCol.colour = 0;
		loCol.argb.a = 128*amount;
		hiCol.argb.a = 128*amount;

		if (!debrief) {
			// Calculate the height of the location, and the objective
			float objHeight, locHeight, wid;
			char *locText, *timeText, *objText;

			// How big must the localisation et al be?
			bNameWidth = B_NAME_WIDTH_MIN;
			wid = psStrWidth ("`LOCATION") * B_SCALE + B_BORDER*2;
			if (wid > bNameWidth)
				bNameWidth = wid;
			wid = psStrWidth ("`TIME") * B_SCALE + B_BORDER*2;
			if (wid > bNameWidth)
				bNameWidth = wid;
			wid = psStrWidth ("`OBJECTIVE") * B_SCALE + B_BORDER*2;
			if (wid > bNameWidth)
				bNameWidth = wid;

			// Location text
			memcpy (textBuf, p, nBytes);
			textBuf[nBytes-1] = '\0';
			locText = textBuf;
			timeText = locText + nBytes;
			// Time text
			nBytes = GetScriptLine (scriptNum, 1, &p);
			memcpy (timeText, p, nBytes);
			timeText[nBytes-1] = '\0';
			objText = timeText + nBytes;
			// Objective text
			nBytes = GetScriptLine (scriptNum, 2, &p);
			memcpy (objText, p, nBytes);
			objText[nBytes-1] = '\0';

			locHeight = psDrawWordWrap (locText, -10000, 0, 
				B_TEXT_WIDTH/B_SCALE)  * B_SCALE + B_BORDER/2;

			objHeight = psDrawWordWrap (objText, -10000, 0, 
				B_TEXT_WIDTH/B_SCALE) * B_SCALE + B_BORDER/2;

			// Draw in the rest of the black bars
			// Location BB
			BLACKBOX(B_OUTER_BORDER + B_EDGE_BORDER_X + B_NAME_WIDTH,
				B_OUTER_BORDER + B_EDGE_BORDER_Y + B_BORDER/4,
				PHYS_SCR_X - B_OUTER_BORDER - B_EDGE_BORDER_X,
				B_OUTER_BORDER + B_EDGE_BORDER_Y + locHeight - B_BORDER/4,6500);

			// Time
			BLACKBOX(B_OUTER_BORDER + B_EDGE_BORDER_X + B_NAME_WIDTH,
				B_OUTER_BORDER + B_EDGE_BORDER_Y + B_BORDER/4 + locHeight,
				PHYS_SCR_X - B_OUTER_BORDER - B_EDGE_BORDER_X,
				B_OUTER_BORDER + B_EDGE_BORDER_Y + locHeight + B_ONE_LINE- B_BORDER/4,6500);

			// Objective
			BLACKBOX(B_OUTER_BORDER + B_EDGE_BORDER_X + B_NAME_WIDTH,
				B_OUTER_BORDER + B_EDGE_BORDER_Y + B_BORDER/4 + locHeight + B_ONE_LINE,
				PHYS_SCR_X - B_OUTER_BORDER - B_EDGE_BORDER_X,
				B_OUTER_BORDER + B_EDGE_BORDER_Y + locHeight + B_ONE_LINE + objHeight - B_BORDER/4,6500);

			// Briefing text
			BLACKBOX(B_OUTER_BORDER + B_EDGE_BORDER_X,
				B_BRIEFING_TOP + B_BORDER/4,
				PHYS_SCR_X - B_OUTER_BORDER - B_EDGE_BORDER_X,
				B_BRIEFING_BOT - B_BORDER/4,6500);

			// Inner bit
			BLACKBOX(B_OUTER_BORDER + B_EDGE_BORDER_X + B_BORDER*4,
				B_BRIEFING_TOP - B_ONE_LINE/4 - B_BORDER/4,
				PHYS_SCR_X - B_OUTER_BORDER - B_EDGE_BORDER_X - B_BORDER*4,
				B_BRIEFING_TOP - B_ONE_LINE/4 + B_BORDER/4,6500);

			psSetWindow (-32, -8, PHYS_SCR_X/B_SCALE, PHYS_SCR_Y/B_SCALE);
			psSetPriority(7000);
			psSetColour (0x00ff00);
			psSetAlpha (amount*amount*amount);
			mUnit32 (psGetMatrix());
			mPostScale32 (psGetMatrix(), B_SCALE, B_SCALE);

			// Location text
			width = psStrWidth ("`LOCATION") + B_BORDER*2;
			psDrawString ("`LOCATION", (B_OUTER_BORDER + B_EDGE_BORDER_X + B_NAME_WIDTH) / B_SCALE - width, 
				(B_OUTER_BORDER + B_EDGE_BORDER_Y + B_BORDER/2) / B_SCALE);

			// Time text
			width = psStrWidth ("`TIME") + B_BORDER*2;
			psDrawString ("`TIME", (B_OUTER_BORDER + B_EDGE_BORDER_X + B_NAME_WIDTH) / B_SCALE - width, 
				(B_OUTER_BORDER + locHeight + B_EDGE_BORDER_Y + B_BORDER/2) / B_SCALE);

			// Objective text
			width = psStrWidth ("`OBJECTIVE") + B_BORDER*2;
			psDrawString ("`OBJECTIVE", (B_OUTER_BORDER + B_EDGE_BORDER_X + B_NAME_WIDTH) / B_SCALE - width, 
				(B_OUTER_BORDER + B_ONE_LINE + locHeight + B_EDGE_BORDER_Y + B_BORDER/2) / B_SCALE);

			psSetColour (0xcfcfcf);

			// Hit start text
			if (amount > 0.9f)
				psDrawCentred ("`HITSTART", (B_BRIEFING_BOT + B_BORDER/2) / B_SCALE);

			// Display the location text itself
			psDrawWordWrap (locText, (B_OUTER_BORDER + B_EDGE_BORDER_X + B_NAME_WIDTH + B_BORDER)/B_SCALE,
				(B_OUTER_BORDER + B_EDGE_BORDER_Y + B_BORDER/2)/B_SCALE, B_TEXT_WIDTH/B_SCALE);

			// Display the time text
			psDrawString (timeText, (B_OUTER_BORDER + B_EDGE_BORDER_X + B_NAME_WIDTH + B_BORDER)/B_SCALE,
				(B_OUTER_BORDER + B_EDGE_BORDER_Y + B_BORDER/2 + locHeight)/B_SCALE);

			// Display the objective text
			psDrawWordWrap (objText, (B_OUTER_BORDER + B_EDGE_BORDER_X + B_NAME_WIDTH + B_BORDER)/B_SCALE,
				(B_OUTER_BORDER + B_EDGE_BORDER_Y + B_BORDER/2 + B_ONE_LINE + locHeight)/B_SCALE, B_TEXT_WIDTH/B_SCALE);

			// And finally draw the briefing
			nBytes = GetScriptLine (scriptNum, 3, &p);
			memcpy (textBuf, p, nBytes);
			textBuf[nBytes-1] = '\0';
			psTop = B_BRIEFING_TOP + 36.f;
			psBot = B_BRIEFING_BOT - 36.f;
			psAlphaScale = 1.f / 32.f;
			height = psDrawWordWrap (textBuf, (B_OUTER_BORDER + B_EDGE_BORDER_X + B_BORDER)/B_SCALE,
				(B_BRIEFING_TOP+18)/B_SCALE - scrollAmt, 
				B_BRIEF_WIDTH/B_SCALE);
			height -= (B_BRIEFING_TOP+18)/B_SCALE - scrollAmt;
			psTop = -1e6f;
			psBot = 1e6f;
			psAlphaScale = 0.f;
			if (SavedPadPress & PADDOWN) {
				scrollSpeed += 2.f;
			} else if (SavedPadPress & PADUP) {
				scrollSpeed -= 2.f;
			}
			scrollSpeed += 2.5f * SavedJoyY;
			scrollSpeed *= 0.6f;
			if (fabs(scrollSpeed) > 0.2f)
				waitframes = -1;
			if (waitframes > B_WAIT_SCROLL)
				scrollAmt++;
			else
				scrollAmt += scrollSpeed;
			if (height < ((B_BRIEFING_HEIGHT-36)/B_SCALE)) {
 				scrollAmt = 0;
 			} else {
				scrollAmt = RANGE (0, scrollAmt, height - ((B_BRIEFING_HEIGHT-36)/B_SCALE));
			}

			if (amount > 0.9f && (SavedPadPush & (PADSTART))) {
				//fadeN[0] = -10;
				//fadeLength[0] = 15;
				//fadeMode[0] = FADE_TO_BLACK;
				FE_ACK();
				return 1;
			}
		} else {
			int doneIt = 0;
			int total = 0;
			bool challenge = IsChallengeLevel();
			extern bool IsScoreImportant (int level);

			bNameWidth = B_NAME_WIDTH_MIN;

			psSetWindow (-32, -8, PHYS_SCR_X/1, PHYS_SCR_Y/1);
			psSetPriority(7000);
			psSetColour (0x00ff00);
			psSetAlpha (amount*amount*amount);
			mUnit32 (psGetMatrix());
			mPostScale32 (psGetMatrix(), 1, 1);

			// Clamp the time on challenge missions XXX Level dependency
			switch (LevelNum) {
			case LEV_VEHICLE_MANOEUVRING:
				GameTime = RANGE (0, GameTime, FRAMES_PER_SECOND*100); // 1 minute 40
				break;
			case LEV_DEFENSIVE:
				GameTime = RANGE (0, GameTime, FRAMES_PER_SECOND*120); // 2 minutes
				break;
			case LEV_FOOTSOLDIER_RAMPAGE:
				GameTime = RANGE (0, GameTime, FRAMES_PER_SECOND*180); // 3 minutes
				break;
			}

			// Mission Complete (or failed))
			if (MissionFailure)
				psDrawCentred ("`MISFAILED", (B_OUTER_BORDER + B_EDGE_BORDER_Y + B_BORDER/2));
			else {
				psDrawCentred ("`MISCOMPLETE", (B_OUTER_BORDER + B_EDGE_BORDER_Y + B_BORDER/2));
				LevelComplete = 1;
			}

			BLACKBOX(B_OUTER_BORDER + B_EDGE_BORDER_X + B_BORDER*4,
				B_OUTER_BORDER + B_EDGE_BORDER_Y + B_BORDER + 32,
				PHYS_SCR_X - B_OUTER_BORDER - B_EDGE_BORDER_X - B_BORDER*4,
				B_OUTER_BORDER + B_EDGE_BORDER_Y + B_BORDER + 33,6500);

			psSetColour (0xcfcfcf);

			// Score text
			width = psStrWidth ("`EM_SCORE") + B_BORDER*2;
			psDrawString ("`EM_SCORE", (B_OUTER_BORDER + B_EDGE_BORDER_X + B_NAME_WIDTH) - width, 
				(B_OUTER_BORDER + 32 + B_BORDER*2 + B_BORDER + B_EDGE_BORDER_Y + B_BORDER/2));
			
			// Score number
			if (!challenge || IsScoreImportant(LevelNum)) {
				psDrawCentred (psFormatNum (Score), (B_OUTER_BORDER + 32 + B_BORDER*2 + B_BORDER + B_EDGE_BORDER_Y + B_BORDER/2));
			} else {
				psDrawCentred ("`EM_NA", (B_OUTER_BORDER + 32 + B_BORDER*2 + B_BORDER + B_EDGE_BORDER_Y + B_BORDER/2));
			}
			// Score rating
			if (!challenge) {
				psDrawString (psFormat ("(%d/25)", GetScoreRating(LevelNum, Score)), B_OUTER_BORDER + B_EDGE_BORDER_X + B_BORDER*4 + B_TEXT_WIDTH,
					(B_OUTER_BORDER + 32 + B_BORDER*2 + B_BORDER + B_EDGE_BORDER_Y + B_BORDER/2));
			}
			total += GetScoreRating(LevelNum, Score);

			// Lives text
			width = psStrWidth ("`EM_LIVES") + B_BORDER*2;
			psDrawString ("`EM_LIVES", (B_OUTER_BORDER + B_EDGE_BORDER_X + B_NAME_WIDTH) - width, 
				(B_OUTER_BORDER + (32 + B_BORDER*2)*2 + B_BORDER + B_EDGE_BORDER_Y + B_BORDER/2));
			
			// Lives number
			psDrawCentred (psFormatNum (3 - LivesLost), (B_OUTER_BORDER + (32 + B_BORDER*2)*2 + B_BORDER + B_EDGE_BORDER_Y + B_BORDER/2));

			// Lives rating
			if (!challenge) {
				psDrawString (psFormat ("(%d/25)", GetLivesRating(LevelNum, LivesLost)), B_OUTER_BORDER + B_EDGE_BORDER_X + B_BORDER*4 + B_TEXT_WIDTH,
					(B_OUTER_BORDER + (32 + B_BORDER*2)*2 + B_BORDER + B_EDGE_BORDER_Y + B_BORDER/2));
			}
			total += GetLivesRating(LevelNum, LivesLost);

			// Time text
			width = psStrWidth ("`EM_TIME") + B_BORDER*2;
			psDrawString ("`EM_TIME", (B_OUTER_BORDER + B_EDGE_BORDER_X + B_NAME_WIDTH) - width, 
				(B_OUTER_BORDER + (32 + B_BORDER*2)*3 + B_BORDER + B_EDGE_BORDER_Y + B_BORDER/2));
			
			// Time
			psDrawCentred (psFormatTimeMilli(Time/(60*FRAMES_PER_SECOND), (Time/FRAMES_PER_SECOND)%60, (int)((float)(Time%FRAMES_PER_SECOND) * 100/FRAMES_PER_SECOND)), 
				(B_OUTER_BORDER + (32 + B_BORDER*2)*3 + B_BORDER + B_EDGE_BORDER_Y + B_BORDER/2));

			// Time rating
			if (!challenge) {
				psDrawString (psFormat ("(%d/25)", GetTimeRating(LevelNum, (float)Time/FRAMES_PER_SECOND)), B_OUTER_BORDER + B_EDGE_BORDER_X + B_BORDER*4 + B_TEXT_WIDTH,
					(B_OUTER_BORDER + (32 + B_BORDER*2)*3 + B_BORDER + B_EDGE_BORDER_Y + B_BORDER/2));
			}
			total += GetTimeRating(LevelNum, (float)Time/FRAMES_PER_SECOND);

			// Boss time text
			width = psStrWidth ("`EM_BOSS") + B_BORDER*2;
			psDrawString ("`EM_BOSS", (B_OUTER_BORDER + B_EDGE_BORDER_X + B_NAME_WIDTH) - width, 
				(B_OUTER_BORDER + (32 + B_BORDER*2)*4 + B_BORDER + B_EDGE_BORDER_Y + B_BORDER/2));
			
			// Boss Time
			if (!challenge) {
				psDrawCentred (psFormatTimeMilli(BTime/(60*FRAMES_PER_SECOND), (BTime/FRAMES_PER_SECOND)%60, (int)((float)(BTime%FRAMES_PER_SECOND) * 100/FRAMES_PER_SECOND)), 
					(B_OUTER_BORDER + (32 + B_BORDER*2)*4 + B_BORDER + B_EDGE_BORDER_Y + B_BORDER/2));
			} else {
				psDrawCentred ("`EM_NA", 
					(B_OUTER_BORDER + (32 + B_BORDER*2)*4 + B_BORDER + B_EDGE_BORDER_Y + B_BORDER/2));
			}
			// Time rating
			if (!challenge) {
				psDrawString (psFormat ("(%d/25)", GetBossTimeRating(LevelNum, (float)BTime/FRAMES_PER_SECOND)), B_OUTER_BORDER + B_EDGE_BORDER_X + B_BORDER*4 + B_TEXT_WIDTH,
					(B_OUTER_BORDER + (32 + B_BORDER*2)*4 + B_BORDER + B_EDGE_BORDER_Y + B_BORDER/2));
			}
			total += GetBossTimeRating(LevelNum, (float)BTime/FRAMES_PER_SECOND);

			BLACKBOX(B_OUTER_BORDER + B_EDGE_BORDER_X + B_BORDER*4,
				(B_OUTER_BORDER + (32 + B_BORDER*2)*5 + B_BORDER + B_EDGE_BORDER_Y),
				PHYS_SCR_X - B_OUTER_BORDER - B_EDGE_BORDER_X - B_BORDER*4,
				(B_OUTER_BORDER + (32 + B_BORDER*2)*5 + B_BORDER + B_EDGE_BORDER_Y)+1, 6500);

			if (!challenge) {
				// Total text
				width = psStrWidth ("`EM_TOTAL") + B_BORDER*2;
				psDrawString ("`EM_TOTAL", (B_OUTER_BORDER + B_EDGE_BORDER_X + B_NAME_WIDTH) - width, 
					(B_OUTER_BORDER + (32 + B_BORDER*2)*5 + B_BORDER*3 + B_EDGE_BORDER_Y + B_BORDER/2));
				
				// Total rating
				psDrawCentred (psFormat ("%d/100", total),
					(B_OUTER_BORDER + (32 + B_BORDER*2)*5 + B_BORDER*3 + B_EDGE_BORDER_Y + B_BORDER/2));
				
				// And the rating is:
				psSetColour (0x00ff00);
				width = psStrWidth ("`EM_RATING") + B_BORDER*2;
				psDrawString ("`EM_RATING", (B_OUTER_BORDER + B_EDGE_BORDER_X + B_NAME_WIDTH) - width, 
					(B_OUTER_BORDER + (32 + B_BORDER*2)*6 + B_BORDER*3 + B_EDGE_BORDER_Y + B_BORDER/2));
				
				// Total rating
				psDrawCentred (psFormat ("%c", GetGrading(total) + 'A'),
					(B_OUTER_BORDER + (32 + B_BORDER*2)*6 + B_BORDER*3 + B_EDGE_BORDER_Y + B_BORDER/2));
				
				// Better than the current best?
				if (!challenge &&
					CurProfile[0].cheatWeapon == 0 && 
					(CurProfile[0].activeSpecials & SPECIAL_CHEATING_MASK) == 0 &&
					total > (GetBossTimeRating(LevelNum, CurProfile[0].missionData[GameDifficulty][LevelNum].bestBossTime) + GetTimeRating(LevelNum, CurProfile[0].missionData[GameDifficulty][LevelNum].bestTime) + GetScoreRating(LevelNum, CurProfile[0].missionData[GameDifficulty][LevelNum].bestScore) + GetLivesRating(LevelNum, CurProfile[0].missionData[GameDifficulty][LevelNum].livesLost))
					) {
					psSetColour (ColLerp (0xff0000, 0x800000, sin(currentFrame*0.2f)*0.25f +0.75f));
					psDrawString ("`EM_RECORD",
						B_OUTER_BORDER + B_EDGE_BORDER_X + B_BORDER*4 + B_TEXT_WIDTH,
						(B_OUTER_BORDER + (32 + B_BORDER*2)*6 + B_BORDER + B_EDGE_BORDER_Y + B_BORDER/2));
					
				}
			} else {
				bool better = false;
				float gTime = ((float)GameTime / FRAMES_PER_SECOND);
				Matrix32 mat;
				mat = *psGetMatrix();
				mPostTranslate32 (psGetMatrix(), -PHYS_SCR_X/2, -(B_OUTER_BORDER + (32 + B_BORDER*2)*5.5 + B_BORDER*3 + B_EDGE_BORDER_Y + B_BORDER/2));
				mPostScale32 (psGetMatrix(), 1.5f, 1.5f);
				mPostTranslate32 (psGetMatrix(), PHYS_SCR_X/2, (B_OUTER_BORDER + (32 + B_BORDER*2)*5.5 + B_BORDER*3 + B_EDGE_BORDER_Y + B_BORDER/2));
				psSetColour (ColLerp (0xff0000, 0x800000, sin(currentFrame*0.2f)*0.25f +0.75f));
				psDrawCentred (MissionFailure ? "`EM_FAIL" : "`EM_PASS",
					(B_OUTER_BORDER + (32 + B_BORDER*2)*5.5 + B_BORDER*3 + B_EDGE_BORDER_Y + B_BORDER/2));
				psSetMatrix (&mat);

				if (LevelNum == LEV_DEFENSIVE) // special case: key off of score
					better = score > CurProfile[0].missionData[GameDifficulty][LevelNum].bestScore;
				else
					better = gTime < CurProfile[0].missionData[GameDifficulty][LevelNum].bestTime;
				if (!CurProfile[0].missionData[GameDifficulty][LevelNum].completed)
					better = true;
				if (!MissionFailure && better) {
					if (LevelNum == LEV_DEFENSIVE) {
						psDrawString ("`EM_RECORD",
							B_OUTER_BORDER + B_EDGE_BORDER_X + B_BORDER*4 + B_TEXT_WIDTH - 32,
							(B_OUTER_BORDER + 32 + B_BORDER*2 + B_BORDER + B_EDGE_BORDER_Y + B_BORDER/2));
					} else {
						psDrawString ("`EM_RECORD",
							B_OUTER_BORDER + B_EDGE_BORDER_X + B_BORDER*4 + B_TEXT_WIDTH - 32,
							(B_OUTER_BORDER + (32 + B_BORDER*2)*3 + B_BORDER + B_EDGE_BORDER_Y + B_BORDER/2));
					}
				}
			}
			BLACKBOX(B_OUTER_BORDER + B_EDGE_BORDER_X + B_BORDER*4,
				(B_OUTER_BORDER + (32 + B_BORDER*2)*7 + B_BORDER*3 + B_EDGE_BORDER_Y + B_BORDER),
				PHYS_SCR_X - B_OUTER_BORDER - B_EDGE_BORDER_X - B_BORDER*4,
				(B_OUTER_BORDER + (32 + B_BORDER*2)*7 + B_BORDER*3 + B_EDGE_BORDER_Y + B_BORDER)+1, 6500);

			psSetColour (0xffffff);
			psDrawCentred ("`HITSTART", (B_OUTER_BORDER + (32 + B_BORDER*2)*8 - B_EDGE_BORDER_Y - B_BORDER/2));

			if (amount > 0.9f) {
				if (Score < score)
					Score += 250, doneIt++;
				if (Score > score)
					Score = score;
				if (LivesLost < (GetLevLives(LevelNum) - PlayerLives))
					LivesLost ++, doneIt++;
				if (Time < GameTime)
					Time += 50, doneIt++;
				if (Time > GameTime)
					Time = GameTime;
				if (BTime < BossTime)
					BTime += 50, doneIt++;
				if (BTime > BossTime)
					BTime = BossTime;
			}
			if (doneIt)
				FE_Sound();
			if (PadPush[0] & (PADSTART)) {
				FE_ACK();
				if (doneIt == 0)
					return 1;
				else {
					Score = score;
					Time = GameTime;
					BTime = BossTime;
					LivesLost = LivesLost;
				}
			}
		}
	}
	return 0;
}

void Overlay (int charNum, float amt) { rOverlay (charNum, amt); }
void rOverlay (int charNum, float amount)
{
	float dist;
	float width, height, yAdd;
	float a = lAmbient.r;
	Colour hiCol, loCol;
	Uint32 BKG_COL;
	float boxHeight;

	if (charNum == -1)
		return;
	
	boxHeight = psDrawWordWrap ("`CAN_SCRIPT1", -100000, 0, 590) * 0.75f + 16.f;
	
	amount = RANGE(0,amount,1);
	yAdd = (Delta(amount)-1) * 500;

	dAssert (charNum >= 0 &&
		charNum < asize (overlay) - 1, "Invalid overlay number");
	width = OVERLAY_SIZE_X * 0.5f;
	height = OVERLAY_SIZE_Y * 0.5f;

#if 0	
	rSetMaterial (&overlay[charNum + 1].mat);
	
	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip (&vertBuf);
	
	kmxxSetVertex_5 (KM_VERTEXPARAM_NORMAL,		OVERLAY_MID_X - width, OVERLAY_MID_Y - height + yAdd, 9, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0);
	kmxxSetVertex_5 (KM_VERTEXPARAM_NORMAL,		OVERLAY_MID_X - width, OVERLAY_MID_Y + height + yAdd, 9, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0);
	kmxxSetVertex_5 (KM_VERTEXPARAM_NORMAL,		OVERLAY_MID_X + width, OVERLAY_MID_Y - height + yAdd, 9, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0);
	kmxxSetVertex_5 (KM_VERTEXPARAM_ENDOFSTRIP, OVERLAY_MID_X + width, OVERLAY_MID_Y + height + yAdd, 9, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0);
	
	kmxxReleaseCurrentPtr (&vertBuf);
#endif

	// Draw the text bar
	hiCol.colour = 0x404040;
	loCol.colour = 0;
	hiCol.argb.a = 128*amount;
	loCol.argb.a = 128*amount;
	BKG_COL = 0x80202020;

	rSetMaterial (&fadeMat);
	kmStartVertexStrip (&vertBuf);
	BLACKBOX (BAR_MIN_X, BAR_MIN_Y+yAdd,
			BAR_MIN_X+BAR_SIZE_X, BAR_MIN_Y+boxHeight+yAdd, 6000);

	psSetWindow (BAR_MIN_Y-32+yAdd, BAR_MIN_X-8, BAR_MIN_X + dist, BAR_MIN_Y + boxHeight + yAdd);

}
