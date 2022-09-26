#include "Menus.h"
#include "MiscScreens.h"

/////////////////////////////////////////////////////
// Move the screen around
/////////////////////////////////////////////////////

float adjustedX, adjustedY;
class AdjustScreen : public Screen
{
private:
	UVSet set;
	float increment, timer;
public:
	// Initialise a screen prior to drawing
	void Initialise()
	{
		// XXX
		set = UVSet(0, 0, 640.f/1024.f, 480.f/1024.f);
		increment = 1.f;
		timer = 0;
	}
	// Draws a screen 
	void Draw() const
	{
		psSetAlpha(1);
		psSetPriority(0);
		psSetColour (COL_TEXT);
		psCentreWordWrapFE ("`ADJSCREENEXPLAIN", 170, 53, 170 + 301, 53 + 50, 0.6f, timer);
	}
	// Processes input
	void ProcessInput(Uint32 press)
	{
		if (press & (PDD_CANCEL|PDD_OK)) {
			FE_ACK();
			Back();
			return;
		} else if (press & PDD_DGT_TY) {
			adjustedX = adjustedY = increment = 0;
		}
		if (press & PDD_DGT_KL) {
			adjustedX-=increment;
		} else if (press & PDD_DGT_KR) {
			adjustedX+=increment;
		}
		if (press & PDD_DGT_KU) {
			adjustedY-=increment;
		} else if (press & PDD_DGT_KD) {
			adjustedY+=increment;
		}

		if (press & (PDD_DGT_KU|PDD_DGT_KD|PDD_DGT_KL|PDD_DGT_KR))
			increment *= 1.2f;
		else 
			increment = 1.f;
		adjustedX = RANGE(-64, adjustedX, 64);
		adjustedY = RANGE(-64, adjustedY, 64);
		increment = RANGE(1, increment, 15);
		kmAdjustDisplayCenter (adjustedX, adjustedY);

		timer += 2;
	}
	// Finalises a screen
	void Finalise()
	{
		// Somehow save the settings here
	}
	virtual bool HasShutdown() { return true; }
	virtual Material *Background() { return &testCard; }
	virtual Uint32 BackgroundColour() { return 0xffffff; }
	virtual UVSet *BackgroundOverride() { return &set; }
};
// The example adjust screen
Screen *adjustScreen() { return new AdjustScreen; }

/////////////////////////////////////////////////////
// 50/60 switch screen
/////////////////////////////////////////////////////

class HZScreen : public Menu
{
public:
	HZScreen() : Menu(&toggleOptions)
	{}
	virtual float DrawExtra(int line, float alpha, float nChars) const
	{
		char buf[64];
		int len;

		len = sprintf (buf, LangLookup("CURRENTLYRUN"), PAL?50:60);
		if (nChars < 0)
			return len;
		psSetAlpha (alpha);
		psSetColour (COL_TEXT);
		psDrawStringFE (buf, 0, 0, nChars);
		return float(len);
	}
	virtual int NumExtra () const{ return 1; }
};

Screen *hzScreen() { return new HZScreen; }

/////////////////////////////////////////////////////
// 60Hz test screen
/////////////////////////////////////////////////////
class Test60Screen2 : public Screen
{
private:
	UVSet set;
	float timer;
	bool origPAL;
public:
	// Initialise a screen prior to drawing
	void Initialise()
	{
		set = UVSet(0, 0, 640.f/1024.f, 480.f/1024.f);
		origPAL = PAL;
		if (PAL) {
			ShutdownFrontEnd();
			rShutdownRenderer();
			PAL = false;
			sbReInitMode (PAL ? KM_DSPMODE_PALNI640x480EXT : KM_DSPMODE_NTSCNI640x480);
			rInitRenderer(TRUE, FE_TEXRAM);
			InitialiseFrontEnd();
			theWindowSequence->Recreate();
		}
		timer = 0;
	}
	// Draws a screen 
	void Draw() const
	{
		psSetAlpha(1);
		psSetPriority(0);
		psSetColour (COL_TEXT);
		psCentreWordWrapFE (psFormat (LangLookup("EXPLAINDURING"), 10 - int(timer / (4*FRAMES_PER_SECOND))), 170, 53, 170 + 301, 53 + 50, 0.65f, timer);
	}
	// Processes input
	void ProcessInput(Uint32 press)
	{
		timer += 2;
		if (press & (PDD_CANCEL|PDD_OK) || timer > (40*FRAMES_PER_SECOND))
			Back();
	}
	// Finalises a screen
	void Finalise()
	{
		if (origPAL) {
			ShutdownFrontEnd();
			rShutdownRenderer();
			PAL = true;
			sbReInitMode (PAL ? KM_DSPMODE_PALNI640x480EXT : KM_DSPMODE_NTSCNI640x480);
			rInitRenderer(TRUE, FE_TEXRAM);
			InitialiseFrontEnd();
			theWindowSequence->Recreate();
		}
	}
	virtual bool HasShutdown() { return true; }
	virtual Material *Background() { return &testCard; }
	virtual Uint32 BackgroundColour() { return 0xffffff; }
	virtual UVSet *BackgroundOverride() { return &set; }
};
Screen *test60Screen2() { return new Test60Screen2; }

class Test60Screen : public Screen
{
private:
	int timer;
	int firstTime;
public:
	Test60Screen() { firstTime = -1; }
	void Initialise()
	{
		timer = 0;
		firstTime++;
	}
	void Draw() const
	{
		float adj = 0;
		const char *text;
		psSetAlpha(1);
		psSetPriority(1);
		psSetColour (COL_TEXT);
		if (firstTime == 0) {
			text = LangLookup ("EXPLAINPRE");
		} else {
			text = LangLookup ("EXPLAINPOST");
		}
		psCentreWordWrapFE (text, 
			40, 196, PHYS_SCR_X - 64, PHYS_SCR_Y-256-96, 1.f, timer);
		adj += strlen (text);
		psDrawCentredFE ("`PRESSABX", PHYS_SCR_Y-128-35, timer - adj);
		adj += strlen ("`PRESSABX");
		if (timer > adj) {
			Menu::PendingCursor();
		}
	}
	void ProcessInput (Uint32 press)
	{
		timer += 2;
		if (press & (PDD_CANCEL|PDD_OK)) {
			if (firstTime == 0)
				Change (ScreenID(test60Screen2));
			else Back();
		}
	}
	void Finalise() {}
	virtual Uint32 BackgroundColour() { return 0x808080; }
};

Screen *test60Screen() { return new Test60Screen; }


////////////////////////////////////////////////////
// Allow jumping of fe state
////////////////////////////////////////////////////
class StateChanger : public Screen
{
private:
	int init;
public:
	StateChanger()
	{
		init = 0;
	}
	virtual void Initialise() {
		init++; 
	}	
	virtual void Finalise() {}
	virtual void Draw() const {}
	virtual void ProcessInput(Uint32 press)
	{
		if (init == 1) {
			SpecialCheatingNumber = TAST_Person;
			DoTheChange();
		} else
			Back();
	}
	virtual bool HasShutdown() { return true; }
	virtual bool SquashOverride() { return true; }
	virtual Uint32 BackgroundColour()
	{
		if (init == 2)
			return 0;
		else
			return Screen::BackgroundColour();
	}
	virtual void DoTheChange()
	{
		ScreenID list[4] = { ScreenID(pressStart), ScreenID(pressStart), ScreenID(pressStart), ScreenID(pressStart) };
		theWindowSequence->Proceed(list, 4);
	}
};
Screen *stateChanger() { return new StateChanger; }

/////////////////////////////////////////
// Go back to selection
/////////////////////////////////////////

class ReselectChanger : public StateChanger
{
	virtual void DoTheChange()
	{
		GetRoot()->UnlockAllProfiles();
		theWindowSequence->Back();
		StateChanger::DoTheChange();
	}	
};
Screen *reselectChanger() 
{ 
	return new ReselectChanger;
}


class MPSetupChanger : public StateChanger
{
	virtual void DoTheChange()
	{
		// Count the number of players we have ready
		NumberOfPlayers = 0;
		for (int i = 0; i < 4; ++i) 
			if (Window::GetPlayerState(i) == Window::WAITING_FOR_OTHERS)
				NumberOfPlayers++;
		ScreenID id(&gameSelect);
		theWindowSequence->Back();
		theWindowSequence->Proceed(&id, 1);
	}
};

Screen *mpSetupChanger() { return new MPSetupChanger; }

class MPOptionsMenu : public Menu
{
public:
	MPOptionsMenu() : Menu(&mpOptionsMenu)
	{
	}
	virtual void Draw() const
	{
		psSetAlpha (1);
		psSetColour (TankPalette[CurProfile[GetPlayer()].multiplayerStats.colour]);
		psDrawCentredFE (CurProfile[GetPlayer()].name, 2, 1000);

		mPostTranslate32 (psGetMatrix(), 0, 4.f);
		Menu::Draw();
	}
	virtual void ProcessInput (Uint32 press)
	{
		if (press & PDD_CANCEL) {
			Change (readyScreen);
		} else
			Menu::ProcessInput (press);
	}
};

Screen *MPOptionsMenuMaker() { return new MPOptionsMenu; }

class MPOptionChanger : public StateChanger
{
public:
	virtual void DoTheChange()
	{
		int i;
		ScreenID list[4];
		for (i = 0;i < 4; ++i) {
			if (Window::GetPlayerState(i) == Window::WAITING_FOR_OTHERS)
				list[i] = ScreenID(ScreenID(MPOptionsMenuMaker));
		}
		theWindowSequence->Proceed(list, 4);
	}
};
Screen *mpOptionChanger() { return new MPOptionChanger; }

class MPStatsChanger : public StateChanger
{
public:
	virtual void DoTheChange()
	{
		int i;
		ScreenID list[4];
		for (i = 0;i < 4; ++i) {
			if (Window::GetPlayerState(i) == Window::WAITING_FOR_OTHERS)
				list[i] = ScreenID(ScreenID(mpStatsScreen));
		}
		theWindowSequence->Proceed(list, 4);
	}
};
Screen *mpStatsChanger() { return new MPStatsChanger; }

class BackStateChanger : public Screen
{
public:
	virtual void Initialise() 
	{ 
		SpecialCheatingNumber = TAST_Person;
//		CurrentFrontEndState = NextFrontEndState;
	}	
	virtual void Finalise() {}
	virtual void Draw() const {}
	virtual void ProcessInput(Uint32 press)
	{
		theWindowSequence->Back();
	}
	virtual bool HasShutdown() { return true; }
	virtual bool SquashOverride() { return true; }
};
Screen *backStateChanger() { return new BackStateChanger; }


////////////////////////////////////////////////////
// VMU select screen and supporting sprite function
////////////////////////////////////////////////////
#define SPRITE_FALLOFF 16
Uint32 FESpriteColour = COL_TEXT;
float FEZ = 100.f;
void DrawFESprite (Material *mat, float x0, float y0, float width, float height, float cutOff, float visAmt, float addAmt, float alpha)
{
	float y1, x1, w[4], bottom, top;
	float v, v2;
	Uint32 endCol, topCol, specCol;

	// Not visible yet?
	if (cutOff < -SPRITE_FALLOFF)
		return;

	// Sound action?
	if (cutOff < height && (currentFrame & 1))
		FE_Sound();

	cutOff += y0;

	// Work out where we need to place this
	psGetWindow(w);

	addAmt = RANGE(0, addAmt, 1);
	specCol = (int(addAmt*255)<<16)|(int(addAmt*255)<<8)|(int(addAmt*255)<<0);

	// How much can be drawn at 'normal' colour?
	x1 = x0 + width;
	y1 = bottom = y0 + height;
	if (y1 > (cutOff - SPRITE_FALLOFF))
		y1 = (cutOff - SPRITE_FALLOFF);

	v = (y1 - y0) / height;

	rSetMaterial (mat);
	kmxxGetCurrentPtr (&vertBuf);
	kmxxStartVertexStrip (&vertBuf);

	topCol = ColLerp (0, FESpriteColour, visAmt) | (int(alpha*255)<<24);

	kmxxSetVertex_3 (0xe0000000, x0+w[1], y0+w[0], FEZ, 0, 0, topCol, specCol);
	kmxxSetVertex_3 (0xe0000000, x0+w[1], y1+w[0], FEZ, 0, v, topCol, specCol);
	kmxxSetVertex_3 (0xe0000000, x1+w[1], y0+w[0], FEZ, 1, 0, topCol, specCol);
	kmxxSetVertex_3 (0xf0000000, x1+w[1], y1+w[0], FEZ, 1, v, topCol, specCol);

	// Do we need to draw the end bit?
	top = y0;
	y1 = cutOff;
	y0 = y1 - SPRITE_FALLOFF;
	if (y0 < top)
		y0 = top;
	if (y0 < bottom) {
		endCol = 0xffffffff;

		v2 = (y1 - top) / height;
		kmxxSetVertex_3 (0xe0000000, x0+w[1], y0+w[0], 100, 0, v, topCol, specCol);
		kmxxSetVertex_3 (0xe0000000, x0+w[1], y1+w[0], 100, 0, v2, endCol, specCol);
		kmxxSetVertex_3 (0xe0000000, x1+w[1], y0+w[0], 100, 1, v, topCol, specCol);
		kmxxSetVertex_3 (0xf0000000, x1+w[1], y1+w[0], 100, 1, v2, endCol, specCol);
	}

	kmxxReleaseCurrentPtr (&vertBuf);
}

void VMUSelectScreen::Initialise()
{
	timer = 0;
	initialVMUCheck = true;
}
void VMUSelectScreen::Draw() const
{
	int i, drawAmt;
	float xSize, ySize, xAdd, yAdd, yGap;
	xSize = (currentCamera->screenX - 16) / 5.f;
	ySize = xSize;
	xAdd = 3.f * xSize / 4.f - 8.f;
	yAdd = (currentCamera->screenY - 64) / 4.f - 32.f;
	if (currentCamera->screenY < 400)
		yAdd += 16.f;
	yGap = ySize / 2.5f;
	
	drawAmt = timer;
	for (i = 0; i < 8; ++i) {
		Matrix32 m;
		float x, y;
		const int lookup[8] = { 0, 2, 4, 6, 1, 3, 5, 7 };
		
		memcpy (&m, psGetMatrix(), sizeof (m));
		
		x = xAdd + (lookup[i]>>1) * xSize;
		y = yAdd + (lookup[i] & 1) * (ySize + yGap);
		DrawFESprite (&vmuMaterial, x, y, xSize, ySize, drawAmt, 
			theVMUManager->GetVMU(lookup[i])->Connected() ? 1.f : 0.25f,
			(lookup[i] == cursor) ? ((currentFrame & 8) ? 1.f : 0.75f) : 0.f);
		
		char text[3];
		text[0] = 'A' + (lookup[i]>>1);
		text[1] = '1' + (lookup[i] & 1);
		text[2] = '\0';
		psSetColour ((lookup[i] == cursor) ? 0xffffff : COL_TEXT);
		mPostTranslate32 (psGetMatrix(), x + xAdd/2 - ((psStrWidth(text)/psGetMultiplier())/2), y + ySize);
		psDrawStringFE (text, 0, 0, drawAmt / 8.f);
		drawAmt -= ySize;
		
		memcpy (psGetMatrix(), &m, sizeof (m));
	}
	
	psSetAlpha (1);
	psSetPriority(0);
	psSetColour (COL_TEXT);
	
	psDrawCentredFE (TitleMessage(), 0.f, timer/8.f);
}
void VMUSelectScreen::ProcessInput(Uint32 press)
{
	// Ensure we haven't lost all the VMUs
	int nVMUs = 0;
	
	if (initialVMUCheck) {
		// Lets count the VMUs
		int nVMUs = 0, vmu = 0;
		for (int i = 0; i < 8; ++i) {
			if (theVMUManager->GetVMU(i)->Connected()) {
				if (cursor == -1)
					cursor = i;
				vmu = i;
				nVMUs ++;
			}
		}
		if (nVMUs == 0) {
			NoVMUs();
			return;
		} else if (nVMUs == 1) {
			SelectVMU (vmu, true);
		}
		initialVMUCheck = false;
	} else {	
		for (int i = 0; i < 8; ++i) {
			if (theVMUManager->GetVMU(i)->Connected())
				nVMUs ++;
			else {
				if (cursor == i) {
					// We've lost the cursor VMU
					press |= PDD_DGT_KR;
				}
			}
		}
		if (nVMUs == 0 || (press & PDD_CANCEL)) {
			if (press & PDD_CANCEL)
				FE_ACK();
			Back();
			return;
		}
	}
	
	// Move the cursor as necessary
	int i;
	if (press & PDD_DGT_KR) {
		for (i = 2; i < 8; i+=2) {
			int vmuNum = (cursor + i) & 7;
			if (theVMUManager->GetVMU(vmuNum)->Connected()) {
				cursor = vmuNum;
				break;
			}
			vmuNum ^= 1;
			if (theVMUManager->GetVMU(vmuNum)->Connected()) {
				cursor = vmuNum;
				break;
			}
		}
		FE_MoveSound();
	} else if (press & PDD_DGT_KL) {
		for (i = 2; i < 8; i+=2) {
			int vmuNum = (cursor - i) & 7;
			if (theVMUManager->GetVMU(vmuNum)->Connected()) {
				cursor = vmuNum;
				break;
			}
			vmuNum ^= 1;
			if (theVMUManager->GetVMU(vmuNum)->Connected()) {
				cursor = vmuNum;
				break;
			}
		}
		FE_MoveSound();
	} else if (press & (PDD_DGT_KU|PDD_DGT_KD)) {
		if (theVMUManager->GetVMU(cursor ^ 1)->Connected())
			cursor ^= 1;
	} else if (press & PDD_OK) {
		SelectVMU (cursor, false);
		FE_ACK();
	}
	
	dAssert (theVMUManager->GetVMU(cursor)->Connected(), "Not connected!");
	
	timer += 16 / psGetMultiplier();
}
Screen *vmuSelectScreen() { return new VMUSelectScreen; }

// Multiplayer select VMU for saving
class MPVMUSelectScreen : public VMUSelectScreen
{
public:
	virtual void Initialise()
	{
		VMUSelectScreen::Initialise();
		GetRoot()->UnlockProfile();
	}
	virtual const char *TitleMessage() const
	{ 
		#if USRELEASE
			return "`USWHICHVMU";
		#else
			return "`WHICHVMU";
		#endif
	}
	virtual void SelectVMU(int number, bool replace) 
	{
		if (!replace)
			Change (ScreenID(singleVMUProfile, (void *)number));
		else
			Replace (ScreenID(singleVMUProfile, (void *)number));
	};
};
Screen *mpVMUSelectScreen() { return new MPVMUSelectScreen; }

//////////////////////////////////////////////////////////////
// Question with two answers
//////////////////////////////////////////////////////////////
TwoAnswers::TwoAnswers (TA_Block *parm) : block(*parm) 
{ 
	delete parm; 
	yesEntry = new MenuEntry (block.yes, ScreenID());
	noEntry = new MenuEntry (block.no, ScreenID());
}
TwoAnswers::~TwoAnswers ()
{
	delete yesEntry;
	delete noEntry;
}
void TwoAnswers::Initialise()
{
	curOption = timer = 0;
}
void TwoAnswers::Finalise()
{
}
void TwoAnswers::Draw() const
{
	int adj = 0;

	psSetPriority(0);
	psSetAlpha (1);
	psSetColour (COL_TEXT);

	psDrawCentredFE (block.title, 0, timer - adj);
	adj += strlen (block.title);
	mPreTranslate32 (psGetMatrix(), 30, 70);

	adj += noEntry->Draw((curOption == 0) ? 1.f : 0.f, 1.f, timer - adj);
	if (curOption == 0)
		psDrawString ("*", -25, 0);
	mPreTranslate32 (psGetMatrix(), 0, 35);
	adj += yesEntry->Draw((curOption == 1) ? 1.f : 0.f, 1.f, timer - adj);
	if (curOption == 1)
		psDrawString ("*", -25, 0);
	if (timer > adj)
		Menu::PendingCursor();
}
void TwoAnswers::ProcessInput(Uint32 press)
{
	timer += 2;
	if (press & PDD_CANCEL) {
		FE_ACK();
		Back();
	} else if (press & (PDD_DGT_KU|PDD_DGT_KD)) {
		FE_MoveSound();
		curOption ^= 1;
	} else if (press & PDD_OK) {
		FE_ACK();
		if (curOption == 0)
			Back();
		else
			Replace (block.onYes);
	}
}
Screen *twoAnswers(void *vo) { return new TwoAnswers((TA_Block *)vo); }


////////////////////////////////////////////////////
// Enter your name kind of doobry
////////////////////////////////////////////////////
EnterName::EnterName() : Screen()
{
	strcpy (nameBuf, GetInitialName());
	cursor = 0;
}
void EnterName::Initialise()
{
	timer = 0;
}
void EnterName::Finalise()
{
}
void EnterName::Draw() const
{
	const char *expl1 = LangLookup("VMUEXPLAIN1");
	const char *expl2 = LangLookup("VMUEXPLAIN2");
	int adj = 0;

	psSetPriority(0);
	psSetAlpha (1);
	psSetColour (COL_TEXT);

	psDrawCentredFE (GetTitle(), 0, timer - adj);
	adj += strlen (GetTitle());

	mPreTranslate32(psGetMatrix(), 0, 70);
	float curPos = psDrawCentredFE (nameBuf, 0, timer - adj);
	extern bool SoundOverride;
	SoundOverride = true;
	if (currentFrame & 8)
		Menu::Cursor (curPos, 0);
	SoundOverride = false;
	adj += strlen (nameBuf);

	int i, start, end;
	start = cursor - 4;
	if (start < 0)
		start += psNumChars();
	end = (cursor + 5) % psNumChars();

	mPreTranslate32(psGetMatrix(), 0, 70);
	Matrix32 m;
	memcpy (&m, psGetMatrix(), sizeof (m));
	float f[4];
	psGetWindow(f);
	mPreTranslate32(psGetMatrix(), (f[2]-f[1])*psGetMultiplier()/2 - 9*35/2, 0);
	for (i = start; i != end; i = (i+1) % psNumChars()) {
		char character[2] = { 'a', '\0' };
		character[0] = psGetChar(i);
		psSetColour (ColLerp (COL_TEXT, 0xffffff, 1.f - RANGE(0, SQR(i - cursor), 1)));
		psDrawStringFE (character, 0, 0, timer - adj);
		mPreTranslate32(psGetMatrix(), 35, 0);
		adj++;
	}

	memcpy (psGetMatrix(), &m, sizeof (m));
	mPreTranslate32(psGetMatrix(), 0, 70);

	psDrawCentredFE (expl1, 0, timer - adj);
	adj += strlen (expl1);

	mPreTranslate32(psGetMatrix(), 0, 35);
	psDrawCentredFE (expl2, 0, timer - adj);
	adj += strlen (expl2);

}

void EnterName::ProcessInput (Uint32 press)
{
	if (press & PDD_CANCEL) {
		if (nameBuf[0] == '\0') {
			FE_ACK();
			Back();
		} else {
			FE_MoveSound();
			nameBuf[strlen(nameBuf)-1] = '\0';
		}
	} else if (press & PDD_DGT_KL) {
		FE_MoveSound();
		cursor = cursor - 1;
		if (cursor < 0)
			cursor += psNumChars();
	} else if (press & PDD_DGT_KR) {
		FE_MoveSound();
		cursor = (cursor + 1) % psNumChars();
	} else if (press & PDD_DGT_TA) {
		int len = strlen (nameBuf);
		if (len == 8) {
			FE_NACK();
		} else {
			FE_MoveSound();
			nameBuf[len] = psGetChar(cursor);
			nameBuf[len+1] = '\0';
		}
	} else if (press & (PDD_DGT_KU|PDD_DGT_KD)) {
		if (cursor < 26*2) {
			FE_MoveSound();
			cursor = (cursor > 25) ? (cursor - 26) : (cursor + 26);
		} else {
			FE_NACK();
		}
	} else if (press & PDD_DGT_ST) {
		if (nameBuf[0] == '\0')
			FE_NACK();
		else
			Select (nameBuf);
	}
	timer += 2;
}




