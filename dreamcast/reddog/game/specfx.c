#include "specfx.h"
#include "command.h"
#include "player.h"
#include "View.h"
#include "Render\Internal.h"
#include "VMSStruct.h"

extern	PLAYER player[4];

static	Object	*splashObj;
		int		cWaterCP = 0, noWaterCP = 0;
		WaterCP	water[64];

void ClearSplash()
{
	cWaterCP = 0;
	noWaterCP = 0;
}

void SetNextSplash()
{
	cWaterCP++;
}

int Splash()
{
	if (noWaterCP)
		return 1;
	else
		return 0;
}

void GetSplashPos(STRAT *strat)
{
	strat->pos = water[cWaterCP].pos;
}

int LastSplash()
{
	if ((noWaterCP > 0) && (cWaterCP < noWaterCP - 1))
		return 0;
	else
		return 1;
}

#define SKID_WIDTH	0.3f

/*
 * This is now changed to allow the landscape renderer to draw the skids:
 * StripPos replaces both the position and introduces a new colour (ARGB)
 * entry per point, so the alpha value can be changed
 * StripEntry refers to strip, and for reasons of speed within the 'scape
 * renderer, has a packed [16f:16f] UV value, and a 32-bit reference which
 * is an offset into the StripPos array, in bytes (ie vertex * 16)
 */


void resetSkid(int pn)
{
	int i, j;
	
	for (i=0; i<4; i++)
	{
		player[pn].rdWheel[i].p.x = player[pn].rdWheel[i].p.y = player[pn].rdWheel[i].p.z = 0.0f;
		player[pn].rdWheel[i].n.x = player[pn].rdWheel[i].n.y = player[pn].rdWheel[i].n.z = 0.0f;
		
		for (j=0; j<MAX_SKID; j++)
		{
			
			player[pn].rdWheel[i].v[j].p.x = player[pn].rdWheel[i].v[j].p.y = player[pn].rdWheel[i].v[j].p.z = 0.0f;
			player[pn].rdWheel[i].stripStart[j] = 1;
			player[pn].rdWheel[i].uv[j] = 0;
		}

		player[pn].rdWheel[i].i = 0;
		player[pn].rdWheel[i].coll = 0;
		player[pn].rdWheel[i].mat = 0;
		player[pn].rdWheel[i].oldMat = 0;
	}
}

#define PACK_UV(fU, fV)  ((*((Uint32 *) &fU) & 0xFFFF0000) | (*((Uint32 *) &fV) >> 16))
void RedDogSkid(STRAT *strat)
{
	Vector3	dirn, skidx, smalln;
	int i, pn;
	RedDogWheel	*w;
	float u, v;

	// MattG added:
	dAssert (strat->Player, "Not a player");
	pn = strat->Player->n;

	// DANGER WILL ROBINSON!!
	// we need to set up Pn before we do this:
	if (v3SqrLength(&strat->vel) < 0.0001f)
	{
		player[pn].rdWheel[0].coll = 0;
		player[pn].rdWheel[1].coll = 0;
		player[pn].rdWheel[2].coll = 0;
		player[pn].rdWheel[3].coll = 0;
		return;
	}

	// used to be here:
	//pn = strat->Player->n;

	if (player[pn].PlayerAfterJumpCountDown)
	{
		player[pn].PlayerAfterJumpCountDown--;
	}
	else
	{
		if (player[pn].Player->flag & COLL_HITSTRAT)
		{
			player[pn].rdWheel[0].coll = 0;
			player[pn].rdWheel[1].coll = 0;
			player[pn].rdWheel[2].coll = 0;
			player[pn].rdWheel[3].coll = 0;
			return;
		}

		if ((!player[pn].PlayerBreaking) && (!player[pn].PlayerWheelSpin))
		{
			if (player[pn].PlayerState & PS_SIDESTEP)
			{
				// Just to prevent a compiler warning:
				register float length = v3Length (&player[pn].Player->vel);
				if ((player[pn].playerSideStepHold >= SKID_FRAMES) || (length < 0.3))
				{
					player[pn].rdWheel[0].coll = 0;
					player[pn].rdWheel[1].coll = 0;
					player[pn].rdWheel[2].coll = 0;
					player[pn].rdWheel[3].coll = 0;
					return;
				}
			}
			else
			{
				if ((player[pn].playerBoostButtonCount != 2) && (player[pn].playerRevBoostButtonCount != 2))
				{
					player[pn].rdWheel[0].coll = 0;
					player[pn].rdWheel[1].coll = 0;
					player[pn].rdWheel[2].coll = 0;
					player[pn].rdWheel[3].coll = 0;
					return;
				}
				if ((player[pn].playerBoostButtonHold > 15) && (fabs(player[pn].Player->relVel.x)< 0.3))
				{
					player[pn].rdWheel[0].coll = 0;
					player[pn].rdWheel[1].coll = 0;
					player[pn].rdWheel[2].coll = 0;
					player[pn].rdWheel[3].coll = 0;
					return;
				}

				if ((player[pn].playerRevBoostButtonHold > 15) && (fabs(player[pn].Player->relVel.x)< 0.3))
				{
					player[pn].rdWheel[0].coll = 0;
					player[pn].rdWheel[1].coll = 0;
					player[pn].rdWheel[2].coll = 0;
					player[pn].rdWheel[3].coll = 0;
					return;
				}
			}
		}
	}


	dirn = player[pn].Player->vel;
	v3Normalise(&dirn);

	for (i=0; i<4; i++)
	{
		w = &player[pn].rdWheel[i];

		if ((w->p.x == 0.0f) || (w->n.z < 0.7f))
		{
			w->coll = 0;
		}
		else
		{
			/*if ((w->oldMat != w->mat) && w->coll)
			{
				if (w->stripStart[w->i] == 1)
					w->stripStart[(w->i + 2) & (MAX_SKID - 1)] = 1;

				w->stripStart[w->i] = 0;

				w->v[w->i] = w->v[w->i-2 & (MAX_SKID - 1)];
				w->v[w->i+1] = w->v[w->i-1 & (MAX_SKID - 1)];

				switch(w->i & 3)
				{
					case 0:
						u = (0.25f *  (float)(GET_SURF_TYPE(w->mat) & 3)) + 0.02f;
						v = (0.25f * (float)(GET_SURF_TYPE(w->mat) >> 2)) + 0.02f;
						w->uv[w->i]   = PACK_UV (u, v);
						u += 0.21;
						w->uv[w->i+1]   = PACK_UV (u, v);
						break;
					case 2:
						u = (0.25f *  (float)(GET_SURF_TYPE(w->mat) & 3)) + 0.02f;
						v = (0.25f * (float)(GET_SURF_TYPE(w->mat) >> 2)) + 0.23f;
						w->uv[w->i]   = PACK_UV (u, v);
						u += 0.21;
						w->uv[w->i+1]   = PACK_UV (u, v);
						break;
				}
				w->i = (w->i + 2) & (MAX_SKID - 1);
			}*/

			if (w->stripStart[w->i] == 1)
				w->stripStart[(w->i + 2) & (MAX_SKID - 1)] = 1;

			if (w->coll)
				w->stripStart[w->i] = 0;
			else
				w->stripStart[w->i] = 1;

			v3Scale(&smalln, &w->n, 0.05f + ((float)(i)) * 0.01f);
			v3Cross(&skidx, &dirn, &w->n);
			v3Scale(&skidx, &skidx, SKID_WIDTH);
			v3Sub(&w->v[w->i].p, &w->p, &skidx);
			v3Inc(&w->v[w->i].p, &smalln);
			w->v[w->i].colour = 0xffffffff; // TODO: Change the alpha amount here
			v3Add(&w->v[w->i+1].p, &w->p, &skidx);
			v3Inc(&w->v[w->i+1].p, &smalln);
			w->v[w->i+1].colour = 0xffffffff; // TODO: Change the alpha amount here

			switch(w->i & 3)
			{
				case 0:
						w->uv[w->i]   = 0;//PACK_UV (0.0f, 0.0f);
						w->uv[w->i+1]   = 0x3f800000;//PACK_UV (1.0f, 0.0f);
					break;
				case 2:
						w->uv[w->i]   = 0x00003f80; //PACK_UV (0.0f, 1.0f);
						w->uv[w->i+1]   = 0x3f803f80;//PACK_UV (1.0f, 1.0f);
					break;
			}

			
			w->i = (w->i + 2) & (MAX_SKID - 1);
			w->coll = 1;
		}
		//w->oldMat = w->mat;
		//w->mat = 0;
	}
}


StripEntry	strip[MAX_SKID];
#define SKID_ALPHA_BITS_START	4
#define SKID_ALPHA_BITS_STOP	8

void DrawSkid(int pn)
{
	register int ii, j, vCount;
	register RedDogWheel	*w;
	register StripEntry *s;
	int	i, foundStart, n, vp, k;
	int	*CS;
	Point3	*RedDogSkidArrayC;
	UV	*CUV;
	extern Material skidMat;
	
	if (CurProfile[0].activeSpecials & SPECIAL_TRIPPY_TRAILS) {
		skidMat.info.TSPPARAMBUFFER = (skidMat.info.TSPPARAMBUFFER & 0x1fffffff) | 0x94000000;
	} else {
		skidMat.info.TSPPARAMBUFFER = (skidMat.info.TSPPARAMBUFFER & 0x1fffffff) | 0x0c000000;
	}
	rStartSkid(pn);
	
	// For each wheel
	for (k=0; k<4; k++)
	{
		// Reset variables for this wheel
		i = vCount = foundStart = n = 0;
		
		// Get a pointer to the wheel
		w = &player[pn].rdWheel[k];
		
		// Find the first beginning of a skid
		while(w->stripStart[n] == 0)
		{
			n += 2;
		}
		
		// Loop round and find how long this skid is
		while(i < MAX_SKID + 2)
		{
			// ii is the position in the skid ring buffer
			ii = (i + n) & (MAX_SKID - 1);
			// Is this the beginning/end of a skid
			if (w->stripStart[ii] == 1) {
				// Beginning?
				if (vCount == 0)
				{
					goto startStrip;
				} // Is this the end segment of a skid?
				else if ((vCount > 2) && (foundStart))
				{
					s = strip;
					// mg notes> my prefetching here is cock and arse
					//				prefetch(strip);
					for (j=0; j<vCount; j++)
					{
						Uint32 ref = ((vp + j) & (MAX_SKID - 1));
						Uint8 t;
						float amt = 1.f;
						//					prefetch(s + 1);
						if (j < SKID_ALPHA_BITS_START) {
							amt = ((float)j * (1.f / SKID_ALPHA_BITS_START));
						}
						if (j > (vCount - SKID_ALPHA_BITS_STOP)) {
							amt *= ((float)(vCount-j) * (1.f / SKID_ALPHA_BITS_STOP));
						}
						
						t = (Uint8)(amt * 255.f);
						w->v[ref].colour = t | (t<<8) | (t<<16);
						s->vertRef = ref << 2;
						s->uv = w->uv[ref];
						s++;
					}
					// Second pass for trippy trails
					if (CurProfile[0].activeSpecials & SPECIAL_TRIPPY_TRAILS) {
						for (j=0; j<vCount; j++)
						{
							Uint32 ref = ((vp + j) & (MAX_SKID - 1));
							w->v[ref].colour = (rand() ^ (rand() << 8)) | 0xff000000;//(w->v[ref].colour & 0xff000000 );
						}
					}
					rDrawSkid(vCount, strip, w->v);
				}
				// start a new strip, regardless of whether we found the end
				goto startStrip;
			}
			else if (foundStart) // Are we mid-skid?
			{
				vCount += 2;
			}
			else
			{
startStrip:
				foundStart = 1;
				vp = ii;
				vCount = 2;
			}
			
			i += 2;
		}
	}
}

int GetUniqueID(void)
{
	static int ID = 0xf00fb00b;
	return ID++;
}
int OnScreen (STRAT *strat)
{
	Point pos;
	Matrix m;
	Outcode code;

	m = mWorldToScreen;
	mPostScale (&m, 1.f/1.4f, 1.f/1.4f, 1.f);
	mLoad(&m);

	code = mPointXYZOut (&pos, strat->pos.x, strat->pos.y, strat->pos.z);
	return (!(code & OC_OFF));
}
int Obscured (STRAT *s)
{
	Point3 collPt;
	return obscured (&currentCamera->pos, &s->pos, &collPt, -1, NULL);
}
int GetCurrentFrame(void)
{
	return currentFrame;
}