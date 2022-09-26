#include "RedDog.h"
#include "strat.h"
#include "coll.h"
#include "render\animate.h" 
#include "process.h"
#include "draw.h"
#include "view.h"
#include "camera.h"
#include "debugdraw.h"
#include "specfx.h"

#define ROOT2 1.414213562f

float CurrentStratIntensity = 0.5f;
float TargetStratIntensity = 0.5f;
float CurrentRedDogIntensity = 0.5f;
float TargetRedDogIntensity = 0.5f;
float DomeFadeAmount = 1.f, TargetDomeFadeAmount = 1.f;

void AdjustAmbient (float amt)
{
	TargetRedDogIntensity += amt;
	TargetStratIntensity += amt;
}
void AdjustRedDogAmbient (float amt)
{
	TargetRedDogIntensity += amt;
}

void DrawDome (Camera *cam)
{
	DomeFadeAmount += (TargetDomeFadeAmount - DomeFadeAmount) * 0.05f;
	if (Dome && DomeFadeAmount > 0.01f) {
		if (Dome->noObjId)
		{
			if (Dome->objId[1] && !PauseMode) {
				Dome->objId[1]->crntRot.z += (PI / 3600.f);
				if (Dome->objId[1]->crntRot.z > 2*PI)
					Dome->objId[1]->crntRot.z -= 2*PI;
			}
		}
		mUnit (&mCurMatrix);
		if (DomeFadeAmount < 0.99f)
			rGlobalModelFlags |= MODELFLAG_GLOBALTRANS, rGlobalTransparency = DomeFadeAmount;
		rGlobalModelFlags |= MODELFLAG_UNLIT;
		mPostTranslate (&mCurMatrix, cam->pos.x, cam->pos.y, cam->pos.z);
		rDrawObject (Dome->obj);
		rGlobalModelFlags &= ~(MODELFLAG_UNLIT | MODELFLAG_GLOBALTRANS);
	}

}
static void RecursePatchTSPFlags (Object *obj, Uint32 and, Uint32 eor)
{
	if (obj->model) {
		int i;
		for (i=0; i < obj->model->nMats; ++i) {
			obj->model->material[i].info.TSPPARAMBUFFER = (obj->model->material[i].info.TSPPARAMBUFFER & and) ^ eor;
		}
	}
	if (obj->no_child) {
		int i;
		for (i=0; i < obj->no_child; ++i)
			RecursePatchTSPFlags (&obj->child[i], and, eor);
	}
}


/*
 * Loads the dome and patches up for NO_FOG
 */
static void RecurseNoFade (Object *obj)
{
	int i;
	obj->collFlag |= OBJECT_NOFADE;
	for (i = 0; i < obj->no_child; ++i)
		RecurseNoFade (&obj->child[i]);
}
void LoadDome (Stream *s)
{
	int i;
	Dome = PNodeLoad (NULL, s, (Allocator *)SHeapAlloc, MissionHeap);
	RecursePatchTSPFlags (Dome->obj, 0xff3fffff, KM_NOFOG << 0x16);
	RecurseNoFade (Dome->obj);
}

/*
 * Sets a model as non-fadeable
 */
void DontWhiteMeOut (STRAT *s)
{
	dAssert (s->obj, "No object!!");
	RecursePatchTSPFlags (s->obj, ~0x00200000, 0);
}
/*
 * Sets a model as non-fogable
 */
void DontFogMeOut (STRAT *s)
{
	dAssert (s->obj, "No object!!");
	RecursePatchTSPFlags (s->obj, 0xff3fffff, KM_NOFOG << 0x16);
}


void SetWhiteOut (float r, float g, float b)
{
	extern int rColClampMin;
	Uint8 rr = RANGE(0, r*255, 255);
	Uint8 gg = RANGE(0, g*255, 255);
	Uint8 bb = RANGE(0, b*255, 255);
	rColClampMin = bb | (gg<<8) | (rr<<16);
}

//extern	Map		*Savmap;
	
void setCalcMatrixFlag(Object *obj)
{
	int i;

	obj->collFlag |= OBJECT_CALC_MATRIX;
	if (obj->no_child > 0)
		for (i=0; i<obj->no_child; i++)
			setCalcMatrixFlag(&obj->child[i]);
}

static void rDrawObjectMattSpr(Object *Obj,STRAT *strat)
{
	static const Vector3 defvec = {0,0,0};

	mPreTranslate(&mCurMatrix,Obj->crntPos.x,Obj->crntPos.y,Obj->crntPos.z);		
/*	mPreRotateXYZ(&mCurMatrix,Obj->crntRot.x,Obj->crntRot.y,Obj->crntRot.z); */
/*	mPreScale(&mCurMatrix,Obj->scale.x,Obj->scale.y,Obj->scale.z); */

	rSprite (&defvec, strat->scale.x, strat->scale.y, fatan2 (-mCurMatrix.m[1][0], mCurMatrix.m[1][1]) * (PI * 2.f), Obj);

}

void ProcessAllStrats(void)
{
	STRAT *strat;
	int	i, j;

	nActiveCollisions = 0;

	//strat = FirstStrat;

	for (j=0; j<4; j++)
		for (i=0; i<4; i++)
			player[j].rdWheel[i].p.x = player[j].rdWheel[i].p.y = player[j].rdWheel[i].p.z = 0.0f;


	strat = FirstStrat;

   while (strat)
	{
	   if ((strat->flag) && (strat->obj))
	  // if (strat->obj)
			objectToWorld(strat);
	
		strat = strat->next;
	} 
  
	
	strat = FirstStrat;

	while (strat)
	{
		if (strat->flag)	
		 	ProcessStrat(strat);
		
		strat = strat->next;
	}

	for (i=0; i<noHits; i++)
	{
		if (hitStratArray[i]->flag)
		{
			hitStratArray[i]->flag |= COLL_HITSTRAT;
		
			if (hitObjectArray[i])
				hitObjectArray[i]->collFlag |= COLL_HITSTRAT;

			hitStratArray[i]->CollWith = hitCollWithArray[i];
		}

		hitStratArray[i] = NULL;
		hitObjectArray[i] = NULL;
		hitStratArray[i] = NULL;
	}
	noHits = 0;
}



// Tweak paramaters
float CloakedScale = 0.8f;
float CloakedAdd = 0.2f;

#define PAUSED 1
void DrawAllStrats(unsigned char mode, int pn)
{
	STRAT *strat;
	int i,i2,found;
	
//	TrapFPUExceptions();

	// Move the intensities towards their targets
	CurrentStratIntensity += (TargetStratIntensity - CurrentStratIntensity) * 0.1f;
	CurrentRedDogIntensity += (TargetRedDogIntensity - CurrentRedDogIntensity) * 0.1f;
	
	// Clear global transparency

	strat = FirstStrat;
	mPush(&mCurMatrix);
	
	while (strat)
	{
		/*DRAW IF THERE'S AN ATTACHED OBJECT AND IT'S NOT HIDDEN AND IT'S  */
		/*NOT DEAD !! */
		if ((strat->flag) && (!(strat->flag & STRAT_HIDDEN)) && (strat->obj))
		{		
#if DRAWNET
			DrawNet(strat);
#endif
			 mCurMatrix = strat->m;
			mPreScale(&mCurMatrix, strat->scale.x, strat->scale.y, strat->scale.z);
		   
#if DRAW_DEBUG
			if (strat->anim->anim.flags & MODEL_ANIM )
				rDrawWayPoint(strat);  	  	   
#endif
			if (strat->flag2 & STRAT2_TRANSLUCENT)
			{
				rGlobalTransparency = strat->trans;
			 	rGlobalModelFlags |= MODELFLAG_GLOBALTRANS;
			}

			if (strat->flag2 & STRAT2_SPECULAR)
			 	rGlobalModelFlags |= MODELFLAG_SPECULAR_FLASH;

			mPostTranslate(&mCurMatrix, strat->pos.x, strat->pos.y, strat->pos.z );
			
		   /*	if (strat->lights)
				UpdateStratLights (strat->lights);
			*/
			if (strat->pnode)	
			{
			   	if ((strat->anim) && (mode != PAUSED) && (!pn))
				{
					rUpdateAnims(strat->pnode,&strat->anim->anim,strat->obj);
					//strat->anim->anim.flags |= ANIM_FLAG_TRIGGERED + ANIM_FLAG_ONCE_DONE;
				}
				if (strat->flag & STRAT_SPRITE)
					rDrawObjectMattSpr (strat->obj,strat);
				else
				{		
					if (!Multiplayer) {
						// It's red dog; swap in RedDogIntensity
						if (strat->Player) {
							float temp = CurrentStratIntensity;
							CurrentStratIntensity = CurrentRedDogIntensity;
							rSetModelContext();
							rDrawObject (strat->obj);
							CurrentStratIntensity = temp;
							rSetModelContext();
						} else {
							rDrawObject (strat->obj);
						}
					} else {
						// Check for LOD on the players
						int pNum = GetPlayerNumber (strat), i;
						Uint32 AA, BB;
						if (pNum == -1) {
							rDrawObject (strat->obj);
						} else {
							float dist;
							Uint32 rGM = rGlobalModelFlags;
							float trans = rGlobalTransparency;

							// Apply cloaking effects
							if (player[pNum].cloaked) {
								float add;
								if (player[pNum].cloaked > 0 && player[pNum].cloaked < 30)
									add = (1.f / 30.f) * (float)(30 - player[pNum].cloaked);
								else
									add = 0;
								add += CloakedAdd;
								rGlobalTransparency = 
									(RANGE (0, player[pNum].playerFireButtonHold * (1.f / 10.f), 1.f) +
									RANGE (0, v3SqrLength (&player[pNum].Player->vel), 1.f)) * 
									CloakedScale + add;
								rGlobalTransparency = MIN(1, rGlobalTransparency);
								rGlobalModelFlags |= MODELFLAG_GLOBALTRANS /*| MODELFLAG_GLOBALSUBTRACTIVE*/;
								AA = -1;
								BB = MODELFLAG_GLOBALSUBTRACTIVE;
							} else {
								AA = ~MODELFLAG_GLOBALSUBTRACTIVE;
								BB = 0;
							}
							dist = pSqrDist (&strat->pos, &currentCamera->pos);
							if (currentCamera == player[pNum].cam) {
								//if (player[pNum].cameraPos != FIRST_PERSON)
								for (i = 0; i < strat->pnode->noObjId; ++i) {
									if (i != 12 && (i < 18 || i > 22) && strat->objId[i] && strat->objId[i]->model)
										strat->objId[i]->model->modelFlags = (strat->objId[i]->model->modelFlags & AA) | BB;
								}
								rDrawObject (strat->obj);
							} else {
								// Needs to be LODded
								int i, objNum;
								Object *obj;
								objNum = (dist < SQR(40)) ? 1 : 2;
								if ((player[pn].PlayerSecondaryWeapon.type == LASER) && (player[pn].zooming < -0.2f))
									objNum = 1;
								obj = player[pNum].lodObject[objNum];
								// Copy the ObjID information across
								for (i = 1; i <= player[pNum].lod[objNum]->noObjId; ++i) {
									if (player[pNum].lodObjId[objNum][i]) {
										Object *lodded, *original;
										// Find the corresponding objId in the master model
										lodded   = player[pNum].lodObjId[objNum][i];
										original = player[pNum].Player->objId[i];
										lodded->crntRot = original->crntRot;
										lodded->m		= original->m;
										lodded->scale	= original->scale;
										lodded->transparency = original->transparency;
										lodded->collFlag= original->collFlag;
										if (i != 12 && (i < 18 || i > 22) && lodded->model)
											lodded->model->modelFlags = (lodded->model->modelFlags & AA) | BB;
									}
								}
								rDrawObject (obj);
							}
							rGlobalModelFlags = rGM;
							rGlobalTransparency = trans;
						} 
					}
				}
			}
		}
		#if 0
		else 
		{ 
			/* Strat is hidden */
			/* Hide any strat lights */
		  /* 	if (strat->lights) {
				mTranslate (&mCurMatrix, 10000.f, 10000.f, 10000.f);
				UpdateStratLights (strat->lights);
			} */			
		}
		#endif
		strat=strat->next;
		rGlobalModelFlags &= ~(MODELFLAG_GLOBALTRANS | MODELFLAG_SPECULAR_FLASH | MODELFLAG_GLOBALSUBTRACTIVE);

	}	
	mPop(&mCurMatrix);
	// Reset the colours and positions
	lColour.r = 1.f;
	lColour.g = 1.f;
	lColour.b = 1.f;
	lOffset.r = 0.f;
	lOffset.g = 0.f;
	lOffset.b = 0.f;


//	UnTrapFPUExceptions();
}

	