//void CamMoveTo(Point3 *p, int pn);
//void CamLookAt(Point3 *p, int pn);
//extern void CamModeChange();
//extern void  LookPlayer();
//extern void findCamLookAt(Vector3 *pos, Vector3	*point, int pn); 
//extern void findCamPos(Vector3	*pos, Vector3 *v, int pn);
//extern void findTarget(Point3 *p, int pn);
void doCam(int pn);
//void doCameraShield(int pn);
void doTarget(int pn);
void resetCamera(int pn);
void addToTargetArray(STRAT *strat, Object *obj, int pn);
//extern void resetPlayerVariables(int pn);
//extern int	findObjId(STRAT *strat, Object *obj);


#define CAMERANORMALVAR 0.0f


#define CAMERASCRIPTVAR 1.0f


extern PNode	*targetTLSpriteHolder;
extern PNode	*targetTRSpriteHolder;
extern PNode	*targetBRSpriteHolder;
extern PNode	*targetBLSpriteHolder;
extern PNode	*targetSpriteHolder;
extern float wideAmt;
/*matt add*/
