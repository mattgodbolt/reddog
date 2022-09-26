/*
 * animcontrol.h
 */

#ifndef _animcontrol_H
#define _animcontrol_H

#include "HelpBase.h"
#include "resource.h"

#define animcontrol_CLASS_ID_A 0xcc495118
#define animcontrol_CLASS_ID_B 0xcc3618ef
#define animcontrol_CLASS_ID	Class_ID(animcontrol_CLASS_ID_A, animcontrol_CLASS_ID_B)

#define PB_CALC_MATRIX		0
#define PB_ID				1
#define PB_TARGETABLE		2
#define PB_HIGH_D_COLL		3

extern ParamUIDesc ACdescParam[];
#define AC_PARAMDESC_LENGTH 0

extern ParamBlockDescID descVer1[];
extern ParamBlockDescID descVer2[];
extern ParamBlockDescID descVer3[];

#define AC_CURRENT_VERSION	3
#define DOME_OBJECT_ROOT		"P:\\GAME\\NEWOBJECTS\\DOMES"

class animcontrolCreatorClass; class animcontrol; class animcontrolParamDlg;

#define ANIMCONTROL HelperBase<animcontrolCreatorClass, IDD_ANIMCONTROL, IDS_ANIMCONTROL, animcontrolParamDlg, animcontrol_CLASS_ID_A, animcontrol_CLASS_ID_B, ACdescParam, AC_PARAMDESC_LENGTH>

class animcontrolParamDlg : public ParamMapUserDlgProc
{
private:
	animcontrol	*ref;
	static HWND instHwnd;
	static ICustEdit *nameCtrl;
	static ISpinnerControl *frameSpinner;
	static ISpinnerControl *endframeSpinner;
public:
	static animcontrolParamDlg *dlgInstance;
	animcontrolParamDlg (void *r) { ref = (animcontrol *)r; dlgInstance = this;}
	~animcontrolParamDlg () { dlgInstance = NULL; ParamMapUserDlgProc::~ParamMapUserDlgProc(); }

	void VarUpdate (); // internally called
	void Update (TimeValue t); // max called
	void UpdateAnimInfo(HWND hWnd);
	void DeleteThis() { delete this; }
	BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
};

/* Creation of a flags object */
class animcontrolCreatorClass : public CreateMouseCallBack
{
private:
	animcontrol		*ref;
public:
	animcontrolCreatorClass() {}
	void setRef (ANIMCONTROL *newRef) 
	{ ref = (animcontrol *)newRef; }

	int proc (ViewExp *vpt, int msg, int point, 
		int flags, IPoint2 m, Matrix3& mat);
};


typedef struct anim
{
	char	name[32];		//THE NAME OF THE ANIMATION
	int		keyframe;		//THE MAX KEYFRAME IN WHICH IT OCCURS
	int		endframe;		//THE MAX KEYFRAME ON WHICH IT ENDS
	int		output;			//temp message warez
}	ANIM;

#define MAX_ANIMS 32
extern ANIM anims[];

extern  void ClearAnims();
class animcontrol : public ANIMCONTROL
{
private:
   
	friend class	animcontrolDesc;
	friend class	animcontrolCreatorClass;
	friend class	animcontrolParamDlg;
	friend class	RDObjectExporter;
	Mesh				objMesh;						// The mesh of this object, if any

public:

 //	int		interpolate;		//IS ANIM OF INTERPOLATE TYPE
	int		currentanim;
	animcontrol ();
	virtual void WindowFinal (IObjParam *ip);
	virtual void DrawMarker (GraphicsWindow *gw, bool Selected = FALSE, bool Frozen = FALSE);
	virtual RefTargetHandle Clone (RemapDir& remap = NoRemap())
	{ 
		animcontrol *ret = new animcontrol();
		ret->ReplaceReference (0, pblock->Clone (remap));
		return (ret);
	}
	BOOL SetValue(int i, TimeValue t, float v) { return FALSE; }
	BOOL SetValue(int i, TimeValue t, Point3 &v) { return FALSE; }
	BOOL GetValue(int i, TimeValue t, float &v, Interval &ivalid) { return FALSE; }
	BOOL GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid) { return FALSE; }
	Class_ID ClassID() { return animcontrol_CLASS_ID; }

	
	void Reset();
 //	IOResult Load (ILoad *iload);
 //	IOResult Save (ISave *isave);

	
};

extern int interpo;
extern int createid;

extern ClassDesc *GetanimcontrolDesc();

#endif
