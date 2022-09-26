/*
 * camcontrols.h
 */

#ifndef _camcontrols_H
#define _camcontrols_H

#include "HelpBase.h"
#include "resource.h"

#define camcontrols_CLASS_ID_A 0xff495118
#define camcontrols_CLASS_ID_B 0xee3618ef
#define camcontrols_CLASS_ID	Class_ID(camcontrols_CLASS_ID_A, camcontrols_CLASS_ID_B)

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

extern ParamUIDesc CCdescParam[];
#define CC_PARAMDESC_LENGTH 0

extern ParamBlockDescID descVer1[];
extern ParamBlockDescID descVer2[];
extern ParamBlockDescID descVer3[];

#define CC_CURRENT_VERSION	3
#define DOME_OBJECT_ROOT		"P:\\GAME\\NEWOBJECTS\\DOMES"

class camcontrolCreatorClass; class camcontrol; class camcontrolsParamDlg;

#define CAMCONTROLS HelperBase<camcontrolCreatorClass, IDD_CAMCONTROLS, IDS_CAMCONTROLS, camcontrolsParamDlg, camcontrols_CLASS_ID_A, camcontrols_CLASS_ID_B, CCdescParam, CC_PARAMDESC_LENGTH>

class camcontrolsParamDlg : public ParamMapUserDlgProc
{
private:
	camcontrol	*ref;
	static HWND instHwnd;
	static ICustEdit *nameCtrl;
	static ISpinnerControl 
		*speedSpinner,
		*amountSpinner;
	/*
		*upxposSpinner,
		*upyposSpinner,
		*upzposSpinner,
		*downxposSpinner,
		*downyposSpinner,
		*downzposSpinner;	*/
public:
	static camcontrolsParamDlg *dlgInstance;
	camcontrolsParamDlg (void *r) { ref = (camcontrol *)r; dlgInstance = this;}
	~camcontrolsParamDlg () { dlgInstance = NULL; ParamMapUserDlgProc::~ParamMapUserDlgProc(); }

	void Update (void); // internally called
	void Update (TimeValue t); // max called
	void DeleteThis() { delete this; }
	BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
};

/* Creation of a flags object */
class camcontrolCreatorClass : public CreateMouseCallBack
{
private:
	camcontrol		*ref;
public:
	camcontrolCreatorClass() {}
	void setRef (CAMCONTROLS *newRef) 
	{ ref = (camcontrol *)newRef; }

	int proc (ViewExp *vpt, int msg, int point, 
		int flags, IPoint2 m, Matrix3& mat);
};




class camcontrol : public CAMCONTROLS
{
private:
   
	//static TSTR		name;
	friend class	camcontrolsDesc;
	friend class	camcontrolCreatorClass;
	friend class	camcontrolsParamDlg;
	friend class	RDObjectExporter;
	Mesh				objMesh;						// The mesh of this object, if any

public:
/*  	float	upxpos;
  	float	upypos;
  	float	upzpos;
  	float	downxpos;
  	float	downypos;
  	float	downzpos;*/

	float amount;
	float speed;
	camcontrol ();
	virtual void WindowFinal (IObjParam *ip);
	virtual void DrawMarker (GraphicsWindow *gw, bool Selected = FALSE, bool Frozen = FALSE);
	virtual RefTargetHandle Clone (RemapDir& remap = NoRemap())
	{ 
		camcontrol *ret = new camcontrol();
		ret->ReplaceReference (0, pblock->Clone (remap));
		return (ret);
	}
	BOOL SetValue(int i, TimeValue t, float v) { return FALSE; }
	BOOL SetValue(int i, TimeValue t, Point3 &v) { return FALSE; }
	BOOL GetValue(int i, TimeValue t, float &v, Interval &ivalid) { return FALSE; }
	BOOL GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid) { return FALSE; }
	Class_ID ClassID() { return camcontrols_CLASS_ID; }

	
	void Reset();
	IOResult Load (ILoad *iload);
	IOResult Save (ISave *isave);

	int CAMNO;
	int FOUND;


	void FindCamNode(INode *node)
	{
		int i,numchild;
		INode *oldnode=node;
		
		numchild = node->NumberOfChildren();

		if (!FOUND)
		{
			Object *obj = node->GetObjectRef();
			camcontrol *object = (camcontrol*)obj;
//			line that was fucking things up
//			camcontrol *object = (camcontrol*)(node->EvalWorldState(GetCOREInterface()->GetTime()).obj);
		  	if (object)
				if ((object->ClassID() == camcontrols_CLASS_ID))
					CAMNO++;
		   	if (object == this)
			{
				FOUND++;
				return;
			}  

		}

		if (!numchild)
			return;

		for (i=0;i<numchild;i++)
		{
			node = oldnode->GetChildNode(i);
			FindCamNode(node);
		}
	  
	}


	/*RETURNS THE INDEX INTO THE CAMCONTROL ARRAY THAT WILL BE OUTPUT BY THE STRATMAP EXPORTER*/
	int GetMyCamIndex ()
	{
		//PARSE THE SCENE LOOKING FOR THE NODE THAT IS THIS CAMERA

		INode *newnode = GetCOREInterface()->GetRootNode();

		CAMNO = 0;
		FOUND = 0;
	 
	 	FindCamNode(newnode);
		return(CAMNO-1);

	}


/*	static int GetLightValue(int i)
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
  */

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
   //	IOResult Load(ILoad *iload);
	
};


extern ClassDesc *GetcamcontrolDesc();

#endif
