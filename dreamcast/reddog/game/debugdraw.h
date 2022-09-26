#ifndef _DEBUGDRAW_H
#define _DEBUGDRAW_H

#if DRAW_DEBUG


extern void savLine(float xs, float ys, float zs, float xe, float ye, float ze);
extern void DrawMyTriangle(Point3 *p1, Point3 *p2, Point3 *p3);
extern void DrawSquare(Point3 *tl, Point3 *br);


extern void DrawCollBBox(struct CollBBox *cb);

extern void DrawLine(Point3 *a, Point3 *b, Uint32 colour);
	
extern void DrawSubCollGrid(int x, int y, int	subX, int subY);
extern void DrawTriangle(Point3 *a, Point3 *b, Point3 *c);
extern void DrawMarker(float x, float y, float z);
extern void drawBox(Point3 *low, Point3 *hi, Matrix *m);
extern void ObjectBBoxDraw(Object *obj);
extern void stratObjectBBoxDraw(STRAT *strat);
extern void DrawPath(STRAT* strat);

//debug
extern void DrawNet(STRAT *strat);
extern void rDrawWayPoint(STRAT *strat);

extern void rDrawBBoxRec(Matrix *m, Object *obj);

extern void rDrawBBox(STRAT *strat);
extern void DrawSplinePath(struct waypath *path);
extern void DrawObjectCentre(STRAT *strat, int objn);
extern void DrawVector(Point3 *p, Vector3 *v);

#endif
#endif