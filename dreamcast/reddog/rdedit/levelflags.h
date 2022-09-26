/*
 * levelflags.h
 */

#ifndef _levelflags_H
#define _levelflags_H

#include "HelpBase.h"
#include "resource.h"

#define levelflags_CLASS_ID_A 0x6d495118
#define levelflags_CLASS_ID_B 0x743618ef
#define levelflags_CLASS_ID	Class_ID(levelflags_CLASS_ID_A, levelflags_CLASS_ID_B)

#define RED 0
#define GREEN 1
#define BLUE 2
#define FOGRED 3
#define FOGGREEN 4
#define FOGBLUE 5

#define FOGNEAR 0
#define FOGFAR 1
#define STRATINTENSITY 2
#define SCAPEINTENSITY 3
#define FOV 4
#define FARZ 5
#define CAMHEIGHT 6
#define CAMDIST 7
#define CAMLOOKHEIGHT 8



#define PB_CALC_MATRIX		0
#define PB_ID				1
#define PB_TARGETABLE		2
#define PB_HIGH_D_COLL		3

extern ParamUIDesc LFdescParam[];
#define LF_PARAMDESC_LENGTH 0

extern ParamBlockDescID descVer1[];
extern ParamBlockDescID descVer2[];
extern ParamBlockDescID descVer3[];

#define LF_CURRENT_VERSION	3
#define DOME_OBJECT_ROOT		"P:\\GAME\\NEWOBJECTS\\DOMES"


class levelflagCreatorClass; class levelflag; class levelflagsParamDlg;

#define LEVELFLAG HelperBase<levelflagCreatorClass, IDD_LEVELFLAGS, IDS_LEVELFLAGS, levelflagsParamDlg, levelflags_CLASS_ID_A, levelflags_CLASS_ID_B, LFdescParam, LF_PARAMDESC_LENGTH>

class levelflagsParamDlg : public ParamMapUserDlgProc
{
private:
	levelflag	*ref;
	static HWND instHwnd;
	static ICustEdit *nameCtrl;
	static ISpinnerControl 
		*fovSpinner,*farzSpinner,*cameraheightSpinner,*cameradistSpinner,*cameralookheightSpinner,
		*scapeintensitySpinner, *stratintensitySpinner,*fognearSpinner, *fogfarSpinner,
		*aredSpinner, *ablueSpinner, *agreenSpinner,
		*fredSpinner, *fblueSpinner, *fgreenSpinner;
public:
	static levelflagsParamDlg *dlgInstance;
	levelflagsParamDlg (void *r) { ref = (levelflag *)r; dlgInstance = this;}
	~levelflagsParamDlg () { dlgInstance = NULL; ParamMapUserDlgProc::~ParamMapUserDlgProc(); }

	void Update (void); // internally called
	void Update (TimeValue t); // max called
	void DeleteThis() { delete this; }
	BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
};

/* Creation of a flags object */
class levelflagCreatorClass : public CreateMouseCallBack
{
private:
	levelflag		*ref;
public:
	levelflagCreatorClass() {}
	void setRef (LEVELFLAG *newRef) 
	{ ref = (levelflag *)newRef; }

	int proc (ViewExp *vpt, int msg, int point, 
		int flags, IPoint2 m, Matrix3& mat);
};

class levelflag : public LEVELFLAG
{
private:
   
	//static TSTR		name;
	friend class	levelflagsDesc;
	friend class	levelflagCreatorClass;
	friend class	levelflagsParamDlg;
	friend class	RDObjectExporter;
public:
	static char			name[MAX_FNAME_SIZE];	// Which object this is (dialog)

 	static int		fogred;
	static int		foggreen;
	static int		fogblue;
  	static float	fognear;
	static float	fogfar;
	static int		ambientred;
	static int		ambientgreen;
	static int 		ambientblue;
	static float	scapeintensity;
	static float	stratintensity;
	static float	fov;
	static float	farz;
	static float	cameraHeight;
	static float	cameraDist;
	static float	cameralookHeight;
	levelflag ();
	virtual void WindowFinal (IObjParam *ip);
	virtual RefTargetHandle Clone (RemapDir& remap = NoRemap())
	{ 
		levelflag *ret = new levelflag();
		ret->ReplaceReference (0, pblock->Clone (remap));
		return (ret);
	}
	BOOL SetValue(int i, TimeValue t, float v) { return FALSE; }
	BOOL SetValue(int i, TimeValue t, Point3 &v) { return FALSE; }
	BOOL GetValue(int i, TimeValue t, float &v, Interval &ivalid) { return FALSE; }
	BOOL GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid) { return FALSE; }
	Class_ID ClassID() { return levelflags_CLASS_ID; }

	static int GetLightValue(int i)
	{
		switch (i)
		{
			case RED:
				return (ambientred);
				break;
			case GREEN:
				return (ambientgreen);
				break;
			case BLUE:
				return (ambientblue);
				break;
			case FOGRED:
				return (fogred);
				break;
			case FOGGREEN:
				return (foggreen);
				break;
			case FOGBLUE:
				return (fogblue);
				break;
			default:
				return(0);

		}


	}


	static float GetFloatValue(int i)
	{
		switch (i)
		{
			case FOGNEAR:
				return (fognear);
				break;
			case FOGFAR:
				return (fogfar);
				break;
			case SCAPEINTENSITY:
				return (scapeintensity);
				break;
			case STRATINTENSITY:
				return (stratintensity);
				break;
			case FOV:
				return (fov);
				break;
			case FARZ:
				return (farz);
				break;
			case CAMHEIGHT:
				return (cameraHeight);
				break;
			case CAMDIST:
				return (cameraDist);
				break;
			case CAMLOOKHEIGHT:
				return (cameralookHeight);
				break;
			default:
				return(100.0);

		}


	}


	BOOL GetValue (int i, TimeValue t, int &v, Interval &ivalid)
	{
		switch (i) {
		case PB_TARGETABLE:
		case PB_HIGH_D_COLL:
			break;
		case PB_CALC_MATRIX:
			//v = cCalcMatrix;
			break;
		case PB_ID:
			v = 0;
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
		  //	cCalcMatrix = v==0?false : true;
			break;
		case PB_ID:
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
	int ID() const
	{ 
		int foo; 
		Interval iv = FOREVER;
		pblock->GetValue (PB_ID, TimeValue(0), foo, iv); 
		return foo;
	}
	IOResult Load(ILoad *iload);
	
};

extern ClassDesc *GetLevelFlagDesc();

#endif
