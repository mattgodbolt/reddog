#include "FE.h"

// Distractions!

/////////////////////////////////////////////////
// Ones and noughts
/////////////////////////////////////////////////

class BinaryDistraction : public WindowDistraction
{
private:
	float scrollAmt, alphaMul;
	int nChars, rand1, rand2;
	int nLines;
public:
	BinaryDistraction()
	{
		scrollAmt = nChars = 0;
		alphaMul = 1.f;
		rand1 = rand();
		rand2 = rand();
		nLines = 50 + (rand() & 0x7);
	}
	virtual bool Update()
	{
		nChars+= 2;
		if (nChars > 180.f)
			scrollAmt += 35.f / 4.f;
		if (nChars > (nLines-2)*8) {
			alphaMul -= 0.05f;
		}
		if (alphaMul <= 0)
			return true;
		return false;
	}
	virtual void Draw() const
	{
		int lag = nChars;

		mPostTranslate32 (psGetMatrix(), 0, -scrollAmt);
		mPostScale32 (psGetMatrix(), 0.5f, 0.5f);
		psSetColour (0xff0000);
		psSetAlpha (0.5f * alphaMul);
		psSetPriority (100);

		for (int i = 0; i < nLines; ++i) {
			int num = (i+rand1) * rand2;
			char buf[9];
			buf[8] = '\0';
			for (int j = 0; j < 8; ++j) {
				buf[j] = (num & (1<<j)) ? '1' : '0';
			}
			if (lag <= 0)
				break;
			psDrawStringFE (buf, PHYS_SCR_X*2 - 196.f, 35*i, lag);
			lag -= 8;
		}
	}
};

/////////////////////////////////////////////////
// Hex distraction
/////////////////////////////////////////////////
class HexDistraction : public WindowDistraction
{
private:
	float scrollAmt, alphaMul;
	int nChars;
	Uint32 disassTab[50];
public:
	HexDistraction()
	{
		scrollAmt = nChars = 0;
		alphaMul = 1.f;
		for (int i = 0; i < asize(disassTab); ++i)
			disassTab[i] = rand() | (rand()<<16);
	}
	virtual bool Update()
	{
		nChars+= 2;
		if (nChars > 225.f)
			scrollAmt += 35.f / 5.f;
		if (nChars > (asize(disassTab)+2)*10) {
			alphaMul -= 0.025f;
		}
		if (alphaMul <= 0)
			return true;
		return false;
	}
	virtual void Draw() const
	{
		int lag = nChars;

		mPostTranslate32 (psGetMatrix(), 0, -scrollAmt);
		mPostScale32 (psGetMatrix(), 0.5f, 0.5f);
		psSetColour (0xff0000);
		psSetAlpha (0.5f * alphaMul);
		psSetPriority (100);

		for (int i = 0; i < asize(disassTab); ++i) {
			Uint32 num = disassTab[i];
			char buf[11];
			buf[10] = '\0';
			buf[0] = '0';
			buf[1] = 'x';

			for (int j = 0; j < 8; ++j) {
				static const char HexTable[17] = "0123456789abcdef";
				buf[j+2] = HexTable[(num & 0xf)];
				num>>=4;
			}
			if (lag <= 0)
				break;
			psDrawStringFE (buf, PHYS_SCR_X*2 - 204.f, 35*i, lag);
			lag -= 10;
		}
	}
};

/////////////////////////////////////////////////
// Disassemble distraction
/////////////////////////////////////////////////
class DisDistraction : public WindowDistraction
{
private:
	float scrollAmt, alphaMul;
	int nChars;
	Uint32 disassTab[50];
	struct OpCodeTab {
		int type;
		char *name;
	};
	static const OpCodeTab opTab[16];
public:
	DisDistraction()
	{
		scrollAmt = nChars = 0;
		alphaMul = 1.f;
		for (int i = 0; i < asize(disassTab); ++i)
			disassTab[i] = rand() | (rand()<<16);
	}
	virtual bool Update()
	{
		nChars+= 2;
		if (nChars > 360.f)
			scrollAmt += 35.f / 5.f;
		if (nChars > (asize(disassTab)+2)*16) {
			alphaMul -= 0.025f;
		}
		if (alphaMul <= 0)
			return true;
		return false;
	}
	virtual void Draw() const
	{
		int lag = nChars;

		mPostTranslate32 (psGetMatrix(), 0, -scrollAmt);
		mPostScale32 (psGetMatrix(), 0.5f, 0.5f);
		psSetColour (0xff0000);
		psSetAlpha (0.5f * alphaMul);
		psSetPriority (100);

		for (int i = 0; i < asize(disassTab); ++i) {
			Uint32 num = disassTab[i];
			char buf[64];
			int len;

			switch (opTab[num>>28].type) {
			case 0: // xxx Rx, Ry
				len = sprintf (buf, "%s R%d, R%d", opTab[num>>28].name, num & 0xf, (num & 0xf0)>>4);
				break;
			case 1: // XXX Rx, Ry, Rz
				len = sprintf (buf, "%s R%d, R%d, R%d", opTab[num>>28].name, num & 0xf, (num & 0xf0)>>4, (num & 0xf00)>>8);
				break;
			case 2: // XXX Rx, 0x1234356
				len = sprintf (buf, "%s R%d, 0x%08x", opTab[num>>28].name, num & 0xf, (num & 0xffffff0)>>4);
				break;
			case 3: // XXX 0x1234356
				len = sprintf (buf, "%s 0x%08x", opTab[num>>28].name, (num & 0xfffffff));
				break;
			}

			if (lag <= 0)
				break;
			psDrawStringFE (buf, PHYS_SCR_X*2 - 240.f, 35*i, lag);
			lag -= len;
		}
	}
};

const DisDistraction::OpCodeTab DisDistraction::opTab[16] =
{
	{ 0, "MOV" },
	{ 1, "ADD" },
	{ 1, "SUB" },
	{ 1, "MUL" },
	{ 2, "MOV" },
	{ 3, "B" },
	{ 3, "BL" },
	{ 1, "DIV" },
	{ 0, "MOV" },
	{ 1, "ADD" },
	{ 1, "SUB" },
	{ 1, "MUL" },
	{ 0, "MOV" },
	{ 1, "ADD" },
	{ 1, "SUB" },
	{ 1, "MUL" }
};

/////////////////////////////////////////////////
// Message distraction
/////////////////////////////////////////////////
class MesDistraction : public WindowDistraction
{
private:
	static int mesBase;
	int nChars, nMes, length;
	static const char *MesArray[9];
	float alphaMul, xPos;
	char buf[64];
public:
	MesDistraction()
	{
		nChars = 0;
		nMes = mesBase++;
		if (mesBase == asize (MesArray))
			mesBase = 0;
		length = strlen (MesArray[nMes]) + 80;
		alphaMul = 1.f;
		sprintf (buf, "%s....................", MesArray[nMes]);
		xPos = (PHYS_SCR_X - 16) / 0.75f  - psStrWidth(buf);
	}
	virtual bool Update()
	{
		nChars+= 1;
		if (nChars > length)
			alphaMul -= 0.01f;
		return (alphaMul <= 0);
	}
	virtual void Draw() const
	{
		mPostScale32 (psGetMatrix(), 0.75f, 0.75f);
		psSetColour (0xff0000);
		psSetAlpha (0.5f * alphaMul);
		psSetPriority (100);
		psDrawStringFE (buf, xPos, (PHYS_SCR_Y - 128)/0.75f, nChars);
	}
};
const char *MesDistraction::MesArray[9] =
{
	"Receiving telemetry",
	"Taylor polynomials calculating",
	"Processing data",
	"Data input",
	"Porter effect negated",
	"Triangulating",
	"Accumulation buffer activated",
	"Hill ions discharging",
	"Dual pipeline initiating",
};
int MesDistraction::mesBase;
/////////////////////////////////////////////////
// Factory
/////////////////////////////////////////////////

WindowDistraction *WindowDistraction::MakeRandom()
{
	switch (rand() & 3) {
	case 0:
		return new BinaryDistraction;
	case 1:
		return new HexDistraction;
	case 2:
		return new DisDistraction;
	case 3:
		return new MesDistraction;
	}
	return NULL;
}
