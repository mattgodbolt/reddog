#ifndef MY_MATERIAL_H
#define MY_MATERIAL_H


/*
 * The references
 */
#define REF_PARAMBLOCK			0
#define REF_TEXTURE				1
#define REF_PASTED				2
#define REF_CAMERA				3
#define REF_SUBMATERIAL			4
#define REF_NUM					5

/*
 * MattG's Max Material
 */
#include "IParamM.h"
#include "camcontrol.h"
extern ClassDesc *GetMaterialDesc();
#define MGMATERIAL_CLASS_ID Class_ID(0x14263606, 0x60aa2da4)
//#define MGMATERIAL_CLASS_ID Class_ID(0x1426ff06, 0x60aa2dff)

//UNIQUE #define MGMATERIAL_CLASS_ID Class_ID(0x27224C2e, 0x55f44f50)
class MGMaterial; class MGMaterialDlg;

/*
 * The dialog which deals with it
 */
class MGMaterialDlg : public ParamDlg
{
private:
	HWND			mtlWindow;		// Window handle of the material browser
	HWND			panel;			// The panel
	BOOL			isActive;		// Whether the material dlg is active
	MGMaterial		*mat;			// The material we're editing
	IMtlParams		*ip;			// Interface pointer
	TexDADMgr		dadMgr;			// Drag and drop manager
	MtlDADMgr		dadMgrConv;		// For conversion...
	IParamMap		*stdMap;
	ICustButton		*texBut, *pasteBut, *camBut, *convBut;
	HWND			matType;
public:
	// Constructor
	MGMaterialDlg (HWND w, IMtlParams *params, MGMaterial *m);
	~MGMaterialDlg ();
	// Reload text into the dialog
	void ReloadDialog ()
	{ LoadDialog (TRUE); }
	// Panel update procedure
	BOOL BasicPanelProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
	void LoadDialog(BOOL draw);  // stuff params into dialog
	// Get/set the 'thing' we're editing
	ReferenceTarget *GetThing () { return (ReferenceTarget *)mat; }
	void SetThing (ReferenceTarget *target);
	// Delete this
	void DeleteThis() { delete this; }
	void SetTime (TimeValue t) { }
	Class_ID ClassID() { return MGMATERIAL_CLASS_ID; }
	// Colour swatch notification
	void ActivateDlg(BOOL onOff);
	// Update the submats
	void UpdateSubMtlNames();
	// Window proc
	BOOL WndProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	inline BOOL Active() const { return isActive; }
	inline void SetActive(BOOL b) { isActive = b; }
	int FindSubTexFromHWND(HWND hw) {
		if (texBut->GetHwnd()==hw) return 0;
		if (pasteBut->GetHwnd()==hw) return 1;
		return -1;
	}
	int FindSubMtlFromHWND(HWND hw) {
		if (convBut->GetHwnd()==hw) return 0;
		return -1;
	}
};

/*
 * The material class itself
 */
class MGMaterial : public Mtl
{
private:
	// The RDMaterial we are responsible for
	RDMaterial		material;
	MGMaterialDlg	*paramDlg;
	IParamBlock		*pblock;
	Texmap			*texture, *pasted;
	Mtl				*subMtl;
	bool			resetting;
	friend class	MGMaterialDlg;
	friend class	MGMaterialFixer;
	char			camname[64];
public:
	INode *CamNode;

	// Constructor
	MGMaterial (BOOL loading)
	{
		paramDlg = NULL;
		pblock = NULL;
		pasted = texture = NULL;
		CamNode = NULL;
		subMtl = NULL;
		resetting = false;
		if (!loading) {
			Reset();
	 		if (GetReference(REF_SUBMATERIAL))
				ReplaceReference(REF_SUBMATERIAL, NULL);
		}
	}

	RDMaterial getMat() { return material; }

	// Set material from a Mtl
	void setMat(Mtl *m);

	// Reference stuff
	void UpdateRDMaterial();
	RefResult NotifyRefChanged (Interval i, RefTargetHandle target, PartID &id, RefMessage message)
	{ 
		if (message == REFMSG_TARGET_DELETED) {
			if ((INode*)target == CamNode)	
				CamNode = NULL;
		} else if (message == REFMSG_CHANGE) {
			if (paramDlg && !paramDlg->Active())
				paramDlg->ReloadDialog();
		}
		UpdateRDMaterial();
	  	return REF_SUCCEED; 
	}
	void NotifyChanged() { 
	  	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);		
	  	UpdateRDMaterial();
	  	if (paramDlg && !paramDlg->Active())
	  		paramDlg->ReloadDialog();
	}
	int NumRefs () 
	{ return REF_NUM; }
	RefTargetHandle GetReference (int i)
	{ 
	  	switch (i) {
		case REF_PARAMBLOCK:
			return pblock; 
		case REF_TEXTURE:
			return texture;
		case REF_PASTED:
			return pasted;
		case REF_CAMERA:
			return CamNode;
		case REF_SUBMATERIAL:
			return subMtl;
		}  
		return NULL;
	
	}
	void SetReference (int i, RefTargetHandle target)
	{
		switch (i) {
		case REF_PARAMBLOCK:
			pblock = (IParamBlock *)target;
			break;
		case REF_TEXTURE:
			texture = (Texmap *)target;
			break;
		case REF_PASTED:
			pasted = (Texmap *)target;
			break;
		case REF_CAMERA:
			CamNode = (INode*)target;
		   	UpdateRDMaterial();	   
		   //	obj= (camcontrol*)CamNode->EvalWorldState(GetCOREInterface()->GetTime()).obj;
		   //	obj->GetMyCamIndex();
			break;

		case REF_SUBMATERIAL:
			subMtl = (Mtl *)target;
			break;

		default:
			return;
		}
		
	}

	RefTargetHandle Clone(RemapDir &remap)
	{
		MGMaterial *mtl = new MGMaterial(FALSE);
		*((MtlBase*)mtl) = *((MtlBase*)this);  // copy superclass stuff
		mtl->ReplaceReference(REF_PARAMBLOCK,remap.CloneRef(pblock));
		mtl->ReplaceReference(REF_TEXTURE,remap.CloneRef(texture));
		mtl->ReplaceReference(REF_PASTED,remap.CloneRef(pasted));
//		mtl->ReplaceReference(REF_CAMERA,CamNode);
		// Cloning loses camera information
		mtl->ReplaceReference(REF_CAMERA,NULL);
		mtl->ReplaceReference(REF_SUBMATERIAL,remap.CloneRef(subMtl));
		return mtl;
	}

	// Animatable stuff
	Class_ID ClassID() {return MGMATERIAL_CLASS_ID; }
	SClass_ID SuperClassID() {return MATERIAL_CLASS_ID;}
	void GetClassName(TSTR& s) {s = GetString(IDS_MATERIAL);}  

	// MtlBase
	void Update (TimeValue t, Interval &valid)
	{
		valid = FOREVER; 
		if (texture)
			texture->Update (t, valid);
		if (pasted)
			pasted->Update (t, valid);
		if (subMtl)
			subMtl->Update (t, valid);
		UpdateRDMaterial();
	}
	void Reset(void);

	Interval Validity (TimeValue t) 
	{ return FOREVER; }
	ParamDlg *CreateParamDlg (HWND window, IMtlParams *imp)
	{ return (paramDlg = new MGMaterialDlg (window, imp, this)); }
	int NumSubs() { return REF_NUM; }
	int SubNumToRefNum(int subNum) { return subNum; }

	Animatable *SubAnim(int i)
	{
		switch (i) {
		case REF_PARAMBLOCK: return pblock;
		case REF_TEXTURE: return texture;
		case REF_PASTED: return pasted;
		case REF_CAMERA: return CamNode;
		case REF_SUBMATERIAL: return subMtl;
		default: return NULL;
		}
	}
	TSTR SubAnimName(int i)
	{
		switch (i) {
		case REF_PARAMBLOCK: return GetString(IDS_PARAMS);
		case REF_TEXTURE: return GetString(IDS_TEXTURE);
		case REF_PASTED: return GetString(IDS_PASTED);
		case REF_CAMERA: return _T("Camera");
		case REF_SUBMATERIAL: return GetString(IDS_SUBMAT);
		default: return _T("");
		}
	}
   	int NumSubTexmaps() {return 2;}
	Texmap *GetSubTexmap(int i) {return i?pasted:texture;}
	void SetSubTexmap(int i, Texmap *m)
	{ 
		ReplaceReference(i+1,m); 
		if (paramDlg) paramDlg->UpdateSubMtlNames();
	}
	TSTR GetSubTexmapSlotName(int i) {
		return i?GetString(IDS_PASTED):GetString(IDS_TEXTURE);
	}
	TSTR GetSubTexmapTVName (int i) {
		return GetSubTexmapSlotName(i);
	}
	int NumSubMtls() { return 1; }
	void SetSubMtl(int i, Mtl *m)
	{
		ReplaceReference(REF_SUBMATERIAL, m);
		setMat(subMtl);
		NotifyChanged();
	}
	Mtl *GetSubMtl(int i)
	{
		return subMtl;
	}
	TSTR GetSubMtlSlotName (int i) {
		return GetString(IDS_SUBMAT);
	}
	TSTR GetSubMtlTVName (int i) {
		return GetSubMtlSlotName(i);
	}
	int MapSlotType (int i) { return MAPSLOT_TEXTURE; }
	ULONG Requirements (int i)
	{
		ULONG retVal = 0;
		if (material.flags & RDMF_TWOSIDED)
			retVal |= MTLREQ_2SIDE;
		if (material.opacity < 1.f)
			retVal |= MTLREQ_TRANSP;
		if (texture || pasted)
			retVal |= MTLREQ_UV;
		if (material.additive)
			retVal |= MTLREQ_ADDITIVE_TRANSP;
		return retVal;
	}
	ULONG LocalRequirements (int i)
	{
		return Requirements (i);
	}
	int SubTexmapOn (int i)
	{
		if (i==0 && texture)
			return TRUE;
		if (i==1 && pasted)
			return TRUE;
		return FALSE;
	}

	// Colour methods
	Colour GetAmbient (int mtlId, BOOL backface)
	{ return Colour(0,0,0); }
	Colour GetDiffuse (int mtlId, BOOL backface)
	{ return material.diffuse; }
	Colour GetSpecular (int mtlId, BOOL backface)
	{ return material.specular; }
	void SetAmbient (Colour c, TimeValue t)
	{ }
	void SetDiffuse (Colour c, TimeValue t)
	{ material.diffuse = c; }
	void SetSpecular (Colour c, TimeValue t)
	{ material.specular = c; }

	// Shininess stuff
	float GetShininess (int mtlId, BOOL backface)
	{ return material.specExponent; }
	float GetShinStr (int mtlId, BOOL backface)
	{ return material.specAmount; }
	float GetXParency (int mtlId, BOOL backface)
	{ return 0.f; }
	void SetShininess (float v, TimeValue t)
	{ material.specExponent = v; }
	void SetShinStr (float v, TimeValue t)
	{ material.specAmount = v; }
	void SetXParency (float v, TimeValue t)
	{  }

	// Shade
	void Shade (ShadeContext &sc);
	IOResult Load(ILoad *load);

	// DeleteThis
	void DeleteThis() { delete this; }
};


#endif
