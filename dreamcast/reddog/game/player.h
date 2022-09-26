extern	void RedDogProcessNormal(int pn);
extern	void RedDogProcessTowerLift(int pn);
extern	void RedDogProcessWaterSlide(int pn);
extern	void RedDogProcessEntry(int pn);
extern	void RedDogProcessNoControl(int pn);


typedef struct
{
	Point3	pos;
	float	radius;
}JPOINT;

extern int		noJumpPoints;
extern JPOINT   jumpPoint[32];
