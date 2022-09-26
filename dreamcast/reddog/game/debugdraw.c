#include "reddog.h"
#if DRAW_DEBUG
#include "debugdraw.h"
#include "strat.h"
#include "coll.h"
#include "render\animate.h" 
#include "render\internal.h" 
#include "process.h"
#include "draw.h"
#include "view.h"
#include "camera.h"

			  
#define ROOT2 1.414213562f

STRAT *BBoxStrat;

void IAmBBox(STRAT *strat)
{
	BBoxStrat = strat;
	
}




extern Map *Savmap;


void savLine(float xs, float ys, float zs, float xe, float ye, float ze)
{
	Point3	start, end;
	Colour	startCol, endCol;

	mPush(&mCurMatrix);
	mUnit(&mCurMatrix);
	start.x = xs;
	start.y = ys;
	start.z = zs;
	end.x = xe;
	end.y = ye;
	end.z = ze;
	startCol.argb.a = 0xff;
	startCol.argb.r = 0xff;
	startCol.argb.g = 0xff;
	startCol.argb.b = 0xff;
	endCol.argb.a = 0xff;
	endCol.argb.r = 0xff;
	endCol.argb.g = 0xff;
	endCol.argb.b = 0xff;

	rLineOnTop(&start, &end, startCol, endCol);
	mPop(&mCurMatrix);
}

void DrawMyTriangle(Point3 *p1, Point3 *p2, Point3 *p3)
{
	DrawLine(p1, p2, 0xffffffff);
	DrawLine(p2, p3, 0xffffffff);
	DrawLine(p3, p1, 0xffffffff);
}

void DrawSquare(Point3 *tl, Point3 *br)
{
	Point3	p;

	p.x = br->x;
	p.y = tl->y;
	p.z = tl->z;
	DrawLine(tl, &p, 0xffffffff);
	DrawLine(&p, br, 0xffffffff);
	p.x = tl->x;
	p.y = br->y;
	p.z = tl->z;
	DrawLine(br, &p, 0xffffffff);
	DrawLine(&p, tl, 0xffffffff);
}

void DrawCollBBox(struct CollBBox *cb)
{
	Point3	p1, p2;

#if 0
	p1.z = p2.z = player[currentPlayer].Player->pos.z;

	p1.x = cb->l;
	p1.y = cb->t;
	p2.x = cb->r;
	p2.y = cb->b;
	DrawSquare(&p1, &p2);
#endif
}



	
void DrawSubCollGrid(int x, int y, int	subX, int subY)
{
	Point3	p1, p2, p3, p4;

	p1.x = (Savmap->xMin + x * Savmap->xScale) + (subX * (Savmap->xScale / 4.0f));
	p1.y = (Savmap->yMin + y * Savmap->yScale) + (subY * (Savmap->yScale / 4.0f));
	p1.z = player[currentPlayer].Player->pos.z;
	p2.x = (Savmap->xMin + x * Savmap->xScale) + ((subX+1) * (Savmap->xScale / 4.0f));
	p2.y = (Savmap->yMin + y * Savmap->yScale) + (subY * (Savmap->yScale / 4.0f));
	p2.z = player[currentPlayer].Player->pos.z;
	p3.x = (Savmap->xMin + x * Savmap->xScale) + ((subX+1) * (Savmap->xScale / 4.0f));
	p3.y = (Savmap->yMin + y * Savmap->yScale) + ((subY+1) * (Savmap->yScale / 4.0f));
	p3.z = player[currentPlayer].Player->pos.z;
	p4.x = (Savmap->xMin + x * Savmap->xScale) + (subX * (Savmap->xScale / 4.0f));
	p4.y = (Savmap->yMin + y * Savmap->yScale) + ((subY+1) * (Savmap->yScale / 4.0f));
	p4.z = player[currentPlayer].Player->pos.z;
	DrawLine(&p1, &p2, 0xffffffff);
	DrawLine(&p2, &p3, 0xffffffff);
	DrawLine(&p3, &p4, 0xffffffff);
	DrawLine(&p4, &p1, 0xffffffff);
}

void DrawTriangle(Point3 *a, Point3 *b, Point3 *c)
{
	Point3	m;

	DrawLine(a, b, 0xffffffff);
	DrawLine(b, c, 0xffffffff);
	DrawLine(c, a, 0xffffffff);
	v3Add(&m, a, b);
	v3Inc(&m, c);
	v3Scale(&m, &m, 0.3333f);
	DrawMarker(m.x, m.y, m.z);
}


//debug
void DrawMarker(float x, float y, float z)
{
	Point3	ls, le;
	Colour cs, ce;

	mPush(&mCurMatrix);
	mUnit(&mCurMatrix);
	ls.x = x-0.5f;
	ls.y = y;
	ls.z = z;
	le.x = x+0.5f;
	le.y = y;
	le.z = z;
	cs.argb.a = 0xff;
	cs.argb.r = 0xff;
	cs.argb.g = 0xff;
	cs.argb.b = 0xff;
	ce = cs;
	rLineOnTop(&ls, &le, cs, ce);
	ls.x = x;
	ls.y = y-0.5f;
	ls.z = z;
	le.x = x;
	le.y = y+0.5f;
	le.z = z;
	cs.argb.a = 0xff;
	cs.argb.r = 0xff;
	cs.argb.g = 0xff;
	cs.argb.b = 0xff;
	ce = cs;
	rLineOnTop(&ls, &le, cs, ce);
	ls.x = x;
	ls.y = y;
	ls.z = z-0.5f;
	le.x = x;
	le.y = y;
	le.z = z+0.5f;
	cs.argb.a = 0xff;
	cs.argb.r = 0xff;
	cs.argb.g = 0xff;
	cs.argb.b = 0xff;
	ce = cs;
	rLineOnTop(&ls, &le, cs, ce);
	mPop(&mCurMatrix);
}

//debug
void drawBox(Point3 *low, Point3 *hi, Matrix *m)
{
	Point3	a, b;

	a = (*low);
	b = (*low);
	b.x = hi->x;
	mPoint3Apply(&a, m);
	mPoint3Apply(&b, m);
	DrawLine(&a, &b, 0xffffffff);
	a = (*hi);
	a.y = low->y;
	mPoint3Apply(&a, m);
	DrawLine(&a, &b, 0xffffffff);
	b = (*low);
	b.z = hi->z;
	mPoint3Apply(&b, m);
	DrawLine(&a, &b, 0xffffffff);
	a = (*low);
	mPoint3Apply(&a, m);
	DrawLine(&a, &b, 0xffffffff);
	a = (*hi);
	a.x = low->x;
	mPoint3Apply(&a, m);
	DrawLine(&a, &b, 0xffffffff);
	b = (*low);
	b.y = hi->y;
	mPoint3Apply(&b, m);
	DrawLine(&a, &b, 0xffffffff);
	a = (*low);
	mPoint3Apply(&a, m);
	DrawLine(&a, &b, 0xffffffff);
	a = (*hi);
	a.z = low->z;
	mPoint3Apply(&a, m);
	DrawLine(&a, &b, 0xffffffff);
	b = (*hi);
	mPoint3Apply(&b, m);
	DrawLine(&a, &b, 0xffffffff);
	a = (*hi);
	a.x = low->x;
	mPoint3Apply(&a, m);
	DrawLine(&a, &b, 0xffffffff);
	a = (*hi);
	a.y = low->y;
	mPoint3Apply(&a, m);
	DrawLine(&a, &b, 0xffffffff);
	a = (*hi);
	a.z = low->z;
	b = (*low);
	b.x = hi->x;
	mPoint3Apply(&a, m);
	mPoint3Apply(&b, m);
	DrawLine(&a, &b, 0xffffffff);
}
//debug
void ObjectBBoxDraw(Object *obj)
{
	int	i;

	if (obj->model)
	{
		mCurMatrix = obj->wm;
		mPreScale(&mCurMatrix, obj->model->bounds.hi.x, obj->model->bounds.hi.y, obj->model->bounds.hi.z);
		rDrawObject(BBoxStrat->obj);
	}
	for (i=0; i<obj->no_child; i++)
		ObjectBBoxDraw(&obj->child[i]);
}
//debug
void stratObjectBBoxDraw(STRAT *strat)
{
	if (!BBoxStrat)
		return;

	mPush(&mCurMatrix);
	mUnit(&mCurMatrix);
	ObjectBBoxDraw(strat->obj);
	mPop(&mCurMatrix);
}
//debug
void DrawPath(STRAT* strat)
{
	int i;
	
/*	WAYPATH *Path=strat->path; */
	WAYPATH *Path=strat->route->path;
	Point3 start;
	Point3 end;
	Colour col;
	mPush(&mCurMatrix);
	mUnit(&mCurMatrix);
	col.argb.a = col.argb.b = col.argb.g = col.argb.r = 0xff;

	for (i=0;i<Path->numwaypoints-1;i++)
	{
		start.x = Path->waypointlist[i].x;		
		start.y = Path->waypointlist[i].y;		
		start.z = Path->waypointlist[i].z;		
		end.x = Path->waypointlist[(i+1)%(Path->numwaypoints)].x;		
		end.y = Path->waypointlist[(i+1)%(Path->numwaypoints)].y;		
		end.z = Path->waypointlist[(i+1)%(Path->numwaypoints)].z;		
		rLineOnTop(&start,&end,col,col);
	}
	mPop(&mCurMatrix);

}

//debug
void DrawNet(STRAT *strat)
{
	int	way, node;
   	Point3 start,end;
	Colour col,col2;
	col.argb.a = col.argb.b = col.argb.g = col.argb.r = 0xff;
	col2.argb.a = col2.argb.b = col2.argb.g = col2.argb.r = 0x1e;

	/* Ensure modelToScreen matrix is loaded into XMTRX */
   	mPush(&mCurMatrix);
 	mUnit(&mCurMatrix);
	mLoad (&mWorldToScreen);
	mMul (&modelToScreen, &mCurMatrix);
	mLoadWithXYScale (&modelToScreen, pViewSize.x, pViewSize.y);	
	/*to draw network */
	if ((strat->route))
	{
		if (strat->route->path->waytype == NETWORK)
		{
			for (way=0;way<strat->route->path->numwaypoints;way++)
			{
				if (strat->route->path->net->waybusy[way])
				{	
					end.x = strat->route->path->waypointlist[way].x + 4.0f;
					end.y = strat->route->path->waypointlist[way].y + 4.0f;
					end.z = strat->route->path->waypointlist[way].z + 3.0f;
					
					if (way == strat->route->curway)
						DrawLine(&strat->route->path->waypointlist[way], &strat->pos, 0xff0000ff);
					else
						DrawLine(&strat->route->path->waypointlist[way], &end, 0x0000ffff);
				}

				node = (strat->route->path->net->connectors[way]&255);
				if (!(node))
					continue;
				else
				{
					node--;
					start=strat->route->path->waypointlist[way];
					end=strat->route->path->waypointlist[node];
				 //	start.z = player[currentPlayer].Player->pos.z;
				 //	end.z = player[currentPlayer].Player->pos.z;
					rLineOnTop(&start,&end,col,col);
				}

				node=((strat->route->path->net->connectors[way]>>8)&255);
				if (!(node))
					continue;
				else
				{
					node--;
					start=strat->route->path->waypointlist[way];
			   		end=strat->route->path->waypointlist[node];
			   //		start.z = player[currentPlayer].Player->pos.z;
			   //		end.z = player[currentPlayer].Player->pos.z;
					rLineOnTop(&start,&end,col,col);
				}

				node=((strat->route->path->net->connectors[way]>>16)&255);
				if (!(node))
					continue;
				else
				{
					node--;
					start=strat->route->path->waypointlist[way];
					end=strat->route->path->waypointlist[node];
			//		start.z = player[currentPlayer].Player->pos.z;
			//		end.z = player[currentPlayer].Player->pos.z;
					rLineOnTop(&start,&end,col,col);
				}

				node=((strat->route->path->net->connectors[way]>>24)&255);
				if (!(node))
					continue;
				else
				{
					node--;
					start=strat->route->path->waypointlist[way];
					end=strat->route->path->waypointlist[node];
			 //		start.z = player[currentPlayer].Player->pos.z;
			 //		end.z = player[currentPlayer].Player->pos.z;
					rLineOnTop(&start,&end,col,col);
				}



			}
		}
		else
			DrawPath(strat);
	}
	mPop(&mCurMatrix);
}
//debug
void rDrawWayPoint(STRAT *strat)
{
	Matrix	m;
	Point3 a,b;

	mPush(&mCurMatrix);
	mUnit(&mCurMatrix);
	mLoad (&mWorldToScreen);
	mMul (&modelToScreen, &mCurMatrix);
	mLoadWithXYScale (&modelToScreen, pViewSize.x, pViewSize.y);	
  //	m = strat->m;
	a.x = strat->pos.x;
	a.y = strat->pos.y;
	a.z = strat->pos.z;
	b.x = strat->CheckPos.x;
	b.y = strat->CheckPos.y;
	b.z = strat->CheckPos.z;

	
	DrawLine(&a,&b,0xffffffff);
	
	mPop(&mCurMatrix);
}


void rDrawBBoxRec(Matrix *m, Object *obj)
{
	int	i;

	mPreTranslate(m, obj->crntPos.x, obj->crntPos.y, obj->crntPos.z);
	mPreRotateXYZ(m, obj->crntRot.x, obj->crntRot.y, obj->crntRot.z);
	mPreScale(m, obj->scale.x, obj->scale.y, obj->scale.z);

	if (obj->model)
	{
		drawBox(&obj->model->bounds.low, &obj->model->bounds.hi, m);
	}

	for (i=0; i<obj->no_child; i++)
	{
		mPush(m);
		rDrawBBoxRec(m, &obj->child[i]);
		mPop(m);
	}
}

void rDrawBBox(STRAT *strat)
{
	Matrix	m;

	mPush(&mCurMatrix);
	mUnit(&mCurMatrix);
	m = strat->m;
	mPreScale(&m, strat->scale.x, strat->scale.y, strat->scale.z);
	mPostTranslate(&m, strat->pos.x, strat->pos.y, strat->pos.z);
	rDrawBBoxRec(&m, strat->obj);
	mPop(&mCurMatrix);
}

void DrawSplinePath(struct waypath *path)
{
	int	i;
	float	t, l1, l2;
	Point	tp, ctp, octp;
	Point3	a, b, c, d, e, p, p2;
	Vector3	tempv, tempv2, tangent, av, n, v;
	Matrix m, pm;

	m.m[0][0] = -1.0f;		m.m[0][1] = 3.0f;	m.m[0][2] = -3.0f;		m.m[0][3] = 1.0f;
	m.m[1][0] = 3.0f;		m.m[1][1] = -6.0f;	m.m[1][2] = 3.0f;		m.m[1][3] = 0.0f;
	m.m[2][0] = -3.0f;		m.m[2][1] = 3.0f;	m.m[2][2] = 0.0f;		m.m[2][3] = 0.0f;
	m.m[3][0] = 1.0f;		m.m[3][1] = 0.0f;	m.m[3][2] = 0.0f;		m.m[3][3] = 0.0f;

	for (i=0; i<path->numwaypoints-1; i++)
	{
		if (i == 0)
		{
			a = path->waypointlist[i];
			ctp.x = a.x;
			ctp.y = a.y;
			ctp.z = a.z;
			d = path->waypointlist[i+1];
			v3Sub(&tempv, &d, &a);
			tempv2 = tempv;
			v3Scale(&tempv, &tempv, 0.5f);
			v3Add(&b, &tempv, &a);
			v3Sub(&tempv, &d, &path->waypointlist[i+2]);
			l1 = sqrt(v3Dot(&tempv2, &tempv2));
			l2 = sqrt(v3Dot(&tempv, &tempv));
			v3Normalise(&tempv);
			v3Normalise(&tempv2);
			v3Add(&av, &tempv, &tempv2);
			v3Scale(&av, &av, 0.5f);
			v3Cross(&n, &tempv, &tempv2);
			v3Cross(&tangent, &n, &av);
			v3Normalise(&tangent);
			v = tangent;
			v3Scale(&v, &v, 0.5f * l1);
			v3Sub(&c, &d, &v);
			v = tangent;
			v3Scale(&v, &v, 0.5f * l2);
			v3Add(&e, &d, &v);
			pm.m[0][0] = a.x; pm.m[0][1] = a.y; pm.m[0][2] = a.z; pm.m[0][3] = 1.0f;
			pm.m[1][0] = b.x; pm.m[1][1] = b.y; pm.m[1][2] = b.z; pm.m[1][3] = 1.0f;
			pm.m[2][0] = c.x; pm.m[2][1] = c.y; pm.m[2][2] = c.z; pm.m[2][3] = 1.0f;
			pm.m[3][0] = d.x; pm.m[3][1] = d.y; pm.m[3][2] = d.z; pm.m[3][3] = 1.0f;
			mPreMultiply(&pm, &m);
			for (t=0.0f; t<1.0f; t+=0.1f)
			{
				tp.x = t*t*t;
				tp.y = t*t;
				tp.z = t;
				tp.w = 1.0f;
				mLoad (&pm);
				octp = ctp;
				mPoint (&ctp, &tp);
				p.x = octp.x;
				p.y = octp.y;
				p.z = octp.z;
				p2.x = ctp.x;
				p2.y = ctp.y;
				p2.z = ctp.z;
				DrawLine(&p, &p2, 0xff00ffff);
			}
			
		}
		else if (i == path->numwaypoints - 2)
		{
			a = d;
			b = e;
			d = path->waypointlist[i+1];
			v3Sub(&tempv, &d, &a);
			v3Scale(&tempv, &tempv, 0.5f);
			v3Add(&c, &a, &tempv);
			pm.m[0][0] = a.x; pm.m[0][1] = a.y; pm.m[0][2] = a.z; pm.m[0][3] = 1.0f;
			pm.m[1][0] = b.x; pm.m[1][1] = b.y; pm.m[1][2] = b.z; pm.m[1][3] = 1.0f;
			pm.m[2][0] = c.x; pm.m[2][1] = c.y; pm.m[2][2] = c.z; pm.m[2][3] = 1.0f;
			pm.m[3][0] = d.x; pm.m[3][1] = d.y; pm.m[3][2] = d.z; pm.m[3][3] = 1.0f;
			mPreMultiply(&pm, &m);
			for (t=0.0f; t<1.0f; t+=0.1f)
			{
				tp.x = t*t*t;
				tp.y = t*t;
				tp.z = t;
				tp.w = 1.0f;
				mLoad (&pm);
				octp = ctp;
				mPoint (&ctp, &tp);
				p.x = octp.x;
				p.y = octp.y;
				p.z = octp.z;
				p2.x = ctp.x;
				p2.y = ctp.y;
				p2.z = ctp.z;
				DrawLine(&p, &p2, 0xff00ffff);
			}
			
		}
		else
		{
			a = path->waypointlist[i];
			b = e;
			d = path->waypointlist[i+1];
			v3Sub(&tempv2, &d, &a);
			v3Sub(&tempv, &d, &path->waypointlist[i+2]);
			l1 = sqrt(v3Dot(&tempv2, &tempv2));
			l2 = sqrt(v3Dot(&tempv, &tempv));
			v3Normalise(&tempv);
			v3Normalise(&tempv2);
			v3Add(&av, &tempv, &tempv2);
			v3Scale(&av, &av, 0.5f);
			v3Cross(&n, &tempv, &tempv2);
			v3Cross(&tangent, &n, &av);
			v3Normalise(&tangent);
			v = tangent;
			v3Scale(&v, &v, 0.5f * l1);
			v3Sub(&c, &d, &v);
			v = tangent;
			v3Scale(&v, &v, 0.5f * l2);
			v3Add(&e, &d, &v);
			pm.m[0][0] = a.x; pm.m[0][1] = a.y; pm.m[0][2] = a.z; pm.m[0][3] = 1.0f;
			pm.m[1][0] = b.x; pm.m[1][1] = b.y; pm.m[1][2] = b.z; pm.m[1][3] = 1.0f;
			pm.m[2][0] = c.x; pm.m[2][1] = c.y; pm.m[2][2] = c.z; pm.m[2][3] = 1.0f;
			pm.m[3][0] = d.x; pm.m[3][1] = d.y; pm.m[3][2] = d.z; pm.m[3][3] = 1.0f;
			mPreMultiply(&pm, &m);
			for (t=0.0f; t<1.0f; t+=0.01f)
			{
				tp.x = t*t*t;
				tp.y = t*t;
				tp.z = t;
				tp.w = 1.0f;
				mLoad (&pm);
				octp = ctp;
				mPoint (&ctp, &tp);
				
				p.x = octp.x;
				p.y = octp.y;
				p.z = octp.z;
				p2.x = ctp.x;
				p2.y = ctp.y;
				p2.z = ctp.z;
				DrawLine(&p, &p2, 0xff00ffff);
			}
			
		}
	}
}
void DrawObjectCentre(STRAT *strat, int objn)
{
	Matrix m;
	int i=0;
	Object *obj;

	mUnit(&m);
	obj = strat->objId[objn];
	
	dAssert(obj, "error in draw object centre");
	do
	{
		mPostScale(&m, obj->scale.x, obj->scale.y, obj->scale.z);
		mPostRotateXYZ(&m, obj->crntRot.x, obj->crntRot.y, obj->crntRot.z);
		mPostTranslate(&m, obj->crntPos.x, obj->crntPos.y, obj->crntPos.z);
		obj = obj->parent;
	}while(obj);
	
	/*mPostScale(&m, strat->scale.x, strat->scale.y, strat->scale.z);*/
	mPostMultiply(&m, &strat->m);
	mPostTranslate(&m, strat->pos.x, strat->pos.y, strat->pos.z);
	DrawMarker(m.m[3][0], m.m[3][1], m.m[3][2]);
	mPush(&mCurMatrix);
	mCurMatrix = m;
	rDrawModel(player[currentPlayer].Player->objId[3]->model);
	mPop(&mCurMatrix);

}
void DrawVector(Point3 *p, Vector3 *v)
{
	Point3 p2;
	v3Add(&p2, p, v);
	DrawLine(p, &p2, 0xffffffff);
}





#endif
