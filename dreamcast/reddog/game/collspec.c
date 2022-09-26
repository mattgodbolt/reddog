#include "RedDog.h"
#include "strat.h"
#include "coll.h"
#include "command.h"
#include "camera.h"
#include "specfx.h"
#define OLD_TANK_MODEL 0


float SCORPXMOVES[24] =
{
	.643,
	.643,
	.643,
	.643,
	.643,
	.643,
	1.042,
	1.041,
	1.042,
	1.042,
	.446,
	.446,
	.447,
	.446,
	.268,
	.268,
	.268,
	.268,
	.970,
	.969,
	.970,
	.970,
	.970,
	.970	

};



float CITYBOSSWALKMOVES[] =
{	//key 0 to 1
	(1.556 - -1.973) / 6.0,
	(1.556 - -1.973) / 6.0,
	(1.556 - -1.973) / 6.0,
	(1.556 - -1.973) / 6.0,
	(1.556 - -1.973) / 6.0,
	(1.556 - -1.973) / 6.0,
	//key 1 to 2
	(3.03 - 1.556) / 2.0,
	(3.03 - 1.556) / 2.0,
	//key 2 to 3
	(6.069 - 3.03) / 4.0,
	(6.069 - 3.03) / 4.0,
	(6.069 - 3.03) / 4.0,
	(6.069 - 3.03) / 4.0,
	//key 3 to 4
	(6.898 - 6.069) / 2.0,
 	(6.898 - 6.069) / 2.0,
 	//key 4 to 5
	(7.807 - 6.898) / 4.0,
	(7.807 - 6.898) / 4.0,
	(7.807 - 6.898) / 4.0,
	(7.807 - 6.898) / 4.0,
	//key 5 to 6
	(11.123 - 7.807) / 6.0,
	(11.123 - 7.807) / 6.0,
	(11.123 - 7.807) / 6.0,
	(11.123 - 7.807) / 6.0,
	(11.123 - 7.807) / 6.0,
	(11.123 - 7.807) / 6.0,
	//key 6 to 7
	(12.724 - 11.123) / 2.0,
	(12.724 - 11.123) / 2.0,
	//key 7 to 8
	(15.725 - 12.724) / 4.0,
	(15.725 - 12.724) / 4.0,
	(15.725 - 12.724) / 4.0,
	(15.725 - 12.724) / 4.0,
	//key 8 to 9
	(16.511 - 15.725) / 2.0,
	(16.511 - 15.725) / 2.0,
	//key 9 to 10
	(17.338 - 16.511) / 4.0,
	(17.338 - 16.511) / 4.0,
	(17.338 - 16.511) / 4.0,
  	(17.338 - 16.511) / 4.0

};

float CITYBOSSRUNMOVES[] =
{	//key 0 to 1
	(1.418 - -1.973) / 5.0,
	(1.418 - -1.973) / 5.0,
	(1.418 - -1.973) / 5.0,
	(1.418 - -1.973) / 5.0,
	(1.418 - -1.973) / 5.0,
	//key 1 to 2
	(3.736 - 1.418) / 2.0,
	(3.736 - 1.418) / 2.0,
	//key 2 to 3
	(6.272 - 3.736) / 2.0,
	(6.272 - 3.736) / 2.0,
	//key 3 to 4
	(7.54 - 6.272) / 5.0,
	(7.54 - 6.272) / 5.0,
	(7.54 - 6.272) / 5.0,
	(7.54 - 6.272) / 5.0,
	(7.54 - 6.272) / 5.0,
 	//key 4 to 5
	(10.62 - 7.54) / 5.0,
	(10.62 - 7.54) / 5.0,
	(10.62 - 7.54) / 5.0,
	(10.62 - 7.54) / 5.0,
	(10.62 - 7.54) / 5.0,
	//key 5 to 6
	(13.201 - 10.62) / 2.0,
	(13.201 - 10.62) / 2.0,
	//key 6 to 7
   	(15.626 - 13.201) / 3.0,
	(15.626 - 13.201) / 3.0,
	(15.626 - 13.201) / 3.0,
	//key 7 to 8
	(16.758 - 15.626) / 5.0,
	(16.758 - 15.626) / 5.0,
	(16.758 - 15.626) / 5.0,
	(16.758 - 15.626) / 5.0,
  	(16.758 - 15.626) / 5.0,

};

float CAVEBOSSWALKMOVES[] =
{	//key 0 to 1
	(2.764 - 0.364) / 6.0,
	(2.764 - 0.364) / 6.0,
	(2.764 - 0.364) / 6.0,
	(2.764 - 0.364) / 6.0,
	(2.764 - 0.364) / 6.0,
	(2.764 - 0.364) / 6.0,
	//key 1 to 2
	(3.761 - 2.764) / 3.0,
	(3.761 - 2.764) / 3.0,
	(3.761 - 2.764) / 3.0,
	//key 2 to 3
	(4.607 - 3.761) / 2.0,
	(4.607 - 3.761) / 2.0,
	//key 3 to 4
	(5.481 - 4.607) / 4.0,
	(5.481 - 4.607) / 4.0,
	(5.481 - 4.607) / 4.0,
	(5.481 - 4.607) / 4.0,
 	//key 4 to 5
	(6.069 - 5.481) / 2.0,
	(6.069 - 5.481) / 2.0,
	//key 5 to 6
	(8.465 - 6.069) / 7.0,
	(8.465 - 6.069) / 7.0,
	(8.465 - 6.069) / 7.0,
	(8.465 - 6.069) / 7.0,
	(8.465 - 6.069) / 7.0,
	(8.465 - 6.069) / 7.0,
	(8.465 - 6.069) / 7.0,
	//key 6 to 7
   	(9.385 - 8.465) / 3.0,
   	(9.385 - 8.465) / 3.0,
   	(9.385 - 8.465) / 3.0,
	//key 7 to 8
	(10.42 - 9.385) / 2.0,
	(10.42 - 9.385) / 2.0,
	//key 8 to 9
	(11.326 - 10.42) / 4.0,
	(11.326 - 10.42) / 4.0,
	(11.326 - 10.42) / 4.0,
	(11.326 - 10.42) / 4.0,
	//key 9 to 10
	(11.816 - 11.326) / 2.0,
	(11.816 - 11.326) / 2.0,




};

float PAL_CAVEBOSSWALKMOVES[] =
{	//key 0 to 1
	(2.712 - 0.364) / 5.0,
	(2.712 - 0.364) / 5.0,
	(2.712 - 0.364) / 5.0,
	(2.712 - 0.364) / 5.0,
	(2.712 - 0.364) / 5.0,
	//key 1 to 2
	(3.665 - 2.712) / 2.0,
	(3.665 - 2.712) / 2.0,
	//key 2 to 3
	(4.634 - 3.665) / 2.0,
	(4.634 - 3.665) / 2.0,
	//key 3 to 4
	(5.423 - 4.634) / 3.0,
	(5.423 - 4.634) / 3.0,
	(5.423 - 4.634) / 3.0,
 	//key 4 to 5
	(5.954 - 5.423) / 2.0,
	(5.954 - 5.423) / 2.0,
	//key 5 to 6
	(8.351 - 5.954) / 6.0,
	(8.351 - 5.954) / 6.0,
	(8.351 - 5.954) / 6.0,
	(8.351 - 5.954) / 6.0,
	(8.351 - 5.954) / 6.0,
	(8.351 - 5.954) / 6.0,
	//key 6 to 7
   	(9.235 - 8.351) / 2.0,
   	(9.235 - 8.351) / 2.0,
	//key 7 to 8
	(10.58 - 9.235) / 2.0,
  	(10.58 - 9.235) / 2.0,
	//key 8 to 9
	(11.49 - 10.58) / 3.0,
  	(11.49 - 10.58) / 3.0,
 	(11.49 - 10.58) / 3.0,
	//key 9 to 10
	(12.06 - 11.49) / 2.0,
   	(12.06 - 11.49) / 2.0,

};


float WALKERWALKMOVES[] =
{	//key 0 to 1
	(2.974 - 1.852) / 4.0,
	(2.974 - 1.852) / 4.0,
	(2.974 - 1.852) / 4.0,
	(2.974 - 1.852) / 4.0,
	//key 1 to 2
	(4.165 - 2.974) / 8.0,
	(4.165 - 2.974) / 8.0,
	(4.165 - 2.974) / 8.0,
	(4.165 - 2.974) / 8.0,
	(4.165 - 2.974) / 8.0,
	(4.165 - 2.974) / 8.0,
	(4.165 - 2.974) / 8.0,
	(4.165 - 2.974) / 8.0,
	//key 2 to 3
	(6.623 - 4.165) / 9.0,
	(6.623 - 4.165) / 9.0,
	(6.623 - 4.165) / 9.0,
	(6.623 - 4.165) / 9.0,
	(6.623 - 4.165) / 9.0,
	(6.623 - 4.165) / 9.0,
	(6.623 - 4.165) / 9.0,
	(6.623 - 4.165) / 9.0,
	(6.623 - 4.165) / 9.0,
	//key 3 to 4
	(7.936 - 6.623) / 5.0,
	(7.936 - 6.623) / 5.0,
	(7.936 - 6.623) / 5.0,
	(7.936 - 6.623) / 5.0,
	(7.936 - 6.623) / 5.0,
 	//key 4 to 5
	(9.411 - 7.936) / 7.0,
	(9.411 - 7.936) / 7.0,
	(9.411 - 7.936) / 7.0,
	(9.411 - 7.936) / 7.0,
	(9.411 - 7.936) / 7.0,
	(9.411 - 7.936) / 7.0,
	(9.411 - 7.936) / 7.0,
	//key 5 to 6
	(11.752 - 9.411) / 9.0,
	(11.752 - 9.411) / 9.0,
	(11.752 - 9.411) / 9.0,
	(11.752 - 9.411) / 9.0,
	(11.752 - 9.411) / 9.0,
	(11.752 - 9.411) / 9.0,
	(11.752 - 9.411) / 9.0,
	(11.752 - 9.411) / 9.0,
	(11.752 - 9.411) / 9.0,
};

float PAL_WALKERWALKMOVES[] =
{	//key 0 to 1
	(2.943 - 1.852) / 4.0,
	(2.943 - 1.852) / 4.0,
	(2.943 - 1.852) / 4.0,
	(2.943 - 1.852) / 4.0,
	//key 1 to 2
	(4.005 - 2.943) / 6.0,
	(4.005 - 2.943) / 6.0,
	(4.005 - 2.943) / 6.0,
	(4.005 - 2.943) / 6.0,
	(4.005 - 2.943) / 6.0,
	(4.005 - 2.943) / 6.0,
	//key 2 to 3
	(6.403 - 4.005) / 8.0,
	(6.403 - 4.005) / 8.0,
	(6.403 - 4.005) / 8.0,
	(6.403 - 4.005) / 8.0,
	(6.403 - 4.005) / 8.0,
	(6.403 - 4.005) / 8.0,
	(6.403 - 4.005) / 8.0,
	(6.403 - 4.005) / 8.0,
	//key 3 to 4
	(7.705 - 6.403) / 4.0,
	(7.705 - 6.403) / 4.0,
	(7.705 - 6.403) / 4.0,
	(7.705 - 6.403) / 4.0,
	//key 4 to 5
	(8.858 - 7.705) / 6.0,
	(8.858 - 7.705) / 6.0,
	(8.858 - 7.705) / 6.0,
	(8.858 - 7.705) / 6.0,
	(8.858 - 7.705) / 6.0,
	(8.858 - 7.705) / 6.0,
	//key 5 to 6
	(11.245 - 8.858) / 7.0,
	(11.245 - 8.858) / 7.0,
	(11.245 - 8.858) / 7.0,
	(11.245 - 8.858) / 7.0,
	(11.245 - 8.858) / 7.0,
	(11.245 - 8.858) / 7.0,
	(11.245 - 8.858) / 7.0,
};


float WALKERBACKWALKMOVES[] =
{	//key 0 to 1
	-(0.372 + 1.318) / 6.0,
	-(0.372 + 1.318) / 6.0,
	-(0.372 + 1.318) / 6.0,
	-(0.372 + 1.318) / 6.0,
	-(0.372 + 1.318) / 6.0,
	-(0.372 + 1.318) / 6.0,
	//key 1 to 2
	-(2.034 - 0.372) / 3.0,
	-(2.034 - 0.372) / 3.0,
	-(2.034 - 0.372) / 3.0,
	//key 2 to 3
	-(3.018 - 2.034) / 9.0,
	-(3.018 - 2.034) / 9.0,
	-(3.018 - 2.034) / 9.0,
	-(3.018 - 2.034) / 9.0,
	-(3.018 - 2.034) / 9.0,
	-(3.018 - 2.034) / 9.0,
	-(3.018 - 2.034) / 9.0,
	-(3.018 - 2.034) / 9.0,
	-(3.018 - 2.034) / 9.0,
	//key 3 to 4
	-(4.785 - 3.018) / 6.0,
	-(4.785 - 3.018) / 6.0,
	-(4.785 - 3.018) / 6.0,
	-(4.785 - 3.018) / 6.0,
	-(4.785 - 3.018) / 6.0,
	-(4.785 - 3.018) / 6.0,
 	//key 4 to 5
	-(6.505 - 4.785) / 3.0,
	-(6.505 - 4.785) / 3.0,
	-(6.505 - 4.785) / 3.0,
	//key 5 to 6
	-(7.517 - 6.505) / 9.0,
	-(7.517 - 6.505) / 9.0,
	-(7.517 - 6.505) / 9.0,
	-(7.517 - 6.505) / 9.0,
	-(7.517 - 6.505) / 9.0,
	-(7.517 - 6.505) / 9.0,
	-(7.517 - 6.505) / 9.0,
	-(7.517 - 6.505) / 9.0,
	-(7.517 - 6.505) / 9.0,
};

float PAL_WALKERBACKWALKMOVES[] =
{	//key 0 to 1
	-(0.473 + 1.318) / 5.0,
	-(0.473 + 1.318) / 5.0,
	-(0.473 + 1.318) / 5.0,
	-(0.473 + 1.318) / 5.0,
	-(0.473 + 1.318) / 5.0,
	//key 1 to 2
	-(2.213 - 0.473) / 3.0,
  	-(2.213 - 0.473) / 3.0,
   	-(2.213 - 0.473) / 3.0,
	//key 2 to 3
	-(3.251 - 2.213) / 7.0,
   	-(3.251 - 2.213) / 7.0,
 	-(3.251 - 2.213) / 7.0,
   	-(3.251 - 2.213) / 7.0,
   	-(3.251 - 2.213) / 7.0,
   	-(3.251 - 2.213) / 7.0,
   	-(3.251 - 2.213) / 7.0,
	//key 3 to 4
	-(4.854 - 3.251) / 5.0,
  	-(4.854 - 3.251) / 5.0,
  	-(4.854 - 3.251) / 5.0,
 	-(4.854 - 3.251) / 5.0,
  	-(4.854 - 3.251) / 5.0,
	//key 4 to 5
	-(6.357 - 4.854) / 3.0,
  	-(6.357 - 4.854) / 3.0,
   	-(6.357 - 4.854) / 3.0,
	//key 5 to 6
  	-(7.442 - 6.357) / 7.0,
	-(7.442 - 6.357) / 7.0,
	-(7.442 - 6.357) / 7.0,
	-(7.442 - 6.357) / 7.0,
	-(7.442 - 6.357) / 7.0,
	-(7.442 - 6.357) / 7.0,
	-(7.442 - 6.357) / 7.0,
};





float *AnimJumpNtsc[] =
{
	CITYBOSSWALKMOVES,
	CITYBOSSRUNMOVES,
	CAVEBOSSWALKMOVES,
	WALKERWALKMOVES,
	WALKERBACKWALKMOVES,
	SCORPXMOVES,
};

float *AnimJumpPal[] =
{
	CITYBOSSWALKMOVES,
	CITYBOSSRUNMOVES,
	PAL_CAVEBOSSWALKMOVES,
	PAL_WALKERWALKMOVES,
	PAL_WALKERBACKWALKMOVES,
	SCORPXMOVES,
};






#define CITYBOSSWALK 0
#define CITYBOSSRUN 1
#define CAVEBOSSWALK 2
#define WALKERWALK 3
#define WALKERBACKWALK 4
#define SCORPIONBOSSWALK 5






float AnimMoveOffset(STRAT *strat, int mode)
{
	float *address;
	if (((strat->anim) && (strat->anim->anim.anim)))
	{

		if (!(strat->anim->anim.flags & ANIM_TWEENING))
		{

			if (PAL)
				address = AnimJumpPal[mode];
			else
				address = AnimJumpNtsc[mode];
		
			
			return(address[strat->anim->anim.frame]);

			#if 0
			switch (mode)
			{
				case (CITYBOSSWALK):
					return(CITYBOSSWALKMOVES[strat->anim->anim.frame]);
					break;
				case (CITYBOSSRUN):
					return(CITYBOSSRUNMOVES[strat->anim->anim.frame]);
					break;
				case (CAVEBOSSWALK):
					return(CAVEBOSSWALKMOVES[strat->anim->anim.frame]);
					break;
				case (WALKERWALK):
					return(WALKERWALKMOVES[strat->anim->anim.frame]);
					break;
				case (WALKERBACKWALK):
					return(WALKERBACKWALKMOVES[strat->anim->anim.frame]);
					break;
				case (SCORPIONBOSSWALK):
					return(SCORPXMOVES[strat->anim->anim.frame]);
					break;
			}
			#endif

		}
	}

	return(0.0);

}



static void nonAnimCollFloor(STRAT *crntStrat, Object *obj, Vector3 *relMove, Vector3 *move, Point3 *collP)
{
	Point3	p;
	Vector3	force, B, b, tempv;
	float lambda, dot;

	AddToObjectWM(crntStrat, move);
	v3Inc(&crntStrat->pos, move);
	dAssert(crntStrat->pos.x == crntStrat->pos.x, "qnan");

	b = crntStrat->vel;
	B = *move;
	dot = v3Dot(&B, &B);

	if (dot > 0.001f) {
		
		lambda = -(B.x * b.x + B.y * b.y + B.z * b.z) / dot; 
		
		if (lambda < 1000.0f)
		{
			v3Scale(&B, &B, lambda);
			//	if (PAL)
			//		v3Scale(&B, &B, PAL_MOVESCALE);
			v3Inc(&crntStrat->vel, &B);
		}
	}
	crntStrat->flag |= COLL_ON_FLOOR;
}

static void nonAnimBounceCollFloor(STRAT *crntStrat, Object *obj, Vector3 *relMove, Vector3 *move, Point3 *collP)
{
	Point3	p;
	Vector3	force, B, b;
	float lambda;

	if (crntStrat->flag & STRAT_BULLET)
		crntStrat->pos = crntStrat->oldPos;
	else
	{
		v3Inc(&crntStrat->pos, move);
		b = crntStrat->vel;
		/*B = *move;
		lambda = -(B.x * b.x + B.y * b.y + B.z * b.z) / v3Dot(&B, &B); 

		if (lambda < 1000.0f)
		{
			v3Scale(&B, &B, lambda);
			v3Inc(&crntStrat->vel, &B);
		}*/
		force =*move;
		v3Normalise(&force);
		v3Normalise(&b);
		
		v3Scale(&force, &force, 1.5f * fabs(v3Dot(&force, &b)) * v3Length(&crntStrat->vel) * crntStrat->pnode->mass);

		//v3Scale(&force, move, crntStrat->pnode->mass * 10.0f);
		//v3Sub(&p, collP, &crntStrat->pos);		
		ApplyForce(crntStrat, &force, &p, crntStrat->pnode->collForceRatio);
	}
	crntStrat->flag |= COLL_ON_FLOOR;
}


static void BattleTank1_2CollFloor(STRAT *crntStrat, Object *obj, Vector3 *relMove, Vector3 *move, Point3 *collP)
{
	Point3	pos;
	Vector3	force;
	int		objId, i=1;

	while(crntStrat->objId[i])
	{
		if (obj == crntStrat->objId[i])
		{
			objId = i;
			break;
		}
		i++;
	}

	force = *move;
	force.z = 0.0f;
	v3Inc(&crntStrat->pos, &force);
	force.z = move->z;
	v3Normalise(&force);
	force.z = 0.0f;
	v3Scale(&force, &force, crntStrat->pnode->mass * 0.05f);
	pos.x = 0.0f;
	pos.y = 0.0f;
	pos.z = 0.0f;
	ApplyForce(crntStrat, &force, &pos, 1.0f);

	force.x = crntStrat->m.m[2][0];
	force.y = crntStrat->m.m[2][1];
	force.z = crntStrat->m.m[2][2];
	
	obj->crntPos.z += relMove->z;
	v3Scale(&force, &force, (obj->crntPos.z - obj->idlePos.z) * 100.0f);
	v3Sub(&pos, collP, &crntStrat->pos);
	ApplyForce(crntStrat, &force, &pos, crntStrat->pnode->collForceRatio * 0.5f);
}

/*static void MorterTankCollFloor(STRAT *crntStrat, Object *obj, Vector3 *relMove, Vector3 *move, Point3 *collP)
{
	Point3	pos;
	Vector3	force;
	int		objId, i=1;

	while(crntStrat->objId[i])
	{
		if (obj == crntStrat->objId[i])
		{
			objId = i;
			break;
		}
		i++;
	}

	force.x = crntStrat->m.m[2][0];
	force.y = crntStrat->m.m[2][1];
	force.z = crntStrat->m.m[2][2];
	
	obj->crntPos.z += relMove->z;
	v3Scale(&force, &force, (obj->crntPos.z - obj->idlePos.z) * 50.0f);
	v3Sub(&pos, collP, &crntStrat->pos);
	ApplyForce(crntStrat, &force, &pos, crntStrat->pnode->collForceRatio * 0.5f);
}*/

static void MultipleMissileLauncherCollFloor(STRAT *crntStrat, Object *obj, Vector3 *relMove, Vector3 *move, Point3 *collP)
{
	Point3	pos;
	Vector3	force;
	int		objId, i=1;

	while(crntStrat->objId[i])
	{
		if (obj == crntStrat->objId[i])
		{
			objId = i;
			break;
		}
		i++;
	}

	force.x = crntStrat->m.m[2][0];
	force.y = crntStrat->m.m[2][1];
	force.z = crntStrat->m.m[2][2];
	
	obj->crntPos.z += relMove->z;
	v3Scale(&force, &force, (obj->crntPos.z - obj->idlePos.z) * 50.0f);
	v3Sub(&pos, collP, &crntStrat->pos);
	ApplyForce(crntStrat, &force, &pos, crntStrat->pnode->collForceRatio * 0.5f);
}

static void WaterCollFloor(STRAT *crntStrat, Object *obj, Vector3 *relMove, Vector3 *move, Point3 *collP)
{
}

static void MobileHDHomingLauncherCollFloor(STRAT *crntStrat, Object *obj, Vector3 *relMove, Vector3 *move, Point3 *collP)
{
	float	rotAmount;
	Point3	pos;
	Vector3	force;

	rotAmount = relMove->z * 0.05f;
	force = *move;


	if (obj == crntStrat->objId[1])
		crntStrat->objId[2]->crntRot.y += rotAmount;
	else if (obj == crntStrat->objId[3])
		crntStrat->objId[4]->crntRot.y += rotAmount;
	else if (obj == crntStrat->objId[5])
		crntStrat->objId[6]->crntRot.y += rotAmount;
	else if (obj == crntStrat->objId[7])
		crntStrat->objId[8]->crntRot.y -= rotAmount;
	else if (obj == crntStrat->objId[9])
		crntStrat->objId[10]->crntRot.y -= rotAmount;
	else if (obj == crntStrat->objId[11])
		crntStrat->objId[12]->crntRot.y -= rotAmount;


	/*pos.x = obj->wm.m[3][0] - crntStrat->pos.x; */
	/*pos.y = obj->wm.m[3][1] - crntStrat->pos.y; */
	/*pos.z = obj->wm.m[3][2] - crntStrat->pos.z; */
	v3Sub(&pos, collP, &crntStrat->pos);
	v3Scale(&force, &force, crntStrat->pnode->mass * 0.03f); 
	ApplyForce(crntStrat, &force, &pos, crntStrat->pnode->collForceRatio);
}

static void A00Walker01CollFloor(STRAT *crntStrat, Object *obj, Vector3 *relMove, Vector3 *move, Point3 *collP)
{
#if 0
	Point3	p;
	Vector3	force;
	int		objId;

	v3Sub(&p, collP, &crntStrat->pos);		
	if (obj == crntStrat->objId[4])
		objId = 4;
	else if (obj == crntStrat->objId[6])
		objId = 6;
	else
	{
		v3Scale(&force, move, crntStrat->pnode->mass);
		ApplyForce(crntStrat, &force, &p, crntStrat->pnode->collForceRatio);
		return;
	}

	switch(objId)
	{
		case 4:
			crntStrat->objId[3]->crntRot.x -= relMove->z * 0.1f;
			crntStrat->objId[4]->crntRot.x += relMove->z * 0.1f;
			force = *move;
			v3Normalise(&force);
			v3Scale(&force, &force, (-crntStrat->objId[3]->crntRot.x + crntStrat->objId[3]->idleRot.x) * crntStrat->pnode->mass * 0.1f);
			ApplyForce(crntStrat, &force, NULL, crntStrat->pnode->collForceRatio);
			break;
		case 6:
			crntStrat->objId[5]->crntRot.x -= relMove->z * 0.1f;
			crntStrat->objId[6]->crntRot.x += relMove->z * 0.1f;
			force = *move;
			v3Normalise(&force);
			v3Scale(&force, &force, (-crntStrat->objId[5]->crntRot.x + crntStrat->objId[5]->idleRot.x) * crntStrat->pnode->mass * 0.1f);
			ApplyForce(crntStrat, &force, NULL, crntStrat->pnode->collForceRatio);
			break;
	}
#endif
}

static void MiniRapierCollFloor(STRAT *crntStrat, Object *obj, Vector3 *relMove, Vector3 *move, Point3 *collP)
{
	Point3	p;
	Vector3	force;
	int		objId;

	v3Sub(&p, collP, &crntStrat->pos);
	if (obj == crntStrat->objId[1])
		objId = 1;
	else if (obj == crntStrat->objId[2])
		objId = 2;
	else if (obj == crntStrat->objId[3])
		objId = 3;
	else if (obj == crntStrat->objId[4])
		objId = 4;
	else
	{
		v3Scale(&force, move, crntStrat->pnode->mass);
		ApplyForce(crntStrat, &force, &p, crntStrat->pnode->collForceRatio);
		return;
	}

	force.x = crntStrat->m.m[2][0];
	force.y = crntStrat->m.m[2][1];
	force.z = crntStrat->m.m[2][2];
	v3Normalise(&force);
	v3Scale(&force, &force, crntStrat->pnode->mass * 0.1f);
	ApplyForce(crntStrat, &force, &p, crntStrat->pnode->collForceRatio * 0.1f);
}

static void A00Walker02CollFloor(STRAT *crntStrat, Object *obj, Vector3 *relMove, Vector3 *move, Point3 *collP)
{
#if 0
	Point3	p;
	Vector3	force;
	int		objId;

	v3Sub(&p, collP, &crntStrat->pos);		

	if (obj == crntStrat->objId[1])
		objId = 1;
	else if (obj == crntStrat->objId[2])
		objId = 2;
	else
	{
		v3Scale(&force, move, crntStrat->pnode->mass);
		ApplyForce(crntStrat, &force, &p, crntStrat->pnode->collForceRatio);
		return;
	}

	switch(objId)
	{
		case 1:
			crntStrat->objId[3]->crntRot.y += relMove->z * 0.05f;
			force = *move;
			v3Normalise(&force);
			v3Scale(&force, &force, (-crntStrat->objId[3]->crntRot.y + crntStrat->objId[3]->idleRot.y) * crntStrat->pnode->mass * -0.2f);
			ApplyForce(crntStrat, &force, &p, crntStrat->pnode->collForceRatio);
			break;
		case 2:
			crntStrat->objId[4]->crntRot.y -= relMove->z * 0.05f;
			force = *move;
			v3Normalise(&force);
			v3Scale(&force, &force, (-crntStrat->objId[4]->crntRot.y + crntStrat->objId[4]->idleRot.y) * crntStrat->pnode->mass * 0.2f);
			ApplyForce(crntStrat, &force, &p, crntStrat->pnode->collForceRatio);
			break;
	}
#endif
}

static void OnCameraCollFloor(STRAT *crntStrat, Object *obj, Vector3 *relMove, Vector3 *move, Point3 *collP)
{
}

#if 0
static void moveRedDogCollFloor(STRAT *crntStrat, Object *obj, Vector3 *relMove, Vector3 *move, Point3 *collP)
{
	Point3	p;
	Vector3	force;

	v3Scale(&force, move, crntStrat->pnode->mass * 0.1f);
	v3Sub(&p, collP, &crntStrat->pos);		
	ApplyForce(crntStrat, &force, &p, crntStrat->pnode->collForceRatio);
}
#endif

static void EscortTankerCollFloor(STRAT *crntStrat, Object *obj, Vector3 *relMove, Vector3 *move, Point3 *collP)
{
	Point3	p;
	Vector3	force, r;
	Matrix im;


	v3Sub(&p, collP, &crntStrat->pos);
	//mPoint3Apply3(&p, &crntStrat->m);
	v3Scale(&force, move, crntStrat->pnode->mass * 0.02f);
	
	force.x = force.y = 0.0f;
	ApplyForce(crntStrat, &force, &p, crntStrat->pnode->collForceRatio * 0.05f);
}

static void CameraCollFloor(STRAT *crntStrat, Object *obj, Vector3 *relMove, Vector3 *move, Point3 *collP)
{
	Point3	p;
	Vector3	force, B, b;
	float lambda;

	if (crntStrat->flag & STRAT_BULLET)
		crntStrat->pos = crntStrat->oldPos;
	else
	{
		v3Scale(&force, move, crntStrat->pnode->mass * 0.5f);
		v3Sub(&p, collP, &crntStrat->pos);		
		ApplyForce(crntStrat, &force, &p, crntStrat->pnode->collForceRatio);
	}
	crntStrat->flag |= COLL_ON_FLOOR;
}

/*
static void GroundSpeeder1CollFloor(STRAT *crntStrat, Object *obj, Vector3 *relMove, Vector3 *move, Point3 *collP)
{
	Point3	p;
	Vector3	force, r;
	Matrix im;

	v3Sub(&p, collP, &crntStrat->pos);		

	if (obj == crntStrat->objId[1])
	{
		mPoint3Apply3(&p, &crntStrat->m);
		v3Scale(&force, move, crntStrat->pnode->mass * 0.1f);
	}
	else if (obj == crntStrat->objId[2])
	{	mPoint3Apply3(&p, &crntStrat->m);
		v3Scale(&force, move, crntStrat->pnode->mass * 0.1f);
	}
	else if (obj == crntStrat->objId[3])
	{	mPoint3Apply3(&p, &crntStrat->m);
		v3Scale(&force, move, crntStrat->pnode->mass * 0.1f);
	}
	else	
	{
		dAssert(0, "should not happen");
	}
	
	//force.x = force.y = 0.0f;
	ApplyForce(crntStrat, &force, &p, crntStrat->pnode->collForceRatio * 0.005f);
}

static void GroundSpeeder2CollFloor(STRAT *crntStrat, Object *obj, Vector3 *relMove, Vector3 *move, Point3 *collP)
{
	Point3	p;
	Vector3	force, r, up;
	Matrix im;
	float tempf;

	up.x = 0.0f;
	up.y = 0.0f;
	up.z = 1.0f;
	v3Sub(&p, collP, &crntStrat->pos);		
	//mPoint3Apply3(&p, &crntStrat->m);

	if (obj == crntStrat->objId[1])
	{
		obj->crntPos.z += move->z;
		tempf = obj->crntPos.z - obj->idlePos.z;
		v3Scale(&up, &up, tempf);
		v3Scale(&force, &up, crntStrat->pnode->mass * 0.1f);
	}
	else if (obj == crntStrat->objId[2])
	{
		obj->crntPos.z += move->z;
		tempf = obj->crntPos.z - obj->idlePos.z;
		v3Scale(&up, &up, tempf);
		v3Scale(&force, &up, crntStrat->pnode->mass * 0.1f);

	}
	else if (obj == crntStrat->objId[3])
	{
		obj->crntPos.z += move->z;
		tempf = obj->crntPos.z - obj->idlePos.z;
		v3Scale(&up, &up, tempf);
		v3Scale(&force, &up, crntStrat->pnode->mass * 0.05f);
	}
	else	
	{
		dAssert(0, "should not happen");
	}
	
	//force.x = force.y = 0.0f;
	ApplyForce(crntStrat, &force, &p, crntStrat->pnode->collForceRatio * 0.5f);
}

static void GroundSpeeder3CollFloor(STRAT *crntStrat, Object *obj, Vector3 *relMove, Vector3 *move, Point3 *collP)
{
	Point3	p;
	Vector3	force, r, up;
	Matrix im;
	float tempf;

	up.x = 0.0f;
	up.y = 0.0f;
	up.z = 1.0f;
	v3Sub(&p, collP, &crntStrat->pos);	

	if (obj == crntStrat->objId[1])
	{
		obj->crntPos.z += move->z;
		tempf = obj->crntPos.z - obj->idlePos.z;
		v3Scale(&up, &up, tempf);
		v3Scale(&force, &up, crntStrat->pnode->mass * 0.025f);
	}
	else if (obj == crntStrat->objId[2])
	{
		obj->crntPos.z += move->z;
		tempf = obj->crntPos.z - obj->idlePos.z;
		v3Scale(&up, &up, tempf);
		v3Scale(&force, &up, crntStrat->pnode->mass * 0.025f);

	}
	else if (obj == crntStrat->objId[3])
	{
		obj->crntPos.z += move->z;
		tempf = obj->crntPos.z - obj->idlePos.z;
		v3Scale(&up, &up, tempf);
		v3Scale(&force, &up, crntStrat->pnode->mass * 0.025f);
	}
	else	
	{
		dAssert(0, "should not happen");
	}
	
	//force.x = force.y = 0.0f;
	ApplyForce(crntStrat, &force, &p, crntStrat->pnode->collForceRatio * 0.5f);
}
*/

static void RedDogCollFloor(STRAT *strat, Object *obj, Vector3 *relMove, Vector3 *move, Point3 *collP)
{
	Vector3	force, tempv, fv, B;
	Point3	pos, b, p;
	int	objId=0, i, pn;
	int camno;
	Matrix tempm;
	float	lambda;

	
	pn = strat->Player->n;

	/*if (GET_SURF_TYPE(strat->polyType) == WATERSLIDEWATER)
	{
		player[0].Player->health -= 1.0f;
		return;
	}*/

	if (obj == strat->objId[1])
		objId = 1;
	else if(obj == strat->objId[2])
		objId = 2;
	else if(obj == strat->objId[3])
		objId = 3;
	else if(obj == strat->objId[4])
		objId = 4;
	else if(obj == strat->objId[5])
		objId = 5;
	else if(obj == strat->objId[6])
		objId = 6;
	else if(obj == strat->objId[13])
		objId = 13;

	if ((objId > 0) && (objId < 5))
	{
		fv.x = player[pn].Player->m.m[2][0];
		fv.y = player[pn].Player->m.m[2][1];
		fv.z = player[pn].Player->m.m[2][2];
		if (v3Dot(move, &fv) < 0.0f)
		{
			collP->x = player[pn].Player->objId[objId]->wm.m[3][0];
			collP->y = player[pn].Player->objId[objId]->wm.m[3][1];
			collP->z = player[pn].Player->objId[objId]->wm.m[3][2];
		}
		pos.x = obj->wm.m[3][0];
		pos.y = obj->wm.m[3][1];
		pos.z = obj->wm.m[3][2];
		v3Dec(&pos, &player[pn].Player->pos);
		if (relMove->z > 0.0f)
		{ 
			if (obj->crntPos.z  + relMove->z< 0.1f)
			{
				if (GET_SURF_TYPE(player[pn].Player->polyType) == SAND)
					obj->crntPos.z += relMove->z + RandRange(-0.1f, 0.1f) * player[pn].Player->relVel.y * 2.0f;
				else
				{
					obj->crntPos.z += relMove->z;
				}
			}
			else
				obj->crntPos.z = 0.1f;
		}
		force.x = force.y = 0.0f;
		force.z = obj->crntPos.z - obj->idlePos.z;
		mVector3Apply(&force, &strat->m);
		switch(objId)
		{
			case 1:
				player[pn].rdWheel[0].p = GlobalCollP;
				player[pn].rdWheel[0].n = GlobalCollPolyN;
				player[pn].rdWheel[0].mat = GlobalPolyType;
				v3Scale(&force, &force, 50.0f); 
				break;
			case 2:
				player[pn].rdWheel[1].p = GlobalCollP;
				player[pn].rdWheel[1].n = GlobalCollPolyN;
				player[pn].rdWheel[1].mat = GlobalPolyType;
				v3Scale(&force, &force, 50.0f); 
				break;
			case 3:
				player[pn].rdWheel[2].p = GlobalCollP;
				player[pn].rdWheel[2].n = GlobalCollPolyN;
				player[pn].rdWheel[2].mat = GlobalPolyType;
				v3Scale(&force, &force, 50.0f); 
				break;
			case 4:
				player[pn].rdWheel[3].p = GlobalCollP;
				player[pn].rdWheel[3].n = GlobalCollPolyN;
				player[pn].rdWheel[3].mat = GlobalPolyType;
				v3Scale(&force, &force, 50.0f); 
				break;
		}
	  	if (PAL)
	  		v3Scale(&force,&force,PAL_ACCSCALE);
		ApplyForce(player[pn].Player, &force, &pos, strat->pnode->collForceRatio * 0.1f);
		strat->flag |= COLL_ON_FLOOR;
	}
	else
	{
		v3Sub(&pos, collP, &player[pn].Player->pos);
		v3Inc(&player[pn].Player->pos, move);
		b = player[pn].Player->vel;
		B = *move;
		lambda = -(B.x * b.x + B.y * b.y + B.z * b.z) / v3Dot(&B, &B); 

		if ((lambda < 1000.0f) && (lambda > 0.0f))
		{
			v3Scale(&B, &B, lambda);
		   //	if (PAL)
		   //		v3Scale(&B, &B, PAL_MOVESCALE);
			v3Inc(&player[pn].Player->vel, &B);
			
			if (player[pn].Player->m.m[2][2] < 0.2f)
			{
				v3Scale(&force, &player[pn].Player->vel, -strat->pnode->mass * 0.01f); 
				if ((v3Length(&force) < 5.0f) && (v3Length(&force) > 0.0f))
				{
					v3Normalise(&force);
					v3Scale(&force, &force, 5.0f);
				}
			   	if (PAL)
			   		v3Scale(&force,&force,PAL_ACCSCALE);
				ApplyForce(player[pn].Player, &force, &pos, strat->pnode->collForceRatio * 0.01f);
			}
		}
	}

	if (GET_SURF_TYPE(player[pn].Player->polyType) != SPIDERMAN)
	{
		if (objId > 4)
		{
			Flatten(strat, 3.0);
		}
	}
}

static void BattleTank2CollFloor(STRAT *crntStrat, Object *obj, Vector3 *relMove, Vector3 *move, Point3 *collP)
{
	Point3	pos;
	Vector3	force;
	int		objId, i=1;

	while(crntStrat->objId[i])
	{
		if (obj == crntStrat->objId[i])
		{
			objId = i;
			break;
		}
		i++;
	}

	
/*	force = *move;
	force.z = 0.0f;
	v3Inc(&crntStrat->pos, &force);
	force.z = move->z;
	v3Normalise(&force);
	force.z = 0.0f;
	v3Scale(&force, &force, crntStrat->pnode->mass * 0.05f);
	pos.x = 0.0f;
	pos.y = 0.0f;
	pos.z = 0.0f;
	ApplyForce(crntStrat, &force, &pos, 1.0f);
*/
	force.x = crntStrat->m.m[2][0];
	force.y = crntStrat->m.m[2][1];
	force.z = crntStrat->m.m[2][2];
	
	obj->crntPos.z += relMove->z;
	v3Scale(&force, &force, (obj->crntPos.z - obj->idlePos.z) * 200.0f);
	v3Sub(&pos, collP, &crntStrat->pos);
	ApplyForce(crntStrat, &force, &pos, crntStrat->pnode->collForceRatio * 0.5f);
}

static void SecondaryGunCollFloor(STRAT *crntStrat, Object *obj, Vector3 *relMove, Vector3 *move, Point3 *collP)
{
	Point3	p;
	Vector3	force, B, b;
	float lambda;

	//v3Scale(move, move, 0.9f);
	v3Inc(&crntStrat->pos, move);
	b = crntStrat->vel;
	B = *move;
	lambda = -(B.x * b.x + B.y * b.y + B.z * b.z) / v3Dot(&B, &B); 

	if (lambda < 1000.0f)
	{
		v3Scale(&B, &B, lambda);
		v3Inc(&crntStrat->vel, &B);
	}
	v3Scale(&force, move, crntStrat->pnode->mass * 0.1f);
	v3Sub(&p, collP, &crntStrat->pos);		
	ApplyForce(crntStrat, &force, &p, crntStrat->pnode->collForceRatio);
	crntStrat->flag |= COLL_ON_FLOOR;
}

/*
static void OutPostBossCollFloor(STRAT *crntStrat, Object *obj, Vector3 *relMove, Vector3 *move, Point3 *collP)
{
	Point3	p;
	Vector3	force, B, b;
	float lambda;


}*/

static void CanyonBossCollFloor(STRAT *crntStrat, Object *obj, Vector3 *relMove, Vector3 *move, Point3 *collP)
{
	Point3	p;
	Vector3	force, B, b;
	int id;

	v3Scale(&force, move, crntStrat->pnode->mass * 0.02f);
	p.x = p.y = p.z = 0.0f;
	ApplyForce(crntStrat, &force, &p, crntStrat->pnode->collForceRatio);

#if 0
	id = OBJECT_GET_ID(obj->collFlag);

	if (id < 5)
	{
		//if (obj->idlePos.x < 5.0f)
			//obj->idlePos.x += 0.5f;
	}
	else
	{
		//addSpark(100, collP, &force, 0.8, 5.0f, 100, 1);
	}
#endif
}
/*
static void RaftCollFloor(STRAT *crntStrat, Object *obj, Vector3 *relMove, Vector3 *move, Point3 *collP)
{
	Point3	p;
	Vector3	force, B, b, tempv;
	float lambda;

	if ((GET_SURF_TYPE(crntStrat->polyType) == WATERSLIDEWATER) && (OBJECT_GET_ID(obj->collFlag) == 1))
	{
		v3Scale(&force, move, crntStrat->pnode->mass * 0.05f);
		v3Sub(&p, collP, &crntStrat->pos);
		crntStrat->vel.z *= 0.95f;
		ApplyForce(crntStrat, &force, &p, crntStrat->pnode->collForceRatio);
	}
	else if(OBJECT_GET_ID(obj->collFlag) == 2)
	{
		p.x = p.y = p.z = 0.0f;
		force =*move;
		v3Scale(&force, &force, 0.95f);
		v3Inc(&crntStrat->pos, &force);
		v3Inc(&player[0].Player->pos, &force);
		b = crntStrat->vel;
		
		force.z = 0.0f;
		v3Normalise(&force);
		v3Normalise(&b);
		v3Neg(&b);
		v3Scale(&force, &force, v3Dot(&b, &force));
		v3Scale(&force, &force, v3Length(&crntStrat->vel));
		v3Inc(&crntStrat->vel, &force);
		
		
	}
	crntStrat->flag |= COLL_ON_FLOOR;
}
*/
#if 0
static void FSLevel2DropshipCollFloor(STRAT *crntStrat, Object *obj, Vector3 *relMove, Vector3 *move, Point3 *collP)
{

	Point3	p;
	Vector3	force, B, b, tempv;
	float lambda;


	p = *collP;
	v3Dec(&p,&crntStrat->pos);
	force =*move;
	force.x = force.y = 0.0f;	 //was 2
	v3Scale(&force, &force, 0.1f * crntStrat->pnode->mass);
   //	if (fabs(crntStrat->vel.z ) > 0.05f)
		crntStrat->vel.z *= 0.95f;
	crntStrat->vel.x = crntStrat->vel.y = 0.0f;

	ApplyForce(crntStrat, &force, &p, 0.03f * crntStrat->pnode->collForceRatio);
		
   	crntStrat->rot_speed *= 0.79f;

 }
#endif

static void PresidentsCarCollFloor(STRAT *crntStrat, Object *obj, Vector3 *relMove, Vector3 *move, Point3 *collP)
{
	Point3	p;
	Vector3	force, B, b, tempv;
	float lambda, tempf;
	int i;

	p = *collP;
	v3Dec(&p,&crntStrat->pos);
	obj->crntPos.z += relMove->z;

	tempf = obj->crntPos.z - obj->idlePos.z;
	tempv.x = tempv.y = 0.0f;
	tempv.z = 1.0f;
	if (PAL)
		v3Scale(&tempv, &tempv, crntStrat->pnode->mass * tempf * 0.03f * PAL_MOVESCALE);
	else
		v3Scale(&tempv, &tempv, crntStrat->pnode->mass * tempf * 0.03f);
	ApplyForce(crntStrat, &tempv, &p, 0.1f * crntStrat->pnode->collForceRatio);


	crntStrat->flag |= COLL_ON_FLOOR;
}


void *collSpecProc[]=
{
	&nonAnimCollFloor,						//0
	&RedDogCollFloor,						//1
	&nonAnimCollFloor,						//2
	&MobileHDHomingLauncherCollFloor,		//3
	&WaterCollFloor,						//4
	&BattleTank1_2CollFloor,				//5
	&MiniRapierCollFloor,					//6
	&A00Walker01CollFloor,					//7
	&nonAnimCollFloor,						//8
	&EscortTankerCollFloor,					//9
	&CameraCollFloor,						//10
	&nonAnimCollFloor,						//11
	&RedDogCollFloor,						//12
	&nonAnimCollFloor,						//13
	&nonAnimCollFloor		,				//14
	&MultipleMissileLauncherCollFloor,		//15
	&BattleTank2CollFloor,					//16
	&nonAnimBounceCollFloor,				//17
	&SecondaryGunCollFloor,					//18
	&nonAnimCollFloor,						//19
	&CanyonBossCollFloor,					//20
	&nonAnimCollFloor,						//21
	&nonAnimBounceCollFloor,						//22 FSDROPSHIP LEVEL2
 
   //	&FSLevel2DropshipCollFloor,				//22
	&PresidentsCarCollFloor					//23
};

