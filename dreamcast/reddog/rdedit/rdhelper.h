/*
 * RDHelper.h
 * Interface to the helper objects
 */

#ifndef _RDHELPER_H
#define _RDHELPER_H

#include "util.h"

#define RDHELPER_CLASS_ID Class_ID(0x48e1283d, 0x7b522247)

extern ClassDesc* GetRDHelperDesc();

class RDHelper : public HelperObject {
private:
	/*
	 * Class variables
	 */
	static HWND			paramWindow;
	static IObjParam	*objParams;
	/*
	 * Instance variables
	 */
	int					suspendSnap;
	friend				class RDHCreate;
public:
	/*
	 * Construction and destruction
	 */
	RDHelper() : suspendSnap(0) {}
	~RDHelper() {}

	/*
	 * From ReferenceMaker
	 */
	RefResult NotifyRefChanged (Interval i, RefTargetHandle target,
								PartID &partID, RefMessage message);
	RefTargetHandle Clone (RemapDir& remap = NoRemap());
	IOResult Load (ILoad *iload);
	IOResult Save (ISave *isave);

	/*
	 * From BaseObject
	 */
	CreateMouseCallBack *GetCreateMouseCallBack();
	int HitTest (TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
	void Snap (TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt);
	int Display (TimeValue t, INode* inode, ViewExp *vpt, int flags);
	void BeginEditParams (IObjParam *ip, ULONG flags,Animatable *prev);
	void EndEditParams (IObjParam *ip, ULONG flags,Animatable *next);
	
	TCHAR *GetObjectName() { return GetString (IDS_HELPER_NAME); }
	BOOL HasUVW() { return FALSE; }

	/*
	 * From Object
	 */
	void GetWorldBoundBox (TimeValue t, INode *mat, ViewExp *vpt, Box3& box);
	void GetLocalBoundBox (TimeValue t, INode *mat, ViewExp *vpt, Box3& box);

	ObjectState Eval (TimeValue t) { return ObjectState (this); }
	void InitNodeName (TSTR& s) { s = GetString(IDS_HELPER_NAME); }
	ObjectHandle ApplyTransform (Matrix3& matrix) {return this;}
	Interval ObjectValidity (TimeValue t) {return FOREVER;}
	int CanConvertToType (Class_ID obtype) {return FALSE;}
	Object* ConvertToType (TimeValue t, Class_ID obtype) {assert(0);return NULL;}		
	int DoOwnSelectHilite ()	{ return 1; }

	/*
	 * From Animatable
	 */
	void DeleteThis() { delete this; }
	Class_ID ClassID() { return RDHELPER_CLASS_ID; }
	void GetClassName (TSTR& s) { s = TSTR(GetString(IDS_HELPER_CLASS_NAME)); }
	int IsKeyable(){ return 0;}
};

#endif

