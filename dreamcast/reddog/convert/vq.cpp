#include "StdAfx.h"
#include "Convert.h"
#include "Progress.h"
#include "Vq.h"
#include <kamui.h>
#include <math.h>

CVqCompressor::Uint32 CVqCompressor::xyTab[1024];
bool CVqCompressor::bTabPrepared = false;
extern int mgMessageBox (const char *mes, char *cap = NULL, int type = MB_OK);

#define WEIGHT_AMOUNT 0.075f

void CVqCompressor::MakeTable()
{
	int i;
	Uint32 bar = 0;
	for (i = 0; i < 1024; ++i) {
		xyTab[i] = bar;
		bar = (bar + 0x55555556) & 0xaaaaaaaa;
	}
	bTabPrepared = TRUE;
}

// Constructor
CVqCompressor::CVqCompressor (CString fileName, CProgress &progress) : pBar(progress)
{
	FILE	*f;
	Uint16	*memory, *memPtr;
	char	*memEnd;
	Uint16	xSize, ySize, curSize;
	Uint32	GBIX, size;
	struct {
		int	a, b;
	}		header;
	BOOL	mipMap;

	if (!bTabPrepared)
		MakeTable();

	bOK = FALSE;

	f = fopen (fileName, "rb");

	if (f==NULL) {
		mgMessageBox ("Unable to compress file '" + fileName + "' - go have words with MattG",
			"Message from Converter", MB_OK | MB_ICONINFORMATION);
		return;
	}

	GBIX = 0;

	do {
		fread (&header, 1, sizeof (header), f);
		if (header.a == *(int *)&"PVRT")
			break;
		if (header.a == *(int *)&"GBIX")
			fread (&GBIX, 1, sizeof (GBIX), f);
		else
			fseek (f, header.b, SEEK_CUR);
	} while (!feof(f));

	if (feof(f)) {
		mgMessageBox ("Unable to compress file '" + fileName + "' - go have words with MattG",
			"Message from Converter", MB_OK | MB_ICONINFORMATION);
		return;
	}

	size = header.b - 8;

	fread (&header, 1, sizeof (header), f);
	xSize = (header.b >> 16) & 0xffff;
	ySize = (header.b) & 0xffff;

	switch (header.a & 0xf00) {
	default:
		mgMessageBox ("Unable to compress file '" + fileName + "' - unsupported type - go nag MattG",
			"Message from Converter", MB_OK | MB_ICONINFORMATION);
		return;
		break;
	case KM_TEXTURE_TWIDDLED:
		mipMap = FALSE;
		break;
	case KM_TEXTURE_TWIDDLED_MM:
		mipMap = TRUE;
		break;
	case KM_TEXTURE_VQ:
	case KM_TEXTURE_VQ_MM:
		return;
		break;
	}

	is1555 = false;
	switch (header.a & 0xff) {
	default:
		mgMessageBox ("Unable to compress file '" + fileName + "' - unsupported type - go nag MattG",
			"Message from Converter", MB_OK | MB_ICONINFORMATION);
		return;
		break;
	case 0:
		is1555 = true;
		is565 = false;
		break;
	case 1:
		is565 = true;
		break;
	case 2: //KM_TEXTURE_RGBA_4444:
		is565 = false;
		break;
	}

	if (xSize != ySize) {
		mgMessageBox ("Unable to compress file '" + fileName + "' - only square textures can be compressed",
			"Message from Converter", MB_OK | MB_ICONINFORMATION);
		return;
	}

	memory = (Uint16 *)new char [size];

	if (fread (memory, 1, size, f) != (size_t)size) {
		mgMessageBox ("Unable to compress file '" + fileName + "' - go have words with MattG",
			"Message from Converter", MB_OK | MB_ICONINFORMATION);
		return;
	}

	fclose (f);

	memEnd = ((char *)memory) + size;
	// Convert to Word format
	if (mipMap) {
		curSize = 2;
		memPtr = memory + 2;
	} else {
		curSize = xSize;
		memPtr = memory;
	}
	for ( ; curSize <= xSize; curSize<<=1) {
		Word	*mem = (Word *)new char [sizeof (Word) * (curSize/2) * (curSize/2)];

		ConvWord (mem, memPtr, curSize);

		AddDict(mem, (Uint16)(curSize/2));
 
		memPtr += curSize * curSize;
		delete [] mem;
	}

	ProcessDict();

	// Output
	f = fopen (fileName, "wb");

	if (f==NULL) {
		mgMessageBox ("Unable to compress file '" + fileName + "' - go have words with MattG",
			"Message from Converter", MB_OK | MB_ICONINFORMATION);
		return;
	}

	// Write the header
	header.a = *(int *)&"GBIX";
	header.b = 4;
	fwrite (&header, 1, sizeof (header), f);
	fwrite (&GBIX, 1, sizeof (GBIX), f);

	if (mipMap) {
		size = 0x809 + MipMapSize(xSize);
	} else {
		size = 0x808 + ((xSize * xSize) / 4);
	}

	header.a = *(int *)&"PVRT";
	header.b = size;
	fwrite (&header, 1, sizeof (header), f);

	if (mipMap) {
		GBIX = KM_TEXTURE_VQ_MM | (is565 ? 1 : is1555 ? 0 : 2);
	} else {
		GBIX = KM_TEXTURE_VQ | (is565? 1 : is1555 ? 0 : 2);
	}
	fwrite (&GBIX, 1, sizeof (GBIX), f);

	GBIX = xSize | (xSize << 16);
	fwrite (&GBIX, 1, sizeof (GBIX), f);

	for (size = 0; size < 256; ++size) {
		int i,j;
		for (j=0;j<2;++j) {
			for (i=0;i<2;++i) {
				Uint16 word;
				word = ConvBack (dict[size].c[i][j]);
				fwrite (&word, 1, sizeof (Uint16), f);
			}
		}
	}

	if (mipMap) {
		char dummy = 0;
		curSize = 2;
		memPtr = memory + 2;
		fwrite (&dummy, 1, sizeof (dummy), f);
	} else {
		curSize = xSize;
		memPtr = memory;
	}
	for ( ; curSize <= xSize; curSize<<=1) {
		Word	*mem = (Word *)new char[sizeof (Word) * (curSize/2) * (curSize/2)];

		ConvWord (mem, memPtr, curSize);

		OutputDict (mem, (Uint16)(curSize/2), f);
		
		memPtr += curSize * curSize;
		delete [] mem;;
	}

	fclose (f);

	CFileStatus stat;
	if (CFile::GetStatus (fileName, stat)) {
		CString setFile, tgaFile;
		stat.m_mtime = CTime::GetCurrentTime();
		CFile::SetStatus (fileName, stat);
	}

	delete [] memory;
	bOK = TRUE;
}

void CVqCompressor::ConvWord (Word *wPtr, Uint16 *mPtr, Uint16 size)
{
	Uint16 x, y;
	for (y = 0; y < size; y+=2) {
		for (x = 0; x < size; x+=2) {
			wPtr->c[0][0] = ConvPixel (GetPixel (mPtr, x, y));
			wPtr->c[0][1] = ConvPixel (GetPixel (mPtr, (Uint16)(x+1), y));
			wPtr->c[1][0] = ConvPixel (GetPixel (mPtr, x, (Uint16)(y+1)));
			wPtr->c[1][1] = ConvPixel (GetPixel (mPtr, (Uint16)(x+1), (Uint16)(y+1)));
			wPtr++;
		}
	}
}

CVqCompressor::Uint16 CVqCompressor::GetPixel (CVqCompressor::Uint16 *p, Uint16 x, Uint16 y)
{
	Uint32 offset = (xyTab[y]>>1) | xyTab[x];
	return p[offset];
}

CVqCompressor::LightingValue CVqCompressor::ConvPixel (CVqCompressor::Uint16 src)
{
	float a,r,g,b;
	if (is565) {
		a = 0.f;
		r = ((src >> 11) & 31) / 31.5f;
		g = ((src >> 5) & 63) / 63.5f;
		b = (src & 31) / 31.5f;
	} else if (is1555) {
		if (src & (1<<15))
			a = 1.f;
		else
			a = 0.f;
		r = ((src >> 10) & 31) / 31.5f;
		g = ((src >> 5) & 31) / 31.5f;
		b = (src & 31) / 31.5f;
	} else {
		a = ((src & 0xf000) >> 12) / 15.5f;
		r = ((src & 0xf00) >> 8) / 15.5f;
		g = ((src & 0xf0) >> 4) / 15.5f;
		b = (src & 0xf) / 15.5f;
	}
	return LightingValue (a,r,g,b);
}

CVqCompressor::Uint16 CVqCompressor::ConvBack (const LightingValue &lv)
{
	Uint8 r, g, b, a;
	if (is565) {
		r = (Uint8)(32.0 * lv.r);
		g = (Uint8)(64.0 * lv.g);
		b = (Uint8)(32.0 * lv.b);
		return (r<<11) | (g<<5) | b;
	} else if (is1555) {
		if (lv.a > 0.01f)
			a = 1;
		else
			a = 0;
		r = (Uint8)(32.0 * lv.r);
		g = (Uint8)(32.0 * lv.g);
		b = (Uint8)(32.0 * lv.b);
		return (a<<15) | (r<<10) | (g<<5) | b;
	} else {
		a = (Uint8)(16.0 * lv.a);
		r = (Uint8)(16.0 * lv.r);
		g = (Uint8)(16.0 * lv.g);
		b = (Uint8)(16.0 * lv.b);
		return (a<<12) | (r<<8) | (g<<4) | b;
	}
}

void CVqCompressor::AddDict (const Word *word, Uint16 size)
{
	Uint16 x, y;
//	printf ("Adding mipmap size %d...    ", size);
	for (y = 0; y < size; ++y) {
		for (x = 0; x < size; ++x) {
			AddWord (word[x + size * y]);
		}
//		printf ("\b\b\b\b%3d%%", (int)((100*y) / size));
	}
//	printf ("\n");
}

void CVqCompressor::OutputDict (Word *word, Uint16 size, FILE *f)
{
	Uint16 x, y;
	Uint32 c;
//	printf ("Outputting mipmap size %d\n", size);
	for (c = 0; c < (Uint32)(size*size); ++c) {
		Uint8 w;

		x = getX(c>>1);
		y = getX(c);
		w = FindWord (word[x + size * y]);
//		w = x & 7;
//		bytes++;
		fwrite (&w, 1, sizeof (Uint8), f);
	}
}

void CVqCompressor::ProcessDict (void)
{
	int i;
	int pass = 0;
	float MSE, prevMSE = 10000.f;
//	printf ("Choosing initial dictionary set\n");
	if (words.GetSize() < 256) {
		for (i=0; i<256; ++i) {
			dict[i] = words[i % words.GetSize()];
			weight[i] = WEIGHT_AMOUNT;
		}
	} else {
		int nWords;
		nWords = i = 0;
		for (nWords = 0; nWords < words.GetSize(); ++nWords) {
			for (int j = 0; j < i; ++j) {
				if (dict[j].dist(words[nWords]) < 0.000001f)
					break;
			}
			if (j != i)
				continue;
			dict[i] = words[nWords];
			weight[i] = WEIGHT_AMOUNT;
			++i;
			if (i == 256)
				break;
		}
		for (; i<256; ++i) {
			dict[i] = words[i % words.GetSize()];
			weight[i] = WEIGHT_AMOUNT;
		}
	}
	do {
		pass++;
		CString str;
		if (pass == 1)
			str.Format ("Pass %d", pass);
		else
			str.Format ("Pass %d - mean square error %.6f", pass, prevMSE);
		pBar.SetExtra (str);
		MSE = TrainVectors();
		if (pass > 150)
			break;
		if (fabs(prevMSE - MSE) < 0.000001f)
			break;
		if (MSE > prevMSE)
			break;
		prevMSE = MSE;
	} while (MSE >= 0.000001f);
}

CVqCompressor::Uint16 CVqCompressor::getX(CVqCompressor::Uint32 c)
{
	Uint16 retVal = 0;
	Uint16 bit;
	for (bit = 0; bit < 16; bit+=2) {
		if (c & (1<<bit))
			retVal |= (1<<(bit/2));
	}
	return retVal;
}

float CVqCompressor::TrainVectors ()
{
	int i;
	float SE = 0.f;
	for (i=0;i<words.GetSize();++i) {
		Uint8 nearby;
		nearby = FindWord (words[i]);
		Group (&dict[nearby], &words[i], weight[nearby]);
	}
	for (i=0;i<words.GetSize();++i) {
		Uint8 nearby;
		float dist;
		nearby = FindWord (words[i]);
		dist = dict[nearby].dist(words[i]);
#if 0
		/*
		 * New: if dist is relatively small, remove element 'i'
		 */
		if (dist < 0.001f) {
			words.RemoveAt(i);
			--i;
			weight[nearby] *= WEIGHT_AMOUNT;
		} else 
#endif
			SE += dist;
	}
	return SE / (float) words.GetSize();
}

void CVqCompressor::Group (Word *g1, const Word *g2, const float alpha)
{
	g1->c[0][0] = colLerp (g1->c[0][0], g2->c[0][0], alpha);
	g1->c[0][1] = colLerp (g1->c[0][1], g2->c[0][1], alpha);
	g1->c[1][0] = colLerp (g1->c[1][0], g2->c[1][0], alpha);
	g1->c[1][1] = colLerp (g1->c[1][1], g2->c[1][1], alpha);
}

void CVqCompressor::AddWord (const Word &word)
{
	words.Add(word);
}

CVqCompressor::Uint8 CVqCompressor::FindWord (const Word &w) const
{
	float BestDist = 100000.f;
	Uint8 BestMatch = 0;
	const Word *d = dict;
	int n;

	for (n = 0; n < 256; ++n, ++d) {
		float dist;
		dist = w.dist(*d);
		if (dist < BestDist) {
			BestDist = dist;
			BestMatch = n;
		}
	}
	return BestMatch;
}

CVqCompressor::LightingValue CVqCompressor::colLerp (const CVqCompressor::LightingValue &c1,
													 const CVqCompressor::LightingValue &c2, float alpha)
{
	return LightingValue (c1.a + (c2.a - c1.a) * alpha, 
		c1.r + (c2.r - c1.r) * alpha, c1.g + (c2.g - c1.g) * alpha,
		c1.b + (c2.b - c1.b) * alpha);
}
