/*  in the following defines "a" is a (STRAT *) and "b" is  */
/*	an (Angle) */

#define SIDESTEP_WAIT 0
#define SKID_FRAMES 16



#define StratTiltForward(a, b) mPreRotateX(a->m, b);
#define StratTiltBackward(a, b) mPreRotateX(a->m, -b);
#define StratTiltLeft(a, b) mPreRotateY(a->m, b);
#define StratTiltRight(a, b) mPreRotateY(a->m, -b);
#define StratTurnLeft(a, b) mPreRotateZ(a->m, b);
#define StratTurnRight(a, b) mPreRotateZ(a->m, -b);
extern int ValidToBeam(STRAT *strat, float x, float BeamLength, float z, int mode);
extern int obscured(Point3 *p1, Point3 *p2, Point3 *p, int pn, STRAT *strat);

extern	int	findHoldPlayerNumber(STRAT *strat);

extern void objectToWorld(STRAT *strat);
extern void pointFromWorldToObject(Point3	*objP, Point3	*worldP, STRAT *strat, Object *obj);
extern void ApplyForce(STRAT *crnt_strat, Vector3 *v, Point3 *p, float ratio);
extern void combineRotation(STRAT *crntStrat, Vector3 *v2, float speed2);
extern void pointObjectTo(STRAT *crntStrat, int objId, Point3 *p);
extern void stratMakeRotMatrix(STRAT *crntStrat);

extern void pointToXZ(STRAT *crntStrat, Point3 *p);

/*FILES FOR STRATS SHOULD BE IN STRAT.C */
extern void pointToXY(STRAT *crntStrat, Point3 *p);
extern void InitValidAreaArray();
extern void DeleteValidAreaEntry(STRAT *strat);
extern void SetPosRelOffsetPlayerObject(STRAT *strat, int pn, int objId, float x, float y, float z);
extern void SetCheckPosRelOffsetPlayerObject(STRAT *strat, int objId, float x, float y, float z);
extern SPARK *addSpark(int n, Point3	*pos, Vector3 *dir, float speed, float spread, int time, int type, int icol);
typedef struct
{
	float	radius;
	STRAT	*strat;
}GroupCheckPos;

