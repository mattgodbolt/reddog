/*
 * ObjFlags.h
 */

#ifndef _OBJFLAGS_H
#define _OBJFLAGS_H

#include "HelpBase.h"
#include "resource.h"

#define OBJFLAGS_CLASS_ID_A 0x6d495008
#define OBJFLAGS_CLASS_ID_B 0x743618dc
#define OBJFLAGS_CLASS_ID	Class_ID(OBJFLAGS_CLASS_ID_A, OBJFLAGS_CLASS_ID_B)

#define PB_CALC_MATRIX		0
#define PB_ID				1
#define PB_TARGETABLE		2
#define PB_HIGH_D_COLL		3
#define PB_SHADOW			4
#define PB_UNLIT			5
#define PB_ROUNDSHADOW		6
#define PB_FACEON			7
#define PB_STRATMOVE	   	8
#define PB_WATEREFFECT		9
#define PB_LAVAEFFECT		10

extern ParamUIDesc OFdescParam[];
#define OF_PARAMDESC_LENGTH 11

#define OF_CURRENT_VERSION	8
#define OF_CURVERSIONDESC OFdescVer8

#define OBJFLAG HelperBase<ObjFlagCreatorClass, IDD_OBJFLAGS, IDS_OBJ_FLAGS_CLASS_NAME, ObjFlagsParamDlg, OBJFLAGS_CLASS_ID_A, OBJFLAGS_CLASS_ID_B, OFdescParam, OF_PARAMDESC_LENGTH>

class ObjFlagCreatorClass; class ObjFlag; class ObjFlagsParamDlg;

class ObjFlagsParamDlg : public ParamMapUserDlgProc
{
private:
	ObjFlag	*ref;
	static HWND instHwnd;
	static ICustEdit *nameCtrl;
	static ISpinnerControl *radiusSpinner, *massSpinner, *uidSpinner, *moiSpinner, *expSpinner,
		*frictionXSpinner, *frictionYSpinner, *frictionZSpinner,
		*airFrictionXSpinner, *airFrictionYSpinner, *airFrictionZSpinner;

public:
	static ObjFlagsParamDlg *dlgInstance;
	ObjFlagsParamDlg (void *r) { ref = (ObjFlag *)r; dlgInstance = this;}
	~ObjFlagsParamDlg () { dlgInstance = NULL; ParamMapUserDlgProc::~ParamMapUserDlgProc(); }

	void Update (void); // internally called
	void Update (TimeValue t); // max called
	void DeleteThis() { delete this; }
	BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
};

/* Creation of a flags object */
class ObjFlagCreatorClass : public CreateMouseCallBack
{
private:
	ObjFlag		*ref;
public:
	ObjFlagCreatorClass() {}
	void setRef (OBJFLAG *newRef) 
	{ ref = (ObjFlag *)newRef; }

	int proc (ViewExp *vpt, int msg, int point, 
		int flags, IPoint2 m, Matrix3& mat);
};

class ObjFlag : public OBJFLAG
{
private:
	static bool		cCalcMatrix;
	static bool		ownInstance;
	static float	mass;
	static float	radius;
	static Point3	friction, airFriction;
	static TSTR		name;
	static float	moi, expRot;
	static int		uid;

	friend class	ObjFlagsDesc;
	friend class	ObjFlagCreatorClass;
	friend class	ObjFlagsParamDlg;
	friend class	RDObjectExporter;
public:
	ObjFlag ();
	virtual void WindowFinal (IObjParam *ip);
	virtual void DrawMarker (GraphicsWindow *gw, bool Selected = FALSE, bool Frozen = FALSE);
	virtual RefTargetHandle Clone (RemapDir& remap = NoRemap())
	{ 
		ObjFlag *ret = new ObjFlag();
		ret->ReplaceReference (0, pblock->Clone (remap));
		return (ret);
	}
	BOOL SetValue(int i, TimeValue t, float v) { return FALSE; }
	BOOL SetValue(int i, TimeValue t, Point3 &v) { return FALSE; }
	BOOL GetValue(int i, TimeValue t, float &v, Interval &ivalid) { return FALSE; }
	BOOL GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid) { return FALSE; }
	
	BOOL GetValue (int i, TimeValue t, int &v, Interval &ivalid)
	{
		switch (i) {
		case PB_TARGETABLE:
		case PB_HIGH_D_COLL:
			break;
		case PB_CALC_MATRIX:
			v = cCalcMatrix;
			break;
		case PB_ID:
			v = uid;
			break;
		default:
			return FALSE;
		}
		return TRUE;
	}
	
	BOOL SetValue (int i, TimeValue t, int v)
	{
		switch (i) {
		case PB_CALC_MATRIX:
			cCalcMatrix = v==0?false : true;
			break;
		case PB_ID:
			uid = v;
			break;
		default:
			return FALSE;
		}
		return TRUE;
	}

	bool CalcMatrix() const 
	{ 
		int foo;
		Interval iv = FOREVER;
		pblock->GetValue (PB_CALC_MATRIX, TimeValue(0), foo, iv); 
		return foo==1?true:false;
	}
	bool Targetable() const 
	{ 
		int foo;
		Interval iv = FOREVER;
		pblock->GetValue (PB_TARGETABLE, TimeValue(0), foo, iv); 
		return foo==1?true:false;
	}
	bool HighDColl() const 
	{ 
		int foo;
		Interval iv = FOREVER;
		pblock->GetValue (PB_HIGH_D_COLL, TimeValue(0), foo, iv); 
		return foo==1?true:false;
	}
	bool Unlit() const 
	{ 
		int foo;
		Interval iv = FOREVER;
		pblock->GetValue (PB_UNLIT, TimeValue(0), foo, iv); 
		return foo==1?true:false;
	}
	int Shadow() const 
	{ 
		int foo, bar;
		Interval iv = FOREVER;
		pblock->GetValue (PB_SHADOW, TimeValue(0), foo, iv); 
		pblock->GetValue (PB_ROUNDSHADOW, TimeValue(0), bar, iv); 
		return bar==1?2:foo==1?1:0;
	}
	bool FaceOn() const 
	{ 
		int foo;
		Interval iv = FOREVER;
		pblock->GetValue (PB_FACEON, TimeValue(0), foo, iv); 
		return foo==1?true:false;
	}
	bool StratMove() const 
	{ 
		int foo;
		Interval iv = FOREVER;
		pblock->GetValue (PB_STRATMOVE, TimeValue(0), foo, iv); 
		return foo==1?true:false;
	}
	bool WaterEffect() const 
	{ 
		int foo;
		Interval iv = FOREVER;
		pblock->GetValue (PB_WATEREFFECT, TimeValue(0), foo, iv); 
		return foo==1?true:false;
	}
	bool LavaEffect() const 
	{ 
		int foo;
		Interval iv = FOREVER;
		pblock->GetValue (PB_LAVAEFFECT, TimeValue(0), foo, iv); 
		return foo==1?true:false;
	}
	int ID() const
	{ 
		int foo; 
		Interval iv = FOREVER;
		pblock->GetValue (PB_ID, TimeValue(0), foo, iv); 
		return foo;
	}
 	void SetID(int param) 
	{ 
	   	pblock->SetValue (PB_ID, TimeValue(0), param); 
	   //	return foo;
	}

	IOResult Load(ILoad *iload);
									   
};

extern ClassDesc *GetFlagDesc();

#endif
