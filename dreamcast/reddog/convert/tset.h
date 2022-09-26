// Imp'd in ConvertDlg.c
#ifndef _TSET_H
#define _TSET_H

struct TextureSettings
{
	TextureSettings (CString fileName, bool fromDB = TRUE);

	void WriteBack (CString fileName);
	
	bool	dontMipmap;
	bool	dontDither;
	bool	dontDitherAlpha;
	bool	vqCompress;
	bool	firstAnimFrame;
	bool	globalAnim;
	bool	bumpMap;
	bool	punchThru;
	int		animSpeed;
	CString	animName;
	CString	setFile;
	long	GBIX;
	long	wadGBIX;

	bool	wasCreated;
};

#endif
