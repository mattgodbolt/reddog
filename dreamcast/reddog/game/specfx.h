#include "RedDog.h"

typedef struct tagWaterCP
{
	Point3		pos;
}WaterCP;




extern WaterCP	water[64];
extern int	cWaterCP, noWaterCP;

extern void rResetSplash (void);
extern void ClearSplashList(void);
extern void SpawnSplash(void);
extern	void			ClearSplash(void);
extern	void			SetNextSplash(void);
extern	int				Splash(void);
extern	void			GetSplashPos(STRAT *strat);
extern	int				LastSplash(void);