/*
 * Animate.h
 * Handle animation stuff
 */

#ifndef _ANIMATE_H_CONVERT
#define _ANIMATE_H_CONVERT

// Must be kept in sync with the game
struct TexAnim
{
#define TEXANIM_TAGHEADER	(*(int *)&"ANIM")
	int		tagHeader;		// ANIM
#define TEXANIM_SIZE		(sizeof (TexAnim) - 8)
	int		size;			// = skip amount
#define TEXANIM_GLOBAL		1
#define TEXANIM_STRAT		2
#define TEXANIM_OFFSET		3
	int		nFrames;		// Number of frames in total prepended to this file
	char	type;
	int		speed;
	struct {
		char		name[16];	// 15 bytes max, + terminator
	} stratAnim;

	TexAnim()
	{
		tagHeader	= TEXANIM_TAGHEADER;
		size		= TEXANIM_SIZE;
	}
};

class CAnimatedTexture
{
protected:
	CProgress		&pBar;

	void			processFrames (CString format, TexAnim anim, int startFrame);

public:
	static CString getFormat (CString name, int &startNum);
	CAnimatedTexture (CString fileName, CString stratName, unsigned int stratSpeed, CProgress &progressBar);
	CAnimatedTexture (CString fileName, unsigned int stratSpeed, CProgress &progressBar);
	~CAnimatedTexture () {}
};

#endif
