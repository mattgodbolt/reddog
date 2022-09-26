/*
 * Input.h
 * Input stuff
 */
#ifndef _INPUT_H
#define _INPUT_H

#define NORMALINPUT 0
#define DEMOREAD	1
#define DEMOWRITE	2

extern Point2 joyState[4];
extern const PDS_PERIPHERAL *pad[4];
extern int	InputMode, PadOK, PadReset;
/*extern Uint32 PadPress, PadPush, PadRelease;*/

#define		PADFORWARD		1
#define		PADBACKWARD		2
#define		PADLEFT			4
#define		PADRIGHT		8
#define		PADUP			16
#define		PADDOWN			32
#define		PADFIRE			64
#define		PADB			128
#define		PADX			256
#define		PADY			512
#define		PADSHOULDERL	1024
#define		PADSHOULDERR	2048
#define		PADSTART		4096

/*CAMERA PAD BUTTONS DIRECTLY MAPPED TO THE SAME BUTTONS AS NORMAL CONTROL*/
#define		PAD_CAMERA0		1
#define		PAD_CAMERA1 	2
#define		PAD_CAMERA2		4
#define		PAD_CAMERA3		8
#define		PAD_CAMERA4		16
#define		PAD_CAMERA5		32
#define		PAD_CAMERA6		64
#define		PAD_CAMERA7		128
#define		PAD_CAMERA8		256
#define		PAD_CAMERA9		512

void InputCheck (void);
void lcdClear (int pad);
void lcdUpdate (int pad);
void lcdSetPixel (int pad, int x, int y);
void lcdClrPixel (int pad, int x, int y);
void puruVibrate (int logPort, int priority, float freq, float power, float seconds);

void InitInput (void);


void InputClose ();


#define MAXKEYS (3*30*60*4)	
typedef struct Input
{
	Uint32	padpress;
	Uint32	padpush;
	Point2	joystate;
	float	PlayerAcc;

}INPUT;

typedef struct	CamCommand
{
	short	camnum;
	short	numframes;
}CAMCOMMAND;


extern INPUT *InputBuffer;
extern CAMCOMMAND *CamBuffer;
extern int	CamCommands;
extern int	CamCommandsFrame;
extern int CamMode;
extern int DEMOREC;
#define CAMSCRIPT 1


extern float	PlayerControlJoyX;
extern float	PlayerControlJoyY;
extern float	PlayerControlPlayerAcc;
extern Uint32	PlayerControlPadPress;
extern Uint32	PlayerControlPadPush;


void CapturePort (int port);
void ReleasePort (int port);

const PDS_PERIPHERAL *FindPhysicalController(int i);

extern int PadMapTable[4];

#endif
