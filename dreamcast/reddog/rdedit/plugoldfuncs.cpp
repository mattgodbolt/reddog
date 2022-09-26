#include "stdafx.h"
#if 0
void OutputMyKeyFrames(INode *node,Matrix3 basemat,TimeValue t, TimeValue nextt)
{

		INode	*mum		= node->GetParentNode();
 		Matrix3	localTM,parentTM;
	   	if (mum)
	   	{
			/*Object *mumobj = mum->EvalWorldState(GetCOREInterface()->GetTime()).obj;
	 		if ((mumobj) && (mumobj->ClassID() == Class_ID(DUMMY_CLASS_ID,0)))
				parentTM	= mum->GetParentNode()->GetNodeTM(t);
			else*/
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

			int delta = GetTicksPerFrame();

			Quat currentquat;
			Point3 currentrot,currenttrans,currentscale; 
  
			//GET THE ACTUAL ROT,SCALE AND TRANS DETAILS FOR OUR MATRIX AT TIME T				
			DecomposeMatrix (localTM, currenttrans, currentquat, currentscale);

		   
			#if 0
				// Unscale the matrix
				Matrix3 unscaled = localTM;
				unscaled.SetRow (0, Normalize(unscaled.GetRow(0)));
				unscaled.SetRow (1, Normalize(unscaled.GetRow(1)));
				unscaled.SetRow (2, Normalize(unscaled.GetRow(2)));

				/*..and rip out its quaternion repr*/			
				MatToQuat(&unscaled,&currentquat);
				currentquat.Normalize();
			#endif

			currentrot = GetRealRot(localTM,&currentscale);



			//FETCH THE MATRIX DATA FOR TIME T+DELTA
			
			Matrix3 nextmat = node->GetNodeTM(nextt);

			if (mum)
			{
	 			Matrix3 nparentTM;
				/*Object *mumobj = mum->EvalWorldState(GetCOREInterface()->GetTime()).obj;
				if ((mumobj) && (mumobj->ClassID() == Class_ID(DUMMY_CLASS_ID,0)))
					nparentTM	= mum->GetParentNode()->GetNodeTM(nextt);
				else*/ 
					nparentTM	= mum->GetNodeTM(nextt);
				localTM	= nextmat * Inverse(nparentTM);
		   	}
		   	else
		   		localTM = nextmat;
			   
			//GET THE ACTUAL ROT,SCALE AND TRANS DETAILS FOR OUR MATRIX AT TIME T+DELTA				

			Quat nextquat;
			Point3 nextrot,nexttrans,nextscale; 
			DecomposeMatrix (localTM, nexttrans, nextquat, nextscale);
			nextrot = GetRealRot(localTM,&nextscale);

		  int interprange = (t/delta)-(nextt/delta);
	
#if 0
			int thisframe = t/delta;
			int nextframe = nextt/delta;
			nextquat.Normalize();

			Quat nowquat = Slerp(nextquat,currentquat,(float)1/interprange);
		   	nowquat.Normalize();
#endif			
	   
			/*DERIVE A ROTATION MATRIX IN THE SAME WAY AS IN GAME*/
			InterpArray[ID].rot.x = NJM_RAD_ANG(currentrot.x); 
			InterpArray[ID].rot.y = NJM_RAD_ANG(currentrot.y); 
			InterpArray[ID].rot.z = NJM_RAD_ANG(currentrot.z); 

			InterpArray[ID].x = NJM_RAD_ANG(currentquat.x);
			InterpArray[ID].y = NJM_RAD_ANG(currentquat.y);
			InterpArray[ID].z = NJM_RAD_ANG(currentquat.z);
			InterpArray[ID].w = currentquat.w;

			InterpArray[ID].scale = currentscale; 



			//FAILS
#if 0
			if 	(
				(currentscale.x != 1.0f) ||
				(currentscale.y != 1.0f) ||
				(currentscale.z != 1.0f))
		   	{
			 	ScaleNeeded = 1;

			}
			if 	(
				(nextscale.x != currentscale.x) ||
			   (nextscale.y != currentscale.y) ||
			(nextscale.z != currentscale.z))
			{
				ScaleNeeded = 1;
		   	}
#endif

			float len1 = LengthSquared(nextscale);
			float len2 = LengthSquared(currentscale);

		  //	if ((Length(nextscale) != (Length(currentscale)))
			if ((len1 != len2))
			  	ScaleNeeded = 1;

			if ((len2) != 3.0f)
			  	ScaleNeeded = 1;



			InterpArray[ID].trans = currenttrans; 

			InterpArray[ID].drot.x = NJM_RAD_ANG((nextrot.x - currentrot.x)/(float)interprange); 
			InterpArray[ID].drot.y = NJM_RAD_ANG((nextrot.y - currentrot.y)/(float)interprange); 
			InterpArray[ID].drot.z = NJM_RAD_ANG((nextrot.z - currentrot.z)/(float)interprange); 

			InterpArray[ID].dscale = (nextscale - currentscale)/(float)interprange; 
			InterpArray[ID].dtrans = (nexttrans - currenttrans)/(float)interprange; 
				
		}

		for (int child = 0; child < node->NumberOfChildren(); ++child)
		{ 
		/*	Object *obj = node->GetChildNode(child)->EvalWorldState(GetCOREInterface()->GetTime()).obj;
		 	Matrix3 UnitMat;
			UnitMat.IdentityMatrix();

			if ((obj) && (obj->ClassID() == Class_ID(DUMMY_CLASS_ID,0)))
				OutputMyKeyFrames(node->GetChildNode(child),UnitMat,t,nextt);
			else*/			
				OutputMyKeyFrames(node->GetChildNode(child),node->GetChildNode(child)->GetNodeTM(t),t,nextt);
		}

 //			OutputMyAnims(output,node->GetChildNode(child),t,node->GetChildNode(child)->GetObjectTM(t));

}

#define DIFFERENT 1
int MatDiff (Matrix3 *src1, Matrix3 *src2)
{

	for (int i=0;i<3;i++)
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
#endif
#if 0
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
#endif

 #if 0
#define DIFFERENT 1
int MatDiff (Matrix3 *src1, Matrix3 *src2)
{

	for (int i=0;i<3;i++)
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
#endif

