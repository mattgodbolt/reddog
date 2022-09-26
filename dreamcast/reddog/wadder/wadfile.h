/*
 * WadFile.h
 */
#ifndef _WADFILE_H
#define _WADFILE_H

class WadFile
{
private:
	CFile		output;
	int			nOutput;
	DWORD		markPos;
public:
	// Create a wadfile
	WadFile ();
	WadFile (CString filename);
	// Open
	void Open (CString filename);
	// Write bytes out
	void Write (const void *buf, int nBytes);
	// Mark the end of a chunk
	void EndChunk (void);
	void Close (void);
	void Mark (void);
	void WriteMark (void);
};

#endif
