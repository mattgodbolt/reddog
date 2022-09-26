#ifndef _VQ_H
#define _VQ_H

#include <afxtempl.h>

class CVqCompressor
{
protected:
	// Types
	typedef unsigned short Uint16;
	typedef unsigned long Uint32;
	typedef unsigned char Uint8;
	struct LightingValue
	{
		float a,r,g,b;
		LightingValue (float A, float R, float G, float B)
		{
			a = A; r = R; g = G; b = B;
		}
		inline float dist (const LightingValue &c2) const
		{
			float aDist, rDist, gDist, bDist;

			aDist = 8.f * (a - c2.a); /* More priority to alpha */
			rDist = 1.f * (r - c2.r);
			gDist = 1.f * 6.f/5.f * (g - c2.g); /* More priority to green */
			bDist = 1.f * (b - c2.b);

			return (aDist*aDist + rDist*rDist + gDist*gDist + bDist*bDist);
		}
		inline friend bool operator == (const LightingValue &a, const LightingValue &b)
		{
			return (a.a == b.a && a.r == b.r && a.g == b.g && a.b == b.b);
		}
		LightingValue ()
		{
			a = 0.f; r = 0.f; g = 0.f; b = 0.f;
		}
	};
	struct Word
	{
		LightingValue		c[2][2];
		float dist (const Word &b) const
		{
			return c[0][0].dist (b.c[0][0]) +
				c[0][1].dist (b.c[0][1]) +
				c[1][0].dist (b.c[1][0]) +
				c[1][1].dist (b.c[1][1]) * 0.25f;
		}
		Word() { }
	};

	// Class variables
	static Uint32			xyTab[1024];
	static bool				bTabPrepared;

	// Instance variables
	bool					is565, is1555;
	bool					bOK;
	Word					dict[256];
	float					weight[256];
	CArray<Word, const Word &>	words;
	CProgress				&pBar;

	// Internal functions
	void	MakeTable();
	void	AddWord (const Word &word);
	void	AddDict (const Word *word, Uint16 size);
	int MipMapSize (Uint16 size)
	{
		if (size==2)
			return 1;
		else
			return MipMapSize((Uint16)(size>>1)) + ((size*size) / 4);
	}
	LightingValue colLerp (const LightingValue &c1, const LightingValue &c2, float alpha);
	Uint8 FindWord (const Word &w) const;
	void ConvWord (Word *wPtr, Uint16 *mPtr, Uint16 size);
	Uint16 GetPixel (Uint16 *p, Uint16 x, Uint16 y);
	LightingValue ConvPixel (Uint16 src);
	Uint16 ConvBack (const LightingValue &lv);
	void ProcessDict (void);
	void OutputDict (Word *word, Uint16 size, FILE *f);
	Uint16 getX (Uint32 x);
	float TrainVectors ();
	void Group (Word *g1, const Word *g2, const float alpha);

public:
	CVqCompressor (CString fileName, CProgress &progress);
	~CVqCompressor() {}
	inline bool isOK () { return bOK; }
};

#endif
