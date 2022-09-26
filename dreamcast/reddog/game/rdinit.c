#include "RedDog.h"
#include "MPlayer.h"
#include "SndUtls.h"
#include <ctype.h>
#include <sg_lcd.h>

/*
 * All the initialisation happens in this file
 */
void InitSubSystems()
{
	int port, i, j;
	Stream *s;

	/*
	 * Initialise the Red Dog memory management subsystem
	 */
	InitMemory();
	/*
	 * Initialise the stream subsystem
	 */
	sInit();
	/*
	 * Initialise sound before the renderer
	 */
#if !AUDIO64
	SoundInitialize ("MANATEE.DRV", "DEMO.MLT");
#else
	SoundInitialize ("AUDIO64.DRV", "DEMO.KAT");
#endif
	/*
	 * Parse the multiplayer maps
	 */
	s = sOpen ("MP.TXT");
	ReadMPMapNames (s);
	sClose (s);

	// Initialise the language subsystems
	tLangInit ();

#if 0 // no longer!
	/* 
	 * Bring the Renderer into life
	 */
	rInitRenderer(FALSE, 3*1024*1024);
#endif

	/*
	 * Set up the LCD
	 */
	pdLcdInit();
	pdExecPeripheralServer();
	for (j = 0; j < 4; ++j) {
		lcdClear(j);
		lcdUpdate(j);
	}
	
	/* Set up Red Dog's wheel v's */
	for (j=0; j<4; j++)
	{
		for (i = 0; i < 4; ++i) {
			player[j].rdWheel[i].v = (StripPos *)syMalloc (sizeof (StripPos)*MAX_SKID);
			dAssert (player[j].rdWheel[i].v, "Whoopsie!");
			dAssert ((((int)player[j].rdWheel[i].v) & 7) == 0, "Oops");
		}
	}

	// Reset the multiplayer settings
	ResetMPSettings();

	/* Initialise the shadows */
	shInit();

	initialised = TRUE;
}


Bool initialised;
Uint32 currentFrame = 0;

/* Not the best place for this, and its slow */
#if 0
int strnicmp (const char *a, const char *b, int num)
{
	char A, B;

	while (num--) {
		A = tolower(*a);
		B = tolower(*b);
		if (A != B || A == 0)
			break;
		++a, ++b;
	}
	return (A < B) ? -1 : (A > B) ? 1 : 0;
}
#endif

int stricmp (const char *a, const char *b)
{
	char A, B;

	while (1) {
		A = tolower(*a);
		B = tolower(*b);
		if (A != B || A == 0)
			break;
		++a, ++b;
	}
	return (A < B) ? -1 : (A > B) ? 1 : 0;
}
