#ifndef _MISCSCREENS_H
#define _MISCSCREENS_H

struct TA_Block
{
	char		*title;
	char		*yes;
	char		*no;
	ScreenID	onYes;
	TA_Block (char *t, char *y, char *n, ScreenID &id) : onYes(id)
	{
		title = t;
		yes = y;
		no = n;
	}
};

class TwoAnswers : public Screen
{
private:
	TA_Block block;
	int curOption, timer;
	MenuEntry *yesEntry, *noEntry;
public:
	// Created with a single parm, that being the address of a TA_Block
	TwoAnswers (TA_Block *parm);
	virtual ~TwoAnswers();
	// Inherited from Screen
	virtual void Initialise();
	virtual void Finalise();
	virtual void Draw() const;
	virtual void ProcessInput (Uint32 press);
};

////////////////////////////////////

class EnterName : public Screen
{
private:
	char nameBuf[16];
	int cursor, timer;
public:
	EnterName();
	// ...must...write...virtual...destructors.....
	virtual ~EnterName() {}
	// Inherited from Screen
	virtual void Initialise();
	virtual void Finalise();
	virtual void Draw() const;
	virtual void ProcessInput (Uint32 press);
	virtual Uint32 BackgroundColour() { return 0x203020; }
	// Other madness
	virtual const char *GetTitle() const = 0;
	virtual void Select(const char *name) = 0;
	virtual const char *GetInitialName() const { return ""; }
};	

class VMUSelectScreen : public Screen
{
private:
	int timer, cursor;
	bool initialVMUCheck;
public:
	VMUSelectScreen() : Screen() { cursor = -1; }
	// Inherited from Screen
	virtual void Initialise();
	virtual void Draw() const;
	virtual void ProcessInput(Uint32 press);
	virtual void Finalise() {}
	virtual Uint32 BackgroundColour() { return 0x203020; }

	// Overridables:
	virtual const char *TitleMessage() const
	{
		#if USRELEASE
			return "`USSELVMU";
		#else
			return "`SELVMU";
		#endif
	}
	virtual void SelectVMU(int number, bool replace) {};
	virtual void NoVMUs() { Back(); }
};

// Game type entry
class GameTypeEntry : public MenuEntry
{
private:
	float len;
protected:
	virtual void SetText (char *text);
public:
	GameTypeEntry(const char *text, ScreenID &sub) : MenuEntry(text, sub) 
	{
		len = psStrWidth(text) + 16.f;
	}
	virtual float Draw(float selAmt, float alpha, float nChars);
	virtual void Left();
	virtual void Right();
	virtual const char *GetText();
	virtual float GetScale();
};

void DrawLineFE (float x0, float y0, float x1, float y1, float len, float dist, float alpha);

#endif
