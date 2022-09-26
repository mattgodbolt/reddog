#include "StdAfx.h"
#include "Convert.h"
#include "TGAload.h"
#include "Kamui.h"
#include <float.h>
#include <math.h>
#define PI 3.141592653548f
#define GAMMA_R 1.f
#define GAMMA_G 1.f
#define GAMMA_B 1.f

/*
#define ADJUST_H	0//(-12.f / 60.f)
#define GAMMA_S 	1.1f
#define MULTIPLER_S	0.8f
#define GAMMA_V		1.1f
*/
 // Constructors
	
Targa::Targa (CString fileName, bool sizeOverride /* = false */)
{
	CFile	file;

	ok = false;
	size = 0;
	pixelArray = NULL;
 
	if (!file.Open (fileName, CFile::modeRead | CFile::typeBinary | CFile::shareDenyNone))
		return;
	
	// Read the header information
	TargaHdr header;
	
	if (file.Read(&header, sizeof (header)) != sizeof (header))
		return;
	
	// Check the header information
	if ((header.imgType != 2 && header.imgType != 3) ||
/*		header.height != header.width ||*/
		(header.bpp != 16 && header.bpp != 8 && header.bpp != 24 && header.bpp != 32))
		return;
	if (!sizeOverride) {
		if ((header.height != 8 && header.height != 16 && header.height != 32 && header.height != 64 && header.height != 128 &&
			header.height != 256 && header.height != 512 && header.height != 1024))
		return;
	}
	
	// Skip past any colour map and ID
	file.Seek(header.nIDBytes + header.mapEntrySize, CFile::current);
	
	// Set up internal variables
	size = (header.height >  header.width) ? header.height : header.width;
	pixelArray = new Pixel[size*size];
	if (pixelArray == NULL)
		return;
	
	// Read the colour information
	int y, yDir;
	Pixel *pixelRow;
	
	if (header.descByte & (1<<5)) {
		y = 0; yDir = 1;
	} else {
		y = size - 1;
		yDir = -1;
	}
	char *tempMem;
	int i;
	switch (header.bpp) {
	case 8:
		tempMem = new char[size*size];
		if (tempMem == NULL)
			return;
		if (file.Read (tempMem, header.height*header.width) != UINT(header.height*header.width)) {
			delete [] tempMem;
			return;
		}
		for (i = 0; i < size; ++i) {
			pixelRow = &pixelArray[y * size];
			y += yDir;
			for (int pix = 0; pix < size; ++pix) {
				pixelRow[pix].a = 0;
				pixelRow[pix].r = pixelRow[pix].g = pixelRow[pix].b = tempMem[size*i+pix];
			}
		}

		delete [] tempMem;
		break;

	case 16:
		tempMem = new char[size*size*2];
		if (tempMem == NULL)
			return;
		if (file.Read (tempMem, header.height*header.width*2) != UINT(header.height*header.width*2)) {
			delete [] tempMem;
			return;
		}
		for (i = 0; i < size; ++i) {
			pixelRow = &pixelArray[y * size];
			y += yDir;
			for (int pix = 0; pix < size; ++pix) {
				unsigned short orig = tempMem[size*2*i+pix*2] | (tempMem[size*2*i+pix*2+1]<<8);
				if ((header.descByte &0xf)==1) {
					pixelRow[pix].a = orig & (1<<15) ? 0xff : 0;
				} else
					pixelRow[pix].a = 0;

				pixelRow[pix].r = ((orig >> 10) << 3) | (orig >> 11);
				pixelRow[pix].g = (((orig >> 5) & 0x1f) << 5) | ((orig >> 6) & 0x3);
				pixelRow[pix].b = ((orig & 0x1f) << 5) | ((orig>>1) & 0x3);
			}
		}

		delete [] tempMem;
		break;
		
	case 24:
		tempMem = new char[size*size*3];
		if (tempMem == NULL)
			return;
		if (file.Read (tempMem, header.height*header.width*3) != UINT(header.height*header.width*3)) {
			delete [] tempMem;
			return;
		}
		for (i = 0; i < size; ++i) {
			pixelRow = &pixelArray[y * size];
			y += yDir;
			for (int pix = 0; pix < size; ++pix) {
				pixelRow[pix].a = 0;
				pixelRow[pix].b = tempMem[(size*3*i)+pix*3];
				pixelRow[pix].g = tempMem[(size*3*i)+pix*3 + 1];
				pixelRow[pix].r = tempMem[(size*3*i)+pix*3 + 2];
			}
		}

		delete [] tempMem;
		break;
		
	case 32:
		for (i = 0; i < size; ++i) {
			pixelRow = &pixelArray[y * size];
			y += yDir;
			file.Read (pixelRow, size * 4);
		}
		for (i = 0; i < size * size; ++i)
			pixelArray[i].Adjust();
		break;
	}
	
	// Finally mark as OK and return
	ok = true;
	file.Close();
}

// Destructor
Targa::~Targa()
{
	if (pixelArray)
		delete [] pixelArray;
}

// Constructor based on another targa
Targa::Targa (Targa &source, int resample /* = 0 */)
{
	ok = false;
	size = 0;
	pixelArray = NULL;

	// OK to copy from this one?
	if (source.ok == false)
		return;
	
	// Are we generating a mipmap?
	if (resample) {
		if ((source.size>>resample) == 0) // Can't make a smaller map than this
			return;
		size = source.size >> resample;
		
		pixelArray = new Pixel[size*size];
		if (pixelArray == NULL)
			return;
		
		SuperSampler sampler(resample);

		int mulBy = 1<<resample;

		for (int y = 0; y < size; ++y)
			for (int x = 0; x < size; ++x)
				pixelArray[y*size + x] = sampler.Sample(source, x * mulBy, y * mulBy);
	} else {
		size = source.size;
		pixelArray = new Pixel[size*size];
		if (pixelArray == NULL)
			return;
		
		memcpy (pixelArray, source.pixelArray, sizeof (Pixel)*size*size);
	}
	
	ok = true;
}

static inline int getP (double u)
{
	int ret;
	ret = int(100.f * ((u + 1.0f) / 2.0f));
	if (ret < 0)
		ret = 0;
	if (ret >= 100)
		ret = 99;
	return ret;
}

// Create an environment map targa
Targa::Targa (Targa &up, Targa &dn, Targa &lf, Targa &rt, Targa &fr, Targa &bk)
{
	// Internally we create a 128x128 image
	ok = false;
	size = 128;
	pixelArray = new Pixel[size*size];
	if (pixelArray == NULL)
		return;
	if (!up.isOK() || !dn.isOK() || !lf.isOK() || !rt.isOK() || !fr.isOK() || !bk.isOK())
		return;
	if (up.size != dn.size || up.size != lf.size || up.size != rt.size ||
		up.size != fr.size || up.size != dn.size)
		return;
	
	for (int y = 0; y < size; ++y) {
		for (int x = 0; x < size; ++x) {
			Pixel &pixel = pixelArray[y*size + x];
			Pixel samp[4];
			// X and Y refer to a latitude and longitude, respectively
			// Generate the vector
			double vx, vy, vz;
			int i = 0;
			for (float xof = -0.5f; xof < 1.1f; xof += 1.0f) {
				for (float yof = -0.5f; yof < 1.1f; yof += 1.0f) {
					vx =  sin (PI*(x+xof) / size) * cos (2*PI*(y+yof)/size);
					vy =  -sin (PI*(x+xof) / size) * sin (2*PI*(y+yof)/size);
					vz =  cos (PI*(x+xof) / size);
					samp[i++] = DoMap (vx, vy, vz, up, dn, lf, rt, fr, bk);
				}
			}
			pixel = Pixel (samp[0], samp[1], samp[2], samp[3]);
		}
	}
	ok = TRUE;
}

Pixel Targa::DoMap (double vx, double vy, double vz, 
				Targa &up, Targa &dn, Targa &lf, Targa &rt, Targa &fr, Targa &bk)
{
	double lambda;
	double u, v;
	
	// Calculate the point of intersection with the positive Z plane
	lambda = 1.0f /  vz;
	u = vx * lambda;
	v = vy * lambda;
	if (lambda > 0.0f && u >= -1.0f && u <= 1.0f && v >= -1.0f && v <= 1.0f) {
		// A point on the Z positive plane
		return up[getP(v)][getP(u)];
	}
	
	// Calculate the point of intersection with the negative Z plane
	lambda = -lambda;
	u = vx * lambda;
	v = vy * lambda;
	if (lambda > 0.0f && u >= -1.0f && u <= 1.0f && v >= -1.0f && v <= 1.0f) {
		// A point on the Z negative plane
		return dn[getP(v)][getP(u)];
	}
		
	// Calculate the point of intersection with the positive X plane
	lambda = 1.0f /  vx;
	u = vy * lambda;
	v = -vz * lambda;
	if (lambda > 0.0f && u >= -1.0f && u <= 1.0f && v >= -1.0f && v <= 1.0f) {
		// A point on the X positive plane
		return rt[getP(v)][getP(u)];
	}
	// Calculate the point of intersection with the negative X plane
	lambda = -lambda;
	u = vy * lambda;
	v = -vz * lambda;
	if (lambda > 0.0f && u >= -1.0f && u <= 1.0f && v >= -1.0f && v <= 1.0f) {
		// A point on the X negative plane
		return lf[getP(v)][getP(u)];
	}
	
	// Calculate the point of intersection with the positive Y plane
	lambda = 1.0f /  vy;
	u = vx * lambda;
	v = -vz * lambda;
	if (lambda > 0.0f && u >= -1.0f && u <= 1.0f && v >= -1.0f && v <= 1.0f) {
		// A point on the Y positive plane
		return fr[getP(v)][getP(u)];
	}
	// Calculate the point of intersection with the negative Y plane
	lambda = -lambda;
	u = vx * lambda;
	v = -vz * lambda;
	if (lambda > 0.0f && u >= -1.0f && u <= 1.0f && v >= -1.0f && v <= 1.0f) {
		// A point on the Y negative plane
		return bk[getP(v)][getP(u)];
	}
	return Pixel();
}

// Does the targa have alpha data
Targa::AlphaType Targa::getAlphaType()
{
	AlphaType type = NONE;
	int pixel;
	
	// Check all the pixels for alpha content
	for (pixel = 0; pixel < size*size; ++pixel) {
		unsigned char c = pixelArray[pixel].a;
		if (c!=0) {
			if (c==255)
				type = PUNCHTHRU;
			else
				return FULL;
		}
	}
	return type;
}

// PVR stuff
PVR::PVR (Targa &targa, int mipmap, Targa::AlphaType alphaType, Targa *baseTarga /* = NULL */)
{
	ok = false;
	size = 0;
	pixels = NULL;
	nextMipmap = NULL;
	isBumpMap = false;
	int R, G, B;
	int x, xAdd;
	
	if (!tabReady)
		MakeTable();
	
	if (!targa.isOK())
		return;

	pixels = new PvrPixel[targa.getSize() * targa.getSize()];
	if (pixels == NULL)
		return;

	size = targa.getSize();
	
	xAdd = 1;
	R = G = B = 0;
	for (int y = 0; y < size; y++) {
		for (x = (xAdd==1) ? 0 : size - 1; (xAdd==1) ? (x < size) : (x>=0); x+=xAdd) {
			int i;
			Pixel p = targa[y][x];
			p.GammaCorrect();
			i = p.r + R;
			p.r = (i > 0xff) ? 0xff : i;

			i = p.g + G;
			p.g = (i > 0xff) ? 0xff : i;

			i = p.b + B;
			p.b = (i > 0xff) ? 0xff : i;
			int offset = (xyTab[y]>>1) | xyTab[x];
			switch (alphaType) {
			case Targa::PUNCHTHRU:
				pixels[offset] = (((p.a < 2)?0:(1<<15)) | ((p.r >> 3)<<10) | ((p.g >> 3)<<5) | (p.b >> 3));
				R&= 7;
				B&= 7;
				G&= 7;
				R += (p.r & 7);
				G += (p.g & 7);
				B += (p.b & 7);
				break;
			case Targa::FULL:
				pixels[offset] = (((p.a >> 4)<<12) | ((p.r >> 4)<<8) | ((p.g >> 4)<<4) | (p.b >> 4));
				R&= 15;
				B&= 15;
				G&= 15;
				R += (p.r & 15);
				G += (p.g & 15);
				B += (p.b & 15);
				break;
			default:
				pixels[offset] = ((p.r >> 3)<<11) | ((p.g >> 2)<<5) | (p.b >> 3);
				R&= 7;
				B&= 3;
				G&= 7;
				R += (p.r & 7);
				G += (p.g & 3);
				B += (p.b & 7);
				break;
			}
		}
		xAdd = -xAdd;
	}
	if (mipmap != -1 && size != 1) {
		if (baseTarga == NULL)
			baseTarga = &targa;
		nextMipmap = new PVR (Targa(*baseTarga, mipmap+1), mipmap+1, alphaType, baseTarga);
	}
	
	// Work out what type of texture we are
	if (mipmap != -1) {
		tType = KM_TEXTURE_TWIDDLED_MM;
	} else {
		tType = KM_TEXTURE_TWIDDLED;
	}
	
	switch (alphaType) {
	default:
		tType |= KM_TEXTURE_RGB565;
		break;
	case Targa::PUNCHTHRU:
		tType |= KM_TEXTURE_ARGB1555;
		break;
	case Targa::FULL:
		tType |= KM_TEXTURE_ARGB4444;
		break;		
	}

	ok = true;
}

unsigned long PVR::Crc32Table[256];
int PVR::xyTab[1024];

bool PVR::tabReady = false;

PVR::~PVR()
{
	if (pixels)
		delete [] pixels;
	if (nextMipmap)
		delete nextMipmap;
}

unsigned char Pixel::R_Gamma[256];
unsigned char Pixel::G_Gamma[256];
unsigned char Pixel::B_Gamma[256];

void PVR::MakeTable()
{
	int i, j;
	unsigned long crc;
	int bar = 0;
	for (i = 0; i < 1024; ++i) {
		xyTab[i] = bar;
		bar = (bar + 0x55555556) & 0xaaaaaaaa;
	}

	for (i=0; i < 256; ++i) {
		crc = (i<<24);
		for (j=0;j<8;++j)
			crc = (crc<<1) ^ ((crc & 0x80000000L) ? 0x04c11db7L : 0);
		Crc32Table[i] = crc;
	}

	for (i = 0; i < 256; ++i) {
		Pixel::R_Gamma[i] = unsigned char(255.f * (pow (i / 256.f, GAMMA_R)));
		Pixel::G_Gamma[i] = unsigned char(255.f * (pow (i / 256.f, GAMMA_G)));
		Pixel::B_Gamma[i] = unsigned char(255.f * (pow (i / 256.f, GAMMA_B)));
	}
	
	tabReady = true;
}

bool PVR::Output (CFile &file)
{
	unsigned int dataSize, xAndY;
	
	if (!isOK())
		return false;
	
	// Calculate the size of the pixel data
	dataSize = 8;
	for (PVR *pvrImage = this; pvrImage; pvrImage = pvrImage->nextMipmap)
		dataSize += pvrImage->size * pvrImage->size * 2;
	
	if (tType & KM_TEXTURE_TWIDDLED_MM)
		dataSize += 2; // Two dummy bytes in a mipmap
	
	// Make the size
	xAndY = size | (size<<16);
	
	if (isBumpMap) {
		// Change the tType
		tType &= 0xff00;
		tType |= KM_TEXTURE_BUMP;
	}

	try {
		int four = 4, gbix = makeGBIX();
		if (gbix == 0) {
			MessageBox (NULL, "Totally black image found - texture won't work!", "Error", MB_OK);
		}
		GBIX = gbix; // store value
		// Output the GBIX
		file.Write ("GBIX", 4);
		file.Write (&four, 4);
		file.Write (&gbix, 4);
		// Output the header
		file.Write ("PVRT", 4);
		file.Write (&dataSize, 4);
		file.Write (&tType, 4);
		file.Write (&xAndY,4);
	
		// Output the image data
		OutputData (file);
	} 
	catch (CFileException *excep) {
		excep->Delete();
		return false;
	}
	
	return true;
}

void PVR::OutputData (CFile &file)
{
	if (nextMipmap)
		nextMipmap->OutputData (file);
	
	// Output dummy zeros before 1x1 mipmap
	if (size==1) {
		PvrPixel dummy = 0;
		file.Write (&dummy, 2);
	}

	// Output the bytes
	file.Write (pixels, 2*size*size);
}

unsigned long PVR::makeGBIX ()
{
	unsigned long ret = 0;
	char *mem = (char *)pixels;
	int len = size * size * 2;

	while (len) {
		ret = Crc32Table[((ret>>24) ^ *mem++) & 0xff] ^ (ret<<8);
		len--;
	}
	return ret & 0x7fffffff;
}
template <class T>
static inline T Minimum (T a, T b)
{
	return (a < b) ? a : b;
}
template <class T>
static inline T Maximum (T a, T b)
{
	return (a > b) ? a : b;
}

				
// Gamma correct, and also Saturation correct
void Pixel::GammaCorrect ()
{
#if 0
	float h, s, v;
	float max, min;
	float colR = r * (1/255.f), colG = g * (1/255.f),
		colB = b * (1/255.f);

	// Convert to HSV
	
	max = Maximum (Maximum (colR, colG), Maximum(colB, colB));
	min = Minimum (Minimum (colR, colG), Minimum(colB, colB));
	
	v = max;
	
	if (max != 0.f)
		s = (max - min) / max;
	else
		s = 0.f;
	
	if (s == 0.f)
		h = 0.f;
	else {
		float delta = max - min;
		if (colR == max) 
			h = (colG - colB) / delta;
		else if (colG == max)
			h = 2.f + (colB - colR) / delta;
		else
			h = 4.f + (colR - colG) / delta;
		if (h < 0.f)
			h += 60.f;
	}
	
	// Adjust H, S and V
	h += ADJUST_H;
	s = MULTIPLER_S * (float) pow (s, GAMMA_S);
	v = (float) pow (v, GAMMA_V);

	// Convert back to rgb values
	if (s==0.f)
		colR = colG = colB = v;
	else {
		while (h >= 60.f)
			h -= 60.f;
		float i = float(floor (h));
		float f = h - i;
		float p = v * (1-s);
		float q = v * (1-(s*f));
		float t = v * (1-(s*(1-f)));
		switch (int(i)) {
		case 0: colR = v; colG = t; colB = p; break;
		case 1: colR = q; colG = v; colB = p; break;
		case 2: colR = p; colG = v; colB = t; break;
		case 3: colR = p; colG = q; colB = v; break;
		case 4: colR = t; colG = p; colB = v; break;
		case 5: colR = v; colG = p; colB = q; break;
		}
	}

	r = R_Gamma[int(colR * 255.f)];
	g = G_Gamma[int(colG * 255.f)];
	b = B_Gamma[int(colB * 255.f)];
#endif
}

// SuperSampler class
#define ABS(a) ((a)<0?(-(a)):(a))
#define CLAMP(a,b,c) ((b) < (a) ? (a) : (b) > (c) ? (c) : (b))
SuperSampler::SuperSampler (int s)
{
	assert (s != 0);
	size = (1<<s) + 1;		// supersampling by 1 == half size - uses 3x3, quarter size uses 5x5 etc
	grid = new float[size*size];
	float max = 0;
	int centre = (size-1)>>1;
	int Max = (centre+1) * (centre+1);
	float Div;
	float res;

	for (int x = 0; x < size; ++x) {
		for (int y = 0; y < size; ++y) {
			res = float (Max - (ABS(x - centre) + ABS(y - centre)));
			grid[x*size+y] = res;
			max += res;
		}
	}
	Div = 1.f / max;
	for (x = 0; x < size; ++x) {
		for (int y = 0; y < size; ++y) {
			grid[x*size+y] *= Div;
		}
	}
}

Pixel SuperSampler::Sample (Targa &t, int x, int y)
{
	int startoffset = size>>1;
	int X, Y;
	Pixel retVal;
	float a = 0, r = 0, g = 0, b = 0;

	for (Y = 0; Y < size; ++Y) {
		for (X = 0; X < size; ++X) {
			int XX, YY;
			XX = x - startoffset + X;
			YY = y - startoffset + Y;
			XX = CLAMP (0, XX, t.getSize());
			YY = CLAMP (0, YY, t.getSize());

			Pixel p = t[YY][XX];
			a += float(p.a) * grid[Y*size+X];
			r += float(p.r) * grid[Y*size+X];
			g += float(p.g) * grid[Y*size+X];
			b += float(p.b) * grid[Y*size+X];
		}
	}
	retVal.a = (char)a;
	retVal.r = (char)r;
	retVal.g = (char)g;
	retVal.b = (char)b;
	return retVal;
}

/*
 * Turns an ordinary PVR into a bump map
 */
void PVR::MakeBumpMap (void)
{
	if (!isOK())
		return;

	if (!(tType & KM_TEXTURE_RGB565))
		return;

	float *luminance = new float[size*size], *l;
	int i, x, y, mask = (size-1);
	PvrPixel *p = pixels;
	l = luminance;
	for (i = 0; i < (size*size); ++i) {
		/*
		 * Must be in 5/6/5 format for this to work
		 */
		PvrPixel pix = *p++;
		*l = (((float)(pix & 31) / 32.f) + ((float)((pix>>5) & 63) / 64.f) + ((float)((pix>>11) & 31) / 32.f)) * (1.f/3.f);
		l++;
	}

#define GETXY(x,y) ((xyTab[(y)&mask]>>1) | xyTab[(x)&mask])

	for (y = 0; y < size; ++y) {
		for (x = 0; x < size; ++x) {
			float top, bottom;
			float left, right;
			float height, perturbY, perturbX, perturbZ, rScale;
			float S, R;
			unsigned char s, r;
			/* Appallingly suboptimal */
			height		= luminance[GETXY(x,y)];
			top			= height - luminance[GETXY(x,y-1)];
			bottom		= luminance[GETXY(x,y+1)] - height;
			left		= height - luminance[GETXY(x-1, y)];
			right		= luminance[GETXY(x+1, y)] - height;

			// *0.5 taken out to accentuate the bumps
			perturbY = (top  + bottom)/* * 0.5f */;
			perturbX = (left + right)/* * 0.5f */;
			perturbZ = 1.f;
			rScale = (float)(1.f / (sqrt (perturbX*perturbX + perturbY*perturbY + 1.f*1.f)));
			perturbX *= rScale;
			perturbY *= rScale;
			perturbZ *= rScale;

			S = (float)asin (perturbZ); assert (S>=0 && S <= PI/2);
			R = (float)atan2 (perturbY, perturbX);
			
			s = (unsigned char) (255.f * (S * 1.f/(PI/2)));
			r = (unsigned char) (255.f * (R * 1.f/(2*PI)));
			pixels[GETXY(x,y)] = r | (s<<8);
		}
	}
	delete [] luminance;

	isBumpMap = true;

	if (nextMipmap)
		nextMipmap->MakeBumpMap();
}
