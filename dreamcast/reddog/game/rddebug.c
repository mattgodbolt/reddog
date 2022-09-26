/*
 * RDDebug.c
 * Debug support
 */








#include "RedDog.h"
#include <stdarg.h>
#define O_RDONLY		0x0000	/* open for read only */
#define O_WRONLY		0x0001	/* open for write only */
#define O_RDWR		0x0002	/* open for read and write */
#define O_APPEND		0x0010	/* writes done at end of file */
#define O_CREAT 		0x0020	/* create new file */
#define O_TRUNC 		0x0040	/* truncate existing file */
#define O_NOINHERIT	0x0080	/* file is not inherited by child process */
#define O_TEXT		0x0100	/* text file */
#define O_BINARY		0x0200	/* binary file */
#define O_EXCL		0x0400	/* exclusive open */
#include "DebugIO.h"

void internalfAssert (char *fail, char *mes, char *file, unsigned line)
{
	dPrintf ("Fatal assertion failure:\n%s (%s) at %s:%d\n",
		mes, fail, file, line);
#if defined(_ARTIST) || defined(_RELEASE)
	while (1) {
		int i;
		KMPACKEDARGB colour;

		colour.dwPacked = 0xffff0000;
		for (i = 0; i < 500; ++i)
			kmSetBorderColor (colour);

		colour.dwPacked = 0xff00ffff;
	
		for (i = 0; i < 500; ++i)
			kmSetBorderColor (colour);

	}
#endif
}


#ifdef _DEBUG
void internalwAssert (char *fail, char *mes, char *file, unsigned line)
{
	dPrintf
		(">> %s (%s)\n  %s:%d frame %d\n",
		mes, fail, file, line, currentFrame);
}
#endif

void dPrintf (const char *fmt, ...)
{
#ifndef _ARTIST
#ifndef _RELEASE
	char buf[1024];
	int nBytes;
	va_list args;

	va_start (args, fmt);
	nBytes = vsprintf (buf, fmt, args);
	va_end (args);
	write (STDOUT, buf, nBytes);
#endif
#endif
}

#ifndef _RELEASE

// Regression test code - check the bloody compiler is working
void RegressionTest (void)
{
	register int i;
	// Declare old C-version
	extern float __fatan2(float x, float y);

	for (i=1000;i != 0; --i) {
		float x, y;
		float a, b, c;
		x = frand() * 2 - 1;
		y = frand() * 2 - 1;
	 /*	a = atan2(x, y) * (1.f / (2*PI));
		if (a<0)a++;
		b = fatan2(x, y);
		c = __fatan2(x, y);
		dAssert (fabs(a-b) < 0.01f, "Ug");	*/
	}
	
}
#endif

#include "view.h"
#include "Render\Internal.h"
// Speed test
KMVERTEXCONTEXT test;
void SpeedBenchmark (void)
{
	PNode *RedDog;
	PNode *BattleTank;
	PNode *SefAir;
	PNode *Ferry;
	PNode *FootSoldier;
	Camera *cam;
	Stream *s;
	float xr = 0, yr = 0, zr = 0;
	int i;
	KMVERTEX2 vert[3];

	cam = CameraCreate();
	cam->type = CAMERA_LOOKAROUND;
	cam->farZ = /*GlobalFogFar = */500.f;
	cam->fogNearZ = 0.2f * cam->farZ;
	cam->u.lookDir.rotX = cam->u.lookDir.rotY = cam->u.lookDir.rotZ = 0;
	cam->fogColour.colour = 0x001040;
	cam->pos.x = 0;
	cam->pos.y = -35.f;
	cam->pos.z = 0;
	CameraSet (cam, 0);

	BeginLoading();
	s = sOpen ("TITLE.TXP");
	sSetBuf (s, GameHeap->end - 256*1024, 256*1024);
	texInit();
	texLoad(s);
	sClose (s);
	
	s = sOpen ("TITLE.LVL");
	RedDog		= PNodeLoad (NULL, s, (Allocator *)SHeapAlloc, MissionHeap);
/*	BattleTank	= PNodeLoad (NULL, s, (Allocator *)SHeapAlloc, MissionHeap);
	SefAir		= PNodeLoad (NULL, s, (Allocator *)SHeapAlloc, MissionHeap);
	Ferry		= PNodeLoad (NULL, s, (Allocator *)SHeapAlloc, MissionHeap);
	FootSoldier	= PNodeLoad (NULL, s, (Allocator *)SHeapAlloc, MissionHeap);
*/	sClose (s);
	EndLoading();

	while (1)
	{
		CameraSet (cam, 0);
		mRotateXYZ(&mCurMatrix, xr += 0.01f, yr+= 0.011f, zr-=0.009f);

		pbMark (0x000000, TRUE, PB_SFX);

#if 1
		// The blob
		mPostTranslate (&mCurMatrix, -20, 0, 0);
		rDrawObject (RedDog->obj);
		mPostTranslate (&mCurMatrix, 10, 0, 0);
		rDrawObject (RedDog->obj);
		mPostTranslate (&mCurMatrix, 10, 0, 0);
		rDrawObject (RedDog->obj);
		mPostTranslate (&mCurMatrix, 10, 0, 0);
		rDrawObject (RedDog->obj);
		mPostTranslate (&mCurMatrix, 10, 0, 0);
		rDrawObject (RedDog->obj);
		pbMark (0xff0000, TRUE, PB_SFX); // RedDog is red

#else
		// RedDog
		rDrawObject (RedDog->obj);
		pbMark (0xff0000, TRUE, PB_SFX); // RedDog is red
/*
		// Two battle tanks
		for (i = 0; i < 2; ++i)
			rDrawObject (BattleTank->obj);
		pbMark (0x00ff00, TRUE); // Battle tanks are green


		// Three sefairs
		for (i = 0; i < 1; ++i)
			rDrawObject (SefAir->obj);
		pbMark (0x0000ff, TRUE); // Sefairs are blue

		// One ferry
		mPreScale (&mCurMatrix, 0.1, 0.1, 0.1);
		rDrawObject (Ferry->obj);
		pbMark (0xffff00, TRUE); // Ferry is yellow

		// Ten footsoldiers
		mRotateXYZ(&mCurMatrix, xr += 0.01f, yr+= 0.011f, zr-=0.009f);
		for (i = 0; i < 10; ++i)
			rDrawObject (FootSoldier->obj);
		pbMark (0xff00ff, TRUE); // Footsoldiers are magenta
*/
#endif
		currentFrame++;
		rEndFrame();
	}

}

#ifndef _RELEASE
void TrapDiv0(void)
{
	set_fpscr(get_fpscr() | (1<<10));
}
void UnTrapDiv0(void)
{
	set_fpscr(get_fpscr() & ~(1<<10));
}

void TrapFPUExceptions(void)
{
	set_fpscr(get_fpscr() | (1<<11) | (1<<10));
}
void UnTrapFPUExceptions(void)
{
	set_fpscr(get_fpscr() & ~(1<<11));
}
#endif

bool ValidAddress(const void *addr)
{
	int a = (int)addr;
	// Make sure it's in normal cachable RAM
	if (a >= 0x8c000000 && a <= (0x8c000000+16*1024*1024))
		return true;
	else
		return false;
}
