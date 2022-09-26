#ifndef _TGALOAD_H
#define _TGALOAD_H

#include <assert.h>
#include <math.h>

struct Pixel
{
	unsigned char a, r, g, b;
	Pixel () { a = r = g = b = 0; }
	Pixel (const Pixel &p) { a = p.a; r = p.r; g = p.g; b = p.b;}
	Pixel (const Pixel &p1, const Pixel &p2, const Pixel &p3, const Pixel &p4)
	{
		a = (p1.a + p2.a + p3.a + p4.a) >> 2;
		r = (p1.r + p2.r + p3.r + p4.r) >> 2;
		g = (p1.g + p2.g + p3.g + p4.g) >> 2;
		b = (p1.b + p2.b + p3.b + p4.b) >> 2;
	}
	void Adjust() {
		unsigned char temp;
		temp = a; a = b; b = temp;
		temp = r; r = g; g = temp;
	}
	static unsigned char R_Gamma[256];
	static unsigned char G_Gamma[256];
	static unsigned char B_Gamma[256];
	void GammaCorrect ();
};

class PixelRow
{
private:
	Pixel		*pixelRow;
	int			size;
public:
	PixelRow (Pixel *pR, int s)
	{
		size = s;
		pixelRow = pR;
	}
	Pixel &operator [] (int i)
	{
		assert (i < size);
		return pixelRow[i];
	}
};

class PVR;

#pragma pack (push, 1)

class Targa
{
private:
	int			size;
	Pixel		*pixelArray;

	struct TargaHdr {
		char		nIDBytes;
		char		mapType;
		char		imgType;
		short		mapOrigin;
		short		mapLength;
		char		mapEntrySize;
		
		short		xOrigin;
		short		yOrigin;
		short		width;
		short		height;
		char		bpp;
		char		descByte;
	};
	
	bool		ok;

	static Pixel DoMap (double vx, double vy, double vz, 
				Targa &up, Targa &dn, Targa &lf, Targa &rt, Targa &fr, Targa &bk);	
	
public:
	Targa (CString fileName, bool sizeOverride = false);
	Targa (Targa &sourcem, int resample = 0); // generate a mipmap 
	Targa (Targa &up, Targa &dn, Targa &lf, Targa &rt, Targa &fr, Targa &bk); // Create an environment map targa
	~Targa ();
	
	bool isOK() const { return ok;	}

	enum AlphaType { NONE, PUNCHTHRU, FULL };
	
	AlphaType getAlphaType();
	
	int getSize() const { return size; }
	
	PixelRow operator [] (int i)
	{
		assert (i < size);
		return PixelRow(&pixelArray[i * size], size);
	}
	
};

#pragma pack (pop)

class PVR
{
private:
	typedef unsigned short PvrPixel;
	bool		ok;
	int			size;
	PvrPixel 	*pixels;		// Pixel data
	PVR			*nextMipmap;	// Next mipmap in the chain
	unsigned int	tType;		// Which type of texture we are
	unsigned long GBIX;
	bool		isBumpMap;
	
	// here
	
	static int	xyTab[1024];
	static unsigned long Crc32Table[256];
	static bool	tabReady;
	
	void		MakeTable();
	unsigned long makeGBIX ();
	void		OutputData (CFile &file);
	
public:
	PVR (Targa &targa, int mipmap, Targa::AlphaType alphaType, Targa *baseTarga = NULL);
	~PVR ();

	bool isOK() const { if (nextMipmap) return nextMipmap->isOK() && ok; else return ok; }

	bool Output (CFile &file);
	inline unsigned long getGBIX(void) const { return GBIX; }
	void MakeBumpMap (void);
};

class SuperSampler
{
private:
	int		size;			// How big is this?
	float	*grid;			// size x size grid of floats
public:
	SuperSampler (int size);
	~SuperSampler () { if (grid) delete []grid; }
	Pixel Sample (Targa &t, int x, int y);
};

#endif
