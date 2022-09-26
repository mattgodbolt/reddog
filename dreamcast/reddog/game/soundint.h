#ifndef _SOUND_INT_H
#define _SOUND_INT_H

#include <ac.h>
#include <ambnkhdr.h>
#include <amstrhdr.h>
#include "LocalDefs.h"
#include "SndUtls.h"

typedef Point3	ALVector;
typedef struct strat ALStrategy;
typedef ACFILE	ALFilePtr;
typedef AM_BANK_WAVE	ALSound;

#define AL_FILE_VBLANK_ID 			0xc0c0cafe
#define AL_FILE_VBLANK_PRIORITY 	0x7f

#define AL_FLAG_DMA_REQUEST			0x1

#define AL_TRANSFER_WAIT			0
#define AL_TRANSFER_PEEK			1

#define AL_STRM_DANGER_LOW			3
#define AL_STRM_DANGER_MED			5
#define AL_STRM_DANGER_HI			1
#define AL_STRM_DANGER_NONE			1
#define AL_STRM_MIX_DELAY			8

#ifndef RANGE
#define RANGE(a,b,c)				((b) < (a) ? (a) : (b) > (c) ? (c) : (b))
#endif

#define AL_GetChainFlag(p)			((p)->flags & AF_CHAINSOUNDS)

#define AL_MAX_SMOOTH_VOLUME_CHANGE	(0x0100)
#define AL_MAX_SMOOTH_REVERB_CHANGE	(0x0100)
#define AL_MAX_REVERB_LEVEL			(0x0fff)

#define AL_PRIORITY_DEFAULT			64
#define AL_PRIORITY_AMBIENCE		6
#define AL_REVERB_ON				0x8000

#define AL_ERR_MASK_NO_SOUND		0x01
#define AL_ERR_MASK_NOTE_OFF		0x02
#define AL_ERR_MASK_NO_VOICE		0x04

#ifdef _DEBUG
# define	AL_ERR_NO_UPDATE		"Sound: ERROR: No ParamUpdate\n"
# define	AL_ERR_NO_VOICE			"Sound: ERROR: No Voice\n"
# define	AL_ERR_NO_CHANNEL		"Sound: ERROR: No Channel\n"
# define	AL_ERR_NO_SOUND			"Sound: ERROR: No Sound\n"
# define	AL_ERR_OFF_VOICE		"Sound: ERROR: Off Voice\n"
# define	AL_ERR_POLY_VOICE		"Sound: ERROR: Poly Voice\n"
# define	AL_ERR_EVENT_OVERFLOW	"Sound: ERROR: EventQueue Overflow!\n"
# define	AL_ERR_NO_MEMORY		"Sound: ERROR: Not enough memory\n"
# define	AL_ERR_MALLOC			"Sound: ERROR: alMalloc Function Failed\n"
# define	AL_ERR_INVALID_ADDRESS	"Sound: ERROR: Invalid address\n"
# define	AL_ERR_OUT_OF_RANGE		"Sound: ERROR: Out of range\n"
# define	AL_ERR_READ_FAILED		"Sound: ERROR: Read Failed\n"
# define	AL_ERR_SEEK_FAILED		"Sound: ERROR: Seek Failed\n"
# define	AL_ERR_WRITE_FAILED		"Sound: ERROR: Write Failed\n"
# define	AL_ERR_BEYOND_RANGE		"Sound: WARNING: Out of earshot\n"
#else								
# define	AL_ERR_NO_UPDATE		0
# define	AL_ERR_NO_VOICE			0
# define	AL_ERR_NO_CHANNEL		0
# define	AL_ERR_NO_SOUND			0
# define	AL_ERR_OFF_VOICE		0
# define	AL_ERR_POLY_VOICE		0
# define	AL_ERR_EVENT_OVERFLOW	0
# define	AL_ERR_NO_MEMORY		0
# define	AL_ERR_MALLOC			0
# define	AL_ERR_INVALID_ADDRESS	0
# define	AL_ERR_OUT_OF_RANGE		0
# define	AL_ERR_READ_FAILED		0
# define	AL_ERR_WRITE_FAILED		0
# define	AL_ERR_SEEK_FAILED		0
# define	AL_ERR_BEYOND_RANGE		0
#endif

#ifdef _DEBUG
//#define alAssertIf(condition, error)			if (condition) { dPrintf(error); }
//#define alFailIf(condition, error)				if (condition) { dPrintf(error); return; }
//#define alFailIfRet(condition, error, ret)		if (condition) { dPrintf(error); return ret; }
//#define alFlagFailIf(condition, flag, error)	if (condition) { if(flag) printf(error); return; }
#else
//#define alAssertIf(condition, error)
//#define alFailIf(condition, error)				if (condition) { return; }
//#define alFailIfRet(condition, error, ret)		if (condition) { return ret; }			
//#define alFlagFailIf(condition, flag, error)	if (condition) { return; }
#endif

////////////////////////////////////////////////////////////////////////////////
// Event Messages
typedef enum eALMsgType {
	AL_EVENT_STOP,
	AL_EVENT_STOPPING,
	AL_EVENT_PLAY,
	AL_EVENT_VOLUME,
	AL_EVENT_PAN,
	AL_EVENT_PITCH,
	AL_EVENT_FX,
	AL_EVENT_POSITION,
	AL_EVENT_DECAY,
	AL_EVENT_API,
	AL_EVENT_DMA,
	AL_EVENT_STITCH,
	AL_EVENT_CDREAD,
	AL_EVENT_CDSEEK,
	AL_EVENT_PREPARE,
	AL_EVENT_READY,
	AL_EVENT_SEQP_REF,
	AL_EVENT_MIDI,
	AL_EVENT_META,
	AL_EVENT_NOTEOFF,
	AL_EVENT_NOTEEND,
	AL_EVENT_FLUSH,
	AL_EVENT_END
} ALMsgType;

#define AL_EVTQ_END     0x7fffffff

////////////////////////////////////////////////////////////////////////////////
// Audio Bank Identifiers
typedef enum  e_ALAudioBankType	{
	AL_AUDIO_BANK_FRONTEND,
	AL_AUDIO_BANK_GENERIC,
	AL_AUDIO_BANK_LEVELSPECIFIC,
	AL_AUDIO_BANK_MUSIC,
	AL_AUDIO_BANK_AMBIENCE,
	AL_AUDIO_BANK_MAX
} ALAudioBankType;

////////////////////////////////////////////////////////////////////////////////
// Speaker 
typedef enum e_ALSpeakerMode {
	AL_SPEAKER_MODE_MONO, 
	AL_SPEAKER_MODE_STEREO, 
	AL_SPEAKER_MODE_DOLBYSURROUND
} ALSpeakerMode;

////////////////////////////////////////////////////////////////////////////////
// Voice State
typedef enum eVoiceStates {
	AL_STATE_STOPPED,
	AL_STATE_PREPARED,
	AL_STATE_PLAYING,
	AL_STATE_STOPPING
} VoiceState;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// The Audio System Memory Manager
typedef struct {
	char *start;
	char *end;
	char *current;
} ALMallocBlock;

////////////////////////////////////////////////////////////////////////////////
// Audio Linked List Util
typedef struct s_ALLink {
	struct s_ALLink	*next;
	struct s_ALLink	*prev;
} ALLink;

////////////////////////////////////////////////////////////////////////////////
// The Audio Voice Structure
typedef struct s_ALVoice {
    ALLink				node;
    struct s_ALPVoice *	pvoice;
    void              *	clientPrivate;

    Sint16				state;				// Current Play status
    Sint16				priority;			// Obvious innit!	
} ALVoice;

typedef struct s_ALPVoice {
    ALLink				node;
    struct s_ALVoice  *	vvoice;
	Sint16				offset;
	Sint16				stolen;
} ALPVoice;

typedef struct {
    Sint16				priority;		// voice priority               
    Sint16				fxSend;			// bus assignment               
} ALVoiceConfig;

////////////////////////////////////////////////////////////////////////////////
// The Voice State Structure
typedef struct s_ALVoiceState {
	struct s_ALVoiceState *	next;			// MUST be first             
	ALVoice				voice;     	
#ifdef _AL_SND_PLAYER_
	ALSound			*	sound;				// sound  referenced here
#endif
#ifdef _AL_SEQ_PLAYER_
	struct s_ALSeqSound *seqsound;			// sequence sounds
#endif
#ifdef _AL_STRM_PLAYER_
	struct s_ALStream *	stream;				// stream referenced here
#endif

	Sint32				state;				// play state for this sound                 
	Sint32				offset;				// index to this array

	ALStrategy		*	strat;				// which strat was this generated from
	ALVector			pos;				// the position of the sound

	Sint32				inner, outer;		// radius of sound emitter
	Uint32				flags;				// sound specific flags
	Uint32				distance;			// store the last known distance
	Uint32 				endTime;			
	Uint32 				startTime;			

	void			*	user_data0;			// passed in user data 0
	void			*	user_data1;			// passed in user data 1
	
	KTU32				acVolume;
	KTU32				lastVolume;
    Uint32				pan;				// channel assignment           
	Sint32 				pitch;				// default pitch
	Sint32 				lastPitch;			// for pause/resume
	Sint32 				pitchOffset;		// current + or - from relPitch
	Sint32				priority;			// obvious innit!
	Sint32				fxMix;				// wet/dry mix - 0 = dry, 127 = wet          
	Sint32				filter;				// 

	Uint32				angle;				// the last know angle
	Uint32 				loop;				// this channel contains a looping sfx
	
    Uint32				channel;			// channel assignment           
    Uint32				key;				// note on key number           
    Uint32				velocity;			// note on velocity             
	Uint32				type;

} ALVoiceState;

////////////////////////////////////////////////////////////////////////////////
// The Audio Player
typedef Sint32		(*ALVoiceHandler)(void *);

typedef struct s_ALPlayer {
	struct s_ALPlayer *	next;
	void              *	clientData;		// storage for client callback  
	ALVoiceHandler      handler;        // voice handler for player     
	Uint32				postdraw;
} ALPlayer;

////////////////////////////////////////////////////////////////////////////////
// The Audio System Voice and Player Manager
typedef struct {
	Sint32				maxVVoices;
	Sint32				maxPVoices;
	Sint32				maxUpdates;
	Sint32				maxBanks;
	Sint32				maxStreams;
	Sint32				streamSize;
	Sint32				mainRamSize;
    Sint32				*params;
} ALSynConfig;

typedef struct {
	ALPlayer    *		head;			// client list head                     
    Sint32				numPhysicalVoices;

	ALLink				freeList;      // list of free physical voices         
	ALLink				allocList;     // list of allocated physical voices    
	ALLink				usedList;      // list of voices ready to be freed     

	Uint32				timer;
	Uint32				lastError;
	Uint32				dmaFlags;
	Uint32				voiceMaskOnLoopHi;
	Uint32				voiceMaskOnLoopLo;
	Uint32				voiceMaskOnHi;
	Uint32				voiceMaskOnLo;
	Uint32				voiceMaskOffHi;
	Uint32				voiceMaskOffLo;
	Uint32				fxCurrent;
	Uint32				fxSendLevel;
	Uint32				fxType;

#ifdef _AL_STRM_PLAYER_
	struct s_ALStream **streams;
	Sint32				streamSize;
	Sint32				mainRamSize;
	Sint32				maxStreams;						// Max simultaneous streams
	Sint32				streamsLoaded;
	Uint32				vblankId;
	Uint32				vblankFlag;
#endif

	ALSound			**	dramAddr;						// Header address in main RAM
	Uint32			*	spuAddr ;						// Current start address in SPU for sfx
	Uint32			*	sfxCount;						// Number of sfx in bank
	Uint32				bankSelect;						// The Current Sound FX Bank
						
	Sint32				speakerType;					// Mono, Stereo or Surround
						
	Uint32				sfxVolume;						// Main Sound Effects volume
	Uint32				cdVolume;						// Main CD volume
	Uint32				midiVolume;						// Main MIDI volume

} ALSynth;

////////////////////////////////////////////////////////////////////////////////
// The Audio Driver
typedef struct s_ALGlobals {
	ALSynth	drvr;
} ALGlobals;

////////////////////////////////////////////////////////////////////////////////
// The Audio Event Queue

#ifdef _AL_SEQ_PLAYER_
typedef struct s_ALVolumeEvent {
	struct ALVoice_s *	voice;
} ALVolumeEvent;

typedef struct s_ALTempoEvent {
	Sint32				ticks;
	Uint8				status;
	Uint8				type;
	Uint8				len;
	Uint8				byte1;
	Uint8				byte2;
	Uint8				byte3;
} ALTempoEvent;

typedef struct s_ALMIDIEvent {
	Sint32				ticks;		// MIDI, Tempo and End events must start with ticks
	Uint8				status;
	Uint32				duration;
	Uint8				byte1;
	Uint8				byte2;
} ALMIDIEvent;
#endif	// _AL_SEQ_PLAYER_

typedef struct {
	Sint16		type;
	Sint32		data1;
	Sint32		data2;
	union {
		ALVoiceState	*		state;
        ALVoice			*		voice;
#ifdef _AL_SEQ_PLAYER_
		ALVolumeEvent	volume;
		ALMIDIEvent		midi;
		ALTempoEvent	tempo;
#endif
#ifdef _AL_STRM_PLAYER_
		struct s_ALStreamFile *	sfile;
#endif
	} msg;
} ALEvent;

typedef struct {
	ALLink			node;
	Sint32			delta;
	ALEvent			evt;
} ALEventListItem;

typedef struct {
	ALLink			freeList;
	ALLink			allocList;
	Sint32			eventCount;
	Sint32			maxEventCount;
} ALEventQueue;


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#ifdef _AL_SND_PLAYER_	

////////////////////////////////////////////////////////////////////////////////
// The Sound Player

typedef struct {
	Sint32			maxSounds;
	Sint32			maxEvents;
	Sint32			frameTime;
	Sint32			maxDistance;
} ALSndpConfig;

typedef struct {
    ALPlayer		node;			// note: must be first in structure
    ALEventQueue	evtq;
    ALEvent			nextEvent;
    ALSynth		*	drvr;			// reference to the client driver
    Sint32			target;
    ALVoiceState *	sndState;
    Sint32			maxSounds;
    Sint32			frameTime;
	Sint32			nextDelta;		// microseconds to next callback 
	Sint32			curTime;
	Sint32			mainVolume;
	Sint32			maxDistance;
} ALSndPlayer;

////////////////////////////////////////////////////////////////////////////////
#endif	// _AL_SND_PLAYER_

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#ifdef _AL_STRM_PLAYER_
////////////////////////////////////////////////////////////////////////////////
// The Stream Player

#define STRM_SECTOR_SIZE			(0x0800)
#define STRM_SAMPLE_RATE			(44100)
#define STRM_IRQ_SIZE				(STRM_SECTOR_SIZE)
#define STRM_IRQ_STEP				(STRM_IRQ_SIZE>>2)
#define STRM_IRQ_RATE				(STRM_SAMPLE_RATE)
#define STRM_SNDRAM_BUFFERS			(8)
#define STRM_SNDRAM_SIZE			(STRM_SECTOR_SIZE*STRM_SNDRAM_BUFFERS)
#define STRM_MAINRAM_BUFFERS		(4)
#define STRM_MAINRAM_SIZE			(STRM_SECTOR_SIZE*STRM_MAINRAM_BUFFERS)
#define STRM_CHANNEL(x)				(SPU_VOICECH(x)|SPU_VOICECH(x+1))
#define STRM_INT_LOOP				(0)

#define	AF_STRM_CDREAD				0x0001
#define	AF_STRM_CDSEEK				0x0002
#define	AF_STRM_READ_REQUEST		0x0004
#define	AF_STRM_SEEK_REQUEST		0x0008
#define	AF_STRM_DMA_REQUEST			0x0010
#define	AF_STRM_FIRST_READ			0x0020
#define	AF_STRM_STARTED				0x0040
#define	AF_STRM_ALLOCATED			0x0080
#define	AF_STRM_DMA_TRANSFER		0x0100
#define AF_STRM_FINISHED			0x0200
#define AF_STRM_LOOPED				0x0400
#define AF_STRM_STITCHED			0x0800
#define AF_STRM_INTERRUPT			0x1000
#define AF_STRM_IN_FADE				0x2000
#define AF_STRM_IN_CUTSCENE			0x4000
#define AF_STRM_RESTART				0x8000
#define	AF_STRM_CDINUSE				(AF_STRM_CDREAD|AF_STRM_CDSEEK)

typedef struct
{
	AC_TRANSFER_REQUEST 	request;
	Uint32					stream;
} ALStreamDMAPool;

////////////////////////////////////////////////////////////////////////////////
// Stream File Handle
typedef struct s_ALStreamFile {
	Uint8				name[16];
	Uint32 				numEntries;
	Uint32				startSector;
	Sint32				size;
	Uint32				tempo;
	Uint32				time;
	Uint32				flags;	
	ALFilePtr 			fp;
} ALStreamFile;

////////////////////////////////////////////////////////////////////////////////
// Stream Handle Structure
typedef struct s_ALStream {
	struct s_ALPVoice *	pvoice;			// the SPU voice
	ALStreamFile *		sfile;			// the File details
	Sint32				channel;		// the SPU channel
	Uint32				statusFlags;	// the current ALStream/CD status
	Sint32				readSize;
	Uint32				cdReadLen;		// the File size
	Uint32				cdReadPos;		// the current File Position
	Sint32				mramSize;		
	Sint32				mramBuffer;		// the main ram double buffer index for reading
	Sint32				mramUsedSize[STRM_MAINRAM_BUFFERS];
	Uint32				mramStart   [STRM_MAINRAM_BUFFERS];
	Sint32				sramBuffer;		// the main ram double buffer index for playing
	Sint32				sramLastPlayPos;
	Sint32				sramPlayPos;
	Sint32				sramPlayedSize;
	Sint32				sramWritePos;
	Sint32				sramUsedSize;
	Uint32				sramStart;
	Sint32				dmaRequest;
	Sint32				interrupts;
} ALStream;

typedef struct {
	Uint32	size, count;
	Uint8	name[8];
} ALStreamHeader;

typedef struct {
	Sint32			maxStreamFiles;
	Sint32			maxStreams;
	Sint32			mainRamSize;
	Sint32			maxEvents;
	Sint32			frameTime;
	Sint32			maxDistance;
} ALStrmpConfig;

typedef struct {
    ALPlayer				node;			// note: must be first in structure
    ALEventQueue			evtq;
    ALEvent					nextEvent;
    ALEventQueue			stitchEvtq;
    ALSynth		*			drvr;			// reference to the client driver
	ALStreamFile **			streamFiles;
    ALVoiceState *			strmState;
	Sint32					maxStreamsFiles;		// simultaneous streams
	Sint32					maxStreams;			// simultaneous streams
	AM_STREAM_FILE_HEADER *	headerBuffer;
	Sint32					mainRamSize;
	Sint32					target;
    Sint32					frameTime;
	Sint32					nextDelta;		// microseconds to next callback 
	Sint32					curTime;
	Sint32					mainVolume;
	Sint32					maxDistance;
	ALFilePtr				loc;
	Uint32					playPos;
	Uint32					playBuff;
	Uint32					com;
	Uint32					flags;
	Uint32					voiceMaskHi;
	Uint32					voiceMaskLo;
	Uint32					voiceStateHi;
	Uint32					voiceStateLo;
	Uint32					currentStream;
	Uint32					currentTrackId;
	Uint32					currentIntensity;
	Uint32					currentDanger;
	Uint32					currentTime;
	Uint32					currentTempo;
	Uint32					currentBar, lastBar;
	Uint32					currentBeat;
	Uint32					numStreams;
	Uint32					delayLeft;
	Uint32					readLength;
	Uint32					repeatCount;
} ALStrmPlayer;

void alStrmpDeAllocateStream	(ALStream *strm);
void alStreamCallbackHandler	(volatile KTU32 messageNumber);
void alStreamProcessChunk		(void *obj);
void alStreamStartDMA			(ALVoiceState *state, Sint32 size);
KTBOOL alDmaSuspendAll(void);
KTBOOL alDmaResumeAll(void);

////////////////////////////////////////////////////////////////////////////////
#endif	// _AL_STRM_PLAYER_

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
	  
#endif