/*
 * Object.c
 * Object related functions
 */
#include "..\Reddog.h"
#include "Internal.h"
#include "quat.h"
#include "..\Input.h"
#include "..\strat.h"
#include "..\View.h"
#include "Rasterisers.h"

#define LOW_SHAD_DIST (60.f)

static Object *subBuf[MAX_SUB_OBJECTS];
static int numSub;
static Bool NewColFormat;
/*
 * Recursively load an object
 */
static void RecurseLoadObject (Object *retVal, Object *parent, Allocator *alloc, void *ptr, Stream *s)
{
	int child;

	/* Read in the base structure */
	sRead (retVal, sizeof (Object), s);

	retVal->initRot = retVal->idleRot;
	retVal->initPos = retVal->idlePos;
	
	/* Read in the associated collision points */
	if (retVal->ncp) {
		int i;
		retVal->cp = alloc (ptr, sizeof (CP2) * retVal->ncp);
		retVal->ocpt = alloc (ptr, sizeof (Point3) * retVal->ncp);
		if (NewColFormat) {
			for (i = 0; i < retVal->ncp; ++i) {
				sRead (&retVal->cp[i].centre, sizeof (Point3), s);
				sRead (&retVal->cp[i].radius, sizeof (float), s);
				retVal->cp[i].worldPos.x = retVal->cp[i].worldPos.y = retVal->cp[i].worldPos.z = 0.0f;
			}
		} else {
			for (i = 0; i < retVal->ncp; ++i) {
				sRead (&retVal->cp[i], sizeof (Point3), s);
				retVal->cp[i].radius = 0.f;
			}
		}
		for (i = 0; i < retVal->ncp; ++i)
			memcpy (&retVal->ocpt[i], &retVal->cp[i].centre, sizeof (Point3));
	}
	/* Now to read in the model */
	if (retVal->model) {
		retVal->model = ModelLoad (s, alloc, ptr);
	}
	/* Fill in any data */
	retVal->parent = parent;
	dAssert (OBJECT_GET_ID(retVal->collFlag) < MAX_SUB_OBJECTS, "Out of range subobject encountered");
	subBuf[OBJECT_GET_ID(retVal->collFlag)] = retVal;
	numSub = (Uint16)MAX(OBJECT_GET_ID(retVal->collFlag), numSub);

	retVal->transparency = 1.f;
	retVal->specularCol.colour = 0;

	/* Recursively load children */
	if (retVal->no_child)
		retVal->child = alloc (ptr, sizeof (Object) * retVal->no_child);
	for (child = 0; child < retVal->no_child; ++child) {
		RecurseLoadObject (&retVal->child[child], retVal, alloc, ptr, s);
	}
}

/*
 * Get the animation block for an object
 */
AnimBlock *rGetAnimBlock (Object *obj, Allocator alloc, void *ptr)
{
	int nTexAnims, nAnims;
	AnimBlock *retVal;
	/* Get the number of texture animations */
	nTexAnims = GetTexAnims (obj, NULL);

	/* Get total animation count */
	nAnims = nTexAnims;

	/* Allocates anim block only if needed */
	if (nAnims == 0) {
		retVal = NULL;
	} else {
		/* Allocate memory for the AnimBlock */
		retVal = alloc (ptr, sizeof (AnimBlock));
		retVal->nAnims = nAnims;
		retVal->anim = alloc (ptr, sizeof (Animation) * nAnims);

		/* Get the texture animations */
		GetTexAnims (obj, retVal->anim);
	}
	return retVal;	
}

/*
 * Load an object from the given stream with PNode information
 * node should point to an address of a PNode structure
 * in which pNode information is stored.  If NULL, then a PNode
 * is allocated first. The memory
 * for the name and SubObjectId of the pnode is allocated
 * from alloc(ptr).If you are not interested in allocating this
 * then pass NULL, and memory is also taken from alloc, ptr
 * It is the caller's responsibility to free up the object
 * and any PNode information
 */
PNode *PNodeLoad (PNode *node, Stream *s, Allocator *alloc, void *ptr)
{
	ObjectHeader hdr;
	Object *obj;
	char buf[256];
	int i,size;
	int animflag=0;
	int numframes;
	int interpo;
	short numkey;
 	short maxkey;
	short kloop;
	struct tagAnimBlock *tempanim;
	int loop;
	KEYJUMP *key;
	Bool	NewAnimFormat;
	Bool	NewObjFormat;
	int		error = 0;
	int palVersion;

	/* Allocate the node, if necessary */
	if (node == NULL)
		node = alloc (ptr, sizeof (PNode));

	/* See if we have a PAL version to contend with */
	sRead (&palVersion, 4, s);
	if (palVersion && PAL) {
		//Do we need to skip the NTSC?
		sIgnore (palVersion-4, s);
		sSkip (s);
		sRead (&palVersion, 4, s);
	}
	
	/* Read in the header */
	sRead (&hdr, sizeof (hdr), s);
	dAssert (hdr.tag == 0x544a424f, "Invalid object file");
	/* Check for the new collision point stuff */
	NewColFormat = (hdr.version >= 0x190);
	
	NewObjFormat = (hdr.version >= 0x191);
	
	NewAnimFormat = (hdr.version >= 0x192);
	
	/* Now recursively load in the objects */
	numSub = 0;
	memset (subBuf, 0, sizeof (subBuf));
	obj = alloc (ptr, sizeof (Object));
	RecurseLoadObject (obj, NULL, alloc, ptr, s);
	
	if (NewObjFormat)
		/*READ IN THE FLAG THAT SAYS ANIM ATTACHED/NON-ATTACHED*/
		sRead (&animflag, sizeof (int), s);
	
	if (animflag)
	{
		int nAnims;
		/*READ IN THE TYPE OF ANIM*/
		sRead (&interpo, sizeof (int), s); 
		
		/*	interpo = 0;  */
		
		
		/*READ IN THE NUMBER OF FRAMES*/
		sRead (&numframes, sizeof (int), s);
		
		/*ALLOCATE THE ANIM DATA FOR MODELS,
		SAVE OUT THE POINTER VALUE AS THE PNODE SREAD WILL OVERWRITE IT
		*/
		
		// Here I add in the texture animations as well as the geometric animations
		nAnims = GetTexAnims (obj, NULL) + 1;
		tempanim = node->anim = alloc (ptr,sizeof (AnimBlock));
		node->anim->anim = alloc (ptr,sizeof(Animation) * nAnims);
		node->anim->nAnims = nAnims;
		GetTexAnims (obj, node->anim->anim + 1);
		node->anim->anim->type = ANIM_MODELANIM;
		
		node->anim->anim->typeData = (MODELANIM*) alloc (ptr,sizeof(MODELANIM));
		
		if (interpo)
		{
			
			/*OLD ANIM FORMAT IS THE FULL MONTY WITH SCALE INFO. IN WHETHER OR NOT IT NEEDS IT*/
			if (!NewAnimFormat)
				interpo = 3;
			
			node->anim->anim->type |= INTERPOLATE_ANIM;
			if (interpo == 3)
			{
				((MODELANIM*)node->anim->anim->typeData)->animdata.sinterpolate = (SINTERP*) alloc (ptr,sizeof(SINTERP)*numframes);
				node->anim->anim->type += SCALE_ANIM;
			}
			else
				((MODELANIM*)node->anim->anim->typeData)->animdata.interpolate = (INTERP*) alloc (ptr,sizeof(INTERP)*numframes);
			
			
		}
		else
		{
			((MODELANIM*)node->anim->anim->typeData)->animdata.matrices = (Matrix*) alloc (ptr,sizeof(Matrix)*numframes);
		}
		
		/*READ IN THE NUMBER OF MODELS PER FRAME*/
		sRead (&((MODELANIM*)node->anim->anim->typeData)->numsubmods, sizeof (int), s);
		
		/*READ INT THE NUMBER OF SPECIFIED KEYFRAME JUMPTABLE ENTRIES*/
		sRead (&((MODELANIM*)node->anim->anim->typeData)->numkeyframes, sizeof (short), s);
		numkey = ((MODELANIM*)node->anim->anim->typeData)->numkeyframes;
		
		/*READ THE KEYFRAME JUMPTABLE DATA*/
		key = ((MODELANIM*)node->anim->anim->typeData)->KeyFrames	= (KEYJUMP*) alloc (ptr,sizeof(KEYJUMP) * numkey);
		
		{
			int maxoffset = numframes;//*(((MODELANIM*)node->anim->anim->typeData)->numsubmods-1);
			
			for (kloop=0;kloop<numkey;kloop++)
			{
				
				sRead (((MODELANIM*)node->anim->anim->typeData)->KeyFrames[kloop].name,sizeof(char) * 32, s);
				sRead (&((MODELANIM*)node->anim->anim->typeData)->KeyFrames[kloop].offset, sizeof (short), s);
				sRead (&((MODELANIM*)node->anim->anim->typeData)->KeyFrames[kloop].numframes, sizeof (short), s);
				sRead(&maxkey,sizeof(short),s);
				((MODELANIM*)node->anim->anim->typeData)->KeyFrames[kloop].numkeys = maxkey;
				
				dAssert (((MODELANIM*)node->anim->anim->typeData)->KeyFrames[kloop].offset < maxoffset, "Bad offset");
				//	maxkey = ((MODELANIM*)node->anim->anim->typeData)->KeyFrames[kloop].numkeys = 17;
				((MODELANIM*)node->anim->anim->typeData)->KeyFrames[kloop].key = (short*) alloc (ptr,sizeof(short)*(maxkey+1));
				sRead (((MODELANIM*)node->anim->anim->typeData)->KeyFrames[kloop].key, sizeof (short) * maxkey, s);
				((MODELANIM*)node->anim->anim->typeData)->KeyFrames[kloop].key[maxkey + 1] = -1;
			}
		}
		
		//	sRead (((MODELANIM*)node->anim->anim->typeData)->KeyFrames, sizeof (KEYJUMP) * numkey, s);
		
		/*NOW THE ACTUAL ANIM TRANSFORMS THEMSELVES*/
		
		if (interpo)
		{
			if (interpo == 3)
			{
				sRead(((MODELANIM*)node->anim->anim->typeData)->animdata.sinterpolate,sizeof(SINTERP)*numframes,s); 
				
				for (loop=0;loop<numframes;loop++)
				{
					Point3 temp;
#ifdef _DEBUG
					temp.x = ((MODELANIM*)node->anim->anim->typeData)->animdata.sinterpolate[loop].scale.x; 
					temp.y = ((MODELANIM*)node->anim->anim->typeData)->animdata.sinterpolate[loop].scale.y; 
					temp.z = ((MODELANIM*)node->anim->anim->typeData)->animdata.sinterpolate[loop].scale.z; 
#endif
					if ((temp.x <= 0.00001f) || (temp.y <= 0.00001f) || (temp.z <= 0.00001f))
						error = 1;
					
					//dAssert (temp.x >= 0.00001f && temp.y > 0.00001f && temp.z > 0.00001f, "Bad scale");
				} 
			}
			else
			{
			/*	sRead(((MODELANIM*)node->anim->anim->typeData)->animdata.interpolate,sizeof(INTERP)*numframes,s); 
				*/
				
				for (loop=0;loop<numframes;loop++)
				{
					sRead(&((MODELANIM*)node->anim->anim->typeData)->animdata.interpolate[loop].trans.x,sizeof(float),s); 
					sRead(&((MODELANIM*)node->anim->anim->typeData)->animdata.interpolate[loop].trans.y,sizeof(float),s); 
					sRead(&((MODELANIM*)node->anim->anim->typeData)->animdata.interpolate[loop].trans.z,sizeof(float),s); 
					sRead(&((MODELANIM*)node->anim->anim->typeData)->animdata.interpolate[loop].dtrans.x,sizeof(float),s); 
					sRead(&((MODELANIM*)node->anim->anim->typeData)->animdata.interpolate[loop].dtrans.y,sizeof(float),s); 
					sRead(&((MODELANIM*)node->anim->anim->typeData)->animdata.interpolate[loop].dtrans.z,sizeof(float),s); 
					sRead(&((MODELANIM*)node->anim->anim->typeData)->animdata.interpolate[loop].quat.x,sizeof(float),s); 
					sRead(&((MODELANIM*)node->anim->anim->typeData)->animdata.interpolate[loop].quat.y,sizeof(float),s); 
					sRead(&((MODELANIM*)node->anim->anim->typeData)->animdata.interpolate[loop].quat.z,sizeof(float),s); 
					sRead(&((MODELANIM*)node->anim->anim->typeData)->animdata.interpolate[loop].quat.w,sizeof(float),s); 
					
				}  
				
			}	
			
#ifdef _DEBUG
			if (error)
				error = 0;
#endif
			
				/*	for (loop=0;loop<numframes;loop++)
				{
				
				  mUnit(&tempmat);
				  tempx = ((MODELANIM*)node->anim->anim->typeData)->interpolate[loop].rot.x;
				  tempy = ((MODELANIM*)node->anim->anim->typeData)->interpolate[loop].rot.y;
				  tempz = ((MODELANIM*)node->anim->anim->typeData)->interpolate[loop].rot.z;
				  
					mPostRotateXYZ	(&tempmat, tempx, tempy, tempz);
					MatToQuat(&tempmat,&tempquat);
					NormalizeQuat(&tempquat); 			
					((MODELANIM*)node->anim->anim->typeData)->interpolate[loop].x = tempquat.x;
					((MODELANIM*)node->anim->anim->typeData)->interpolate[loop].y = tempquat.y;
					((MODELANIM*)node->anim->anim->typeData)->interpolate[loop].z = tempquat.z;
					((MODELANIM*)node->anim->anim->typeData)->interpolate[loop].w = tempquat.w;
					}
			*/
		}	
		else	  
			sRead(((MODELANIM*)node->anim->anim->typeData)->animdata.matrices,sizeof(Matrix)*numframes,s); 
		
	}
	
	
	/* Now to load in the PNode itself */
	sRead (node, sizeof (PNode), s);


	if (PAL)
	{
		v3Scale(&node->airFriction,&node->airFriction,PAL_MOVESCALE);
		v3Scale(&node->groundFriction,&node->groundFriction,PAL_MOVESCALE);
	
	}
	/* Copy in the subobject array */
	if (numSub) {
		node->objId = alloc (ptr, sizeof (Object *) * (numSub+1));
		memcpy (node->objId, subBuf, sizeof (Object *) * (numSub+1));
	}
	node->noObjId = numSub;
	
	/* Read in the name */
	i=0;
	sRead (&i, sizeof (int), s);
	sRead (buf, i, s);
	node->name = alloc (ptr, i);
	strcpy (node->name, buf);
	node->obj = obj;
	
   	if (!animflag) 
		/* Get the animation block */
		node->anim = rGetAnimBlock (obj, alloc, ptr);
	else
		node->anim = tempanim;
	
	sSkip (s);

	if (palVersion && !PAL) {
		//Do we need to skip the PAL?
		sRead (&palVersion, 4, s);
		sIgnore (palVersion-4, s);
		sSkip (s);
	}

	return node;
}

static void RecurseUpdateAnim(PNode *node, Object *obj, Matrix *mp)
{
	int i;
	int OBJID = OBJECT_GET_ID(obj->collFlag);
	
	
	if (OBJID)	  

		obj->m = mp[OBJID-1];	 
		

   	obj->collFlag &=  (0xffffffff-OBJECT_CALC_MATRIX);


	for (i = 0; i < obj->no_child; ++i)
	{
		RecurseUpdateAnim (node,&obj->child[i],mp);	
	}

}

/*GLOBALLY USED AND SET UP PER ANIM*/
static float CurrSlerp;
static float FRAME;
static int GlobalAnimType;

static Matrix newmat;

//#define TWEENTIME 5.0f  

#define TWEENTIME 15.0f
#define TWEENTIMERCP 1/15.0f
  
static void RecurseUpdateAnimInt(Object *obj,  ANIMPOINT mp, ANIMPOINT NextKey)
{
	int i;
	int OBJID = OBJECT_GET_ID(obj->collFlag);
	QUAT Delta;
   //	int testmatt;
	

	if ((OBJID) )	  
	{

		if (GlobalAnimType & SCALE_ANIM)
		{ 

			obj->animscale.x += mp.sinterpolate[OBJID-1].dscale.x;	 
			obj->animscale.y += mp.sinterpolate[OBJID-1].dscale.y;	 
			obj->animscale.z += mp.sinterpolate[OBJID-1].dscale.z;	 
		

	 //		if 
	   //			((obj->animscale.x < 0) ||
		 //		(obj->animscale.y < 0) ||
		  //		(obj->animscale.z < 0))
		   //		testmatt = 1;




		   //	QuatSlerp(&mp.sinterpolate[OBJID-1].quat,&NextKey.sinterpolate[OBJID-1].quat,CurrSlerp,&Delta);  
		   
		   //	QuatToMat(&Delta,&obj->m);
		   	QuatSlerpMat(&mp.sinterpolate[OBJID-1].quat,&NextKey.sinterpolate[OBJID-1].quat,CurrSlerp,&Delta,&obj->m);  
			   
		   
			if (obj->collFlag & OBJECT_STRATMOVE)
			{
		 		//BUILD IN STRAT ROTATIONS IF NEEDED
			   	mUnit(&newmat);
				mRotateXYZ(&newmat,obj->crntRot.x,obj->crntRot.y,obj->crntRot.z);
				mPreMultiply(&obj->m,&newmat);   
			}
		   
		   	mPreScale 	(&obj->m, obj->scale.x * obj->animscale.x, obj->scale.y * obj->animscale.y, obj->scale.z * obj->animscale.z);  
			obj->crntPos.x += mp.sinterpolate[OBJID-1].dtrans.x;	 
			obj->crntPos.y += mp.sinterpolate[OBJID-1].dtrans.y;	 
			obj->crntPos.z += mp.sinterpolate[OBJID-1].dtrans.z;	 
		}				  
		else
		{
		
			//QuatSlerp(&mp.interpolate[OBJID-1].quat,&NextKey.interpolate[OBJID-1].quat,CurrSlerp,&Delta);  
		   
			//QuatToMat(&Delta,&obj->m);
			QuatSlerpMat(&mp.interpolate[OBJID-1].quat,&NextKey.interpolate[OBJID-1].quat,CurrSlerp,&Delta,&obj->m);  
		  
			if (obj->collFlag & OBJECT_STRATMOVE)
			{
		 		//BUILD IN STRAT ROTATIONS IF NEEDED
			   	mUnit(&newmat);
				mRotateXYZ(&newmat,obj->crntRot.x,obj->crntRot.y,obj->crntRot.z);
				mPreMultiply(&obj->m,&newmat);   
			}
		   
			mPreScale(&obj->m,obj->scale.x,obj->scale.y,obj->scale.z);
			obj->crntPos.x += mp.interpolate[OBJID-1].dtrans.x;	 
			obj->crntPos.y += mp.interpolate[OBJID-1].dtrans.y;	 
			obj->crntPos.z += mp.interpolate[OBJID-1].dtrans.z;	 


		
		} 


		mPostTranslate	(&obj->m, obj->crntPos.x, obj->crntPos.y, obj->crntPos.z);		


		obj->collFlag &=  (0xffffffff-OBJECT_CALC_MATRIX);



	 }	


	for (i = 0; i < obj->no_child; ++i)
	{
		RecurseUpdateAnimInt (&obj->child[i],mp,NextKey);	
	}

}




/*TWEENING CONTROL..IT IS VITAL TO NOTE THAT OBJ->INITROT,OBJ->INITPOS,OBJ->IDLEPOS, AND OBJ->IDLEROT
  ARE CURRENTLY OVERLOADED WITH THE TRANS,POS AND QUATS OF THE KEYFRAME WE ARE GOING FROM */
static void RecurseUpdateAnimIntTween(Object *obj, ANIMPOINT mp)
{
	int i;
	int OBJID = OBJECT_GET_ID(obj->collFlag);
	QUAT Delta,Current;
	Point3 deltpos;

	if ((OBJID) )	  
	{
	   	if (GlobalAnimType & SCALE_ANIM)
		{
			/*DERIVE POSITION OFFSET*/
			deltpos.x = (mp.sinterpolate[OBJID-1].trans.x - obj->idlePos.x) * TWEENTIMERCP;
			deltpos.y = (mp.sinterpolate[OBJID-1].trans.y - obj->idlePos.y) * TWEENTIMERCP;
			deltpos.z = (mp.sinterpolate[OBJID-1].trans.z - obj->idlePos.z) * TWEENTIMERCP;
			deltpos.x *= FRAME;
			deltpos.y *= FRAME;
			deltpos.z *= FRAME;
			obj->crntPos.x = obj->idlePos.x+ deltpos.x;	 
			obj->crntPos.y = obj->idlePos.y+ deltpos.y;	 
			obj->crntPos.z = obj->idlePos.z+ deltpos.z;	 


			/*DERIVE SCALE OFFSETS*/
			deltpos.x = ((mp.sinterpolate[OBJID-1].scale.x) - obj->idleRot.x) * TWEENTIMERCP;
			deltpos.y = ((mp.sinterpolate[OBJID-1].scale.y) - obj->idleRot.y) * TWEENTIMERCP;
			deltpos.z = ((mp.sinterpolate[OBJID-1].scale.z) - obj->idleRot.z) * TWEENTIMERCP;
			deltpos.x *= FRAME;
			deltpos.y *= FRAME;
			deltpos.z *= FRAME;
			obj->animscale.x = obj->idleRot.x + deltpos.x;
			obj->animscale.y = obj->idleRot.y + deltpos.y;
			obj->animscale.z = obj->idleRot.z + deltpos.z;

			/*	CurrSlerp = FRAME * CurrTime;*/

			Current.x = obj->initRot.x;
			Current.y = obj->initRot.y;
			Current.z = obj->initRot.z;
			Current.w = obj->initPos.x;

		 

		   //	QuatSlerp(&Current,&mp.sinterpolate[OBJID-1].quat,CurrSlerp,&Delta);  
		   
		   //	QuatToMat(&Delta,&obj->m);
		   	QuatSlerpMat(&Current,&mp.sinterpolate[OBJID-1].quat,CurrSlerp,&Delta,&obj->m);  
			
			if (obj->collFlag & OBJECT_STRATMOVE)
			{
	 		 	//BUILD IN STRAT ROTATIONS IF NEEDED
			   	mUnit(&newmat);
				mRotateXYZ(&newmat,obj->crntRot.x,obj->crntRot.y,obj->crntRot.z);
	 			mPreMultiply(&obj->m,&newmat);   
			}
			   
			mPreScale  		(&obj->m, obj->scale.x * obj->animscale.x, obj->scale.y * obj->animscale.y, obj->scale.z * obj->animscale.z);  
		}	
		else
		{
			/*DERIVE POSITION OFFSET*/
			deltpos.x = (mp.interpolate[OBJID-1].trans.x - obj->idlePos.x) * TWEENTIMERCP;
			deltpos.y = (mp.interpolate[OBJID-1].trans.y - obj->idlePos.y) * TWEENTIMERCP;
			deltpos.z = (mp.interpolate[OBJID-1].trans.z - obj->idlePos.z) * TWEENTIMERCP;
			deltpos.x *= FRAME;
			deltpos.y *= FRAME;
			deltpos.z *= FRAME;
			obj->crntPos.x = obj->idlePos.x+ deltpos.x;	 
			obj->crntPos.y = obj->idlePos.y+ deltpos.y;	 
			obj->crntPos.z = obj->idlePos.z+ deltpos.z;	 



			/*	CurrSlerp = FRAME * CurrTime;*/

			Current.x = obj->initRot.x;
			Current.y = obj->initRot.y;
			Current.z = obj->initRot.z;
			Current.w = obj->initPos.x;

		 

	  //		QuatSlerp(&Current,&mp.interpolate[OBJID-1].quat,CurrSlerp,&Delta);  
		   
		//	QuatToMat(&Delta,&obj->m);
			QuatSlerpMat(&Current,&mp.interpolate[OBJID-1].quat,CurrSlerp,&Delta,&obj->m);  
		   


			if (obj->collFlag & OBJECT_STRATMOVE)
			{
	 		 	//BUILD IN STRAT ROTATIONS IF NEEDED
			   	mUnit(&newmat);
				mRotateXYZ(&newmat,obj->crntRot.x,obj->crntRot.y,obj->crntRot.z);
	 			mPreMultiply(&obj->m,&newmat);   
			}
			mPreScale(&obj->m,obj->scale.x,obj->scale.y,obj->scale.z);
			   

		}
		
		
		mPostTranslate	(&obj->m, obj->crntPos.x, obj->crntPos.y, obj->crntPos.z);		


		obj->collFlag &=  (0xffffffff-OBJECT_CALC_MATRIX);

	 }	


	for (i = 0; i < obj->no_child; ++i)
	{
		RecurseUpdateAnimIntTween (&obj->child[i],mp);	
	}

}




static void RecurseUpdateAnimIntTween2(Object *obj, ANIMPOINT mp)
{
	int i;
	int OBJID = OBJECT_GET_ID(obj->collFlag);
	QUAT Delta,Current;
	Point3 deltpos;

	if ((OBJID))	  
	{
		   if (GlobalAnimType & SCALE_ANIM)
		{
	   
			/*DERIVE POSITION OFFSET*/
			deltpos.x = (mp.sinterpolate[OBJID-1].trans.x - obj->idlePos.x) * TWEENTIMERCP;
			deltpos.y = (mp.sinterpolate[OBJID-1].trans.y - obj->idlePos.y) * TWEENTIMERCP;
			deltpos.z = (mp.sinterpolate[OBJID-1].trans.z - obj->idlePos.z) * TWEENTIMERCP;
			deltpos.x *= FRAME;
			deltpos.y *= FRAME;
			deltpos.z *= FRAME;
			obj->crntPos.x = obj->idlePos.x+ deltpos.x;	 
			obj->crntPos.y = obj->idlePos.y+ deltpos.y;	 
			obj->crntPos.z = obj->idlePos.z+ deltpos.z;	 
			obj->idlePos.x = obj->crntPos.x;
			obj->idlePos.y = obj->crntPos.y;
			obj->idlePos.z = obj->crntPos.z;



			/*DERIVE SCALE OFFSETS*/
			deltpos.x = ((mp.sinterpolate[OBJID-1].scale.x) - obj->idleRot.x) * TWEENTIMERCP;
			deltpos.y = ((mp.sinterpolate[OBJID-1].scale.y) - obj->idleRot.y) * TWEENTIMERCP;
			deltpos.z = ((mp.sinterpolate[OBJID-1].scale.z) - obj->idleRot.z) * TWEENTIMERCP;
			deltpos.x *= FRAME;
			deltpos.y *= FRAME;
			deltpos.z *= FRAME;
			obj->animscale.x = obj->idleRot.x + deltpos.x;
			obj->animscale.y = obj->idleRot.y + deltpos.y;
			obj->animscale.z = obj->idleRot.z + deltpos.z;
			obj->idleRot.x = obj->animscale.x;
			obj->idleRot.y = obj->animscale.y;
			obj->idleRot.z = obj->animscale.z;


		  /*	CurrSlerp = FRAME * CurrTime;*/

			Current.x = obj->initRot.x;
			Current.y = obj->initRot.y;
			Current.z = obj->initRot.z;
			Current.w = obj->initPos.x;

		 

		  //	QuatSlerp(&Current,&mp.sinterpolate[OBJID-1].quat,CurrSlerp,&Delta);  
		   
		   //	QuatToMat(&Delta,&obj->m);
			QuatSlerpMat(&Current,&mp.sinterpolate[OBJID-1].quat,CurrSlerp,&Delta,&obj->m);  

			if (obj->collFlag & OBJECT_STRATMOVE)
			{
				//BUILD IN STRAT ROTATIONS IF NEEDED
			   	mUnit(&newmat);
				mRotateXYZ(&newmat,obj->crntRot.x,obj->crntRot.y,obj->crntRot.z);
				mPreMultiply(&obj->m,&newmat);   
			}


			   
		 	mPreScale  		(&obj->m, obj->scale.x * obj->animscale.x, obj->scale.y * obj->animscale.y, obj->scale.z * obj->animscale.z);  
			obj->initRot.x = Delta.x;
			obj->initRot.y = Delta.y;
			obj->initRot.z = Delta.z;
			obj->initPos.x = Delta.w;

		}
		else
		{
			/*DERIVE POSITION OFFSET*/
			deltpos.x = (mp.interpolate[OBJID-1].trans.x - obj->idlePos.x) * TWEENTIMERCP;
			deltpos.y = (mp.interpolate[OBJID-1].trans.y - obj->idlePos.y) * TWEENTIMERCP;
			deltpos.z = (mp.interpolate[OBJID-1].trans.z - obj->idlePos.z) * TWEENTIMERCP;
			deltpos.x *= FRAME;
			deltpos.y *= FRAME;
			deltpos.z *= FRAME;
			obj->crntPos.x = obj->idlePos.x+ deltpos.x;	 
			obj->crntPos.y = obj->idlePos.y+ deltpos.y;	 
			obj->crntPos.z = obj->idlePos.z+ deltpos.z;	 
			obj->idlePos.x = obj->crntPos.x;
			obj->idlePos.y = obj->crntPos.y;
			obj->idlePos.z = obj->crntPos.z;



			Current.x = obj->initRot.x;
			Current.y = obj->initRot.y;
			Current.z = obj->initRot.z;
			Current.w = obj->initPos.x;

		 

		   //	QuatSlerp(&Current,&mp.interpolate[OBJID-1].quat,CurrSlerp,&Delta);  
		   
		   //	QuatToMat(&Delta,&obj->m);
			QuatSlerpMat(&Current,&mp.interpolate[OBJID-1].quat,CurrSlerp,&Delta,&obj->m);  
		   
			if (obj->collFlag & OBJECT_STRATMOVE)
			{
				//BUILD IN STRAT ROTATIONS IF NEEDED
			   	mUnit(&newmat);
				mRotateXYZ(&newmat,obj->crntRot.x,obj->crntRot.y,obj->crntRot.z);
				mPreMultiply(&obj->m,&newmat);   
			}

			mPreScale(&obj->m,obj->scale.x,obj->scale.y,obj->scale.z);

			obj->initRot.x = Delta.x;
			obj->initRot.y = Delta.y;
			obj->initRot.z = Delta.z;
			obj->initPos.x = Delta.w;


		}
		mPostTranslate	(&obj->m, obj->crntPos.x, obj->crntPos.y, obj->crntPos.z);		


		obj->collFlag &=  (0xffffffff-OBJECT_CALC_MATRIX);

	 }	


	for (i = 0; i < obj->no_child; ++i)
	{
		RecurseUpdateAnimIntTween2 (&obj->child[i],mp);	
	}

}


static void RecurseUpdateAnimIntFirst(Object *obj, ANIMPOINT mp)
{
	int i;
	int OBJID = OBJECT_GET_ID(obj->collFlag);
  
	if ((OBJID) )	  
	{
		if (GlobalAnimType & SCALE_ANIM)
		{

		  //	obj->animscale.x = obj->scale.x * mp.sinterpolate[OBJID-1].scale.x; 
		  //	obj->animscale.y = obj->scale.y * mp.sinterpolate[OBJID-1].scale.y; 
		  //	obj->animscale.z = obj->scale.z * mp.sinterpolate[OBJID-1].scale.z; 
			obj->animscale.x = mp.sinterpolate[OBJID-1].scale.x; 
			obj->animscale.y = mp.sinterpolate[OBJID-1].scale.y; 
			obj->animscale.z = mp.sinterpolate[OBJID-1].scale.z; 
			obj->crntPos = mp.sinterpolate[OBJID-1].trans;	 
  


		
			QuatToMat(&mp.sinterpolate[OBJID-1].quat,&obj->m);
			if((obj->collFlag & OBJECT_STRATMOVE))
			{
				//BUILD IN STRAT ROTATIONS IF NEEDED
			   	mUnit(&newmat);
				mRotateXYZ(&newmat,obj->crntRot.x,obj->crntRot.y,obj->crntRot.z);
				mPreMultiply(&obj->m,&newmat);   
			}
			   
			mPreScale(&obj->m, obj->animscale.x, obj->animscale.y, obj->animscale.z);  
			
		}
		else
		{
			obj->crntPos = mp.interpolate[OBJID-1].trans;	 
  
		
			QuatToMat(&mp.interpolate[OBJID-1].quat,&obj->m);
	   
			if((obj->collFlag & OBJECT_STRATMOVE))
			{
				//BUILD IN STRAT ROTATIONS IF NEEDED
			   	mUnit(&newmat);
				mRotateXYZ(&newmat,obj->crntRot.x,obj->crntRot.y,obj->crntRot.z);
				mPreMultiply(&obj->m,&newmat);   
			}
			mPreScale(&obj->m,obj->scale.x,obj->scale.y,obj->scale.z);


		}
			
			
		mPostTranslate(&obj->m, obj->crntPos.x, obj->crntPos.y, obj->crntPos.z);		

		obj->collFlag &=  (0xffffffff-OBJECT_CALC_MATRIX);
	
	}	


	for (i = 0; i < obj->no_child; ++i)
	{
		RecurseUpdateAnimIntFirst(&obj->child[i],mp);	
	}

}



void UpdateAnim(StratAnim *curAnim)
{

	if (curAnim->flags & ANIM_FLAG_RUNNING) {
		if (curAnim->curDelay-- <= 1) {
			curAnim->curDelay = curAnim->anim->frameDelay * curAnim->multiplier;
			if (curAnim->flags & ANIM_FLAG_BACKWARDS) {
				curAnim->frame--;
				if ((Sint32)curAnim->frame < (Sint32)curAnim->anim->firstFrame) {
					if (curAnim->flags & ANIM_FLAG_TRIGGER)
						curAnim->flags |= ANIM_FLAG_TRIGGERED;
					if (curAnim->flags & ANIM_FLAG_REPEAT)
						curAnim->frame = curAnim->anim->lastFrame - 1;
					else if (curAnim->flags & ANIM_FLAG_PINGPONG) {
						curAnim->frame = curAnim->anim->firstFrame + 1;
						curAnim->flags &= ~ANIM_FLAG_BACKWARDS;
					} else {
						/* Hold the last frame */
						curAnim->frame = curAnim->anim->firstFrame;
						curAnim->flags &= ~ANIM_FLAG_RUNNING;
					}
				}
			} else 
			{
				curAnim->frame++;
			   	if (curAnim->frame >= curAnim->anim->lastFrame)
				{
					if (curAnim->flags & ANIM_FLAG_TRIGGER)
						curAnim->flags |= ANIM_FLAG_TRIGGERED;
					if (curAnim->flags & ANIM_FLAG_REPEAT)
					{
						curAnim->frame = curAnim->anim->firstFrame;
						curAnim->flags &= ~ANIM_FLAG_ONCE_DONE;
					}
					else if (curAnim->flags & ANIM_FLAG_PINGPONG)
					{
						curAnim->frame = curAnim->anim->lastFrame - 2;
						curAnim->flags |= ANIM_FLAG_BACKWARDS;
					} else 
					{
						/* Hold the last frame */
					  	curAnim->frame = curAnim->anim->lastFrame - 1;
						curAnim->flags &= ~ANIM_FLAG_RUNNING;
						curAnim->flags &= ~ANIM_FLAG_ONCE_DONE;
					}
				}
			}
		}
	}


}

static Matrix *GetFrameBase(PNode *node,StratAnim *curAnim)
{
	int start;
	short submods;
	Matrix *mp;

	start = (int)(curAnim->anim->typeData);
	submods = (((MODELANIM*)node->anim->anim->typeData)->numsubmods)-1;
	mp = (Matrix*)(&((MODELANIM*)node->anim->anim->typeData)->animdata.matrices[start+(submods * curAnim->frame)]);
    return(mp);

}



/*this finds the last KEY frame we were on and the one we were going to*/
/*using this a quat repr can be made up, together with scale and transform*/


static ANIMPOINT GetTweenQuats(PNode *node,StratAnim *curAnim,ANIMPOINT *NextKey,Object *obj)
{
	int i;
	int time;
	int start;
	short submods;
	short maxkey;
	ANIMPOINT mp;
	int nexttime;

	/*GET THE ABSOLUTE START OF OUR ANIMATION*/
	start =((KEYJUMP*)curAnim->anim->typeData)->offset;
	submods = (((MODELANIM*)node->anim->anim->typeData)->numsubmods)-1;

	if (GlobalAnimType & SCALE_ANIM)
	{
		mp.sinterpolate = (SINTERP*)(&((MODELANIM*)node->anim->anim->typeData)->animdata.sinterpolate[start]);
		NextKey->sinterpolate = mp.sinterpolate+submods;
	}
	else
	{
		mp.interpolate = (INTERP*)(&((MODELANIM*)node->anim->anim->typeData)->animdata.interpolate[start]);
		NextKey->interpolate = mp.interpolate+submods;

	}

   /* WERE WE STILL RUNNING THE ANIM ? IF SO,
	  THE FRAME WILL BE 1 MORE THAN THE LAST TIME THIS LOT GOT PROCESSED, SO TAKE IT BACK
	  TO THE FRAME WE WERE ON*/

  	if (curAnim->flags & ANIM_FLAG_RUNNING)
	{
		if (curAnim->frame)
			curAnim->frame--;
	}
	else
	{
		//OUR INTERPOLATION STARTS FROM THE LAST FRAME OF THIS ANIM
		//SO SET MP AND NEXTKEY TO BE THE SAME
	//	maxkey = ((KEYJUMP*)curAnim->anim->typeData)->numkeys + 1;
		maxkey = ((KEYJUMP*)curAnim->anim->typeData)->numkeys;

	   //	if (Global

			if (GlobalAnimType & SCALE_ANIM)
				mp.sinterpolate += submods * (maxkey-1);
			else
				mp.interpolate += submods * (maxkey-1);

	 /*	for (i=0;i<maxkey;i++)
		{
			if (((KEYJUMP*)curAnim->anim->typeData)->key[i] < 0)
				break;
			if (GlobalAnimType & SCALE_ANIM)
				mp.sinterpolate += submods;
			else
				mp.interpolate += submods;
		}*/

		//NOW POINTING AT ANIM'S LAST KEY FRAME BASE +1
		//if (curAnim->frame)
   /*	if (GlobalAnimType & SCALE_ANIM)
			mp.sinterpolate -=submods;
	else
			mp.interpolate -= submods;
	 */
		//i--;
	if (GlobalAnimType & SCALE_ANIM)
		NextKey->sinterpolate =mp.sinterpolate;
	else
		NextKey->interpolate =mp.interpolate;
		curAnim->flags |= ANIM_TWEENING;
		obj->slerpDelta = 1.0f * TWEENTIMERCP;

		FRAME = obj->slerpFrame = 1.0f;
		CurrSlerp = FRAME * obj->slerpDelta;

		return (mp);
	}


	/*Check through all our keys, to get the keyframe to go to*/
	maxkey = ((KEYJUMP*)curAnim->anim->typeData)->numkeys + 1;
	for (i=0;i<maxkey;i++)
	{

		
		/*IF THE KEY RECORD IS -1 THEN END OF KEY RECORDS REACHED SO BREAK OUT*/
		if (((KEYJUMP*)curAnim->anim->typeData)->key[i] < 0)
			break;


		/*IF OUR FRAME IS STILL LESS THAN THE FRAME OF THE NEXT KEY, THEN BREAK OUT*/
		if (curAnim->frame < ((KEYJUMP*)curAnim->anim->typeData)->key[i]) 
		   	break;

		

	   	/*IF THE CURRENT FRAME = THE NEXT KEYS FRAME, THEN WE ARE ON A KEYFRAME*/
		   
		if (curAnim->frame == ((KEYJUMP*)curAnim->anim->typeData)->key[i])
		{
			
			obj->slerpDelta = 1.0f * TWEENTIMERCP;
			
			if (((KEYJUMP*)curAnim->anim->typeData)->key[i]) 
			{
			   
				if (GlobalAnimType & SCALE_ANIM)
				{
				   	mp.sinterpolate += submods;
					NextKey->sinterpolate += submods;
				}
				else
				{
				   	mp.interpolate += submods;
					NextKey->interpolate += submods;
				}

			}
			break;
		}	

	

		if (((KEYJUMP*)curAnim->anim->typeData)->key[i]) 
			if (curAnim->frame > ((KEYJUMP*)curAnim->anim->typeData)->key[i]) 
			{
				if (GlobalAnimType & SCALE_ANIM)
				{
				   	mp.sinterpolate  += submods;
				   	NextKey->sinterpolate = mp.sinterpolate+submods;
				   //	NextKey->sinterpolate+=submods;
		  
				}
				else
				{

				   	mp.interpolate  += submods;
				  	NextKey->interpolate = mp.interpolate+submods;

				  //	NextKey->interpolate+=submods;
				}

			}
	}


	curAnim->flags |= ANIM_TWEENING;
	obj->slerpDelta = 1.0f * TWEENTIMERCP;

	FRAME = obj->slerpFrame = 1.0f;
	CurrSlerp = FRAME * obj->slerpDelta;


	return(mp);
}

static void Test(Object *obj)
{
	obj->slerpDelta = 1.0f * TWEENTIMERCP;

	FRAME = obj->slerpFrame = 1.0f;
	CurrSlerp = FRAME * obj->slerpDelta;
}


static void RecurseUpdateTweens (Object *obj, ANIMPOINT mp, ANIMPOINT NextKey)
{
	int i;
	int OBJID = OBJECT_GET_ID(obj->collFlag);
	QUAT Delta;
	

	if ((OBJID))	  
	{
	   
		
		if (GlobalAnimType & SCALE_ANIM)
		{
			QuatSlerpMat(&mp.sinterpolate[OBJID-1].quat,&NextKey.sinterpolate[OBJID-1].quat,CurrSlerp,&Delta,&obj->m);  
		  //	QuatSlerp(&mp.sinterpolate[OBJID-1].quat,&NextKey.sinterpolate[OBJID-1].quat,CurrSlerp,&Delta);  
		   
			/*HACK TO STORE THE QUAT FOR THE OBJECT IN ITS INITROT AND INITPOS*/
			obj->initRot.x = Delta.x;	   
			obj->initRot.y = Delta.y;	   
			obj->initRot.z = Delta.z;	   
			obj->initPos.x = Delta.w;	   

			/*WE'LL NOW USE IDLEPOS FOR THE START TRANS	 */
		  
		  
			obj->idlePos.x = mp.sinterpolate[OBJID-1].trans.x;	   
			obj->idlePos.y = mp.sinterpolate[OBJID-1].trans.y;	   
	   		obj->idlePos.z = mp.sinterpolate[OBJID-1].trans.z;	   
			obj->idlePos.x += FRAME * mp.sinterpolate[OBJID-1].dtrans.x;	 
			obj->idlePos.y += FRAME * mp.sinterpolate[OBJID-1].dtrans.y;	 
			obj->idlePos.z += FRAME * mp.sinterpolate[OBJID-1].dtrans.z;	 

			/*WE'LL NOW USE IDLEROT FOR THE START SCALE	 */
	
			obj->idleRot.x = mp.sinterpolate[OBJID-1].scale.x;	   
	   		obj->idleRot.y = mp.sinterpolate[OBJID-1].scale.y;	   
	   		obj->idleRot.z = mp.sinterpolate[OBJID-1].scale.z;	   
			obj->idleRot.x += FRAME * mp.sinterpolate[OBJID-1].dscale.x;	 
			obj->idleRot.y += FRAME * mp.sinterpolate[OBJID-1].dscale.y;	 
			obj->idleRot.z += FRAME * mp.sinterpolate[OBJID-1].dscale.z;	 
	 //		QuatToMat(&Delta,&obj->m);
	 	  	if (obj->collFlag & OBJECT_STRATMOVE)
			{
	 		 	//BUILD IN STRAT ROTATIONS IF NEEDED
			   	mUnit(&newmat);
				mRotateXYZ(&newmat,obj->crntRot.x,obj->crntRot.y,obj->crntRot.z);
	 			mPreMultiply(&obj->m,&newmat);   
			}


			mPreScale(&obj->m,obj->scale.x * obj->animscale.x,obj->scale.y * obj->animscale.y,obj->scale.z * obj->animscale.z);

		
		}
		else
		{

			QuatSlerpMat(&mp.interpolate[OBJID-1].quat,&NextKey.interpolate[OBJID-1].quat,CurrSlerp,&Delta,&obj->m);  
		   //	QuatSlerp(&mp.interpolate[OBJID-1].quat,&NextKey.interpolate[OBJID-1].quat,CurrSlerp,&Delta);  
		   
			/*HACK TO STORE THE QUAT FOR THE OBJECT IN ITS INITROT AND INITPOS*/
			obj->initRot.x = Delta.x;	   
	  		obj->initRot.y = Delta.y;	   
			obj->initRot.z = Delta.z;	   
			obj->initPos.x = Delta.w;	   

			/*WE'LL NOW USE IDLEPOS FOR THE START TRANS	 */
		  
		  
			obj->idlePos.x = mp.interpolate[OBJID-1].trans.x;	   
			obj->idlePos.y = mp.interpolate[OBJID-1].trans.y;	   
	   		obj->idlePos.z = mp.interpolate[OBJID-1].trans.z;	   
			obj->idlePos.x += FRAME * mp.interpolate[OBJID-1].dtrans.x;	 
			obj->idlePos.y += FRAME * mp.interpolate[OBJID-1].dtrans.y;	 
			obj->idlePos.z += FRAME * mp.interpolate[OBJID-1].dtrans.z;	 

		//	QuatToMat(&Delta,&obj->m);
	 	  	if (obj->collFlag & OBJECT_STRATMOVE)
			{
	 		 	//BUILD IN STRAT ROTATIONS IF NEEDED
			   	mUnit(&newmat);
				mRotateXYZ(&newmat,obj->crntRot.x,obj->crntRot.y,obj->crntRot.z);
	 			mPreMultiply(&obj->m,&newmat);   
			}


			mPreScale(&obj->m,obj->scale.x,obj->scale.y,obj->scale.z);




		}


	
	}


	for (i = 0; i < obj->no_child; ++i)
	{
		RecurseUpdateTweens (&obj->child[i],mp,NextKey);	
	}

}



static ANIMPOINT GetFrameBaseInt(PNode *node,StratAnim *curAnim, int *KeyFrame, ANIMPOINT *NextKey,Object *obj)
{
	int i;
	int time;
	int start;
	short maxkey;
	short submods;
	ANIMPOINT mp,BASE;
	int nexttime;

	/*tPrintf(2, 20, "FRAME = %d", curAnim->frame);	 */


	/*GET THE ABSOLUTE START OF OUR ANIMATION*/
	start =((KEYJUMP*)curAnim->anim->typeData)->offset;
	submods = (((MODELANIM*)node->anim->anim->typeData)->numsubmods)-1;


	if (GlobalAnimType & SCALE_ANIM)
	{
		BASE.sinterpolate = mp.sinterpolate = (SINTERP*)(&((MODELANIM*)node->anim->anim->typeData)->animdata.sinterpolate[start]);
		NextKey->sinterpolate = mp.sinterpolate+submods; 
	}
	else
	{

		BASE.interpolate = mp.interpolate = (INTERP*)(&((MODELANIM*)node->anim->anim->typeData)->animdata.interpolate[start]);
		NextKey->interpolate = mp.interpolate+submods; 

	}

	//INCREMENT THE FRAME INFORMATION
	obj->slerpFrame++;

	maxkey = ((KEYJUMP*)curAnim->anim->typeData)->numkeys + 1;
	
	/*Check through all our keys*/
	for (i=0;i<maxkey;i++)
	{

		
		/*IF THE KEY RECORD IS -1 THEN END OF KEY RECORDS REACHED SO BREAK OUT*/
		/*AND THE DRUMMER FOR SWING OUT SISTER DID CROC'S PERCUSSION*/
		if (((KEYJUMP*)curAnim->anim->typeData)->key[i] < 0)
			break;


		/*IF OUR FRAME IS STILL LESS THAN THE FRAME OF THE NEXT KEY, THEN BREAK OUT*/
		if (curAnim->frame < ((KEYJUMP*)curAnim->anim->typeData)->key[i]) 
		   	break;

		

	   	/*IF THE CURRENT FRAME = THE NEXT KEYS FRAME, THEN WE ARE ON A KEYFRAME*/
		   
		if (curAnim->frame == ((KEYJUMP*)curAnim->anim->typeData)->key[i])
		{
			
			*KeyFrame=1;
			nexttime = ((KEYJUMP*)curAnim->anim->typeData)->key[i+1];

			//SET THE DELTA VALUE
			obj->slerpDelta = 1.0f/(float)((nexttime - curAnim->frame));
		 
			//RESET THE FRAME INFORMATION
			obj->slerpFrame = 0;

			//IF IT ISN'T THE FIRST KEYFRAME, THEN INCREMENT PTR BY SIZE OF ANIMS
			if (((KEYJUMP*)curAnim->anim->typeData)->key[i]) 
			{

				if (GlobalAnimType & SCALE_ANIM)
				{
					mp.sinterpolate += submods;
					NextKey->sinterpolate += submods;	
				}
				else
				{																  
					mp.interpolate += submods;
					NextKey->interpolate += submods;	

				}
			}
			break;
		}	

	
		//NO KEYFRAME FOUND, IF OUR FRAME IS GREATER THAN THE CURRENTLY CHECKED
		//KEYFRAME ID, THEN INCREMENT UP THEM BADGERS
		if (((KEYJUMP*)curAnim->anim->typeData)->key[i]) 
			if (curAnim->frame > ((KEYJUMP*)curAnim->anim->typeData)->key[i]) 
			{
			  	if (GlobalAnimType & SCALE_ANIM)
				{
					mp.sinterpolate += submods;
				
					NextKey->sinterpolate += submods;	  
				}
				else
				{
					mp.interpolate += submods;
				
					NextKey->interpolate += submods;
				}
			}
	
	}



	FRAME = obj->slerpFrame;
  	CurrSlerp = FRAME * obj->slerpDelta;

	/*final check, if the number of frames is 1 then make sure quat we going to is this quat*/

	if (curAnim->anim->lastFrame == 1)
	{

		if (GlobalAnimType & SCALE_ANIM)
		{
			mp.sinterpolate = BASE.sinterpolate;
			NextKey->sinterpolate = mp.sinterpolate;
		}
		else
		{
			mp.interpolate = BASE.interpolate;
			NextKey->interpolate = mp.interpolate;
		}
	}
    return(mp);

}


static ANIMPOINT GetFrameBaseIntTween2(PNode *node,StratAnim *curAnim,Object *obj)
{
	int start,i;
	short submods;
	short maxkey;
	ANIMPOINT mp;

/*	tPrintf(2, 20, "FRAME = %d", curAnim->frame);*/


	/*GET THE ABSOLUTE START OF OUR ANIMATION*/
	start =((KEYJUMP*)curAnim->anim->typeData)->offset;
	submods = (((MODELANIM*)node->anim->anim->typeData)->numsubmods)-1;

	if (GlobalAnimType & SCALE_ANIM)
		mp.sinterpolate = (SINTERP*)(&((MODELANIM*)node->anim->anim->typeData)->animdata.sinterpolate[start]);
	else
		mp.interpolate = (INTERP*)(&((MODELANIM*)node->anim->anim->typeData)->animdata.interpolate[start]);

//	UTD
   	if (curAnim->frame)
   		curAnim->frame--;
	
	maxkey = ((KEYJUMP*)curAnim->anim->typeData)->numkeys + 1;
	
	/*Check through all our keys*/
	for (i=0;i<maxkey;i++)
	{

	
	
	   	/*IF THE CURRENT FRAME = THE NEXT KEYS FRAME, THEN WE ARE ON A KEYFRAME*/
		   
		if (curAnim->frame == ((KEYJUMP*)curAnim->anim->typeData)->key[i])
		{
			

			if (((KEYJUMP*)curAnim->anim->typeData)->key[i]) 
				if (GlobalAnimType & SCALE_ANIM)
					mp.sinterpolate += submods;
				else
					mp.interpolate += submods;

			break;
		}	

	

		if (((KEYJUMP*)curAnim->anim->typeData)->key[i]) 
			if (curAnim->frame > ((KEYJUMP*)curAnim->anim->typeData)->key[i]) 
				if (GlobalAnimType & SCALE_ANIM)
					mp.sinterpolate += submods;
				else
					mp.interpolate += submods;

	}

	
	
	
	
	/*CHECK TO SEE WHETHER THE FRAME WE ARE ON == A KEY FRAME*/
 
 /*	obj->slerpFrame++; */
 /*	obj->slerpFrame=0; */
 /*	FRAME++;*/
 
	//here
	FRAME = obj->slerpFrame; 
	CurrSlerp = FRAME * obj->slerpDelta;


	
    return(mp);
}

static ANIMPOINT GetFrameBaseIntTween(PNode *node,StratAnim *curAnim,Object *obj)
{
	int start,i;
	short submods;
	ANIMPOINT mp;


	/*GET THE ABSOLUTE START OF OUR ANIMATION*/
	start =((KEYJUMP*)curAnim->anim->typeData)->offset;
	submods = (((MODELANIM*)node->anim->anim->typeData)->numsubmods)-1;

	if (GlobalAnimType & SCALE_ANIM)
		mp.sinterpolate = (SINTERP*)(&((MODELANIM*)node->anim->anim->typeData)->animdata.sinterpolate[start]);
 	else
		mp.interpolate = (INTERP*)(&((MODELANIM*)node->anim->anim->typeData)->animdata.interpolate[start]);

	//INCREMENT THE FRAME INFORMATION
 	obj->slerpFrame++; 
	FRAME = obj->slerpFrame;
	CurrSlerp = FRAME * obj->slerpDelta;
	
    return(mp);
}

void SetTweenKey(PNode *node,Object *obj,StratAnim *curAnim)
{
	ANIMPOINT ip;
	ANIMPOINT NextKey;
	
	GlobalAnimType = node->anim->anim->type;
	
	if (curAnim->flags & ANIM_TWEENING)
	{
		/*run the tween anim to get new positions*/
		ip = GetFrameBaseIntTween2(node,curAnim,obj);
		RecurseUpdateAnimIntTween2(obj,ip);

		Test(obj);		


	}
	else
	{ 
	
	
		/* GET OUR CURRENT FRAME DETAILS*/
		ip = GetTweenQuats(node,curAnim,&NextKey,obj);
	 
		/*Now build up the base quat for each object that shall be used*/
		RecurseUpdateTweens(obj,ip,NextKey);
	   
	   //EL HACKO NEEDO MUCHO LOOKO AT WHY IT'S NEEDED WAREZ0, WHEN I HAVE MOMENTO TIMEEO
		// your Spanish is wicked mate
		ip = GetFrameBaseIntTween(node,curAnim,obj);
	   	RecurseUpdateAnimIntTween(obj,ip);

	}



}

static void UpdateModelAnim(PNode *node, StratAnim *curAnim, Object *obj)
{
	Matrix *mp;
	ANIMPOINT ip;
	ANIMPOINT NextKey;
	int KeyFrame=0;
	
	GlobalAnimType = node->anim->anim->type;

//	tPrintf(2, 20, "FRAME = %d", curAnim->frame);	 

					 			   
   	  //	if (((PadPush[0] & PADX)))
   	 	  	
	 	//	{


		if (curAnim->flags & ANIM_FLAG_PAUSED)
			return; 	 	   			  	 
   	  
	if (curAnim->flags & ANIM_FLAG_RUNNING)
	{

		if (GlobalAnimType & INTERPOLATE_ANIM)
		{

			if (curAnim->flags & ANIM_TWEENING)
 			{
 			   //	curAnim->flags &= (0xffffffff - ANIM_INVALID);
				
				/*GET THE START BASE*/
				ip = GetFrameBaseIntTween(node,curAnim,obj);
				RecurseUpdateAnimIntTween(obj,ip);
				 
   				if (CurrSlerp > (1.0f - (1.0f * TWEENTIMERCP))) 
				{

					/*WE HAVE MORPHED TO AND INCLUDING THE NEXT ANIM'S START KEY FRAME*/
				 	curAnim->flags &= (0xffffffff - ANIM_TWEENING);
				  	obj->slerpFrame = 0;
					curAnim->frame =0;
				}
			}
			else
			{
				/*GET THE START BASE, AND WHETHER OR NOT IT'S A KEY FRAME*/
				ip = GetFrameBaseInt(node,curAnim,&KeyFrame,&NextKey,obj);
				if (KeyFrame)
					RecurseUpdateAnimIntFirst(obj,ip);
			   	else 		  
			   		RecurseUpdateAnimInt(obj,ip,NextKey);

			}	
		}
		else
		{
			/*GET THE START BASE*/
			mp = GetFrameBase(node,curAnim);

			RecurseUpdateAnim(node,obj,mp);


		}


		if (!(curAnim->flags & ANIM_TWEENING))
			UpdateAnim(curAnim);  


	}  	 				 
//}
}

void rUpdateAnims (const PNode *node, StratAnim *curAnim, Object *objs)
{
	int anim;
	const AnimBlock *block;
  /*	Object *obj; */

	dAssert (node, "NULL node");
	dAssert (curAnim, "NULL current animation");

	block = node->anim;
	dAssert (block, "NULL animation block");
   /*	obj = node->obj;
	dAssert (obj, "NULL object"); */

	/*Check for Model Anim Type */

	if (curAnim->flags & MODEL_ANIM) {
		UpdateModelAnim(node,curAnim, objs);
		/* Reset the texture animations */
		for (anim = 0; anim < block->nAnims; ++anim)
		{
			/* Is it a texture animation, and not the currently
			 * playing anim? */
			if (block->anim[anim].type == ANIM_TEXTURE) {
				TexPaletteEntry *ent = (TexPaletteEntry *)block->anim[anim].typeData;			
				ent->desc = ent->animation->descArray[0];
			}
		}
	}
	else
	{
	
	
	
		/* Setup the texture animations */
		for (anim = 0; anim < block->nAnims; ++anim)
		{
			/* Is it a texture animation, and not the currently
			 * playing anim? */
			if (&block->anim[anim] != curAnim->anim && block->anim[anim].type == ANIM_TEXTURE) {
				TexPaletteEntry *ent = (TexPaletteEntry *)block->anim[anim].typeData;			
				ent->desc = ent->animation->descArray[0];
			}
		}

		/* Generic update stuff */
		if (curAnim->anim == NULL)
			return;

		UpdateAnim(curAnim);

		if (curAnim->anim->type == ANIM_TEXTURE)
		{
			TexPaletteEntry *ent = (TexPaletteEntry *)curAnim->anim->typeData;
			ent->desc = ent->animation->descArray[curAnim->frame];
		}

	}

}

/*
 * Starts an animation off
 */
void PlayAnimation (const PNode *node, StratAnim *anim, int offset)
{

	const AnimBlock *block;
	dAssert (node != NULL, "NULL block");
	dAssert (anim != NULL, "NULL anim");
   /* return;		*/
	block = node->anim;
  
	dAssert (block != NULL, "No animation table");

	/* Invalid anim? */
	if (offset <0 || offset >= block->nAnims) {
		dAssert (offset >= 0, "Invalid animation number");
		anim->anim = NULL;
		return;
	}

	/* Set up the animation */
	anim->anim		= &block->anim[offset];
	anim->frame		= anim->anim->firstFrame;
	anim->curDelay	= anim->anim->frameDelay;
	anim->flags		= ANIM_FLAG_RUNNING | ANIM_FLAG_TRIGGER;
	anim->multiplier= 1.f;
}

/*
 * Draw an object
 */
static rDrawObjectInternal (Object *obj)
{
	int child;

	mPush (&mCurMatrix);
	
	if (obj->model && obj->model->modelFlags & MODELFLAG_FACEON) {
		Matrix arse;
		mCurMatrix.m[0][0] = 1.f;	mCurMatrix.m[0][1] = 0.f;	mCurMatrix.m[0][2] = 0.f;
		mCurMatrix.m[1][0] = 0.f;	mCurMatrix.m[1][1] = 1.f;	mCurMatrix.m[1][2] = 0.f;
		mCurMatrix.m[2][0] = 0.f;	mCurMatrix.m[2][1] = 0.f;	mCurMatrix.m[2][2] = 1.f;
		arse = obj->m;
		mPostRotateXYZ (&arse, currentCamera->u.lookDir.rotX, currentCamera->u.lookDir.rotZ, -currentCamera->u.lookDir.rotY);
		/* Obj->m is up-to-date now */
		mPreMultiply (&mCurMatrix, &arse);
	} else {
		/* Obj->m is up-to-date now */
		mPreMultiply (&mCurMatrix, &obj->m);
	}

	/* Set up the specular colour */
	dummyContext.fOffsetColorRed	= lOffset.r;
	dummyContext.fOffsetColorGreen	= lOffset.g;
	dummyContext.fOffsetColorBlue	= lOffset.b;

	/* Draw the model, if any */
	if ((obj->model) && (!(obj->collFlag & OBJECT_INVISIBLE))) {
		Uint32 mF;
		float tr;
		mF = rGlobalModelFlags;
		tr = rGlobalTransparency;
		if (obj->transparency < 0.99f) {
			rGlobalTransparency *= obj->transparency;
			rGlobalModelFlags |= MODELFLAG_GLOBALTRANS;
		}
		if (obj->collFlag & OBJECT_SPECULARFLASH || rGlobalModelFlags & MODELFLAG_SPECULAR_FLASH) {
			rColToLV ((LightingValue *) &dummyContext.fOffsetColorAlpha, obj->specularCol.colour);
			dummyContext.fOffsetColorRed += lOffset.r;
			dummyContext.fOffsetColorGreen += lOffset.g;
			dummyContext.fOffsetColorBlue += lOffset.b;
			rGlobalModelFlags |= MODELFLAG_SPECULAR_FLASH;
		}
		// Any special overrides set?
		if (rGlobalModelFlags & ~MODELFLAG_NOCLIPPING) {
			rDrawModel (obj->model);
		} else {
			// No overrides - is this a simple model?
			if ((rGlobalModelFlags & MODELFLAG_NOCLIPPING) && (obj->model->modelFlags & MODELFLAG_CHEAPDRAW))
				rDrawModelSimple (obj->model);
			else
				rDrawModel (obj->model);
		}
		rGlobalModelFlags = mF;
		rGlobalTransparency = tr;
	}

	/* Deal with any children */
	for (child = 0; child < obj->no_child; ++child)
		rDrawObjectInternal (&obj->child[child]);

	mPop (&mCurMatrix);
}
/*
 * Matrix and boundary calculation
 */
static Uint32 RecurseCalcMatrixAndBounds (Object *obj, Point3 *low, Point3 *hi, Uint32 flags)
{
	int child;
	Uint32 retVal = flags;

   	if ((obj->collFlag & OBJECT_INVISIBLE) && (obj->no_child == 0))
   		return retVal;
	mPush (&mCurMatrix);

	/* Calculate the matrix, if necessary */
   	if (obj->collFlag & OBJECT_CALC_MATRIX) {
		mObjectMatrix (&obj->m, &obj->scale, (Point3 *)&obj->crntRot, &obj->crntPos);
   	} 

	mPreMultiply (&mCurMatrix, &obj->m);

	/* Update the boundary */
	if (!(obj->collFlag & OBJECT_INVISIBLE) && obj->model) {
		Point p;
		BBox *b = &obj->model->bounds;
		int i;
		mLoad (&mCurMatrix);

		for (i = 0; i < 8; ++i) {
			mPointXYZ (&p, 
				(i&4)?b->hi.x : b->low.x,
				(i&2)?b->hi.y : b->low.y,
				(i&1)?b->hi.z : b->low.z);
			if (p.x < low->x)
				low->x = p.x;
			if (p.x > hi->x)
				hi->x = p.x;
			
			if (p.y < low->y)
				low->y = p.y;
			if (p.y > hi->y)
				hi->y = p.y;
			
			if (p.z < low->z)
				low->z = p.z;
			if (p.z > hi->z)
				hi->z = p.z;
			
		}
		flags |= obj->model->modelFlags;
	}

	/* Deal with any children */
	for (child = 0; child < obj->no_child; ++child)
		flags |= RecurseCalcMatrixAndBounds (&obj->child[child], low, hi, flags);

	mPop (&mCurMatrix);

	return flags;
}
/*
 * Top-level draw routine
 */
void rDrawObject (Object *obj)
{
	int onScreen;
	Matrix objectToScreen;
	Uint32 flags, pushFlags, pushMask;
	Point3 lowPoint, hiPoint;
	Point3 p;
	float  dist, attenDist, pushTrans;

	/* Sanity check */
	dAssert (obj != NULL, "No Object!");

	// Ensure rGlobalTransp is set up if we're not already doing transparency
	if (!(rGlobalModelFlags & MODELFLAG_GLOBALTRANS))
		rGlobalTransparency = 1.f;

	/* Recursively calculate the low and hi points in object space */
	mPush (&mCurMatrix);
	mUnit (&mCurMatrix);
	lowPoint.x = lowPoint.y = lowPoint.z = 1000000.f;
	hiPoint.x = hiPoint.y = hiPoint.z = -1000000.f;
	flags = RecurseCalcMatrixAndBounds (obj, &lowPoint, &hiPoint, 0);
	mPop (&mCurMatrix);

	/* Calculate modelToScreen */
	mLoad (&mWorldToScreen);
	mMul (&objectToScreen, &mCurMatrix);

	onScreen = rOnScreen (&objectToScreen, &lowPoint, &hiPoint);
	if (onScreen == CLIP_OFFSCREEN)
		return;

	pushFlags = rGlobalModelFlags;
	pushMask = rGlobalModelFlagMask;
	pushTrans = rGlobalTransparency;

	// Get the distance to the object (ish)
	p.x = (lowPoint.x + hiPoint.x) * 0.5f;
	p.y = (lowPoint.y + hiPoint.y) * 0.5f;
	p.z = (lowPoint.z + hiPoint.z) * 0.5f;
	mPoint3Multiply3 (&p, &p, &mCurMatrix);

	// Set up the new lighting code
	if (!(rGlobalModelFlags & MODELFLAG_STATIC_LIGHT))
		lCalculate (&p);
	else
		lNoLighting ();
	
	v3Dec (&p, &currentCamera->pos);
	dist = v3SqrLength (&p);

	// Is the model getting near the farZ value?
	attenDist = SQR(currentCamera->farZ * 0.7f);
	if ((dist > attenDist) && (!(obj->collFlag & OBJECT_NOFADE))) {
		rGlobalModelFlags |= MODELFLAG_GLOBALTRANS;
		rGlobalTransparency *= 1.f - ((dist - attenDist) * (1.f/SQR(currentCamera->farZ * 0.3f)));
	}

	/* Is the model a long way away and in need of a shadow? */
	if ((flags & MODELFLAG_SHADOW_BBOX)) {
		extern STRAT *BossStrat;
		if (dist > SQR(LOW_SHAD_DIST) && (player[0].PlayerState != PS_ONPATH) && (BossStrat == NULL)) {
			BBox bounds;

			bounds.low = lowPoint;
			bounds.hi = hiPoint;

			rGlobalModelFlagMask &= ~MODELFLAG_SHADOW_BBOX;
			shBBox (&bounds);
		}
	} else if (obj->collFlag & OBJECT_STRAT_SHADOW && !Multiplayer) {
		// Give it an ellipsoid shadow
		BBox bounds;
		
		bounds.low = lowPoint;
		bounds.hi = hiPoint;
		
		shBBoxRound (&bounds);
	}

	if (onScreen == CLIP_NEARCLIPPED)
		rGlobalModelFlags &= ~MODELFLAG_NOCLIPPING;
	else
		rGlobalModelFlags |= MODELFLAG_NOCLIPPING;

	rDrawObjectInternal (obj);

	rGlobalTransparency = pushTrans;
	rGlobalModelFlags = pushFlags;
	rGlobalModelFlagMask = pushMask;
}

/*
 * Fix up an object, after loading in the relevant textures
 */
void rFixUpObject (Object *obj)
{
	dAssert (obj != NULL, "No object!");
	if (obj->model && obj->model->nMats) {
		int mat;
		for (mat = 0; mat < obj->model->nMats; ++mat) {
			rFixUpMaterial (&obj->model->material[mat], &objContext);
		}
	}
	if (obj->no_child) {
		int child;
		for (child = 0; child < obj->no_child; ++child)
			rFixUpObject (&obj->child[child]);
	}
}
