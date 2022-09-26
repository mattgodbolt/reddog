#define FPS	60.0f

#define NTSC_GRAVITY 0.0109f
#define PAL_GRAVITY NTSC_GRAVITY * PAL_ACCSCALE


//#define PAL_GRAVITY 0.0157f
extern void ProcessStrat(STRAT *crntStrat);
extern void ResetOCP(STRAT *strat);
extern void MoveAllStrats(void);
extern float GetObjectCrntScale(STRAT *strat, int objId, int mode);
extern float GetObjectAnimScale(STRAT *strat, int objId, int mode);
extern float Gravity;
extern float TGravity;
