#define SPLINE_ACC 0.001
#define SPLINE_TURN	0.3333f

extern int	GetTargetCount(int pn);

extern int	ObjectTowardPlayerXZOffset(STRAT *strat, int	objId, float angx, float angz, float MAXZANG, float MAXXANG, float offx, float offy, float offz);
extern int ObjectTowardPlayerXZ(STRAT *strat, int	objId, float angx, float angz, float MAXZANG, float MAXXANG);

extern int GetFreeCollisionSpace(STRAT *strat, float amount,int direction);
extern void ReturnToStartTransform(STRAT *strat,int objId);

extern void FaceAlongPath(STRAT *strat, int dir);
extern int MoveAlongPathXY(STRAT *strat, float amount) ;
extern void MoveAlongPath(STRAT *strat, float amount);
extern void TowardTargetPosXZ(STRAT *strat, int	objId, float ang, Point3 *TARGPOS);

typedef struct tag_HoldPlayerBlock
{
	STRAT *strat;
	Matrix m;
	struct tag_HoldPlayerBlock *next;
	struct tag_HoldPlayerBlock *prev;
}HoldPlayerBlock;

extern HoldPlayerBlock *hpbFirst;
extern HoldPlayerBlock *hpbLast;

extern void		SetCheckPosMyRotate(STRAT *strat, float ang);
