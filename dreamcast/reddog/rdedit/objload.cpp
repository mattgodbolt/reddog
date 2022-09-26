/*
 * ObjLoad.cpp
 * The loading and saving of Red Dog Objects
 */

//THE ONE BEFORE THIS IS A POTENTIAL SAFE VERSION, IF THIS FILE CAUSES PROBS
//LOOK FOR THE LAST SOURCESAFE FILE WITHOUT THIS TEXT
#include "StdAfx.h"
#include <float.h>
#include <process.h>

#include "ObjFlags.h"
#include "camcontrol.h"
#include "animtbl.h"
#include "ISTDPLUG.h"
#include "CoM.h"


#define ANIM_TEST 0

#if 0
#define NJM_RAD_ANG(n)  ((Uint32)(((n) * 65536.0) / (2 * PI)))
#define TOFLOAT(n) (((float)(n) / 65536.f) * (2*PI))
#else
#undef NJM_RAD_ANG
#undef TOFLOAT
#define NJM_RAD_ANG(n)  n
#define TOFLOAT(n) n
#endif

static bool LoadNewCP = FALSE;
static bool LoadBump = FALSE;


bool kludgeAllCalcMatrix = FALSE;
::Point3 grossCOM;
/*
 * The saving enumerator
 */
class RDObjectEnumerator : public ITreeEnumProc {
private:
	// The interface to max
	Interface		*maxInterface;
	Tab<INode *>	&table;
public:
	RDObjectEnumerator (IScene *scene, Interface *i, Tab<INode *> &tab) : table(tab)
	{
		assert (i != NULL);
		// Set up the variables
		maxInterface = i;
		scene->EnumTree (this);
	}
	
	// A function called once per object to save it
	int callback (INode *node)
	{
		Object *obj = node->EvalWorldState(maxInterface->GetTime()).obj;
		// Don't export any Red Dog Objects
		if (obj->ClassID() == RDOBJECT_CLASS_ID) {
			// Do nowt
			return TREE_IGNORECHILDREN;
		} else {
			// Add this to the table of inodes
			table.Append (1, &node);			
			return TREE_IGNORECHILDREN;
		}
	}
};

int Animated=0;
int AnimNo=0;

int childtot;


//sums up the total number of obids in the model
void GetObjTots(INode *node)
{
	Object *obj = node->EvalWorldState(GetCOREInterface()->GetTime()).obj;
 /*	if ((obj) &&
		(
		(obj->ClassID() == RDOBJECT_CLASS_ID) ||
		(obj->ClassID() == camcontrols_CLASS_ID) ||
		(obj->ClassID() == COLLPOINT_CLASS_ID) ||
	  	(obj->ClassID() == OBJFLAGS_CLASS_ID) ||
		(obj->ClassID() == COMPOINT_CLASS_ID)) )
		 return;
	   */

 	if ((obj) && (obj->ClassID() == OBJFLAGS_CLASS_ID))
   		childtot++;
   /*
	childtot++;
	 */

	for (int child = 0; child < node->NumberOfChildren(); ++child)
		GetObjTots(node->GetChildNode(child));
	 




}

int ID;
typedef struct mat
{
	float m[4][4];

}	MAT;


MAT *MatrixArray;

typedef struct interp
{
    Point3 rot;
	Point3 trans;
	Point3 scale;

    Point3 drot;
	Point3 dtrans;
	Point3 dscale;

	float	x;
	float	y;
	float	z;
	float	w;

}	INTERP;


INTERP *InterpArray;




Point3 GetRealRot(Matrix3 mat,Point3 *scale)
{

	Point3 rotation;
	// Pull out the rotation
	Matrix3 unscaled;

	// Unscale the matrix
	unscaled = mat;
	unscaled.SetRow (0, Normalize(unscaled.GetRow(0)));
	unscaled.SetRow (1, Normalize(unscaled.GetRow(1)));
	unscaled.SetRow (2, Normalize(unscaled.GetRow(2)));

	if (mat.Parity())
	{
	  	unscaled.SetRow (0, unscaled.GetRow(0) * -1.f);
	  	unscaled.SetRow (1, unscaled.GetRow(1) * -1.f);
	  	unscaled.SetRow (2, unscaled.GetRow(2) * -1.f);
	  	scale->x *= -1.f;
	  	scale->y *= -1.f;
	  	scale->z *= -1.f;
	}

	// Find the x rotation
	rotation.x = (float) asin (unscaled.GetRow(1).z);

	// Check cos(x)
	if (fabs(cos(rotation.x)) > 0.001f)
	{
		rotation.y	= (float) atan2 (-unscaled.GetRow(0).z, unscaled.GetRow(2).z);
		rotation.z	= (float) atan2 (-unscaled.GetRow(1).x, unscaled.GetRow(1).y);
	}
	else
	{
		rotation.y	= (float) atan2 (unscaled.GetRow(2).x, unscaled.GetRow(0).x);
		rotation.z	= 0.f;
	}

	return(rotation);

}

void OutputMyAnims(INode *node,TimeValue t,Matrix3 basemat)
{

		INode	*mum		= node->GetParentNode();
 		Matrix3	localTM,parentTM;
	   	if (mum)
	   	{
		 	parentTM	= mum->GetNodeTM(t);
			
			localTM		= basemat * Inverse(parentTM);
	   	}
	   	else
	   		localTM		= basemat;

		Object *obj = node->EvalWorldState(GetCOREInterface()->GetTime()).obj;
		if ( (obj) &&
			((obj->ClassID() == RDOBJECT_CLASS_ID) ||
			(obj->ClassID() == camcontrols_CLASS_ID) ||
			(obj->ClassID() == COLLPOINT_CLASS_ID) ||
			(obj->ClassID() == OBJFLAGS_CLASS_ID) ||
			(obj->ClassID() == COMPOINT_CLASS_ID)) )
			 return;
		
		
		
		//write out the matrix data here, we'll keep it 4x4 to save
		//in game conversion

		float m[4][4];
		Point3 row;
		for (int loop=0;loop<3;loop++)
		{
			row = localTM.GetRow(loop);
			m[loop][0] = row.x;
			m[loop][1] = row.y;
			m[loop][2] = row.z;
			m[loop][3] = 0;
		}
		row = localTM.GetTrans() ;
		m[3][0]= row.x;
		m[3][1]= row.y;
		m[3][2]= row.z;
		m[3][3] = 1.0f;
	 
		//FIND THE ATTACHED OBJ ID FLAG

		ID = 0;
	   	int found=0;
		for (int i = 0; i < node->NumberOfChildren(); ++i)
		{
			INode *Child = node->GetChildNode(i);
			Object *obj = Child->EvalWorldState(GetCOREInterface()->GetTime()).obj;
			if ((obj) && (obj->ClassID() == OBJFLAGS_CLASS_ID))
			{
				ID = ((ObjFlag*)obj)->ID();
			   	found++;
			}	
		} 



		if ((found) || ((!found) && (!mum)))
		{
			MatrixArray[ID].m[0][0] = m[0][0];
			MatrixArray[ID].m[0][1] = m[0][1];
			MatrixArray[ID].m[0][2] = m[0][2];
			MatrixArray[ID].m[0][3] = m[0][3];
			MatrixArray[ID].m[1][0] = m[1][0];
			MatrixArray[ID].m[1][1] = m[1][1];
			MatrixArray[ID].m[1][2] = m[1][2];
			MatrixArray[ID].m[1][3] = m[1][3];
			MatrixArray[ID].m[2][0] = m[2][0];
			MatrixArray[ID].m[2][1] = m[2][1];
			MatrixArray[ID].m[2][2] = m[2][2];
			MatrixArray[ID].m[2][3] = m[2][3];
			
			MatrixArray[ID].m[3][0] = m[3][0];

			MatrixArray[ID].m[3][1] = m[3][1];

			MatrixArray[ID].m[3][2] = m[3][2];

			MatrixArray[ID].m[3][3] = m[3][3];
		}

		for (int child = 0; child < node->NumberOfChildren(); ++child)
			OutputMyAnims(node->GetChildNode(child),t,node->GetChildNode(child)->GetNodeTM(t));

 //			OutputMyAnims(output,node->GetChildNode(child),t,node->GetChildNode(child)->GetObjectTM(t));

}



double LengthSquaredMatt(Point3 *v)
{
	return (sqrt(v->x *v->x + v->y * v->y + v->z * v->z));

}
void MatToQuat(Matrix3 *mat,Quat *quat)
{
	float s;
	Point3 row0 = mat->GetRow(0);
	Point3 row1 = mat->GetRow(1);
	Point3 row2 = mat->GetRow(2);
	Point3 row3 = mat->GetRow(3);
	float m[4][4];
	m[0][0] = row0.x;
	m[0][1] = row0.y;
	m[0][2] = row0.z;
	m[0][3] = 0;


	m[1][0] = row1.x;
	m[1][1] = row1.y;
	m[1][2] = row1.z;
	m[1][3] = 0;


	m[2][0] = row2.x;
	m[2][1] = row2.y;
	m[2][2] = row2.z;
	m[2][3] = 0;



	m[3][0] = 0.0;
	m[3][1] = 0.0;
	m[3][2] = 0.0;
	m[3][3] = 1.0f;




	float tr = m[0][0] + m[1][1] + m[2][2];

	if (tr >= 0.0)
	{
		s = (float)sqrt(tr + 1.0f);
		quat->w = s/2.0f;
		s = (0.5f/s);

		quat->x = (m[1][2] - m[2][1]) * s;

		quat->y = (m[2][0] - m[0][2]) * s;

		quat->z = (m[0][1] - m[1][0]) * s;




	}
	else
	{
		float q[4];
		int n[] = {1,2,0};
		int i=0;
	
		if (m[1][1] > m[0][0])
			i=1;
		if (m[2][2] > m[i][i])
			i=2;

		int j=n[i];
		int k=n[j];

		s = (float)sqrt((m[i][i] - (m[j][j] + m[k][k])) + 1.0);

		q[i] = s / 2.0f;

		if (s)
			s=0.5f /s ;

		q[3] = (m[j][k] - m[k][j]) * s;

		q[j] = (m[i][j] - m[j][i]) * s;

		q[k] = (m[i][k] - m[k][i]) * s;

		quat->x = q[0];

		quat->y = q[1];

		quat->z = q[2];

		quat->w = q[3];

	}



}


void QuatToMat(Quat *quat, Matrix3 *m)
{
  /*	float wx,wy,wz,xx,yy,yz,xy,xz,zz,x2,y2,z2; */

	/*BRENDER METHOD*/

	float xs,ys,zs;
	float wx,wy,wz;
	float xx,xy,xz;
	float yy,yz,zz;
	Point3 row;



	xs = quat->x * 2.0f;
	ys = quat->y * 2.0f;
	zs = quat->z * 2.0f;

	wx = (quat->w * xs);
	wy = (quat->w * ys);
	wz = (quat->w * zs);


	xx = (quat->x * xs);
	xy = (quat->x * ys);
	xz = (quat->x * zs);

	yy = (quat->y * ys);
	yz = (quat->y * zs);
	zz = (quat->z * zs);

	row.x = 1.0f - (yy+zz);
	row.y = xy - wz;
	row.z = xz + wy;

	m->SetRow(0,row);

  	row.x = xy + wz;
	row.y = 1.0f - (xx+zz);
	row.z = yz-wx;
	m->SetRow(1,row);

	row.x = xz - wy;
	row.y = yz + wx;
	row.z = 1.0f - (xx+yy);
	m->SetRow(2,row);






}


Matrix3 prev;




void GetRealQuat(Matrix3 mat,Quat *quat)
{

	Point3 rotation;
	// Pull out the rotation
	Matrix3 unscaled;

	// Unscale the matrix
	unscaled = mat;
	unscaled.SetRow (0, Normalize(unscaled.GetRow(0)));
	unscaled.SetRow (1, Normalize(unscaled.GetRow(1)));
	unscaled.SetRow (2, Normalize(unscaled.GetRow(2)));

   	if (mat.Parity())
	{
	  	unscaled.SetRow (0, unscaled.GetRow(0) * -1.f);
	  	unscaled.SetRow (1, unscaled.GetRow(1) * -1.f);
	  	unscaled.SetRow (2, unscaled.GetRow(2) * -1.f);
	}

 //	prev = unscaled;
 // 	MatToQuat(&unscaled,quat);
 
 //	Quat testquat = *quat;

	*quat = Quat(unscaled);
	quat->Normalize();

}




void NormalizeQuat(Quat *quat)
{
	float magnitude;
	magnitude = (quat->x * quat->x) + 
				(quat->y * quat->y) + 
				(quat->z * quat->z) + 
				(quat->w * quat->w);

	quat->x = quat->x / magnitude;
	quat->y = quat->y / magnitude;
	quat->z = quat->z / magnitude;
	quat->w = quat->w / magnitude;
}


Matrix3 GetParentMatrix(Matrix3 basemat,INode *parent,TimeValue t)
{
	Matrix3 mat;
	Object *obj = parent->GetObjectRef();
	//derive initial rot
	Matrix3 parentTM = parent->GetNodeTM(t);
	Matrix3 startmat = basemat;
	
	//HEAD TO [head]
	Matrix3	localTM		= basemat * Inverse(parentTM);
	Matrix3 newmat = localTM;
	Matrix3 newmat2 = localTM;

	Point3  trans = newmat.GetTrans();
   	//newmat2.NoTrans();
   	//newmat.NoTrans();
   
	
	
   	while ((obj) && (obj->ClassID() == Class_ID(DUMMY_CLASS_ID,0)))
	{
		TCHAR *namenode = parent->GetName();
		parent = parent->GetParentNode();
		obj = parent->GetObjectRef();
		parentTM = parent->GetNodeTM(t);
		basemat = Inverse(parentTM);

		//RELATIVE TRANSFORM FOR THIS NODE
	 	//[head] TO neck
		
		Matrix3 transform = newmat * basemat;

		//ADD THE TRANSLATION
		trans = transform.GetTrans();
	   	transform.NoTrans();
		
		//LAST ROTATION
	   
		newmat2.PreTranslate(trans);
		trans = newmat2.GetTrans();
		Matrix3 temp = newmat2;
		temp.NoTrans();
		newmat2 = transform * temp;
		newmat2.SetTrans(trans);
	
		newmat = parentTM;

	//  mat = startmat * basemat;
	//  return(mat);



	}
	TCHAR *name = parent->GetName();
	
	mat = parent->GetNodeTM(t);
	mat = newmat2;
   //	mat.SetTrans(trans);
	return(mat);

}


//GIVE TIME T AND NEXT TIME, THIS WILL DERIVE THE KEYFRAME DATA FOR EACH OBJECT WITHIN THE MODEL



int ScaleNeeded;

void QuatMul(Quat *q1, Quat *q2, Quat *res)
{
	float A, B, C, D, E, F, G, H;

	A = (q1->w + q1->x) * (q2->w + q2->x);
	B = (q1->z - q1->y) * (q2->y - q2->z);
	C = (q1->x - q1->w) * (q2->y - q2->z);
	D = (q1->y + q1->z) * (q2->x - q2->w);
	E = (q1->x + q1->z) * (q2->x + q2->y);
	F = (q1->x - q1->z) * (q2->x - q2->y);
	G = (q1->w + q1->y) * (q2->w - q2->z);
	H = (q1->w - q1->y) * (q2->w + q2->z);

	res->w = B + (-E -F + G + H) / 2.0f;
	res->x = A - (E +F + G + H) / 2.0f;
	res->y = -C + (E -F + G - H) / 2.0f;
	res->z = -D + (E -F - G + H) / 2.0f;
	res->Normalize();

  
}



void OutputMyKeyFrames(Quat *ParentQuat,INode *node,Matrix3 basemat,TimeValue t, TimeValue nextt)
{

		INode	*mum		= node->GetParentNode();
 		Matrix3	localTM,parentTM;
	   	if (mum)
	   	{
			
			#if ANIM_TEST
				parentTM    = GetParentMatrix(basemat,mum,t);
				localTM = parentTM;   
			#else
				parentTM	= mum->GetNodeTM(t);
				localTM		= basemat * Inverse(parentTM);
			#endif

	   	}
	   	else
	   		localTM		= basemat;

		Object *obj = node->EvalWorldState(GetCOREInterface()->GetTime()).obj;
		if ( (obj) &&
			((obj->ClassID() == RDOBJECT_CLASS_ID) ||
			(obj->ClassID() == camcontrols_CLASS_ID) ||
			(obj->ClassID() == COLLPOINT_CLASS_ID) ||
			(obj->ClassID() == OBJFLAGS_CLASS_ID) ||
			(obj->ClassID() == COMPOINT_CLASS_ID)) )
			 return;
		
		
		//FIND THE ATTACHED OBJ ID FLAG IF ANY

		ID = 0;
	   	int found=0;
		for (int i = 0; i < node->NumberOfChildren(); ++i)
		{
			INode *Child = node->GetChildNode(i);
			Object *obj = Child->EvalWorldState(GetCOREInterface()->GetTime()).obj;
			if ((obj) && (obj->ClassID() == OBJFLAGS_CLASS_ID))
			{
				ID = ((ObjFlag*)obj)->ID();
			   	found++;
			}	
		} 



	  	if ((found) || ((!found) && (!mum)))
	  //	if (found)
		{

			//FIRST CHECK THAT OUR PARENT WAS A GROUP
			//IF IT WAS THEN ADD IN THAT GROUPS TRANSFORMATION
#if 0
			Quat parentquat;
			Point3 parenttrans,parentscale; 

			int addparent=0;
  			if (mum)
			{
			obj = mum->GetObjectRef();

			if ((obj) &&(obj->ClassID() == Class_ID(DUMMY_CLASS_ID,0)))
			{
				addparent = 1;	
				INode	*mumsmum		= mum->GetParentNode();
 			   	Matrix3 basemat2 = mum->GetNodeTM(t);
				Matrix3	mumTM,mumsparentTM;
	   			if (mumsmum)
				{
					mumsparentTM	= mumsmum->GetNodeTM(t);
			
					mumTM		= basemat2 * Inverse(mumsparentTM);
		//			localTM     = basemat * Inverse(mumsparentTM);
				}
			 	else
	   				mumTM		= basemat2;

				DecomposeMatrix (mumTM, parenttrans, parentquat, parentscale);

				GetRealQuat(mumTM,&parentquat);

			}
			}
#endif
			int delta = GetTicksPerFrame();

			Quat currentquat;
			Point3 currentrot,currenttrans,currentscale; 
  
			//GET THE ACTUAL ROT,SCALE AND TRANS DETAILS FOR OUR MATRIX AT TIME T				
			DecomposeMatrix (localTM, currenttrans, currentquat, currentscale);

			GetRealQuat(localTM,&currentquat);

			if (localTM.Parity())
			{
				currentscale.x = currentscale.x * -1.0f;
				currentscale.y = currentscale.y * -1.0f;
				currentscale.z = currentscale.z * -1.0f;



			}


			//now get the nodes object offset

			localTM = node->GetObjectTM(t);

#if 0
			if (ID == 9)
			{
			Quat tquat;
			GetRealQuat(localTM,&tquat);
		   //	currentquat *= tquat;
			Quat prevquat = currentquat;
			QuatMul(&tquat,&prevquat,&currentquat);
		 //	currentquat = tquat * prevquat;
			
			}
#endif




#if 0
		  	if (addparent)
			{
			   
				TCHAR *name = node->GetName();
				Quat newquat = currentquat * parentquat;
			   //	newquat = parentquat * currentquat;
				newquat.Normalize();
				currentquat = newquat;
				currentscale += parentscale;
				currenttrans += parenttrans;

			}

			//FETCH THE MATRIX DATA FOR TIME T+DELTA
			
	 
  			if (addparent)
			{
				INode	*mumsmum		= mum->GetParentNode();
 			   	Matrix3 basemat2 = mum->GetNodeTM(nextt);
				Matrix3	mumTM,mumsparentTM;
	   			if (mumsmum)
				{
					mumsparentTM	= mumsmum->GetNodeTM(nextt);
			
					mumTM		= basemat2 * Inverse(mumsparentTM);
			   	}
			 	else
	   				mumTM		= basemat2;

				DecomposeMatrix (mumTM, parenttrans, parentquat, parentscale);

				GetRealQuat(mumTM,&parentquat);

			}
#endif
			
			
			
			Matrix3 nextmat = node->GetNodeTM(nextt);


		   	if (mum)
		   	{
	 			Matrix3 nparentTM;
			#if ANIM_TEST
				nparentTM    = GetParentMatrix(nextmat,mum,nextt);
				localTM = nparentTM;   
			#else
				nparentTM	= mum->GetNodeTM(nextt);
				localTM	= nextmat * Inverse(nparentTM);
			#endif

		   	}
		   	else
		   		localTM = nextmat;
			   
#if 0
  			if (addparent)
			{
				INode	*mumsmum		= mum->GetParentNode();
 			   	Matrix3 basemat2 = mum->GetNodeTM(nextt);
				Matrix3	mumTM,mumsparentTM;
	   			if (mumsmum)
				{
					mumsparentTM	= mumsmum->GetNodeTM(nextt);
			
					mumTM		= basemat2 * Inverse(mumsparentTM);
			//		localTM		= nextmat * Inverse(mumsparentTM);
			   	}
			 //	else
	   		  //		mumTM		= basemat2;

				DecomposeMatrix (mumTM, parenttrans, parentquat, parentscale);

				GetRealQuat(mumTM,&parentquat);

			}
#endif
			//GET THE ACTUAL ROT,SCALE AND TRANS DETAILS FOR OUR MATRIX AT TIME T+DELTA				

			Quat nextquat;
			Point3 nextrot,nexttrans,nextscale; 
			DecomposeMatrix (localTM, nexttrans, nextquat, nextscale);

			if (localTM.Parity())
			{
				nextscale.x = nextscale.x * -1.0f;
				nextscale.y = nextscale.y * -1.0f;
				nextscale.z = nextscale.z * -1.0f;



			}


		  //	nextrot = GetRealRot(localTM,&nextscale);

#if 0
			if (addparent)
			{
				nextscale += parentscale;
				nexttrans += parenttrans;



			} 
#endif




			int thisframe = t/delta;
			int nextframe = nextt/delta;
			int interprange = nextframe-thisframe;
		   //	nextquat.Normalize();

		   //	Quat nowquat = Slerp(nextquat,currentquat,(float)1/interprange);
		   //	nowquat.Normalize();
			
	   
			/*DERIVE A ROTATION MATRIX IN THE SAME WAY AS IN GAME*/
		   /* 
			InterpArray[ID].rot.x = NJM_RAD_ANG(currentrot.x); 
			InterpArray[ID].rot.y = NJM_RAD_ANG(currentrot.y); 
			InterpArray[ID].rot.z = NJM_RAD_ANG(currentrot.z); 
			*/

			InterpArray[ID].x = NJM_RAD_ANG(currentquat.x);
			InterpArray[ID].y = NJM_RAD_ANG(currentquat.y);
			InterpArray[ID].z = NJM_RAD_ANG(currentquat.z);
			InterpArray[ID].w = currentquat.w;

	/*	  	if 	(
				(nextscale.x <= 0.0) ||
				(nextscale.y <= 0.0)	||
				(nextscale.z <= 0.0)  ||
				(currentscale.x <= 0.0) ||
				(currentscale.y <= 0.0)	||
				(currentscale.z <= 0.0)
				)
				int test = 1;
	  */
			
			InterpArray[ID].scale = currentscale;
			
			/*
		  	if ((currentscale.x <= 0.00001f) ||
		  		(currentscale.y <= 0.00001f) ||
		  		(currentscale.z <= 0.00001f))
				int test = 1;
			  */
			InterpArray[ID].trans = currenttrans; 

		  /*	InterpArray[ID].drot.x = NJM_RAD_ANG((nextrot.x - currentrot.x)/(float)interprange); 
			InterpArray[ID].drot.y = NJM_RAD_ANG((nextrot.y - currentrot.y)/(float)interprange); 
			InterpArray[ID].drot.z = NJM_RAD_ANG((nextrot.z - currentrot.z)/(float)interprange); 
			*/
		  	/*if (nextscale == currentscale)
			{
				InterpArray[ID].dscale.x = 1.0f; 
				InterpArray[ID].dscale.y = 1.0f; 
				InterpArray[ID].dscale.z = 1.0f; 
			}
			else*/
				InterpArray[ID].dscale = (nextscale - currentscale)/(float)interprange; 
			InterpArray[ID].dtrans = (nexttrans - currenttrans)/(float)interprange; 
				
		}

		for (int child = 0; child < node->NumberOfChildren(); ++child)
		{ 
			 	OutputMyKeyFrames(ParentQuat,node->GetChildNode(child),node->GetChildNode(child)->GetNodeTM(t),t,nextt);
				*ParentQuat = Quat(1);

		}


}


void CheckForScale(INode *node,Matrix3 basemat,TimeValue t, TimeValue nextt)
{

		INode	*mum		= node->GetParentNode();
 		Matrix3	localTM,parentTM;
	 
		
		if (mum)
	   	{

			parentTM	= mum->GetNodeTM(t);
			
			localTM		= basemat * Inverse(parentTM);
	   	}
	   	else
	   		localTM		= basemat;

		Object *obj = node->EvalWorldState(GetCOREInterface()->GetTime()).obj;
		if ( (obj) &&
			((obj->ClassID() == RDOBJECT_CLASS_ID) ||
			(obj->ClassID() == camcontrols_CLASS_ID) ||
			(obj->ClassID() == COLLPOINT_CLASS_ID) ||
			(obj->ClassID() == OBJFLAGS_CLASS_ID) ||
			(obj->ClassID() == COMPOINT_CLASS_ID)) )
			 return;
		
		
		//FIND THE ATTACHED OBJ ID FLAG

		ID = 0;
	   	int found=0;
		for (int i = 0; i < node->NumberOfChildren(); ++i)
		{
			INode *Child = node->GetChildNode(i);
			Object *obj = Child->EvalWorldState(GetCOREInterface()->GetTime()).obj;
			if ((obj) && (obj->ClassID() == OBJFLAGS_CLASS_ID))
			{
				ID = ((ObjFlag*)obj)->ID();
			   	found++;
			}	
		} 



		if ((found) || ((!found) && (!mum)))
		{


			Quat currentquat;
			Point3 currentrot,currenttrans,currentscale; 
  
			//GET THE ACTUAL ROT,SCALE AND TRANS DETAILS FOR OUR MATRIX AT TIME T				
			DecomposeMatrix (localTM, currenttrans, currentquat, currentscale);






			//FETCH THE MATRIX DATA FOR TIME T+DELTA
			
			Matrix3 nextmat = node->GetNodeTM(nextt);

			if (mum)
			{
	 			Matrix3 nparentTM;
				nparentTM	= mum->GetNodeTM(nextt);
				localTM	= nextmat * Inverse(nparentTM);
		   	}
		   	else
		   		localTM = nextmat;
			   
			//GET THE ACTUAL ROT,SCALE AND TRANS DETAILS FOR OUR MATRIX AT TIME T+DELTA				

			Quat nextquat;
			Point3 nextrot,nexttrans,nextscale; 
			DecomposeMatrix (localTM, nexttrans, nextquat, nextscale);


			double nextlen = (LengthSquaredMatt(&nextscale));
			double currlen = (LengthSquaredMatt(&currentscale));

			TCHAR *name = node->GetName();
			
			
		   	if (fabs(nextlen - currlen) > 0.001)
		   //	if ((nextlen - currlen) > 0.0001)
		  		ScaleNeeded = 1;

	   //	  	if ((currlen - 3.0) > 0.0001)
	   //	  	  	ScaleNeeded = 1;

			

		 //	ScaleNeeded = 0;

				
		}

		for (int child = 0; child < node->NumberOfChildren(); ++child)
		{ 
				CheckForScale(node->GetChildNode(child),node->GetChildNode(child)->GetNodeTM(t),t,nextt);
		}


}


int IsAnimated(INode *node)
{
 	Object *obj = node->GetObjectRef();

   	if (obj)
	{
		IKeyControl *ikeys;
		int numkey;
		Control *c = node->GetTMController()->GetPositionController();
	  	if (c)
		{
			ikeys = GetKeyControlInterface(c);
		   	if (ikeys)
			{
				numkey = ikeys->GetNumKeys();
			   	if (numkey)
					return(1);
			}
		}

		c = node->GetTMController()->GetRotationController();
	  	if (c)
		{
		
			ikeys = GetKeyControlInterface(c);
		   	if (ikeys)
			{
				numkey = ikeys->GetNumKeys();
			   	if (numkey)
					return(1);
			}
		}

		c = node->GetTMController()->GetScaleController();
	  	if (c)
		{
			ikeys = GetKeyControlInterface(c);
		   	if (ikeys)
			{
				numkey = ikeys->GetNumKeys();
			   	if (numkey)
					return(1);
			}
		}
	}  

	return(0);

}



int NeedObjFlag(INode *node,int numchild,RDObjectExport *rootObject)
{
  //	int numchild = node->NumberOfChildren();

	if (!createid)
		return(0);

//	if (node == rootObject->node)
//	   return(0);

//	if (node->IsGroupHead())
//		return(0);

   //	Object *myobj = node->GetObjectRef();
	Object *myobj = node->EvalWorldState(GetCOREInterface()->GetTime()).obj;
	//ensure the chap has an associated object
	//let's be sure it's not a group 

	if (myobj->ClassID() != Class_ID(DUMMY_CLASS_ID,0))
	{
	
	 /*	if ((!myobj)	|| (!(myobj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0)))))
		{
		 	TCHAR *name = node->GetName();
			return(0);
		}
		TriObject *tri = (TriObject *) myobj->ConvertToType(GetCOREInterface()->GetTime(),
				Class_ID(TRIOBJ_CLASS_ID, 0));

   
		if (!(tri->mesh.numFaces))
		{
		 	TCHAR *name = node->GetName();
			return(0);
		}			   */
		
		int deleteit;
		TriObject *tri;
		if (!(tri = GetTriObjectFromNode(GetCOREInterface(),node,deleteit)))
			return(0);
		else
			if (deleteit)
				delete(tri);
	
	
	}

#if ANIM_TEST
	if (myobj->ClassID() == Class_ID(DUMMY_CLASS_ID,0))
		return(0);
#endif


	//	if (!node->GetRenderData())
   //		return(0);



	if (myobj->ClassID() == OBJFLAGS_CLASS_ID)
		return(0);
	
	if (myobj->ClassID() == COLLPOINT_CLASS_ID)
		return(0);
	
	/*NON GROUP HEAD NODE CHECKS FOR DUMMY BOXES*/
 // 	if ((myobj->ClassID() == Class_ID(DUMMY_CLASS_ID,0)) && (!node->IsGroupHead()))
 // 		return(0);
  
	TCHAR *name = node->GetName();
	
	
	for (int loop=0;loop<numchild;loop++)
	{
		INode *child = node->GetChildNode(loop);
		//Object *obj = child->GetObjectRef();
		Object *obj = child->EvalWorldState(GetCOREInterface()->GetTime()).obj;
		name = child->GetName();
		if (obj->ClassID() == OBJFLAGS_CLASS_ID)
			return(0);

	}

	return(1);


}


typedef struct animrec
{
	char	name[32];
	short 	offset;
	short	numframes;
	short	key[1024];		//MAX KEY FRAMES WITHIN THIS ANIM
}	ANIMREC;


ANIMREC	ModelAnims[MAX_ANIMS]; 

short	CurrentAnimRec;


void ResetModelAnims()
{
	for (int i=0;i<MAX_ANIMS;i++)
	{
		strcpy(ModelAnims[i].name,"DEFAULT");
		ModelAnims[i].offset = 0;
	}

	CurrentAnimRec = 0;
}

#define MAX_MAXKEYS	1024

int KArray[MAX_MAXKEYS];
int KDone[MAX_MAXKEYS];
typedef struct maxkeytype
{
	int frame;
	int used;
} MAXKEYTYPE;



MAXKEYTYPE KSArray[MAX_MAXKEYS];
//int KSArray[MAX_MAXKEYS];
int smaxkey=0,lowkeyind,lowkey;
int nummaxkeys=0;



void AddKeyFrame(int TableOffset,ANIM *anim, TimeValue t,int insanim)
{

	strcpy(ModelAnims[CurrentAnimRec].name,anim->name);
	
	
	ModelAnims[CurrentAnimRec].offset = TableOffset;

	int mykey = anim->keyframe;


	//SPECIAL CASE FOR 1 FRAME ANIMS
	if (anim->keyframe == anim->endframe)
		ModelAnims[CurrentAnimRec].numframes = 1;
	else
	  //	ModelAnims[CurrentAnimRec].numframes = (anim->endframe-1) - mykey;

		ModelAnims[CurrentAnimRec].numframes = (anim->endframe-1) - mykey;


	int delta = GetTicksPerFrame();

	t*=delta;
	
#if 0	
	
	//SEARCH FOR NEXT KEYFRAME
	TimeValue end = GetCOREInterface()->GetAnimRange().End();

	int found=0;
	for (int i=t;i<end;i+=delta)
	{
		int key = i/delta;
	
		 
		for (int checkanim=insanim;checkanim<MAX_ANIMS;checkanim++)
		{
			//SKIP DEFAULT ANIMS
			if (!strnicmp(anims[checkanim].name,"DEFAULT",7))
				continue;
			if (anims[checkanim].keyframe == key)
			{
				//FOUND THE NEXT ANIM .. SO GET ITS FRAME
				//SUBTRACT 1 AND THIS BECOMES THE NUMBER OF FRAMES OF
				//THE PREVIOUS ANIM

			  	ModelAnims[CurrentAnimRec].numframes = (key-1) - mykey;

			 	ModelAnims[CurrentAnimRec].numframes = (anim->endframe-1) - mykey;

				
				found++;
				break;
			}
		}

	}

	//IF NO OTHER ANIMS FOUND THEN NUMBER OF FRAMES OF THIS ANIM IS END-1
	if (!found)
	{
		
		int frames = (end-(t))/delta;
		ModelAnims[CurrentAnimRec].numframes = frames;

	}
#endif

 	 //SOME SPECIAL CASE WAREZ FOR INTERPOLATION
	//if (interpo)
	//{
		int numsubkeys=0;

		//GO THROUGH THE KEY ARRAY
		//any max key frames defined between start frame and end frame
		//shall become frames that shall instigate a change

		//so t is the start frame * delta..therefore, divide by delta to get frame
		int startframe = t/delta;
		
				
	   	//now we can get the end frame
		int endframe = startframe + ModelAnims[CurrentAnimRec].numframes + 1;


	   //	for (int i=0;i<nummaxkeys;i++)
		for (int i=0;i<256;i++)
			ModelAnims[CurrentAnimRec].key[i]=-1;

	   	//now we go through our precalc max keyframe array

		for (i=0;i<nummaxkeys;i++)
		{
			//if conditions are met, add a new keyframe control to this anim
			if ((KSArray[i].frame >= startframe) && (KSArray[i].frame <= endframe))
			{

				ModelAnims[CurrentAnimRec].key[numsubkeys] = (short)(KSArray[i].frame - startframe);
			   	
	   //			if (KSArray[i].frame == 94)
	   //				int utd=1;

				KSArray[i].used=1;
				numsubkeys++;
			}
		}
				
	//}

	CurrentAnimRec++;


}

int GetKeyFrameIndex(int frame)
{
	for (int i=0;i<nummaxkeys;i++)
	{
		if (KSArray[i].frame == frame)
			return(i);

	}

	return(i);
}



void CheckForNewKeyFrame(TimeValue t,int nummods)
{
	for (int i=0;i<MAX_ANIMS;i++)
	{
		//SKIP DEFAULT ANIMS
		if (!strnicmp(anims[i].name,"DEFAULT",7))
			continue;
	//matt add 
	//	if (anims[i].keyframe == t)
	 //	{

			t = anims[i].keyframe;
			if (!anims[i].output)
			{
				MessageBox (NULL, "KEYFRAME FOUND",anims[i].name, MB_OK);
				anims[i].output = 1;
			}	
			if (interpo)
			{
				int offset = GetKeyFrameIndex(t);  
				AddKeyFrame(offset*nummods,&anims[i],t,i+1);

			}
			else
				AddKeyFrame(t*nummods,&anims[i],t,i+1);

	 //	}
	}


}

void AddMaxKeyFrame(int maxkeyframe)
{
	int lop;
	int found=0;

 	//found=0;
	for (lop=0;lop<nummaxkeys;lop++)
	{
		if (KArray[lop] == maxkeyframe)
			found++;

	}
	if (!found)
	{
		KArray[lop]=maxkeyframe;
		nummaxkeys++;
	}
}


void CheckForNewKeyFrameAndAddToMax(TimeValue t)
{
	for (int i=0;i<MAX_ANIMS;i++)
	{
		//SKIP DEFAULT ANIMS
		if (!strnicmp(anims[i].name,"DEFAULT",7))
			continue;
		if ((anims[i].keyframe == t)	|| (anims[i].endframe == t))
		{
			if (!anims[i].output)
			{
			MessageBox (NULL, "KEYFRAME FOUND",anims[i].name, MB_OK);
			anims[i].output=1;
			}
			if (interpo)
				AddMaxKeyFrame(t);
			else
				AddMaxKeyFrame(t);

		}
	}

}




void FindNextLow(int key)
{
	for (int i=0;i<nummaxkeys;i++)
	{
		if (KArray[i] <0)
			continue;

		if (KDone[i])
			continue;

		if (KArray[i] < key)
		{
			KDone[i]++;
			lowkey = KArray[i];
			lowkeyind = i;
			FindNextLow(lowkey);
			break;
		}
	}
}

int StillKeysToSort()
{
	for (int i=0;i<nummaxkeys;i++)
	{
		if (KArray[i] >= 0)
			return(1);

	}
	return(0);
}


void SortKey()
{
	while (StillKeysToSort())
	{
		for (int l=0;l<nummaxkeys;l++)
			KDone[l]=0;

	  	for (int i=0;i<nummaxkeys;i++)
	  	{
	  		lowkey = KArray[i];
	   		if (lowkey>=0)
	   			break;

	  	}

		FindNextLow(lowkey);
		if (KArray[i] == lowkey)
		{
			//KEY SENT IN WAS THE LOWEST KEY ANYWAY
			KSArray[smaxkey].frame = lowkey;
			KArray[i] = -1;

		}
		else
		{

			KSArray[smaxkey].frame = lowkey;
			KArray[lowkeyind] = -1;
		}
		smaxkey++;
	
	}

}






ScaleValue Scale;

TimeValue GetTime(IKeyControl *ikeys,Control *c,int i)
{

	ITCBPoint3Key	tcbPosKey;
	ITCBRotKey		tcbRotKey;
	ITCBScaleKey	tcbScaleKey;

   	IBezPoint3Key	bezPosKey;
	IBezQuatKey		bezRotKey;
	IBezScaleKey	bezScaleKey;

   	ILinPoint3Key	linPosKey;
	ILinRotKey		linRotKey;
	ILinScaleKey	linScaleKey;





	if (c->ClassID() == Class_ID(TCBINTERP_POSITION_CLASS_ID,0))
	{
		ikeys->GetKey(i,&tcbPosKey);
		return(tcbPosKey.time);	   

	}


	if (c->ClassID() == Class_ID(HYBRIDINTERP_POSITION_CLASS_ID,0))
	{
		ikeys->GetKey(i,&bezPosKey);
		return(bezPosKey.time);	   

	}


	if (c->ClassID() == Class_ID(LININTERP_POSITION_CLASS_ID,0))
	{
		ikeys->GetKey(i,&linPosKey);
		return(linPosKey.time);	   

	}


	if (c->ClassID() == Class_ID(TCBINTERP_ROTATION_CLASS_ID,0))
	{
		ikeys->GetKey(i,&tcbRotKey);
		return(tcbRotKey.time);	   

	}


	if (c->ClassID() == Class_ID(HYBRIDINTERP_ROTATION_CLASS_ID,0))
	{
		ikeys->GetKey(i,&bezRotKey);
		return(bezRotKey.time);	   

	}


	if (c->ClassID() == Class_ID(LININTERP_ROTATION_CLASS_ID,0))
	{
		ikeys->GetKey(i,&linRotKey);
		return(linRotKey.time);	   

	}



	if (c->ClassID() == Class_ID(TCBINTERP_SCALE_CLASS_ID,0))
	{
		ikeys->GetKey(i,&tcbScaleKey);

		Scale = tcbScaleKey.val;
	 //	ScaleNeeded = 1;
		return(tcbScaleKey.time);	   

	}


	if (c->ClassID() == Class_ID(HYBRIDINTERP_SCALE_CLASS_ID,0))
	{
		ikeys->GetKey(i,&bezScaleKey);
	  	Scale = bezScaleKey.val;
	   //	ScaleNeeded = 1;
	return(bezScaleKey.time);	   

	}


	if (c->ClassID() == Class_ID(LININTERP_SCALE_CLASS_ID,0))
	{
		ikeys->GetKey(i,&linScaleKey);
			Scale = linScaleKey.val;
		//ScaleNeeded = 1;
		return(linScaleKey.time);	   

	}


	return(-1);

}


//UPDATES A GLOBAL MAXKEY FRAME ARRAY
//GIVEN AN NODES KEYFRAME CONTROL AND NUMBER OF KEYS FOR THE NODE IN THE MAX SCENE


void ProcKey(int numkey,IKeyControl *ikeys,Control *c)
{
	int delta = GetTicksPerFrame();

	for (int i=0;i<numkey;i++)
	{
		
		int maxkeyframe = (int)(GetTime(ikeys,c,i)/delta);
		AddMaxKeyFrame(maxkeyframe);

	}
	
}


//recursively check all nodes in the scene to see whether any have standard MAX keyframes
void BuildMaxKeyList(INode *node)
{
	Object *obj = node->EvalWorldState(GetCOREInterface()->GetTime()).obj;

	TCHAR *name = node->GetName();
 
	
   //	if (obj->ClassID() != Class_ID(DUMMY_CLASS_ID,0))
	
	if (obj)
	{
		IKeyControl *ikeys;
		int numkey;
		Control *c = node->GetTMController()->GetPositionController();
	  	if (c)
		{
			ikeys = GetKeyControlInterface(c);
			
		   	if (ikeys)
			{
				numkey = ikeys->GetNumKeys();
				if (numkey)
					ProcKey(numkey,ikeys,c);
			}
		}

		c = node->GetTMController()->GetRotationController();
	  	if (c)
		{
		
			ikeys = GetKeyControlInterface(c);
			if (ikeys)
			{
   				numkey = ikeys->GetNumKeys();
				if (numkey)
					ProcKey(numkey,ikeys,c);
			}
		}

		c = node->GetTMController()->GetScaleController();
	  	if (c)
		{
			ikeys = GetKeyControlInterface(c);
		   	if (ikeys)
			{
				numkey = ikeys->GetNumKeys();
				if (numkey)
					ProcKey(numkey,ikeys,c);
			}
		}
	}  
	for (int child = 0; child < node->NumberOfChildren(); ++child)
		BuildMaxKeyList(node->GetChildNode(child));
	 
}


void UpdateOffsets(int nummods)
{	int loop,i,offset;

	for (i=0;i<CurrentAnimRec;i++)
	{
		for (loop=nummaxkeys;loop>=0;loop--)
		{
			if (!KSArray[loop].used)
			{ 
				offset = loop * nummods;

				if (offset <= ModelAnims[i].offset)
					ModelAnims[i].offset -= nummods;
			}

		}

	}
}

short FindNumKeys(short i)
{
	for (int loop=0;loop<256;loop++)
	{
		if (ModelAnims[i].key[loop] <0)
			break;

	}

	return (loop);

}


void ProcAnimate (RDObjectExport *rootObject,FILE *output, IScene *scene, Interface *i)
{
   //	Animated = 0;
	int tempmaxkeys;
	int nummats=0;
	
	//GET THE NUMBER OF FRAMES IN THE ANIM
	TimeValue start = i->GetAnimRange().Start();
	TimeValue end = i->GetAnimRange().End();

	int delta = GetTicksPerFrame();

	int numframes = (int)((end - start) / delta);

	//CHECK TO SEE WHETHER THE OBJECT IS ANIMATED

   //	CheckForAnims(start,end,delta,rootObject->node);

	//FLAG OUT THE ANIM ATTACHED/NOT-ATTACHED INFO
	fwrite (&Animated, sizeof (int), 1, output);





	//..AND IF WE NEED IT, DUMP THE ANIM INFO.

	if (Animated)
  	{


	   //	ScaleNeeded = 0;
		//Check our MAX keyframes
		 nummaxkeys=0;
		 int loop;
		 for (loop=0;loop<MAX_MAXKEYS;loop++)
			 KArray[loop]=-1;

		 for (loop=0;loop<MAX_ANIMS;loop++)
			 anims[loop].output =0;

		 BuildMaxKeyList(rootObject->node);

		for (TimeValue s=start;s<=end;s+=delta)
		{
			//Check t against our HELPER keyframe entries
			CheckForNewKeyFrameAndAddToMax(s/delta);
		}

		 smaxkey=0;
		 for (loop=0;loop<MAX_MAXKEYS;loop++)
		 {
			 KSArray[loop].frame=-1;
			KSArray[loop].used=0;
		 }
		 SortKey();

		
		 if (interpo)
		 {
			
			ScaleNeeded = 0;
			for (int i=0;i<nummaxkeys;i++)
			{
				Matrix3 nodeTM	= rootObject->node->GetNodeTM(KSArray[i].frame*delta);
				CheckForScale(rootObject->node,nodeTM,KSArray[i].frame*delta,KSArray[i+1].frame*delta);

			
		  	}

			if (ScaleNeeded)
				interpo = 3;


		 }


		//OUTPUT THE ANIM TYPE 
	   	fwrite(&interpo,sizeof(int),1,output);

		//RESET ANY STUFF POTENTIALLY LEFT OVER FORM A PREVIOUS EXPORT
		ResetModelAnims();
		childtot = 1;

		GetObjTots(rootObject->node);



		//NOW CHECK FOR HELPER KEYFRAME NAMES
	 	//matt add
		//	for (s=start;s<end;s+=delta)
	  //	{
			//Check t against our HELPER keyframe entries
			CheckForNewKeyFrame(s/delta,childtot-1);
	   //	}

		if (!CurrentAnimRec)
			MessageBox (NULL, "YOU SHOULD NAME AT LEAST 1 KEYFRAME","WARNING", MB_OK);

		
		//..TO ADD, CHECK FOR INVALID FRAMES SO THE ARTISTS CAN TAKE OUT KEYFRAMES AT WILL




		 
				//added
		//PUT BACK IN AFTER FIRST TEST
	   
		tempmaxkeys = nummaxkeys;

//#if TEST
		if (interpo)
		{
			nummaxkeys = 0;

			 for (loop=0;loop<MAX_MAXKEYS;loop++)
			 {
				if (KSArray[loop].used)
			 		nummaxkeys++;
			}
		}
//#endif
		if (interpo)
	   		nummats = nummaxkeys * (childtot-1);
	   	else
			nummats = numframes * (childtot-1);

	   
		char message[16];

		itoa(nummaxkeys,message,10);

		MessageBox (NULL, message,"NUMBER OF KEYFRAMES USED IN ANIM", MB_OK);
	 
//#if TEST		
		if (interpo)
		{
			nummaxkeys = tempmaxkeys;

			UpdateOffsets(childtot-1);
		}
//#endif

		//OUTPUT NUMBER OF FRAMES BY NUMBER OBJECTS
	   	fwrite (&nummats, sizeof (int), 1, output);
	
		//OUTPUT THE NUMBER OF SUBMODELS
	   	fwrite (&childtot, sizeof (int), 1, output);


		


  
	 
		//OUTPUT OUR KEYFRAMES RECORD NUMBER
		fwrite(&CurrentAnimRec,sizeof(short),1,output);

		//..AND NOW THE JUMP TABLE INFO. ITSELF
		for (int i=0;i<CurrentAnimRec;i++)
		{
			fwrite(&ModelAnims[i].name,sizeof(char),32,output);					
			fwrite(&ModelAnims[i].offset,sizeof(short),1,output);					
			fwrite(&ModelAnims[i].numframes,sizeof(short),1,output);
			
			short numkeys = FindNumKeys(i);

			fwrite(&numkeys,sizeof(short),1,output);

			fwrite(&ModelAnims[i].key,sizeof(short),numkeys,output);					

			
		//	fwrite(&ModelAnims[i].key,sizeof(short),16,output);					

		}

		if (interpo)
			InterpArray = (INTERP*)malloc(sizeof(INTERP) * childtot);
		else
		 	MatrixArray = (MAT*)malloc(sizeof(MAT) * childtot);


		if (!interpo)
		{
			for (int t=start;t<end;t+=delta)
			{
   
					
				//OUTPUT ALL THE MATRICES FOR THIS NODES OBJECTS FOR THIS ANIM FRAME
				Matrix3 nodeTM	= rootObject->node->GetNodeTM(t);
				OutputMyAnims(rootObject->node,(TimeValue)t,nodeTM); 

				//..THE FIRST ARRAY[0] WILL CONTAIN THE ROOT OBJECT'S ARRAY, SO IGNORE IT			
				for (int i=1;i<childtot;i++)
			 		fwrite (&MatrixArray[i].m, sizeof(float)*16, 1, output);
					
			}
		}
		else
		{
			for (i=0;i<tempmaxkeys;i++)
			{
  //			   	#if	TEST
			 	if (!KSArray[i].used)
			 		continue;
	//		   	#endif
				Matrix3 nodeTM	= rootObject->node->GetNodeTM(KSArray[i].frame*delta);
				Quat ParentQuat = Quat(1);
				OutputMyKeyFrames(&ParentQuat,rootObject->node,nodeTM,KSArray[i].frame*delta,KSArray[i+1].frame*delta);

				for (int ni=1;ni<childtot;ni++)
				{
				 	   	if (ScaleNeeded)
							fwrite (&InterpArray[ni].scale, sizeof(Point3), 1, output);
		  /*	if ((InterpArray[ni].scale.x <= 0.00001f) ||
				(InterpArray[ni].scale.y <= 0.00001f) ||
				(InterpArray[ni].scale.z <= 0.00001f))
				int test = 1;
			*/
				 		fwrite (&InterpArray[ni].trans, sizeof(Point3), 1, output);
				 		
					 	if (ScaleNeeded)
							fwrite (&InterpArray[ni].dscale, sizeof(Point3), 1, output);
				 		fwrite (&InterpArray[ni].dtrans, sizeof(Point3), 1, output);

				 		fwrite (&InterpArray[ni].x, sizeof(float), 1, output);
				 		fwrite (&InterpArray[ni].y, sizeof(float), 1, output);
				 		fwrite (&InterpArray[ni].z, sizeof(float), 1, output);
				 		fwrite (&InterpArray[ni].w, sizeof(float), 1, output);

				
				
				}
			
		  	}

		}

		if (interpo)
		{
		   
			if (ScaleNeeded)
			{
				itoa(nummats * 64,message,10);
		   		MessageBox (NULL, message,"SCALED INTERP ANIM SIZE IN BYTES", MB_OK);
			}
			else
			{
				itoa(nummats * 40,message,10);
			 	MessageBox (NULL, message,"NONSCALED INTERP ANIM SIZE IN BYTES", MB_OK);
			}
		}
		else
		{
			itoa(nummats * 64,message,10);

			MessageBox (NULL, message,"ANIM SIZE IN BYTES", MB_OK);
		}




		if (interpo)
			free (InterpArray);
		else
			free (MatrixArray);
	}
}

#define DIFFERENT 1
int MatDiff (Matrix3 *src1, Matrix3 *src2)
{

	for (int i=0;i<4;i++)
	{
		Point3 srcrow = src1->GetRow(i);
		Point3 src2row = src2->GetRow(i);

		if (srcrow == src2row)
			continue;
		else
			return (DIFFERENT);
	}

	return(0);
}



// checks to see whether any of the node tms change over time 
void CheckForAnims(TimeValue start,TimeValue end, int delta,INode *node,RDObjectExport *rootObject)
{
   //	if (Animated)
   //		return;
   int oldchild = node->NumberOfChildren();
	

   Object *obj = node->GetObjectRef();
   if ((obj) && (
			(obj->ClassID() == camcontrols_CLASS_ID) ||
			(obj->ClassID() == COLLPOINT_CLASS_ID) ||
			(obj->ClassID() == OBJFLAGS_CLASS_ID) ||
			(obj->ClassID() == COMPOINT_CLASS_ID)) )
	 return;
	Matrix3 base = node->GetNodeTM(start);
   //	base = node->GetObjectTM(start);
	/*
	Matrix3 baseobj(1);
	Point3 pos = node->GetObjOffsetPos(start);	
	baseobj.PreTranslate(pos);
	Quat quat = node->GetObjOffsetRot(start);
	PreRotateMatrix(baseobj,quat);
	ScaleValue scaleValue = node->GetObjOffsetScale(start);
	ApplyScaling(baseobj,scaleValue);
	base = base * baseobj;
	  */
	for (int t=start;t<end;t+=delta)
	{
		//OUTPUT ALL THE MATRICES FOR THIS NODES OBJECTS FOR THIS ANIM FRAME
		Matrix3 nodeTM	= node->GetNodeTM(t);
	 //	nodeTM = node->GetObjectTM(t);
		
		/*
		Matrix3 baseobj2(1);
		Point3 pos = node->GetObjOffsetPos(t);	
		baseobj2.PreTranslate(pos);
		Quat quat = node->GetObjOffsetRot(t);
		PreRotateMatrix(baseobj2,quat);
		ScaleValue scaleValue = node->GetObjOffsetScale(t);
		ApplyScaling(baseobj2,scaleValue);
		nodeTM = nodeTM * baseobj2;
		  */


		//compare nodeTM to base
		if (MatDiff(&base,&nodeTM))
		//if (IsAnimated(node))
		{
			if (!Animated)
			{
			   //	if (!createid)
					Animated ++;
				TCHAR *name = node->GetName();
				if (!interpo)
					MessageBox (NULL, "ANIMATED OBJECT FOUND, TYPE MATRIX",name, MB_OK);
				else
					MessageBox (NULL, "ANIMATED OBJECT FOUND, TYPE INTERPOLATED",name, MB_OK);

			 	GetCOREInterface()->SelectNode(node);
				GetCOREInterface()->ExecuteMAXCommand(MAXCOM_ZOOMEXT_SEL);
				GetCOREInterface()->ExecuteMAXCommand(MAXCOM_MODIFY_MODE);
			 //	break;
			}

			//create an objflag and link it to this object
		    //ObjFlagCreatorClass *of = new ObjFlagCreatorClass();
	   
//#if 0
			   //	if (NeedObjFlag(node,oldchild,rootObject) && (!node->IsGroupHead()))
				if (NeedObjFlag(node,oldchild,rootObject))
				{
					   	TCHAR *NODENAME = node->GetName();

					//check to see whether group should have id attached
					if (node->IsGroupMember())
					{
						INode *Group = node->GetParentNode();
						TCHAR *name = Group->GetName();
						if (NeedObjFlag(Group,Group->NumberOfChildren(),rootObject))
						{
							ObjFlag *of = new ObjFlag();
   							of->SetID(AnimNo);
							int id = of->ID();
							INode *newnode = GetCOREInterface()->CreateObjectNode((of));
							Group->AttachChild(newnode,1);
							//mark the obj flag as being part of a group
						  	//to go in when we have a recursive 'open all groups' button
						 //	newnode->SetGroupMember(1);
							AnimNo++;
							#if MAX3
							  	SetSaveRequiredFlag(TRUE);
							#else
								SetSaveRequired(1);
							#endif
						}


					}
					else
					{
						ObjFlag *of = new ObjFlag();
   						of->SetID(AnimNo);
						INode *newnode = GetCOREInterface()->CreateObjectNode((of));
						int id = of->ID();
						node->AttachChild(newnode,1);
					   	
						AnimNo++;
						#if MAX3
							SetSaveRequiredFlag(TRUE);
						#else
							SetSaveRequired(1);
						#endif
					}
				}
//#endif

			break;
		}
		base = nodeTM;
	
	}

  //	for (int child = 0; child < node->NumberOfChildren(); ++child)
   for (int child = 0; child < oldchild; ++child)

		CheckForAnims(start,end,delta,node->GetChildNode(child),rootObject);


}



int OBJIDTOT;

INode *OBJIDLOG[64];

void ObjIDTotal(INode *node)
{
		int i,numchild;
		INode *oldnode=node;
		
		numchild = node->NumberOfChildren();

	  	Object *object = node->GetObjectRef();
	  	if (object)
	  	{
			if (object->ClassID() == OBJFLAGS_CLASS_ID)
			{
				OBJIDLOG[OBJIDTOT] = node;
				OBJIDTOT++;
			}
		}

	  //	if (!numchild)
	  //		return;

		for (i=0;i<numchild;i++)
		{
			node = oldnode->GetChildNode(i);
			ObjIDTotal(node);
		}
	}

#define MAXALLOWEDIDS 64

void ObjIdRename()
{
	TCHAR name[32];
	int i;

	for (i=0;i<OBJIDTOT;i++)
	{
		
		sprintf(name,"%s%d\0","ObjFlag",i+1);

		OBJIDLOG[i]->SetName(name);

		ObjFlag *of = (ObjFlag *)OBJIDLOG[i]->GetObjectRef();

	   
		of->SetID(i+1);
		

	}

	if (OBJIDTOT > MAXALLOWEDIDS)
		MessageBox (NULL, "TOO MANY OBJ FLAGS!!","MAX ALLOWED 64: Reduce the number of model objects by DELETING or ATTACHING", MB_OK);




}


void SumObjIDS()
{

	OBJIDTOT = 0;
	INode *newnode = GetCOREInterface()->GetRootNode();
	 
	ObjIDTotal(newnode);

	ObjIdRename();

}




void CheckForAnimate(RDObjectExport *rootObject,Interface *i)
{
	Animated = 0;
	AnimNo=0;
	int nummats=0;
	
	//GET THE NUMBER OF FRAMES IN THE ANIM
	TimeValue start = i->GetAnimRange().Start();
	TimeValue end = i->GetAnimRange().End();

	int delta = GetTicksPerFrame();

	int numframes = (int)((end - start) / delta);

	//CHECK TO SEE WHETHER THE OBJECT IS ANIMATED

	CheckForAnims(start,end,delta,rootObject->node,rootObject);

	if (Animated)
		SumObjIDS();



}




int ObjSave (FILE *output, IScene *scene, Interface *i)
{
	assert (i);

	Tab <INode *> inodeTable;

	// Get all the parent nodes
	RDObjectEnumerator en (scene, i, inodeTable);

	// For all children, add them to the root object, first for an anim check
	RDObjectExport rootObjectForAnim;

	//animcheck here
   	for (int node = 0; node < inodeTable.Count(); ++node) {
   		RDObjectExport *obj = new RDObjectExport (i, inodeTable[node], &rootObjectForAnim);
   	}

	CheckForAnimate(&rootObjectForAnim,i);

	// For all children, add them to the root object, now for real
	RDObjectExport rootObject;

   	for (node = 0; node < inodeTable.Count(); ++node) {
   		RDObjectExport *obj = new RDObjectExport (i, inodeTable[node], &rootObject);
   	}

  	rootObject.Optimise();

	rootObject.Prepare();



	
	
	
	// Is this a convert, or export?
	if (output) {
		char *name = "Exported object";
	
		RedDog::ObjectHeader header = { OBJECTHEADER_TAG, OBJECTHEADER_VERSION, 0 };
		fwrite (&header, sizeof (header), 1, output);
		rootObject.Output (output);
		
		//animate save here
	  	Interval range = i->GetAnimRange();
		//FIRST CHECK FOR ASSOCIATED ANIMS


	  	ProcAnimate(&rootObject,output,scene,i);
	
	
	} else {
		ofstream o("c:\\WINDOWS\\TEMP\\convert.bat");
		o << "echo Converting textures" << endl;
		rootObject.Convert (o);
		o << "exit" << endl;
		o.close();
		_spawnl (_P_WAIT, "c:\\WINDOWS\\TEMP\\convert.bat", "c:\\WINDOWS\\TEMP\\convert.bat", NULL);
	}
	return TRUE;
}

static void MakeQuad2 (Face *f, int a, int b, int c, int d)
{
	f[0].setVerts (b, a, c);
	f[0].setSmGroup (0);
	f[0].setEdgeVisFlags (1,0,1);
	f[1].setVerts (d, c, a);
	f[1].setSmGroup (0);
	f[1].setEdgeVisFlags (1,0,1);
}
int face=0;
#define MAKE_QUAD(na,nb,nc,nd) {MakeQuad2 (&(mesh.faces[face]),na, nb, nc, nd);face+=2;}

void BoxMake(Mesh &mesh,float minX,float minY,float minZ,float maxX,float maxY, float maxZ)
{
	face = 0;
	
	mesh.setNumVerts(8);
	mesh.setNumFaces(12);
	mesh.setNumTVerts(0);
	mesh.setNumTVFaces(0);
	
	mesh.setVert (0, minX, maxY, maxZ);
	mesh.setVert (1, maxX, maxY, maxZ);
	mesh.setVert (2, maxX, minY, maxZ);
	mesh.setVert (3, minX, minY, maxZ);
	mesh.setVert (4, minX, maxY, minZ);
	mesh.setVert (5, maxX, maxY, minZ);
	mesh.setVert (6, maxX, minY, minZ);
	mesh.setVert (7, minX, minY, minZ);
	
	MAKE_QUAD (4,5,1,0);
	MAKE_QUAD (6,7,3,2);
	MAKE_QUAD (1,5,6,2);
	MAKE_QUAD (4,0,3,7);
	MAKE_QUAD (7,6,5,4);
	MAKE_QUAD (0,1,2,3);
	
}

RedDog::Object *rootObject;
int MaxPoly, Verts;

void TriSet(RedDog::VertRef vert1, RedDog::VertRef vert2, RedDog::VertRef vert3,
			Mesh *mesh, int Poly, int vOff)
{
	mesh->faces[Poly].setVerts (vert1 + vOff, vert2 + vOff, vert3 + vOff);
	mesh->faces[Poly].setSmGroup (0);
	mesh->faces[Poly].setEdgeVisFlags (1,1,1);
}

void QuadSet(RedDog::VertRef vert1, RedDog::VertRef vert2, RedDog::VertRef vert3, RedDog::VertRef vert4,
			 Mesh *mesh, int Poly, int vOff)
{
	mesh->faces[Poly].setVerts (vert1 + vOff, vert2 + vOff, vert3 + vOff);
	mesh->faces[Poly].setSmGroup (0);
	mesh->faces[Poly].setEdgeVisFlags (1, 0, 1);

	mesh->faces[Poly+1].setVerts (vert3 + vOff, vert2 + vOff, vert4 + vOff);
	mesh->faces[Poly+1].setSmGroup (0);
	mesh->faces[Poly+1].setEdgeVisFlags (0, 1, 1);
}

void MeshBuild(RedDog::Model *mod, Mesh *mesh, int mode, Matrix3 &mat)
{
	RedDog::VertRef *point, nStrip, mNum;
	int Poly, strip;

	if (mode == 0) {
		// Just count the number of vertices and polygons
		Verts += mod->nVerts;
		MaxPoly += mod->nTris + mod->nQuads * 2 + mod->nStripPolys;
	} else {
		RedDog::VertRef a, b, c, d;
		// Transform the vertices
		for (int i = 0; i < mod->nVerts; ++i) {
			Point3 p(mod->vertex[i].x, mod->vertex[i].y, mod->vertex[i].z);
			p = p * mat;
			mesh->setVert (Verts + i, p);
		}
		// Tris
		Poly = 0;
		point = mod->strip;
		while (Poly < mod->nTris) {
			mNum = *point++;
			nStrip = *point++;
			for (strip = 0; strip < nStrip; ++strip) {
				a = *point++;
				b = *point++;
				c = *point++;
				TriSet (a, b, c, mesh, MaxPoly, Verts);
				MaxPoly++;
			}
			Poly+=nStrip;
		}
		// Quads
		Poly = 0;
		while (Poly < mod->nQuads) {
			mNum = *point++;
			nStrip = *point++;
			for (strip = 0; strip < nStrip; ++strip) {
				a = *point++;
				b = *point++;
				c = *point++;
				d = *point++;
				QuadSet (a, b, c, d, mesh, MaxPoly, Verts);
				MaxPoly += 2;
			}
			Poly+=nStrip;
		}
		// Strip info
		Poly = 0;
		while (Poly < mod->nStripPolys) {
			bool flip = false;
			mNum = *point++;
			nStrip = *point++ - 2;
			a = *point++;
			b = *point++;
			for (strip = 0; strip < nStrip; ++strip) {
				c = *point++;
				if (!flip)
					TriSet (a, b, c, mesh, MaxPoly, Verts);
				else
					TriSet (b, a, c, mesh, MaxPoly, Verts);
				MaxPoly ++;
				a = b;
				b = c;
				flip = !flip;
			}
			Poly+=(nStrip);
		}
		Verts += mod->nVerts;
	}
}

Matrix3 stack[64];
int stackval = 0;

void Push(Matrix3 *mat)
{
	stack[stackval]=*mat;
	stackval++;
}

void Pop(Matrix3 *mat)
{
	stackval--;
	*mat= stack[stackval];
}


void BuildMesh(RedDog::Object *obj,Mesh *mesh,int mode, Matrix3 mParent)
{
	int i;
  	Matrix3 m(true);
  	Matrix3 m2(true);

	m.Scale (Point3(obj->scale.x, obj->scale.y, obj->scale.z));

	m.RotateY (TOFLOAT(obj->idleRot.y));
	m.RotateX (TOFLOAT(obj->idleRot.x));
	m.RotateZ (TOFLOAT(obj->idleRot.z));
	
  	m.Translate (Point3 (obj->idlePos.x, obj->idlePos.y, obj->idlePos.z));

  	m2 = m * mParent;
	

	if (obj->model)
		MeshBuild(obj->model,mesh,mode,m2);

	for (i=0;i<obj->no_child;i++)
	{
		Push(&m2);
		BuildMesh(&obj->child[i],mesh,mode,m2);
		Pop(&m2);
	}
}

void LoadModel(FILE *rdofile, RedDog::Model *retVal)
{
	RedDog::ModelHeader modelheader;
	
	fread(&modelheader,sizeof(modelheader),1,rdofile);

	if (modelheader.version < 0x140) {
		if (fread(retVal,sizeof(RedDog::Model)-8,1,rdofile) != 1) {
			assert (FALSE);
		};
	} else if (modelheader.version < 0x150) {
		if (fread(retVal,sizeof(RedDog::Model)-4,1,rdofile) != 1) {
			assert (FALSE);
		};
	} else{
		if (fread(retVal,sizeof(RedDog::Model),1,rdofile) != 1) {
			assert (FALSE);
		};
	}

	// Read the material array
	if (retVal->nMats) {
		retVal->material = new RedDog::Material[retVal->nMats];
		for (int mat = 0; mat < retVal->nMats; ++mat) {
			fread (&retVal->material[mat], sizeof (RedDog::Material), 1, rdofile);
			char buffer[1024];
			if (retVal->material[mat].surf.GBIX)
				fread (buffer, retVal->material[mat].surf.GBIX, 1, rdofile);
			if (retVal->material[mat].pasted.surf.GBIX)
				fread (buffer, retVal->material[mat].pasted.surf.GBIX, 1, rdofile);
		}
	}
	// Read the vertex information
	retVal->vertex = new RedDog::Point3[retVal->nVerts];
	retVal->vertexNormal = new RedDog::Vector3[retVal->nVerts];
	fread (retVal->vertex, sizeof (RedDog::Point3), retVal->nVerts, rdofile);
	fread (retVal->vertexNormal, sizeof (RedDog::Vector3), retVal->nVerts, rdofile);

	/* Derive some data */
	int nPolys = retVal->nQuads + retVal->nTris + retVal->nStripPolys;
	int nEdges = (retVal->nQuads * 4) + (retVal->nTris * 3) + (retVal->nStrips * 2 + retVal->nStripPolys);

	/* Read in the plane equations */
	if (retVal->plane) {
		retVal->plane = new RedDog::Plane [nPolys];
		fread (retVal->plane, sizeof (RedDog::Plane), nPolys, rdofile);
	}
	
	/* Read in the UV values */
	retVal->uv = new RedDog::UV[nEdges];
	fread (retVal->uv, sizeof (RedDog::UV), nEdges, rdofile);

	/* Read in the bumpmap info */
	if ((retVal->bump) && (LoadBump)) {
		retVal->bump = new RedDog::BumpMapInfo [retVal->nVerts];
		fread (retVal->bump, sizeof (RedDog::BumpMapInfo), retVal->nVerts, rdofile);
	}
	else
		retVal->bump = NULL;

	/* Finally, read in the strip info */
	int temp = (int)retVal->strip;
	retVal->strip = new RedDog::VertRef[temp];
	fread (retVal->strip, sizeof (RedDog::VertRef), temp, rdofile);
}

void FreeModelData (RedDog::Model *m)
{
	if (m->nMats)
		delete [] m->material;
	delete [] m->vertex;
	delete [] m->vertexNormal;
	delete [] m->plane;
	delete [] m->uv;
	delete [] m->strip;	
	delete [] m->bump;
}

void LoadObject(FILE *rdofile,RedDog::Object *obj)
{
	int i,child;

	Point3 dummy;
	fread (obj, sizeof(RedDog::Object),1,rdofile);
	for (i=0;i<obj->ncp;i++) {
		fread(&dummy,sizeof(dummy),1,rdofile);
		if (LoadNewCP) {
			float giblet;
			fread(&giblet,sizeof(giblet),1,rdofile);
		}
	}


	if (obj->model)
	{
		obj->model=(RedDog::Model*)malloc(sizeof(RedDog::Model));
		LoadModel(rdofile,obj->model);
	}		

	if (obj->no_child)
	{
		obj->child = (RedDog::Object*)malloc(sizeof(RedDog::Object) * obj->no_child);

	   //	obj->no_child = 1;
		for (child=0;child < obj->no_child; ++child)
			LoadObject(rdofile,&obj->child[child]);
	}
}

int LoadPNode(FILE *rdofile)
{
	RedDog::ObjectHeader header;

	fread (&header, sizeof (header), 1, rdofile);
	if (header.version >= 0x192)
		LoadBump = true;
	else
		LoadBump = false;

	if (header.version >= 0x190)
		LoadNewCP = true;
	else LoadNewCP = false;
	if (header.version < 0x185)
		return (0);

	rootObject = (RedDog::Object*)malloc(sizeof(RedDog::Object));
	LoadObject(rdofile,rootObject);
	fclose(rdofile);

	return(1);
}

void PNodeFree (RedDog::Object *obj)
{
	if (obj) {
		if (obj->model) {
			FreeModelData (obj->model);
			free (obj->model);
		}
		if (obj->no_child) {
			for (int i = 0; i < obj->no_child; ++i) {
				PNodeFree (&obj->child[i]);
			}
			free (obj->child);
		}
	}
}

int ObjLoad (char *infile, Mesh &mesh, int Mode)
{
	char buffer[MAX_PATH];
	FILE *input;
	
	
	//mode 0 - set up a box mesh for the object
	//mode 1 - the actual mesh itself
	
	//BOX LOAD
	if (!Mode) // xxx here
	{
		BoxMake (mesh, -2, -2, -2, 2, 2, 2);
		
	}
	else
	{
		//UNTIL ALL THINGS ARE RE-SAVED OUT
   //		return FALSE;
		// Try and open the file
		sprintf (buffer, "P:\\GAME\\NEWOBJECTS\\%s", infile);
		if ((input = fopen (buffer, "rb")) == NULL)
		{
			//ENSURE A BOX IS KEPT FOR AN INVALID OBJECT
			ObjLoad (infile, mesh, 0);
		 
			return FALSE;
		}
		
		//LOADS PNODE OBJECTS WITH VERSION 0X185
		
		if (LoadPNode(input))
		{
			//FREE EXISTING MESH WAREZ
			mesh.InvalidateTopologyCache();
			mesh.InvalidateGeomCache();
			mesh.InvalidateEdgeList();
			mesh.InvalidateStrips();
			mesh.FreeAll();
			mesh.ClearFlag(MESH_LOCK_RENDDATA);
			
			Matrix3 m(true);
			stackval = 0;
			Verts = MaxPoly = 0;
			BuildMesh (rootObject, &mesh, 0, m);
			mesh.setNumVerts (Verts);
			mesh.setNumFaces (MaxPoly);
			mesh.setNumTVerts (0);
			mesh.setNumTVFaces (0);
						
			m.IdentityMatrix();
			stackval = 0;
			Verts = MaxPoly = 0;
			BuildMesh (rootObject, &mesh, 1,m);
			
			mesh.BuildStripsAndEdges();
			mesh.buildBoundingBox();
			
			//CLEAN UP THE LOADED OBJECT
			PNodeFree(rootObject);
			free(rootObject);
		}
		else
			//ENSURE A BOX IS KEPT FOR AN INVALID OBJECT
			ObjLoad (infile, mesh, 0);
	}
	return TRUE;
}


/****************************************************************/


// Convert all the textures in a model
void RDObjectExport::Convert (ofstream &o)
{
	TSTR cmdLine;
	int material;

	for (material = 0; material < matLib.Count(); ++material) {
		char buf[256];//, buf2[256], num[64];
		const char *matName = matLib[material].getTexName();
		char *lastSlash, *p;

		if (!strnicmp (REDDOG_ORIG_TEXMAPS, matName, strlen (REDDOG_ORIG_TEXMAPS))) {
			strcpy (buf, matName + strlen (REDDOG_ORIG_TEXMAPS));
		} else if (!strnicmp (REDDOG_ORIG_TEXMAPS_2, matName, strlen (REDDOG_ORIG_TEXMAPS_2))) {
			strcpy (buf, matName + strlen (REDDOG_ORIG_TEXMAPS_2));
		} else {
			OutputDebugString ("Unknown texture :");
			OutputDebugString (buf);
			OutputDebugString ("\n");
			continue;
		}

		for (lastSlash = p = &buf[0]; *p; ++p)
			if (*p=='\\')
				lastSlash = p;
		*lastSlash = '\0';
		cmdLine.printf ("p:\\utils\\pvrconv.exe -gi %d %s -p P:\\GAME\\TEXMAPS\\%s", rand() & 0xfffffff, matName, &buf[0]);
		o << cmdLine << endl;

		cmdLine.printf ("P:\\GAME\\TEXMAPS\\%s", &buf[0]);
		const char *dir = (const char *)cmdLine, *base = (const char *)cmdLine;
		dir = dir + 8;
		while (*dir) {
			TSTR directory;
			directory = cmdLine.Substr (0, dir - base);
			CreateDirectory (directory, NULL);
			for (;*dir && *dir != '\\'; ++dir );
			while (*dir=='\\') ++dir;
		}
		CreateDirectory (cmdLine, NULL);
	}

	for (int child = 0; child < children.Count(); ++child)
		children[child]->Convert(o);
}

void RDObjectExport::Output (FILE *f)
{

	// Create a RDO from this
	RedDog::Object object;
	memset (&object, 0, sizeof (RedDog::Object));
	if (polygons)
		object.model = (RedDog::Model *)1;
	else
		object.model = NULL;
	object.crntRot.x = NJM_RAD_ANG (rotation.x);
	object.crntRot.y = NJM_RAD_ANG (rotation.y);
	object.crntRot.z = NJM_RAD_ANG (rotation.z);
	object.idleRot.x = NJM_RAD_ANG (rotation.x);
	object.idleRot.y = NJM_RAD_ANG (rotation.y);
	object.idleRot.z = NJM_RAD_ANG (rotation.z);

	object.scale	= *(RedDog::Point3 *)((float *)scale);
	object.idlePos	= object.crntPos = *(RedDog::Point3 *)((float *)translation);
	object.ncp		= collisionPoints.Count();
	object.no_child	= children.Count();
	object.cp		= (collisionPoints.Count()==0) ? NULL : (RedDog::CP2 *)1;
	object.collFlag	= CollFlags | OBJECT_MAKE_ID(Id);
	object.specularCol.colour	= 0;
   /*	for (int collPt = 0; collPt < collisionPoints.Count(); ++collPt) {
		if (collisionPoints[collPt].rotates)
			object.cpType |= (1<<collPt);
	}*/
	if (kludgeAllCalcMatrix)
		object.collFlag |= OBJECT_CALC_MATRIX;

	// Copy in the matrix
	object.m.m[0][0]	= localTM.GetRow(0).x;
	object.m.m[0][1]	= localTM.GetRow(0).y;
	object.m.m[0][2]	= localTM.GetRow(0).z;
	object.m.m[0][3]	= 0.f;
	object.m.m[1][0]	= localTM.GetRow(1).x;
	object.m.m[1][1]	= localTM.GetRow(1).y;
	object.m.m[1][2]	= localTM.GetRow(1).z;
	object.m.m[1][3]	= 0.f;
	object.m.m[2][0]	= localTM.GetRow(2).x;
	object.m.m[2][1]	= localTM.GetRow(2).y;
	object.m.m[2][2]	= localTM.GetRow(2).z;
	object.m.m[2][3]	= 0.f;
	object.m.m[3][0]	= localTM.GetRow(3).x;
	object.m.m[3][1]	= localTM.GetRow(3).y;
	object.m.m[3][2]	= localTM.GetRow(3).z;
	object.m.m[3][3]	= 1.f;

	fwrite (&object, sizeof (object), 1, f);

	if (object.cp) {
		for (int collPt = 0; collPt < object.ncp; ++collPt) {
			fwrite ((float *)collisionPoints[collPt].pt,		sizeof (float), 3, f);
			fwrite (&collisionPoints[collPt].radius,	sizeof (float), 1, f);
		}
	}

	if (polygons) {
		RedDog::Point3 p = {0,0,0};
		RDSave::SaveRedDogModel (boundary, polygons, f, p, &matLib, true, ModelFlags);
	}

	for (int child = 0; child < children.Count(); ++child)
		children[child]->Output(f);

}

void RDObjectExport::Prepare ()
{
	polygons = MeshPrep (accMesh, &uvTLib[0], boundary, matLib);
	if (CollFlags & OBJECT_HIGH_DETAIL_COLLISION)
		polygons = RDSave::Sort (polygons);
	// Recurse down to children
	for (int child = 0; child < children.Count(); ++child)
		children[child]->Prepare();
	// Check for null children
	for (child = 0; child < children.Count(); ++child) {
		if (children[child]->Id == 0 &&
			children[child]->polygons == NULL &&
			children[child]->children.Count() == 0) {
			delete children[child];
			children.Delete(child, 1);
			--child;
		}
	}
}



RDObjectExport::RDObjectExport (Interface *maxInterface, INode *node, RDObjectExport *parent) : boundary (0,0,0,0,0,0)
{
	int child;
	RDObjectExport *dummy = this;

	assert (node); assert (parent); assert (maxInterface);

	this->node = node;

  	TCHAR *name = node->GetName();
	
	
	// Add to parent's list of children
	parent->children.Append (1, &dummy);

	CollFlags	= Id = ModelFlags = 0;

	polygons = NULL;

	//



	// Calculate the translation, rotation and scaling factors
	Matrix3		parentTM, nodeTM;
	INode		*mum;

	TimeValue start = GetCOREInterface()->GetAnimRange().Start();
	TimeValue end = GetCOREInterface()->GetAnimRange().End();


	start = GetCOREInterface()->GetTime();
	int delta = GetTicksPerFrame();


	nodeTM		= node->GetNodeTM(start);
	mum			= node->GetParentNode();
	parentTM	= mum->GetNodeTM(start);

	localTM		= nodeTM * Inverse(parentTM);

	Quat quat;
	DecomposeMatrix (localTM, translation, quat, scale);

//	GetRealQuat(localTM,&quat);
	
	//test going from our quats back to a matrix for display by Matt's renderer
//	Matrix3 test;
//	QuatToMat(&quat,&test);
	
	
	// Pull out the rotation
	Matrix3 unscaled;

	// Unscale the matrix
	unscaled = localTM;
	unscaled.SetRow (0, Normalize(unscaled.GetRow(0)));
	unscaled.SetRow (1, Normalize(unscaled.GetRow(1)));
	unscaled.SetRow (2, Normalize(unscaled.GetRow(2)));

	if (localTM.Parity()) {
		unscaled.SetRow (0, unscaled.GetRow(0) * -1.f);
		unscaled.SetRow (1, unscaled.GetRow(1) * -1.f);
		unscaled.SetRow (2, unscaled.GetRow(2) * -1.f);
		scale	*= -1.f;
	}

//	test.PreScale(scale);
//	test.Translate(translation);

//	localTM = test;



	// Find the x rotation
	rotation.x = (float) asin (unscaled.GetRow(1).z);

	// Check cos(x)
	if (fabs(cos(rotation.x)) > 0.001f) {
		rotation.y	= (float) atan2 (-unscaled.GetRow(0).z, unscaled.GetRow(2).z);
		rotation.z	= (float) atan2 (-unscaled.GetRow(1).x, unscaled.GetRow(1).y);
	} else {
		rotation.y	= (float) atan2 (unscaled.GetRow(2).x, unscaled.GetRow(0).x);
		rotation.z	= 0.f;
	}

  //	QuatToEuler(quat,ang);


  //	rotation.x = ang[0];
  //	rotation.y = ang[1];
  //	rotation.z = ang[2];
#if 0
	// Compare matrices
	Matrix3 comp(TRUE);
	comp.Scale (scale);
	comp.RotateY (rotation.y);
	comp.RotateX (rotation.x);
	comp.RotateZ (rotation.z);
	comp.Translate (translation);

	Point3 proper(10,20,30), mine(10,20,30);
	proper	= proper * localTM;
	mine	= mine * comp;
	if (Length (proper - mine) > 0.01f) {
		printf ("poo");		
	}
#endif

	// Recurse down to children
	for (child = 0; child < node->NumberOfChildren(); ++child)
		new RDObjectExport (maxInterface, node->GetChildNode (child), this);

	// Get any associated geometry
	Object *obj = node->EvalWorldState(maxInterface->GetTime()).obj;

	// Make sure it isn't a Red Dog Object, or collision point
	if (obj->ClassID() == COLLPOINT_CLASS_ID) {
		// It's a collision point, so add it to its parent as such
		Point3 p(0,0,0);
		p = p * localTM;
		CollPt c(p, ((CollPoint *)obj)->Rotates(), ((CollPoint *)obj)->GetRadius());
		parent->collisionPoints.Append (1, &c);
	} else if (obj->ClassID() == OBJFLAGS_CLASS_ID) {
		// It's a flag object, so alter its parent accordingly
		ObjFlag *flags = (ObjFlag *)obj;
		if (flags->CalcMatrix())
			parent->CollFlags |= OBJECT_CALC_MATRIX;
		if (flags->Targetable())
			parent->CollFlags |= COLL_TARGETABLE;
		if (flags->HighDColl())
			parent->CollFlags |= OBJECT_HIGH_DETAIL_COLLISION;
		if (flags->Unlit())
			parent->ModelFlags |= MODELFLAG_UNLIT;
		if (flags->FaceOn())
			parent->ModelFlags |= MODELFLAG_FACEON;
		if (flags->StratMove())
			parent->CollFlags |= OBJECT_STRATMOVE;
		if (flags->WaterEffect())
			parent->ModelFlags |= MODELFLAG_WATER_EFFECT;
		if (flags->LavaEffect())
			parent->ModelFlags |= MODELFLAG_LAVA_EFFECT;
		switch (flags->Shadow()) {
		case 0:
			break;
		case 1:
			parent->ModelFlags |= MODELFLAG_SHADOW_BBOX;
			break;
		case 2:
			parent->ModelFlags |= MODELFLAG_SHADOW_BBOX | MODELFLAG_SHADOW_ROUND;
			break;
		}
		parent->Id = flags->ID();
	} else if (obj->ClassID() == COMPOINT_CLASS_ID) {
		Point3 p(0,0,0);
		grossCOM = p * localTM;
	} else if (
		(obj->ClassID() != RDOBJECT_CLASS_ID) &&
		(obj->ClassID() != camcontrols_CLASS_ID)
		)
	{
		// Retrieve the TriObject from the node
		int deleteIt;
		TriObject *triObject = GetTriObjectFromNode (maxInterface, node, deleteIt);
		
		// Use the TriObject if available
		if (triObject) {
			// Add this mesh to the object
			Matrix3 transform (TRUE);
			// Take into account the object-offset here
			transform.PreTranslate (node->GetObjOffsetPos());
			PreRotateMatrix (transform, node->GetObjOffsetRot());
			ApplyScaling (transform, node->GetObjOffsetScale());

			AddMesh (node->GetMtl(), triObject->mesh, &transform, node->GetName());

			// Delete it when done...
			if (deleteIt) triObject->DeleteMe();
		}
		else
		{
			int test=1;
		}
	}
}

/*
 * Optimises the internal structure of an RDObjectExport
 * by moving geometry up a level in the heirarchy if
 * no variable matrices or Flags are found
 */
bool RDObjectExport::Optimise (RDObjectExport *parent /*= NULL*/)
{
	/*
	 * First, recurse down (using childrenCopy as children could
	 * be altered in the process
	 */
	for (int child = 0; child < children.Count(); ) {
		if (children[child]->Optimise (this)) {
			// This child was optimised away
			delete children[child];
			children.Delete(child,1);
		} else {
			// Was not optimised, move on
			child++;
		}
	}

	/*
	 * If we have no parent, no optimisation can take place
	 */
	if (parent == NULL)
		return false;

	/*
	 * Now, with all children having passed the relevant data into 'us'
	 * we can see if we are 'constant' with respect to our parent
	 */
	if (Id != 0 || CollFlags & OBJECT_CALC_MATRIX)
		return false;	// No further optimisation can be done here

	/*
	 * If we got here, then we are constant wrt our parent.
	 * In order to preserve the local co-ordinate semantics,
	 * I consider a node with any children non-optimisable :
	 * In the case where we have non-optimisable children
	 * we would have to multiply our children's matrices with the
	 * parent offset matrix too - which could result in a change
	 * of co-ordinate system in our child
	 */
	if (children.Count() != 0)
		return false; // No further optimisation possible at the moment

	/*
	 * Put all our collision points to our parent
	 */
	if (collisionPoints.Count()) {
		for (int point = 0; point < collisionPoints.Count(); ++point) {
			CollPt newPoint (collisionPoints[point]);
			newPoint.pt = collisionPoints[point].pt * localTM;
			parent->collisionPoints.Append (1, &newPoint);
		}
	}

	/*
	 * Reassign the material IDs of all the polygons, and reverse the
	 * polygon order as necessary
	 */
	int reverse = localTM.Parity();
	Mesh reassignedMesh (accMesh);

	for (int face = 0; face < reassignedMesh.numFaces; ++face) {
		Face &f = reassignedMesh.faces[face];
		// Find the material associated with this face
		int matID = f.getMatID();
		int slot;
		if (matLib.Count() == 0) {
			slot = 0;
		} else
			slot = parent->matLib.AddObject (&matLib[matID % matLib.Count()]);
		assert (slot < MAX_MTLS);
		// Set the face's material ID to be the slot number
		f.setMatID (slot);
		// Paranoia checks
		assert (slot == f.getMatID());
		parent->uvTLib[slot] = matLib[matID].uvTransform;
		if (reverse) {
			f.setVerts (f.getVert(2), f.getVert(1), f.getVert(0));
			if (reassignedMesh.tvFace) {
				TVFace &tFace = reassignedMesh.tvFace[face];
				tFace.setTVerts (tFace.getTVert(2), tFace.getTVert(1), tFace.getTVert(0));
			}
		}
	}

	/*
	 * Otherwise, just take our matrix, to get us
	 * into parent-space, and combine our mesh with our parent's
	 */
	Mesh resultantMesh;
	CombineMeshes (resultantMesh, reassignedMesh, parent->accMesh, &localTM, NULL, -1);
	parent->accMesh = resultantMesh;
	
	return true; // We have been optimised away
}
