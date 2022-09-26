#ifndef TEXLIB_H
#define TEXLIB_H

#include <afxtempl.h>
/*
 * Unique library of textures
 */
class TexLib
{
private:
	static TexLib SystemTexList;
	CMap <unsigned int, unsigned int, CString, CString> lib;
	POSITION pos;
public:
	static bool DoneSystem;
	TexLib () { invalidTextures = 0; }
	~TexLib() {}
	void Add (unsigned int GBIX, CString fileName);
	void Reset (void);
	bool More (void);
	CString GetElement (void);
	int Count (void);
	void Clear ();
	void SetAsSystem();
	int invalidTextures;
};

#endif