#ifndef __SNDUTLS_H__
#define __SNDUTLS_H__

#ifndef SOUND
#define SOUND			1
#endif

#if SOUND
# define _AL_SND_PLAYER_
# define _AL_STRM_PLAYER_
#endif

#define MAXSOUNDS		412 // XXX MATT G CHANGED THIS!!
#define BASELEVELSFX	128

//played in game code generics
#define GENERIC_BOOST_SOUND			0
#define GENERIC_MOVE_SOUND			13
#define GENERIC_ACCEPT_SOUND		14
#define GENERIC_REJECT_SOUND		12
#define GENERIC_SHIELD_SOUND		4
#define GENERIC_LOCKON_SOUND		10
#define GENERIC_HITMETALFX			9
#define GENERIC_PICKUP_ENERGY		19
#define GENERIC_PICKUP_WEAPON_1		16
#define GENERIC_PICKUP_WEAPON_2		17
#define GENERIC_PICKUP_WEAPON_3		18
#define GENERIC_PICKUP_WEAPON_4		20
#define GENERIC_WEAPON_OPEN			43
#define GENERIC_WEAPON_CLOSE		44
// All these are MattG's fault - blame him :)
#define GENERIC_TIMER_BLEEP_1		21
#define GENERIC_TIMER_BLEEP_2		22
#define GENERIC_WIN_SOUND			47
#define GENERIC_TOURN_VICT			48
#define GENERIC_BOMB_PASS			49
#define GENERIC_TOGGLE_BEEP			50
#define GENERIC_FLAG				51
#define GENERIC_STEALTH				52
#define GENERIC_KINGHILL			53
#define GENERIC_ELIMINATE			54
#define GENERIC_TOGGLEEND			55
#define GENERIC_READY				56
#define GENERIC_GO					57

extern int SFXIndirTab[MAXSOUNDS];

#define SBFLAG_ACTIVE		(1<<0)	/* Sound is active */
#define SBFLAG_FINISHED		(1<<1)	/* Sound has finished playing */
#define SBFLAG_CONTINUOUS	(1<<2)	/* Sound is continous and will replay once back in auditory range */
#define SBFLAG_STOPPING		(1<<3)	/* Sound is in the process of stopping */
#define SBFLAG_RELEASED		(1<<4)	/* Sound has been temporarily released: either out of range or cannot get a port */
#define SBFLAG_RANDOM		(1<<5)	/* Sound needs random pitch and volume */
#define SBFLAG_NOPOSITION	(1<<6)	/* Sound does not need to be positioned */
#define SBFLAG_HARDLEFT		(1<<7)	/* Sound is panned to the listeners' left */
#define SBFLAG_HARDRIGHT	(1<<8)	/* Sound is panned to the listeners' right */
#define SBFLAG_NOUPDATE		(1<<9)	/* Sound is positioned but not spatially updated */
#define SBFLAG_REDUCEMULTI	(1<<10)	/* Limit Number of instances of this sound to SND_MAX_INSTANCE */
#define SBFLAG_NOREVERB		(1<<11)	/* Sound has no reverb */
#define SBFLAG_FILTERENV	(1<<12)	/* Sound has filter envelope applied */
#define SBFLAG_NODIRECTION	(1<<13)	/* Sound emitter is not directional */
#define SBFLAG_NODOPPLER	(1<<14)	/* Sound does not perform doppler pitch changes */
#define SBFLAG_DEACTIVATED	(1<<15)	/* Sound has been muted for a cut scene */
/* No more flags allowed */

/*
 * A sound being kept up-to-date with a strat
 */
typedef struct tagSoundBlock
{
	Point3	offset;
	float	volume;		/* Volume */
	float	pitch;		/* Pitch adjustment */
	float	lastDist;	/* Last distance to player */
	Sint16	innerRange;	/* The distance at which the sound starts to fade */
	Sint16	outerRange;	/* The distance at which the sound can no longer be heard */
	Sint16	flags;		/* Flags */
	Uint16	num;		/* Number of the program */
	Sint8	handle;		/* Handle into the low-level sound system */
} SoundBlock;

typedef struct tagStratSoundHeader
{
	int			nSounds;
	float		danger;
	Uint32		frameTime;
	SoundBlock	block[1];
} StratSoundHeader;

#ifdef __cplusplus
extern "C" {
#endif

int		sfxPlay				(int num, float volume, float pitchAdj);
void	sfxStop				(int num, float time);
float	sfxSetPos			(int sfxNum, Point3 *pos, float volume);
void	sfxSetFXVolume		(float volume);
void	sfxSetCDVolume		(float volume);
float	sfxGetFXVolume		(void);
float	sfxGetCDVolume		(void);
bool	sfxIsStereo			(void);
void	sfxSetStereo		(bool isStereo);
void	SoundInitialize		(char *, char *);
void	SoundUpdatePreDraw	(void);
void	SoundUpdatePostDraw	(void);
void	SoundPlayTrack		(int);
void	SoundStopTrack		(void);
void	SoundFadeOut		(int frames);
void	SoundFadeIn			(int frames);
void	SoundFade			(float amt, int music);
void	MusicFade			(float amt);
void	MusicSetIntensity	(int intensity);
void	SoundPause			(void);
void	SoundResume			(void);
void	SoundReset			(void);
Uint32 * SoundLoadBank		(void *filePtr,int size);

/***************************** Strat interface ************************/

struct strat;

void	AllocStratSoundBlock(struct strat *strat, int nSound);
void	FreeStratSoundBlock (struct strat *strat);
void	UpdateStratSound	(struct strat *strat);
void	PlaySound			(struct strat *strat, int channel, int soundNum, float volume, float x, float y, float z, int flags);
void	StopSound			(struct strat *strat, int channel, float time);
void	SetVolume			(struct strat *strat, int channel, float volume);
void	SetPitch			(struct strat *strat, int channel, float pitch);
Bool	IsPlaying			(struct strat *strat, int channel);
void	SetAudibleRange		(struct strat *strat, int channel, Sint16 inner, Sint16 outer);

#ifdef __cplusplus
}
#endif

#endif

