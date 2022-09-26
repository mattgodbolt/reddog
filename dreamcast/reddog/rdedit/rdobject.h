/*
 * RDObject.h
 * Interface to placing Red Dog objects onto the map
 */

#ifndef _RDOBJECT_H
#define _RDOBJECT_H

#include <max.h>
#include <simpobj.h>
#include "util.h"
#include "resource.h"
#include "MeshSupt.h"
#include "polyshp.h"
#include "BBox.h"
#include "CollPoint.h"

extern void	SetUpNodeChoice(RDObject *ref,TimeValue t,HWND win);

extern ClassDesc* GetRDObjectDesc();
extern ClassDesc* GetRDObjectExportDesc();

#define RDOBJECT_CLASS_ID			Class_ID(0x99b1000, 0xb944ac0)
#define RDOBJECT_CAMCLASS_ID		Class_ID(0xd9b1330, 0xc77bb0a)
#define RDOBJECTEXPORT_CLASS_ID	Class_ID(0x7f1371e9, 0x28091370)
#define RDO_VERSION 100

//#define OBJECT_ROOT				"P:\\GAME\\OBJECTS"

#if ARTIST
	#define STRAT_ROOT				"P:\\DREAMCAST\\REDDOG\\STRATS"
#else
#if GODMODEMACHINE
	#define STRAT_ROOT				"D:\\DREAMCAST\\REDDOG\\GAME\\STRATS"
#else

	#define STRAT_ROOT				"C:\\DREAMCAST\\REDDOG\\GAME\\STRATS"
#endif
#endif
#define STRAT_OBJECT_ROOT		"P:\\GAME\\NEWOBJECTS"

//WAYPOINT DEFINES

#define WAY_NORM	1
#define WAY_OFF		2

#define MAX_PARAMS 32

class PickOperand;

typedef union value
{
	float	fval;
	int		ival;
} VALUE;


typedef struct param
{
	VALUE	value;
	VALUE	def;
	char	*paramname;


} PARAM;
 
#define CAMCONTROL 1

class RDObject : public SimpleObject, public IParamArray
{
public:
	/*
	 * Public data types
	 */
	enum DisplayType {
		DISPLAY_BOX, DISPLAY_ARROW, DISPLAY_MESH
	};
private:
	/*
	 * Class variables
	 */
	static IParamMap	*pmapParam;						// the default parameters
//	static IObjParam	*ip;							// A handle to call MAX with
	static BOOL			dlgHasStrat;					// Whether it has a strat (dialog)
	static char			dlgObjSource[MAX_FNAME_SIZE];	// Which object this is (dialog)
	static DisplayType	dlgDisplayType;					// The thing displayed (Dialog)

//	BOOL				hasStrat;
	char				objSource[MAX_FNAME_SIZE];
	DisplayType			displayType;
	Mesh				objMesh;						// The mesh of this object, if any
 //	int			StartNode;
 //	int			StartPath;

	//MATT'S JOLLY JAPES

	static char	dlgStratSource[MAX_FNAME_SIZE];	// The strat attached to the object
	int			flag;							//The strat's attached flag
	int			wayflag;						//The strat's path's type
	static		PickOperand pickCB;				//Link to pick type
	float		armour;							//ARMOUR FIELD
	float		health;							//HEALTH FIELD
	int			MeshLoaded;
	/*
	 * Friends (..I'll be there for you...CAN WE GET RID OF REFS TO SHITE US SITCOMS, OR THERE
	 *									  WILL HAVE TO BE MUCHO SPILLAGE OF BLOOD..LOVE JOEY +
	 *									  THAT BLONDE TART WITH DIRTY HAIR
	 *			)
	 */
	friend class RDObjectMouseCallBack;
	friend class RDStratExportEnumerator;
	friend class RDObjectClassDesc;
	friend class RDObjectParamMapUserDlgProc;
public:
	char		stratSource[MAX_FNAME_SIZE];
	/*
	 * Constructors and destructor
	 */
	RDObject ();
	~RDObject ();

	/*
	 * Object methods
	 */
	INode *parent;
	INode *netpath;
	INode *waypoint;
	INode *eventpoint;
	INode *deletioneventpoint;
	INode *trigpoint;
	INode *actpoint[5];
//	int   paraminterr;
//	int   paramfloaterr;

	PARAM floatparams[MAX_PARAMS];
	PARAM intparams[MAX_PARAMS];

	//for internal processing reasons

	int	  internalintmax;
	int	  internalfloatmax;
	PARAM Internalintparams[MAX_PARAMS];
	PARAM Internalfloatparams[MAX_PARAMS];


   //	char  *paramfloats[MAX_PARAMS];
   //	float  paramfloatvals[MAX_PARAMS];
   //	float  paramfloatdefs[MAX_PARAMS];
   //	char  *paramints[MAX_PARAMS];
   //	int	  paramintvals[MAX_PARAMS];
   //	int	  paramintdefs[MAX_PARAMS];
	static IObjParam	*ip;							// A handle to call MAX with
	Object* ConvertToType (TimeValue t, Class_ID obtype);
	int CanConvertToType (Class_ID obtype);
	void GetCollapseTypes (Tab<Class_ID> &clist,Tab<TSTR*> &nlist);
 	int			node;							//THE START NODE IN THE ATTACHED SPLINE PATH
 	int			nodemax;							//THE START NODE IN THE ATTACHED SPLINE PATH
 	int			path;							//THE START NODE IN THE ATTACHED SPLINE PATH
 	int			pathmax;							//THE START NODE IN THE ATTACHED SPLINE PATH
	int			entirenet;						//CAN THE STRAT UTILISE THE ENTIRE NETWORK IT IS ON ?
	int			ownrot;						  	//DOES THE STRAT TAKE THE PATH'S ROTATION ?
 	int         ownpos;
	int			floatparam;
	int			floatparammax;
	int			intparam;
	int			intparammax;
	Matrix3		StoreTransform;					//temp holder for matrix
 //	int			Type;
	short		delay;
	/*
	 * BaseObject methods
	 */
	CreateMouseCallBack *GetCreateMouseCallBack();
	void BeginEditParams (IObjParam *ip,ULONG flags,Animatable *prev);
	void EndEditParams (IObjParam *ip, ULONG flags,Animatable *next);
	TCHAR *GetObjectName() { return GetString (IDS_OBJECT_NAME); }
//	BOOL HasUVW() { return FALSE; }
	int Display (TimeValue t, INode *node, ViewExp *view, int flags);
//	BOOL HasSTRAT() { return FALSE; }



	// From IParamArray
	BOOL SetValue(int i, TimeValue t, int v);
	BOOL SetValue(int i, TimeValue t, float v);
	BOOL SetValue(int i, TimeValue t, Point3 &v);
	BOOL GetValue(int i, TimeValue t, int &v, Interval &ivalid);
	BOOL GetValue(int i, TimeValue t, float &v, Interval &ivalid);
	BOOL GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid);

	/*
	 * From ReferenceMaker
	 */
	RefTargetHandle Clone (RemapDir& remap = NoRemap());
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);		
	int NumRefs() {return 12;}
	RefResult NotifyRefChanged(Interval i, RefTargetHandle rtarg, PartID& partID, RefMessage mesage);

//	RefTargetHandle GetReference(int i) {return pblock;}
	IOResult Load (ILoad *iload);
	IOResult Save (ISave *isave);

	/*
	 * Animatable thangs
	 */
	void Delete();
   //	void Update (void); // internally called
	void DeleteThis() { delete this; }
	Class_ID ClassID() { return RDOBJECT_CLASS_ID; }

	/* 
	 * From Simple
	 */
	void BuildMesh(TimeValue t);
//	BOOL OKtoDisplay(TimeValue t) { return TRUE; }
	void InvalidateUI();
	ParamDimension *GetParameterDim(int pbIndex);
	TSTR GetParameterName(int pbIndex);		


	/*
	 * Operators
	 */
	inline int operator == (const RDObject &obj) { return (!stricmp(objSource,obj.objSource)); }
	inline int operator != (const RDObject &obj) { return (stricmp(objSource,obj.objSource)); }
	int			actnum;							//CURRENT ACTIVATION POINT

	INode *MaxNode;

//	void Clean();

	void GetNode(INode *node)
	{
		int i,numchild;
		INode *oldnode=node;
		
		numchild = node->NumberOfChildren();

	 // 	Object *object = node->GetObjOrWSMRef();
		Object *object = node->GetObjectRef();
	  	if (object)
		{
			object = object->FindBaseObject();
			RDObject *robject = (RDObject*)object;
		  	if (robject == this)
		  	{
		  		MaxNode = node;
		  		return;
		  	}
		}
		if (!numchild)
			return;

		for (i=0;i<numchild;i++)
		{
			node = oldnode->GetChildNode(i);
			GetNode(node);
		}






	}



	//RECURSIVE SCENE SEARCH FOR THE OBJECTS NODE
	INode *GetMyNode()
	{

		/*INode *childnode;*/
		//enumerate through the scene until root child = the node we are dealing with 'ref'
		MaxNode = NULL;

		INode *newnode = GetCOREInterface()->GetRootNode();
		int numchild = newnode->NumberOfChildren();


	 
		GetNode(newnode);

		return(MaxNode);
		

	  /*	int found = 0;
		for (int i=0;i<numchild;i++)
		{
			childnode = newnode->GetChildNode(i);
			Object *object = childnode->EvalWorldState(GetCOREInterface()->GetTime()).obj;
			if (object == this)
				return(childnode);

			int numchild = childnode->NumberOfChildren();
			if (numchild)
			{
				INode *subnode = childnode;
				for (int loop=0;loop<numchild;loop++)
				{

					childnode = subnode->GetChildNode(loop);
					Object *object = childnode->EvalWorldState(GetCOREInterface()->GetTime()).obj;
					if (object == this)
						return(childnode);
				}
			}

		}
	   return(NULL);	
		*/
	}						 
	

};

class PickOperand : 
		public	PickModeCallback,
		public PickNodeCallback {
	public:		
		RDObject *pickObj;
		
		PickOperand() {pickObj=NULL;};

		BOOL HitTest(IObjParam *ip,HWND hWnd,ViewExp *vpt,IPoint2 m,int flags);
		BOOL Pick(IObjParam *ip,ViewExp *vpt);
		BOOL RightClick(IObjParam *ip,ViewExp *vpt);
		void EnterMode(IObjParam *ip);
		void ExitMode(IObjParam *ip);
		BOOL GetDefCursor() {return NULL;}
		BOOL GetHitCursor() {return NULL;}
		 PickNodeCallback *GetFilter() {return NULL;}
		BOOL AllowMultiSelect() {return FALSE;}
		BOOL Filter(INode *node);


};


class RDObjectParamDlg : public ParamMapUserDlgProc
{
private:
  	RDObject	*ref;
	static HWND instHwnd;
	static ICustEdit *nameCtrl;
public:
	static ISpinnerControl 
		*armourSpinner,*healthSpinner,*nodeSpinner,*routeSpinner,
		*intparamSpinner,*floatparamSpinner,*delaySpinner;
	static RDObjectParamDlg *dlgInstance;
   	RDObjectParamDlg (void *r) { ref = (RDObject *)r; dlgInstance = this;}
   	~RDObjectParamDlg () { dlgInstance = NULL; ParamMapUserDlgProc::~ParamMapUserDlgProc(); }

	void Update (void); // internally called
	void Update (TimeValue t); // max called
	void DeleteThis() { delete this; }
	BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
};




/*
 * An object as it is exported
 */
class RDObjectExport : public MeshSupport
{
private:
	RDPoly					*polygons;
	BBoxBounds				boundary;
	Point3					translation;
	Point3					scale;
	Point3					rotation;
	Matrix3					localTM;
	int						CollFlags;
	int						Id;
	int						ModelFlags;
	Tab<RDObjectExport *>	children;
	Tab<CollPt>				collisionPoints;
public:
	INode					*node;

	/*
	 * Constructor(s)
	 */
	RDObjectExport () : boundary(0,0,0,0,0,0), translation (0,0,0), 
		rotation(0,0,0), localTM(TRUE), CollFlags(0), Id(0), ModelFlags(0)
	{
		// here matt - the scale value is here!
		float scaleVal = 1.0f; // (float)GetMasterScale (UNITS_METERS);
		scale.x = scale.y = scale.z = scaleVal;
		polygons = NULL; 
		node = GetCOREInterface()->GetRootNode();
	}
	RDObjectExport (Interface *maxInterface, INode *node, RDObjectExport *parent);
	virtual ~RDObjectExport() 
	{ 
		if (polygons) delete polygons; 
		for (int i = 0; i < children.Count(); ++i)
			delete children[i];
	}

	/*
	 * Methods
	 */

	// Outputs the converted mesh
	void Output (FILE *f);
	// Prepares the mesh
	void Prepare ();
	// Optimises the heirarchy - returns TRUE if it needs to be deleted
	bool Optimise (RDObjectExport *parent = NULL);
	// Converts the textures to PVR
	void Convert (ofstream &o);
};


void UpdateNodeName(RDObject *ref,char *source);
extern char  *WhiteSpaceRead(char *cursor);
extern char *ReadToEnd(char *dest,char*src);
extern void Clear(char *input,int amount);

#endif
