/*
 * Animate.cpp
 * Texture animation
 */

#include "StdAfx.h"
#include "Convert.h"
#include "ConvertDlg.h"
#include "Progress.h"
#include "Animate.h"
#include <ctype.h>

#define TEMPFILE "C:\\WINDOWS\\TEMP\\TEMPFILE.PVR"

CString CAnimatedTexture::getFormat (CString name, int &startNum)
{
	int nZeros = 0;
	int length = name.GetLength() - 4;
	while (--length > 1) {
		if (isdigit(name[length]))
			nZeros++;
		else
			break;
	}
	if (nZeros == 0) {
		startNum = 0;
		return name + "%d";
	} else {
		CString retVal;
		startNum = atoi (name.Mid (length+1));
		retVal.Format ("%s%%0%dd.PVR", name.Mid(0, length+1), nZeros);
		return retVal;
	}
}

void CAnimatedTexture::processFrames (CString format, TexAnim anim, int frame)
{
	int fileSize = 0, initFrame = frame;
	bool firstFrame = TRUE;
	unsigned int type = 0, size = 0, GBIX;
	CString firstName;

	firstName.Format (format, frame);

	CFile output(TEMPFILE, CFile::modeReadWrite | CFile::modeCreate | CFile::typeBinary);
	do {
		CFileStatus status;
		CString fileName, tgaFile;
		fileName.Format (format, frame);
		tgaFile = CConvertDlg::getTGAFile (fileName);
		if (!CFile::GetStatus (tgaFile, status))
			break;
		if (!CFile::GetStatus (fileName, status))
			break;
		if (fileSize == 0)
			fileSize = status.m_size;
//		else if (fileSize != status.m_size)
//			break;

		CFile input(fileName, CFile::modeRead | CFile::typeBinary);
		struct {
			unsigned int	type;
			unsigned int	length;
		} header, pvrHeader;
		for (;;) {
			input.Read (&header, sizeof (header));
			if (header.type == *(unsigned int *)&"PVRT")
				break;
			if (header.type == *(unsigned int *)&"GBIX" && firstFrame) {
				input.Read (&GBIX, 4);
				input.Seek (header.length - 4, CFile::current);
			} else
				input.Seek (header.length, CFile::current);
		}

		input.Read (&pvrHeader, sizeof (pvrHeader));
		if (type == 0)
			type = pvrHeader.type, size = pvrHeader.length;
		else if (type != pvrHeader.type || size != pvrHeader.length)
			break;

		// At this point the files are the same, so output this file (outputting a blank TexAnim at the top)
		if (firstFrame) {
			output.Write (&anim, sizeof (anim));
			struct {
				unsigned int	type;
				unsigned int	length;
				unsigned int	gbix;
			} global = { *(unsigned int *)&"GBIX", 4, GBIX };
			output.Write (&global, sizeof (global));
			output.Write (&header, sizeof (header));
			output.Write (&pvrHeader, sizeof (header));
			firstFrame = FALSE;
		}

		// Now read the texture data and copy it across
		char *memory = new char[header.length - 8];
		input.Read (memory, header.length - 8);
		output.Write (memory, header.length - 8);
		delete [] memory;

		input.Close();

		frame++;
	} while (1);

	// Rewind the file pointer
	output.Seek (offsetof (TexAnim, nFrames), CFile::begin);
	// Write out nFrames
	frame -= initFrame;
	output.Write (&frame, sizeof (frame));

	// Close the output
	output.Close();

	// Delete the original .PVR file and rename the temp file
	CFile::Remove (firstName);

	CFile::Rename (TEMPFILE, firstName);

	// Stamp the output file
	CFileStatus stat;
	if (CFile::GetStatus (firstName, stat)) {
		stat.m_mtime = CTime::GetCurrentTime();
		CFile::SetStatus (firstName, stat);
	}
}

CAnimatedTexture::CAnimatedTexture (CString fileName, CString stratName, unsigned int stratSpeed, CProgress &progressBar)
	: pBar(progressBar)
{
	TexAnim anim;
	int start;
	CString format = getFormat (fileName, start);
	anim.type = TEXANIM_STRAT;
	anim.speed = stratSpeed < 1 ? 1 : stratSpeed;
	if (stratName.GetLength() > 15)
		strcpy (anim.stratAnim.name, stratName.Left (15));
	else
		strcpy (anim.stratAnim.name, stratName);
	processFrames (format, anim, start);
}

CAnimatedTexture::CAnimatedTexture (CString fileName, unsigned int stratSpeed, CProgress &progressBar)
	: pBar(progressBar)
{
	TexAnim anim;
	int start;
	CString format = getFormat (fileName, start);
	anim.type = TEXANIM_GLOBAL;
	anim.speed = stratSpeed < 1 ? 1 : stratSpeed;
	processFrames (format, anim, start);
}
