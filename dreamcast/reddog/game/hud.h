#ifndef _HUD_H
#define _HUD_H

/*
 * Head-up display functions
 */

/* Initialise the HUD */
void InitialiseHUD();
/* Finalise the HUD */
void FinaliseHUD();

/* Draw the HUD */
void DrawHUD (int pNum);

/* Variables used in HUD drawing */
extern int nMissiles;
extern float direction; 

#endif
