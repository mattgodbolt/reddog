/* Nicely stolen and modified from MAX by MattG */
// And now changed such that shadow boxes == prelighting box
#include "stdafx.h"
#include "ShadBox.h"

class ShadBoxObject : public IShadBox {
public:
	// Class vars
	static IParamMap *pmapCreate;
	static IParamMap *pmapParam;		
	static IObjParam *ip;
	static int createMeth;
	static float crtWidth, crtHeight, crtLength, crtValue, crtPriority;
	
	::Matrix3 mat;

	ShadBoxObject();
	
	// From BaseObject
	CreateMouseCallBack* GetCreateMouseCallBack();
	void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
	void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
	TCHAR *GetObjectName() { return GetString(IDS_SHADBOX); }
	BOOL HasUVW();
	void SetGenUVW(BOOL sw);
	
	// Animatable methods
	void DeleteThis() { delete this; }
	Class_ID ClassID() { return SHADBOX_CLASSID; }  
	SClass_ID SuperClassID() { return HELPER_CLASS_ID; }
	int IsRenderable() { return FALSE; }
	int UsesWireColor() { return TRUE; }
	
	// From ref
	RefTargetHandle Clone(RemapDir& remap = NoRemap());
	
	// From IParamArray
	BOOL SetValue(int i, TimeValue t, int v);
	BOOL SetValue(int i, TimeValue t, float v);
	BOOL SetValue(int i, TimeValue t, Point3 &v);
	BOOL GetValue(int i, TimeValue t, int &v, Interval &ivalid);
	BOOL GetValue(int i, TimeValue t, float &v, Interval &ivalid);
	BOOL GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid);
	
	// From SimpleObject
	void BuildMesh(TimeValue t);
	BOOL OKtoDisplay(TimeValue t);
	void InvalidateUI();
	ParamDimension *GetParameterDim(int pbIndex);
	TSTR GetParameterName(int pbIndex);	
	virtual void SetMat (const ::Matrix3 &mat);
	virtual bool InBox(Point3 &p);
	virtual float Value();
	virtual float Priority();
};				



#define BOTTOMPIV

#define BMIN_LENGTH		float(0)
#define BMAX_LENGTH		float(1.0E30)
#define BMIN_WIDTH		float(0)
#define BMAX_WIDTH		float(1.0E30)
#define BMIN_HEIGHT		float(-1.0E30)
#define BMAX_HEIGHT		float(1.0E30)

#define BDEF_DIM		float(0)

//--- ClassDescriptor and class vars ---------------------------------

class BoxObjClassDesc:public ClassDesc {
public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) {return new ShadBoxObject;}
	const TCHAR *	ClassName() { return GetString(IDS_SHADBOX); }
	SClass_ID		SuperClassID() { return HELPER_CLASS_ID; }
	Class_ID		ClassID() { return SHADBOX_CLASSID; }
	const TCHAR* 	Category() { return GetString(IDS_HELPER_CATEGORY);}	
	void			ResetClassParams(BOOL fileReset);
};

static BoxObjClassDesc boxObjDesc;

ClassDesc* GetBoxobjDesc() { return &boxObjDesc; }

// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;

// class variable for sphere class.
IObjParam *ShadBoxObject::ip         = NULL;
IParamMap *ShadBoxObject::pmapCreate = NULL;
IParamMap *ShadBoxObject::pmapParam  = NULL;	
float ShadBoxObject::crtWidth        = 0.0f; 
float ShadBoxObject::crtHeight       = 0.0f;
float ShadBoxObject::crtLength       = 0.0f;
float ShadBoxObject::crtValue        = 1.0f;
float ShadBoxObject::crtPriority	 = 1.0f;
int ShadBoxObject::createMeth        = 0;

void BoxObjClassDesc::ResetClassParams(BOOL fileReset)
{
	ShadBoxObject::crtWidth   = 0.0f; 
	ShadBoxObject::crtHeight  = 0.0f;
	ShadBoxObject::crtLength  = 0.0f;
	ShadBoxObject::createMeth = 0;
	ShadBoxObject::crtValue   = 1.0f;
	ShadBoxObject::crtPriority= 1.0f;
}

//--- Parameter map/block descriptors -------------------------------

// Parameter block indices
#define PB_LENGTH	0
#define PB_WIDTH	1
#define PB_HEIGHT	2
#define PB_VALUE	3
#define PB_PRIORITY	4

// Non-parameter block indices
#define PB_CREATEMETHOD		0

//
//
//	Creation method

static int createMethIDs[] = {IDC_CREATEBOX,IDC_CREATECUBE};

static ParamUIDesc descCreate[] = {
	// Diameter/radius
	ParamUIDesc(PB_CREATEMETHOD,TYPE_RADIO,createMethIDs,2)
};
#define CREATEDESC_LENGTH 1

//
//
// Parameters

static ParamUIDesc descParam[] = {
	// Length
	ParamUIDesc(
		PB_LENGTH,
		EDITTYPE_UNIVERSE,
		IDC_LENGTHEDIT,IDC_LENSPINNER,
		BMIN_LENGTH,BMAX_LENGTH,
		SPIN_AUTOSCALE),
		
		// Width
		ParamUIDesc(
		PB_WIDTH,
		EDITTYPE_UNIVERSE,
		IDC_WIDTHEDIT,IDC_WIDTHSPINNER,
		BMIN_WIDTH,BMAX_WIDTH,
		SPIN_AUTOSCALE),	
		
		// Height
		ParamUIDesc(
		PB_HEIGHT,
		EDITTYPE_UNIVERSE,
		IDC_HEIGHTEDIT,IDC_HEIGHTSPINNER,
		BMIN_HEIGHT,BMAX_HEIGHT,
		SPIN_AUTOSCALE),	
		
		// Value
		ParamUIDesc(
		PB_VALUE,
		EDITTYPE_UNIVERSE,
		IDC_LIGHTEDIT,IDC_LIGHTSPIN,
		0, 1, 
		SPIN_AUTOSCALE),	
		
		// Priority
		ParamUIDesc(
		PB_PRIORITY,
		EDITTYPE_UNIVERSE,
		IDC_PRIEDIT,IDC_PRISPIN,
		0, 100, 
		SPIN_AUTOSCALE),	
		
};
#define PARAMDESC_LENGTH 5


ParamBlockDescID descVer0[] = {
	{ TYPE_FLOAT, NULL, FALSE, PB_LENGTH },
	{ TYPE_FLOAT, NULL, FALSE, PB_WIDTH },
	{ TYPE_FLOAT, NULL, FALSE, PB_HEIGHT }, 
	{ TYPE_FLOAT, NULL, FALSE, PB_VALUE },
	{ TYPE_FLOAT, NULL, FALSE, PB_PRIORITY }, 
};

#define PBLOCK_LENGTH	5

// Array of old versions
/*static ParamVersionDesc versions[] = {
};
#define NUM_OLDVERSIONS	0
*/
#define CURRENT_VERSION	0
static ParamVersionDesc curVersion(descVer0,PBLOCK_LENGTH,CURRENT_VERSION);


//--- Box methods -------------------------------

ShadBoxObject::ShadBoxObject()
{
	MakeRefByID(FOREVER, 0, CreateParameterBlock(descVer0, PBLOCK_LENGTH, CURRENT_VERSION));
	assert (pblock);
	
	pblock->SetValue(PB_LENGTH,0,crtLength);
	pblock->SetValue(PB_WIDTH,0,crtWidth);
	pblock->SetValue(PB_HEIGHT,0,crtHeight);
	pblock->SetValue(PB_VALUE,0,crtValue);
	pblock->SetValue(PB_PRIORITY,0,crtPriority);
}

void ShadBoxObject::BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev )
{
	SimpleObject::BeginEditParams(ip,flags,prev);
	this->ip = ip;
	
	if (pmapCreate && pmapParam) {
		
		// Left over from last Box ceated
		pmapCreate->SetParamBlock(this);
		pmapParam->SetParamBlock(pblock);
	} else {
		
		// Gotta make a new one.
		if (flags&BEGIN_EDIT_CREATE) {
			pmapCreate = CreateCPParamMap(
				descCreate,CREATEDESC_LENGTH,
				this,
				ip,
				hInstance,
				MAKEINTRESOURCE(IDD_BOXPARAM1),
				GetString(IDS_SHADBOX_CREATIONMETHOD),
				0);
			
		}
		pmapParam = CreateCPParamMap(
			descParam,PARAMDESC_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_BOXPARAM2),
			GetString(IDS_SHADBOX_PARMS),
			0);
	}
}

void ShadBoxObject::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
{
	SimpleObject::EndEditParams(ip,flags,next);
	this->ip = NULL;
	
	if (flags&END_EDIT_REMOVEUI ) {
		if (pmapCreate) DestroyCPParamMap(pmapCreate);
		DestroyCPParamMap(pmapParam);
		pmapParam  = NULL;
		pmapCreate = NULL;
	}
	
}

// vertices ( a b c d ) are in counter clockwise order when viewd from 
// outside the surface unless bias!=0 in which case they are clockwise
static void MakeQuad(int nverts, Face *f, int a, int b , int c , int d, int sg, int bias) {
	int sm = 1<<sg;
	assert(a<nverts);
	assert(b<nverts);
	assert(c<nverts);
	assert(d<nverts);
	if (bias) {
		f[0].setVerts( b, a, c);
		f[0].setSmGroup(sm);
		f[0].setEdgeVisFlags(1,0,1);
		f[1].setVerts( d, c, a);
		f[1].setSmGroup(sm);
		f[1].setEdgeVisFlags(1,0,1);
	} else {
		f[0].setVerts( a, b, c);
		f[0].setSmGroup(sm);
		f[0].setEdgeVisFlags(1,1,0);
		f[1].setVerts( c, d, a);
		f[1].setSmGroup(sm);
		f[1].setEdgeVisFlags(1,1,0);
	}
}


#define POSX 0	// right
#define POSY 1	// back
#define POSZ 2	// top
#define NEGX 3	// left
#define NEGY 4	// front
#define NEGZ 5	// bottom

int direction(Point3 *v) {
	Point3 a = v[0]-v[2];
	Point3 b = v[1]-v[0];
	Point3 n = CrossProd(a,b);
	switch(MaxComponent(n)) {
	case 0: return (n.x<0)?NEGX:POSX;
	case 1: return (n.y<0)?NEGY:POSY;
	case 2: return (n.z<0)?NEGZ:POSZ;
	}
	return 0;
}

// Remap the sub-object material numbers so that the top face is the first one
// The order now is:
// Top / Bottom /  Left/ Right / Front / Back
static int mapDir[6] ={ 3, 5, 0, 2, 4, 1 };

#define MAKE_QUAD(na,nb,nc,nd,sm,b) {MakeQuad(nverts,&(mesh.faces[nf]),na, nb, nc, nd, sm, b);nf+=2;}

BOOL ShadBoxObject::HasUVW() { 
	return FALSE; 
}

void ShadBoxObject::SetGenUVW(BOOL sw) {  
	return;
}


void ShadBoxObject::BuildMesh(TimeValue t)
{
	int ix,iy,iz,nf,kv,mv,nlayer,topStart,midStart;
	int nverts,wsegs,lsegs,hsegs,nv,nextk,nextm,wsp1;
	int nfaces;
	Point3 va,vb,p;
	float l, w, h;
	int genUVs = 1;
	BOOL bias = 0;
	
	// Start the validity interval at forever and widdle it down.
	ivalid = FOREVER;	
	pblock->GetValue(PB_LENGTH,t,l,ivalid);
	pblock->GetValue(PB_WIDTH,t,w,ivalid);
	pblock->GetValue(PB_HEIGHT,t,h,ivalid);
	lsegs = wsegs = hsegs = 1;
	genUVs = FALSE;
	if (h<0.0f) bias = 1;
	
	// Number of verts
	// bottom : (lsegs+1)*(wsegs+1)
	// top    : (lsegs+1)*(wsegs+1)
	// sides  : (2*lsegs+2*wsegs)*(hsegs-1)
	
	// Number of rectangular faces.
	// bottom : (lsegs)*(wsegs)
	// top    : (lsegs)*(wsegs)
	// sides  : 2*(hsegs*lsegs)+2*(wsegs*lsegs)
	
	wsp1 = wsegs + 1;
	nlayer  =  2*(lsegs+wsegs);
	topStart = (lsegs+1)*(wsegs+1);
	midStart = 2*topStart;
	
	nverts = midStart+nlayer*(hsegs-1);
	nfaces = 4*(lsegs*wsegs + hsegs*lsegs + wsegs*hsegs);
	
	mesh.setNumVerts(nverts);
	mesh.setNumFaces(nfaces);
	mesh.InvalidateTopologyCache();
	
	nv = 0;
	
	vb =  Point3(w,l,h)/float(2);   
	va = -vb;
	
#ifdef BOTTOMPIV
	va.z = float(0);
	vb.z = h;
#endif
	
	float dx = w/wsegs;
	float dy = l/lsegs;
	float dz = h/hsegs;
	
	// do bottom vertices.
	p.z = va.z;
	p.y = va.y;
	for(iy=0; iy<=lsegs; iy++) {
		p.x = va.x;
		for (ix=0; ix<=wsegs; ix++) {
			mesh.setVert(nv++, p);
			p.x += dx;
		}
		p.y += dy;
	}
	
	nf = 0;
	
	// do bottom faces.
	for(iy=0; iy<lsegs; iy++) {
		kv = iy*(wsegs+1);
		for (ix=0; ix<wsegs; ix++) {
			MAKE_QUAD(kv, kv+wsegs+1, kv+wsegs+2, kv+1, 1, bias);
			kv++;
		}
	}
	assert(nf==lsegs*wsegs*2);
	
	// do top vertices.
	p.z = vb.z;
	p.y = va.y;
	for(iy=0; iy<=lsegs; iy++) {
		p.x = va.x;
		for (ix=0; ix<=wsegs; ix++) {
			mesh.setVert(nv++, p);
			p.x += dx;
		}
		p.y += dy;
	}
	
	// do top faces (lsegs*wsegs);
	for(iy=0; iy<lsegs; iy++) {
		kv = iy*(wsegs+1)+topStart;
		for (ix=0; ix<wsegs; ix++) {
			MAKE_QUAD(kv, kv+1, kv+wsegs+2,kv+wsegs+1, 2, bias);
			kv++;
		}
	}
	assert(nf==lsegs*wsegs*4);
	
	// do middle vertices 
	for(iz=1; iz<hsegs; iz++) {
		
		p.z = va.z + dz * iz;
		
		// front edge
		p.x = va.x;  p.y = va.y;
		for (ix=0; ix<wsegs; ix++) { mesh.setVert(nv++, p);  p.x += dx;	}
		
		// right edge
		p.x = vb.x;	  p.y = va.y;
		for (iy=0; iy<lsegs; iy++) { mesh.setVert(nv++, p);  p.y += dy;	}
		
		// back edge
		p.x =  vb.x;  p.y =  vb.y;
		for (ix=0; ix<wsegs; ix++) { mesh.setVert(nv++, p);	 p.x -= dx;	}
		
		// left edge
		p.x = va.x;  p.y =  vb.y;
		for (iy=0; iy<lsegs; iy++) { mesh.setVert(nv++, p);	 p.y -= dy;	}
	}
	
	if (hsegs==1) {
		// do LEFT faces -----------------------
		kv = 0;
		mv = topStart;
		for (ix=0; ix<wsegs; ix++) {
			MAKE_QUAD(kv, kv+1, mv+1, mv, 5, bias);
			kv++;
			mv++;
		}
		
		// do RIGHT faces.-----------------------
		kv = wsegs;  
		mv = topStart + kv;
		for (iy=0; iy<lsegs; iy++) {
			MAKE_QUAD(kv, kv+wsp1, mv+wsp1, mv, 4, bias);
			kv += wsp1;
			mv += wsp1;
		}	
		
		// do BACK faces.-----------------------
		kv = topStart - 1;
		mv = midStart - 1;
		for (ix=0; ix<wsegs; ix++) {
			MAKE_QUAD(kv, kv-1, mv-1, mv, 5, bias);
			kv --;
			mv --;
		}
		
		// do LEFT faces.----------------------
		kv = lsegs*(wsegs+1);  // index into bottom
		mv = topStart + kv;
		for (iy=0; iy<lsegs; iy++) {
			MAKE_QUAD(kv, kv-wsp1, mv-wsp1, mv, 6, bias);
			kv -= wsp1;
			mv -= wsp1;
		}
	}
	
	else {
		// do front faces.
		kv = 0;
		mv = midStart;
		for(iz=0; iz<hsegs; iz++) {
			if (iz==hsegs-1) mv = topStart;
			for (ix=0; ix<wsegs; ix++) 
				MAKE_QUAD(kv+ix, kv+ix+1, mv+ix+1, mv+ix, 3, bias);
			kv = mv;
			mv += nlayer;
		}
		
		assert(nf==lsegs*wsegs*4 + wsegs*hsegs*2);
		
		// do RIGHT faces.-------------------------
		// RIGHT bottom row:
		kv = wsegs; // into bottom layer. 
		mv = midStart + wsegs; // first layer of mid verts
		
		
		for (iy=0; iy<lsegs; iy++) {
			MAKE_QUAD(kv, kv+wsp1, mv+1, mv, 4, bias);
			kv += wsp1;
			mv ++;
		}
		
		// RIGHT middle part:
		kv = midStart + wsegs; 
		for(iz=0; iz<hsegs-2; iz++) {
			mv = kv + nlayer;
			for (iy=0; iy<lsegs; iy++) {
				MAKE_QUAD(kv+iy, kv+iy+1, mv+iy+1, mv+iy, 4, bias);
			}
			kv += nlayer;
		}
		
		// RIGHT top row:
		kv = midStart + wsegs + (hsegs-2)*nlayer; 
		mv = topStart + wsegs;
		for (iy=0; iy<lsegs; iy++) {
			MAKE_QUAD(kv, kv+1, mv+wsp1, mv, 4, bias);
			mv += wsp1;
			kv++;
		}
		
		assert(nf==lsegs*wsegs*4 + wsegs*hsegs*2 + lsegs*hsegs*2);
		
		// do BACK faces. ---------------------
		// BACK bottom row:
		kv = topStart - 1;
		mv = midStart + wsegs + lsegs;
		for (ix=0; ix<wsegs; ix++) {
			MAKE_QUAD(kv, kv-1, mv+1, mv, 5, bias);
			kv --;
			mv ++;
		}
		
		// BACK middle part:
		kv = midStart + wsegs + lsegs; 
		for(iz=0; iz<hsegs-2; iz++) {
			mv = kv + nlayer;
			for (ix=0; ix<wsegs; ix++) {
				MAKE_QUAD(kv+ix, kv+ix+1, mv+ix+1, mv+ix, 5, bias);
			}
			kv += nlayer;
		}
		
		// BACK top row:
		kv = midStart + wsegs + lsegs + (hsegs-2)*nlayer; 
		mv = topStart + lsegs*(wsegs+1)+wsegs;
		for (ix=0; ix<wsegs; ix++) {
			MAKE_QUAD(kv, kv+1, mv-1, mv, 5, bias);
			mv --;
			kv ++;
		}
		
		assert(nf==lsegs*wsegs*4 + wsegs*hsegs*4 + lsegs*hsegs*2);
		
		// do LEFT faces. -----------------
		// LEFT bottom row:
		kv = lsegs*(wsegs+1);  // index into bottom
		mv = midStart + 2*wsegs +lsegs;
		for (iy=0; iy<lsegs; iy++) {
			nextm = mv+1;
			if (iy==lsegs-1) 
				nextm -= nlayer;
			MAKE_QUAD(kv, kv-wsp1, nextm, mv, 6, bias);
			kv -=wsp1;
			mv ++;
		}
		
		// LEFT middle part:
		kv = midStart + 2*wsegs + lsegs; 
		for(iz=0; iz<hsegs-2; iz++) {
			mv = kv + nlayer;
			for (iy=0; iy<lsegs; iy++) {
				nextm = mv+1;
				nextk = kv+iy+1;
				if (iy==lsegs-1) { 
					nextm -= nlayer;
					nextk -= nlayer;
				}
				MAKE_QUAD(kv+iy, nextk, nextm, mv, 6, bias);
				mv++;
			}
			kv += nlayer;
		}
		
		// LEFT top row:
		kv = midStart + 2*wsegs + lsegs+ (hsegs-2)*nlayer; 
		mv = topStart + lsegs*(wsegs+1);
		for (iy=0; iy<lsegs; iy++) {
			nextk = kv+1;
			if (iy==lsegs-1) 
				nextk -= nlayer;
			MAKE_QUAD(kv, nextk, mv-wsp1, mv, 6, bias);
			mv -= wsp1;
			kv++;
		}
		}
		
		if (genUVs) {
			int ls = lsegs+1;
			int ws = wsegs+1;
			int hs = hsegs+1;
			int ntverts = ls*hs + hs*ws + ws*ls ;
			mesh.setNumTVerts( ntverts ) ;
			mesh.setNumTVFaces(nfaces);		
			
			int xbase = 0;
			int ybase = ls*hs;
			int zbase = ls*hs + hs*ws;
			
			float dw = 1.0f/float(wsegs);
			float dl = 1.0f/float(lsegs);
			float dh = 1.0f/float(hsegs);
			
			if (w==0.0f) w = .0001f;
			if (l==0.0f) l = .0001f;
			if (h==0.0f) h = .0001f;
			float u,v;
			
			nv = 0;
			v = 0.0f;
			// X axis face
			for (iz =0; iz<hs; iz++) {
				u = 0.0f; 
				for (iy =0; iy<ls; iy++) {
					mesh.setTVert(nv, u, v, 0.0f);
					nv++; u+=dl;
				}
				v += dh;
			}
			
			v = 0.0f; 
			//Y Axis face
			for (iz =0; iz<hs; iz++) {
				u = 0.0f;
				for (ix =0; ix<ws; ix++) {
					mesh.setTVert(nv, u, v, 0.0f);
					nv++; u+=dw;
				}
				v += dh;
			}
			
			v = 0.0f; 
			for (iy =0; iy<ls; iy++) {
				u = 0.0f; 
				for (ix =0; ix<ws; ix++) {
					mesh.setTVert(nv, u, v, 0.0f);
					nv++; u+=dw;
				}
				v += dl;
			}
			
			assert(nv==ntverts);
			
			for (nf = 0; nf<nfaces; nf++) {
				Face& f = mesh.faces[nf];
				DWORD* nv = f.getAllVerts();
				Point3 v[3];
				for (int ix =0; ix<3; ix++)
					v[ix] = mesh.getVert(nv[ix]);
				int dir = direction(v);
				int ntv[3];
				for (ix=0; ix<3; ix++) {
					int iu,iv;
					switch(dir) {
					case POSX: case NEGX:
						iu = int(((float)lsegs*(v[ix].y-va.y)/l)+.5f); 
						iv = int(((float)hsegs*(v[ix].z-va.z)/h)+.5f);  
						if (dir==NEGX) iu = lsegs-iu;
						ntv[ix] = (xbase + iv*ls + iu);
						break;
					case POSY: case NEGY:
						iu = int(((float)wsegs*(v[ix].x-va.x)/w)+.5f);  
						iv = int(((float)hsegs*(v[ix].z-va.z)/h)+.5f); 
						if (dir==POSY) iu = wsegs-iu;
						ntv[ix] = (ybase + iv*ws + iu);
						break;
					case POSZ: case NEGZ:
						iu = int(((float)wsegs*(v[ix].x-va.x)/w)+.5f);  
						iv = int(((float)lsegs*(v[ix].y-va.y)/l)+.5f); 
						if (dir==NEGZ) iu = wsegs-iu;
						ntv[ix] = (zbase + iv*ws + iu);
						break;
					}
				}
				assert(ntv[0]<ntverts);
				assert(ntv[1]<ntverts);
				assert(ntv[2]<ntverts);
				
				mesh.tvFace[nf].setTVerts(ntv[0],ntv[1],ntv[2]);
				mesh.setFaceMtlIndex(nf,mapDir[dir]);
			}
		}
		else {
			mesh.setNumTVerts(0);
			mesh.setNumTVFaces(0);
			for (nf = 0; nf<nfaces; nf++) {
				Face& f = mesh.faces[nf];
				DWORD* nv = f.getAllVerts();
				Point3 v[3];
				for (int ix =0; ix<3; ix++)
					v[ix] = mesh.getVert(nv[ix]);
				int dir = direction(v);
				mesh.setFaceMtlIndex(nf,mapDir[dir]);
			}
		}
		
		mesh.InvalidateGeomCache();
		mesh.BuildStripsAndEdges();
	}
	
	
class BoxObjCreateCallBack: public CreateMouseCallBack {
	ShadBoxObject *ob;
	Point3 p0,p1;
	IPoint2 sp0, sp1;
	BOOL square;
public:
	int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat );
	void SetObj(ShadBoxObject *obj) { ob = obj; }
};

int BoxObjCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat ) {
	Point3 d;
	if (msg == MOUSE_FREEMOVE)
	{
		vpt->SnapPreview(m,m,NULL, SNAP_IN_3D);
	}
	
	else if (msg==MOUSE_POINT||msg==MOUSE_MOVE) {
		switch(point) {
		case 0:
			sp0 = m;
			ob->pblock->SetValue(PB_WIDTH,0,0.0f);
			ob->pblock->SetValue(PB_LENGTH,0,0.0f);
			ob->pblock->SetValue(PB_HEIGHT,0,0.0f);
			ob->suspendSnap = TRUE;								
			p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_3D);
			p1 = p0 + Point3(.01,.01,.01);
			mat.SetTrans(float(.5)*(p0+p1));				
#ifdef BOTTOMPIV
			{
				Point3 xyz = mat.GetTrans();
				xyz.z = p0.z;
				mat.SetTrans(xyz);
			}
#endif
			break;
		case 1:
			sp1 = m;
			p1 = vpt->SnapPoint(m,m,NULL,SNAP_IN_3D);
			p1.z = p0.z +(float).01; 
			if (ob->createMeth || (flags&MOUSE_CTRL)) {
				mat.SetTrans(p0);
			} else {
				mat.SetTrans(float(.5)*(p0+p1));
#ifdef BOTTOMPIV 					
				Point3 xyz = mat.GetTrans();
				xyz.z = p0.z;
				mat.SetTrans(xyz);					
			}
#endif
			d = p1-p0;
			
			square = FALSE;
			if (ob->createMeth) {
				// Constrain to cube
				d.x = d.y = d.z = Length(d)*2.0f;
			} else 
				if (flags&MOUSE_CTRL) {
					// Constrain to square base
					float len;
					if (fabs(d.x) > fabs(d.y)) len = d.x;
					else len = d.y;
					d.x = d.y = 2.0f * len;
					square = TRUE;
				}
				
				ob->pblock->SetValue(PB_WIDTH,0,float(fabs(d.x)));
				ob->pblock->SetValue(PB_LENGTH,0,float(fabs(d.y)));
				ob->pblock->SetValue(PB_HEIGHT,0,float(fabs(d.z)));
				ob->pmapParam->Invalidate();										
				
				if (msg==MOUSE_POINT && ob->createMeth) {
					ob->suspendSnap = FALSE;
					return (Length(sp1-sp0)<3)?CREATE_ABORT:CREATE_STOP;					
				} else if (msg==MOUSE_POINT && 
					(Length(sp1-sp0)<3 || Length(d)<0.1f)) {
					return CREATE_ABORT;
				}
				break;
		case 2:
#ifdef _OSNAP
			p1.z = p0.z + vpt->SnapLength(vpt->GetCPDisp(p0,Point3(0,0,1),sp1,m,TRUE));
#else
			p1.z = p0.z + vpt->SnapLength(vpt->GetCPDisp(p1,Point3(0,0,1),sp1,m));
#endif				
			if (!square) {
				mat.SetTrans(float(.5)*(p0+p1));
#ifdef BOTTOMPIV
				mat.SetTrans(2,p0.z); // set the Z component of translation
#endif
			}
			
			d = p1-p0;
			if (square) {
				// Constrain to square base
				float len;
				if (fabs(d.x) > fabs(d.y)) len = d.x;
				else len = d.y;
				d.x = d.y = 2.0f * len;					
			}
			
			ob->pblock->SetValue(PB_WIDTH,0,float(fabs(d.x)));
			ob->pblock->SetValue(PB_LENGTH,0,float(fabs(d.y)));
			ob->pblock->SetValue(PB_HEIGHT,0,float(d.z));
			ob->pmapParam->Invalidate();				
			
			if (msg==MOUSE_POINT) {
				ob->suspendSnap = FALSE;					
				return CREATE_STOP;
			}
			break;
		}
	}
	else
		if (msg == MOUSE_ABORT) {		
			return CREATE_ABORT;
		}
		
		return TRUE;
	}
	
	static BoxObjCreateCallBack boxCreateCB;
	
	CreateMouseCallBack* ShadBoxObject::GetCreateMouseCallBack() {
		boxCreateCB.SetObj(this);
		return(&boxCreateCB);
	}
	
	
	BOOL ShadBoxObject::OKtoDisplay(TimeValue t) 
	{
	/*
	float l, w, h;
	pblock->GetValue(PB_LENGTH,t,l,FOREVER);
	pblock->GetValue(PB_WIDTH,t,w,FOREVER);
	pblock->GetValue(PB_HEIGHT,t,h,FOREVER);
	if (l==0.0f || w==0.0f || h==0.0f) return FALSE;
	else return TRUE;
		*/
		return TRUE;
	}
	
	
	// From ParamArray
	BOOL ShadBoxObject::SetValue(int i, TimeValue t, int v) 
	{
		switch (i) {
		case PB_CREATEMETHOD: createMeth = v; break;
		}		
		return TRUE;
	}
	
	BOOL ShadBoxObject::SetValue(int i, TimeValue t, float v)
	{
		return FALSE;
	}
	
	BOOL ShadBoxObject::SetValue(int i, TimeValue t, Point3 &v) 
	{
		return FALSE;
	}
	
	BOOL ShadBoxObject::GetValue(int i, TimeValue t, int &v, Interval &ivalid) 
	{
		switch (i) {
		case PB_CREATEMETHOD: v = createMeth; break;
		}
		return TRUE;
	}
	
	BOOL ShadBoxObject::GetValue(int i, TimeValue t, float &v, Interval &ivalid) 
	{	
		return FALSE;
	}
	
	BOOL ShadBoxObject::GetValue(int i, TimeValue t, Point3 &v, Interval &ivalid) 
	{	
		return FALSE;
	}
	
	
	void ShadBoxObject::InvalidateUI() 
	{
		if (pmapParam) pmapParam->Invalidate();
	}
	
	ParamDimension *ShadBoxObject::GetParameterDim(int pbIndex) 
	{
		switch (pbIndex) {
		case PB_LENGTH:return stdWorldDim;
		case PB_WIDTH: return stdWorldDim;
		case PB_HEIGHT:return stdWorldDim;
		default: return defaultDim;
		}
	}
	
	TSTR ShadBoxObject::GetParameterName(int pbIndex) 
	{
		switch (pbIndex) {
		case PB_LENGTH: return TSTR(GetString(IDS_SB_LENGTH));
		case PB_WIDTH:  return TSTR(GetString(IDS_SB_WIDTH));
		case PB_HEIGHT: return TSTR(GetString(IDS_SB_HEIGHT));
		default: return TSTR(_T(""));
		}
	}
	
	RefTargetHandle ShadBoxObject::Clone(RemapDir& remap) 
	{
		ShadBoxObject* newob = new ShadBoxObject();
		newob->ReplaceReference(0,pblock->Clone(remap));
		newob->ivalid.SetEmpty();	
		return(newob);
	}


bool ShadBoxObject::InBox(Point3 &poo)
{
	Matrix3 scaleMat;
	Interval ivalid = FOREVER;
	float w, l, h;
	Point3 p = poo;

	pblock->GetValue(PB_LENGTH,0,l,ivalid);
	pblock->GetValue(PB_WIDTH,0,w,ivalid);
	pblock->GetValue(PB_HEIGHT,0,h,ivalid);

	scaleMat = ScaleMatrix (Point3(w, l, h));
	scaleMat.Translate (Point3(-w/2, -l/2, 0.f));
	scaleMat = scaleMat * mat;
//	scaleMat = ScaleMatrix (Point3(w, l, h)) * mat;
//	scaleMat.Translate (Point3(-w/2, -l/2, 0.f));

	p = p * Inverse(scaleMat);
	if (p.x >= 0.f && p.x <= 1.f &&
		p.y >= 0.f && p.y <= 1.f &&
		p.z >= 0.f && p.z <= 1.f)
		return true;
	else
		return false;
}

float ShadBoxObject::Value()
{
	Interval ivalid = FOREVER;
	float value;
	pblock->GetValue (PB_VALUE, 0, value, ivalid);
	return value;
}
float ShadBoxObject::Priority()
{
	Interval ivalid = FOREVER;
	float value;
	pblock->GetValue (PB_PRIORITY, 0, value, ivalid);
	return value;
}
void ShadBoxObject::SetMat (const Matrix3 &mat)
{
	this->mat = mat;
}