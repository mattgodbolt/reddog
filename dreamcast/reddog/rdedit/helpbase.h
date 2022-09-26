/*
 * HelpBase.h
 * Base for helper classes
 */
#ifndef _HELPBASE_H
#define _HELPBASE_H

#include "RDEdit.h"
#include "IParamM.h"
template <class Creator, int DialogID, int StringID, class DlgProc, ulong aa, ulong bb, ParamUIDesc descParam[], int PARAMDESC_LENGTH>
class HelperBase : public HelperObject
{
protected:
	/*
	 * Class variables
	 */
	static IParamMap	*pmapParam;						// the default parameter map
	static IObjParam	*objParams;
	/*
	 * Instance variables
	 */
	bool				suspendSnap;
	friend				class Creator;
	IParamBlock			*pblock;

public:
	/*
	 * Construction and destruction
	 */
	HelperBase() : suspendSnap(false), pblock(NULL) {}
	~HelperBase() {}

	virtual bool LoadChunk (ILoad *chunk)
	{ return TRUE; }

	virtual void WindowInit (IObjParam *p) {}
	virtual void WindowFinal (IObjParam *p) {}

	/*
	 * From ReferenceMaker
	 */
	virtual RefResult NotifyRefChanged (Interval i, RefTargetHandle target,
								PartID &partID, RefMessage message)
	{ return REF_SUCCEED; }
	virtual RefTargetHandle Clone (RemapDir& remap = NoRemap()) = 0;
	int NumRefs() {return 1;}
	RefTargetHandle GetReference(int i) {return pblock;}
	void SetReference(int i, RefTargetHandle rtarg) {pblock=(IParamBlock*)rtarg;}


	virtual IOResult Load (ILoad *iload)
	{
		IOResult res = IO_OK;
		
		while (IO_OK==(res=iload->OpenChunk())) {
			if (!LoadChunk (iload))
				return IO_ERROR;
			res = iload->CloseChunk();
			if (res!=IO_OK)  return res;
		}
		
		return IO_OK;
	}
	virtual IOResult Save (ISave *isave)
	{
		// Save instance vars here
		
		return IO_OK;
	}

	/*
	 * From BaseObject
	 */
	virtual CreateMouseCallBack *GetCreateMouseCallBack()
	{	static Creator creator;
		creator.setRef (this);
		return &creator;
	}
	virtual int HitTest (TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt)
	{
		Matrix3 tm(1);	
		HitRegion hitRegion;
		DWORD	savedLimits;
		::Point3 pt(0,0,0);
		
		vpt->getGW()->setTransform(tm);
		GraphicsWindow *gw = vpt->getGW();	
		::Material *mtl = gw->getMaterial();
		
		tm = inode->GetObjectTM(t);		
		MakeHitRegion(hitRegion, type, crossing, 4, p);
		
		gw->setRndLimits(((savedLimits = gw->getRndLimits())|GW_PICK)&~GW_ILLUM);
		gw->setHitRegion(&hitRegion);
		gw->clearHitCode();
		
		vpt->getGW()->setTransform(tm);
		DrawMarker(vpt->getGW());
		
		gw->setRndLimits(savedLimits);
		
		if((hitRegion.type != POINT_RGN) && !hitRegion.crossing)
			return TRUE;
		return gw->checkHitCode();
	}

	virtual void Snap (TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt)
	{
		if(suspendSnap)
			return;
		
		Matrix3 tm = inode->GetObjectTM(t);	
		GraphicsWindow *gw = vpt->getGW();	
		gw->setTransform(tm);
		
		Matrix3 invPlane = Inverse(snap->plane);
		
		// Make sure the vertex priority is active and at least as important as the best snap so far
		if(snap->vertPriority > 0 && snap->vertPriority <= snap->priority) {
			::Point2 fp = ::Point2((float)p->x, (float)p->y);
			::Point2 screen2;
			IPoint3 pt3;
			
			::Point3 thePoint(0,0,0);
			// If constrained to the plane, make sure this point is in it!
			if(snap->snapType == SNAP_2D || snap->flags & SNAP_IN_PLANE) {
				::Point3 test = thePoint * tm * invPlane;
				if(fabs(test.z) > 0.0001)	// Is it in the plane (within reason)?
					return;
			}
			gw->wTransPoint(&thePoint,&pt3);
			screen2.x = (float)pt3.x;
			screen2.y = (float)pt3.y;
			
			// Are we within the snap radius?
			int len = (int)Length(screen2 - fp);
			if(len <= snap->strength) {
				// Is this priority better than the best so far?
				if(snap->vertPriority < snap->priority) {
					snap->priority = snap->vertPriority;
					snap->bestWorld = thePoint * tm;
					snap->bestScreen = screen2;
					snap->bestDist = len;
				}
				else
					if(len < snap->bestDist) {
						snap->priority = snap->vertPriority;
						snap->bestWorld = thePoint * tm;
						snap->bestScreen = screen2;
						snap->bestDist = len;
					}
			}
		}
	}

	virtual void DrawMarker (GraphicsWindow *gw, bool Selected = FALSE, bool Frozen = FALSE)
	{
		::Point3 pt(0,0,0);
		if(!Frozen)
			gw->setColor(LINE_COLOR,GetUIColor(COLOR_POINT_OBJ));
		gw->marker(&pt,X_MRKR);
	}

	virtual int Display (TimeValue t, INode* inode, ViewExp *vpt, int flags) 
	{
		Matrix3 tm(1);
		
		vpt->getGW()->setTransform(tm);
		tm = inode->GetObjectTM(t);		
		
		vpt->getGW()->setTransform(tm);
		
		DrawMarker (vpt->getGW(), inode->Selected()?true:false, inode->IsFrozen()?true:false);
		
		return 0;
	}

	virtual void BeginEditParams (IObjParam *ip, ULONG flags,Animatable *prev)
	{
		objParams = ip;
		if (!pmapParam) {
			pmapParam = CreateCPParamMap (
				descParam, PARAMDESC_LENGTH, 
				pblock, ip, hInstance, 
				MAKEINTRESOURCE (DialogID),
				GetString (StringID), 
				0);
		} else {
			pmapParam->SetParamBlock (pblock);
		}
		pmapParam->SetUserDlgProc (new DlgProc(this));
	}
	virtual void EndEditParams (IObjParam *ip, ULONG flags,Animatable *next)
	{
		WindowFinal(ip);
		if (flags&END_EDIT_REMOVEUI) {
			DestroyCPParamMap (pmapParam);
			pmapParam  = NULL;
		}
		objParams = NULL;
	}
	
	virtual TCHAR *GetObjectName() { return GetString (StringID); }
	virtual BOOL HasUVW() { return FALSE; }

	/*
	 * From Object
	 */
	virtual void GetLocalBoundBox (TimeValue t, INode *mat, ViewExp *vpt, Box3& box)
	{
		box = Box3(::Point3(0,0,0), ::Point3(0,0,0));
	}
	
	virtual void GetWorldBoundBox (TimeValue t, INode* inode, ViewExp* vpt, Box3& box )
	{
		Matrix3 tm;
		tm = inode->GetObjectTM(t);
		GetLocalBoundBox (t, inode, vpt, box);
		box = box * tm;
	}
	
	virtual ObjectState Eval (TimeValue t) { return ObjectState (this); }
	virtual void InitNodeName (TSTR& s) { s = GetString(StringID); }
	virtual ObjectHandle ApplyTransform (Matrix3& matrix) {return this;}
	virtual Interval ObjectValidity (TimeValue t) {return FOREVER;}
	virtual int CanConvertToType (Class_ID obtype) {return FALSE;}
	virtual Object* ConvertToType (TimeValue t, Class_ID obtype) {assert(0);return NULL;}		
	virtual int DoOwnSelectHilite ()	{ return 1; }

	/*
	 * From Animatable
	 */
	void DeleteThis() { delete this; }
	virtual Class_ID ClassID() { return Class_ID (aa, bb); };
	virtual void GetClassName (TSTR& s) { s = GetString (StringID); }
	virtual int IsKeyable(){ return 0;}
};

#endif
