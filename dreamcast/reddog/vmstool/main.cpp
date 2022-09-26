#include <windows.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	HBITMAP handle;
	FILE *f;
	
	if (argc < 3 || argc > 4)
		exit(1);
	
	
	handle = (HBITMAP)LoadImage (NULL, argc==3?argv[1]:argv[2], IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	if (handle == NULL)
		exit(1);
	if (argc == 4) {
		int i;
		int nPal = 0;
		unsigned short pal[16];
		unsigned char pixels[16*32];

		// vmstool colour 32x32
		HDC screenDc = GetDC(NULL);
		HDC dc;
		
		dc = CreateCompatibleDC (screenDc);
		
		memset (pixels, 0, sizeof (pixels));
		memset (pal, 0, sizeof (pal));
		(void)SelectObject (dc, (HGDIOBJ)handle);
		for (int x = 0; x < 32; ++x) {
			for (int y = 0; y < 32; ++y) {
				COLORREF ref;
				unsigned short col;
				ref =  GetPixel(dc, x, y);
				col = (unsigned short) (((ref & 0xf0)<<4) |
					((ref & 0xf000)>>8) |
					((ref & 0xf00000)>>20) | 0xf000);
				// Is it in the palette?
				for (i = 0; i < nPal; ++i) {
					if (pal[i] == col) {
						if (x & 1)
							pixels[(x>>1) + y*16] |= i;
						else
							pixels[(x>>1) + y*16] |= i<<4;
						break;
					}
				}
				if (i == nPal) {
					if (nPal == 16) {
						fprintf (stderr, "Ran out of palette entries!");
						exit(1);
					}
					pal[nPal] = col;
					if (x & 1)
						pixels[(x>>1) + y*16] |= nPal;
					else
						pixels[(x>>1) + y*16] |= nPal<<4;
					nPal++;
				}
			}
		}


		f = fopen(argv[3], "w");
		if (f == NULL)
			return 1;
		
		// Write out the palette data first
		fprintf (f, "/*\n"
			        " * Converted from %s\n"
			        " * Palette data comes first, then pixel image\n"
					" */\n", argv[2]);
		for (i = 0; i < 16; ++i)
			fprintf (f, "0x%x, 0x%x, // Palette entry %d\n", pal[i] & 0xff, pal[i]>>8, i);
		for (int y = 0; y < 32; ++y) {
			for (x = 0; x < 16; ++x)
				fprintf (f, "0x%x, ", pixels[x + y * 16]);
			fprintf (f, "// Line %d\n", y);
		}

		ReleaseDC(NULL, dc);
		ReleaseDC(NULL, screenDc);
		fclose (f);

	} else {
		// 1bpp pixel 48x32
		HDC screenDc = GetDC(NULL);
		HDC dc;
		
		dc = CreateCompatibleDC (screenDc);
		
		f = fopen(argv[2], "w");
		if (f == NULL)
			return 1;
		
		(void)SelectObject (dc, (HGDIOBJ)handle);
		
		fprintf (f, "/* Converted from %s */\n", argv[1]);
		for (int line = 0; line <32; ++line) {
			fprintf (f, "/* Line %d : */\n", line);
			for (int x = 0; x < 48; ++x) {
				if (GetPixel (dc, x, line) != 0)
					fprintf (f, "0x00");
				else
					fprintf (f, "0x08");
				if (x != 47 || (line != 31))
					fprintf (f, ", ");
				if ((x & 0x7) == 7)
					fprintf (f, "\n");
			}
			fprintf (f, "\n");
		}
		ReleaseDC(NULL, dc);
		ReleaseDC(NULL, screenDc);
		fclose (f);		
	}
	return 0;
}