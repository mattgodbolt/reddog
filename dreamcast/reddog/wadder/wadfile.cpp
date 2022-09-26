// WadFile.cpp : implementation file
//

#include "stdafx.h"
#include "Wadder.h"
#include "WadFile.h"
#include "RedDog.h"
#include <assert.h>

static char sector[2048];

// Constructor
WadFile::WadFile (CString fileName)
{
	Open (fileName);
}
WadFile::WadFile ()
{
}

// Opener
void WadFile::Open (CString fileName)
{
	CFileException e;
	nOutput = 0;
	if (!output.Open (fileName, CFile::modeCreate | CFile::modeWrite, &e))
		throw &e;
}

// Write some bytes out
void WadFile::Write (const void *buf, int nBytes)
{
	nOutput += nBytes;
	output.Write (buf, nBytes);
}

// End a chunk - pads to sector size
void WadFile::EndChunk (void)
{
	static const Uint32 wadEnd = 0xdeadbabe;
	output.Write (&wadEnd, 4);
}

// Close it
void WadFile::Close (void)
{
	output.Close();
}

void WadFile::Mark (void)
{
	markPos = output.GetPosition();
}
void WadFile::WriteMark (void)
{
	DWORD curPos;
	DWORD skipAmount;

	curPos = output.GetPosition();

	skipAmount = curPos - markPos;

	output.Seek (markPos, 0);
	output.Write (&skipAmount, 4);

	output.Seek (curPos, 0);
}