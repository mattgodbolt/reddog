

#include "ext.h"
#include <process.h>
#define TRI 1
#define DEV 1


int CALLWAD = 0;
int NOPROMPT = 0;
int GAMECOMPILE = 1;

extern int StratConv(char *strat);
extern void Initialise();


char *temp[64];

void Clearm(char *input,int amount)
{
	// innocent whistling of MattG here...
//	memset (input, 0, amount);
 	
 	int i;
	for (i=0;i<amount;i++)
		input[i]='\0';
	
}


int main(int argc, char *argv[])
{
#if ALLRESIDENT
	FILE			*DB;
	char		*arg;
	char		DBFile[MAX_LINE];
	char		NewArg[MAX_LINE];
	int			NumMaps;
	int			size,loop,ERROR=0;
#endif

	WADRERUN = 0;
	FullRec = 0;

	Initialise();

//#if GODMODEMACHINE
	//if not the final 'testmap' let's clean up, to ensure a build
 //	if ((strnicmp(argv[2],"GODMODEFINAL",12)))
 //		system("command /e:8192 /cP:\\game\\artstrat\\GODclean.bat");
//#endif


	#if DEV
	//	printf("argv1 %s argv2 %s \n",argv[1],argv[2]);


		if (!argv[1])
			printf("MUST SPECIFY A FILENAME\n");
		else
		{
			suppname = argv[1];

			if (argv[2])
			{
				if (!(strnicmp(argv[2],"SINGLE",6)))
				{
					SINGLEMAP = 1;
					GAMECOMPILE = 1;
				}
				else
				{
					if (!(strnicmp(argv[2],"GODMODEFINAL",12)))
					{
						NOPROMPT = 1;
						GAMECOMPILE = 1;
					}
					else
					{
						if (!(strnicmp(argv[2],"GODMODE",6)))
						{
							NOPROMPT = 1;
							GAMECOMPILE = 0;
							
						}
					}

				}

			}
				//IF THIRD ARGUMENT != MAX
			if (argv[3])
			{
				if ((strnicmp(argv[3],"MAX",3)))
	 				CALLWAD = 1;
			}
			else
				CALLWAD = 1;

			ReadLogErrors();

			//read in the generic sound base
		  	ReadGenericSounds();
			
			
			strcpy(CurrentLevel,argv[1] + 1);
			if (StratConv(argv[1]))
			{		

				if (!SINGLEMAP)
				{
					NotSpecifiedMap = 1;
				#if ALLRESIDENT
					arg = argv[1] + 1;
					#if ARTIST
						sprintf(DBFile,"%s%s%s",INTDIR,"GAMEMAPS",".MDB");	
					#else
						sprintf(DBFile,"%s%s%s",STRATDIR,"GAMEMAPS",".MDB");	
					#endif
					DB=fopen(DBFile, "rb");
					fread(&NumMaps,sizeof(int),1,DB);
					//printf("num maps in dbase %d\n",NumMaps);
	
					for (loop=0;loop<NumMaps;loop++)
					{
					

						fread(&size,sizeof(int),1,DB);
						temp[loop] = (char*)malloc(size + 2);
						fread(temp[loop],sizeof(char),size,DB);
						printf("%s\n",temp[loop]);
					}
					fclose(DB);
		  
						
						//CHECK TO ENSURE WE DON'T REDO THE INPUT MAP
					for (loop=0;loop<NumMaps;loop++)
					{
						Clearm(CurrentLevel,128);
						strncpy(CurrentLevel,temp[loop],strlen(temp[loop]));
					 //	CurrentLevel = temp[loop];
						if (strncmp(CurrentLevel,arg,strlen(arg)))
						{

							//WE ONLY WISH TO BUILD MODEL AND ANIM LISTS RELEVANT TO THE CURRENTLY
							//SPECIFIED OUTPUT LEVEL, SO THIS IS THE BIT THAT FACILITATES IT

						   	//fridays addition
							//NumMods = 0;
							//NumAnims = 0;
							//DefinedStrats = 0;
							ResetVars();


							sprintf(NewArg,"@%s",CurrentLevel);
							StratConv(NewArg);
							fcloseall();


						}
					}
				   //	fclose (DB);
				}
				//fridays addition
				CleanMe();	

			#endif
			}
			else
			{
				//ENSURE THE PROMPT CHECK IS BACK ON
				NOPROMPT = 0;
				ERROR++;
				printf("PROBLEM WITH INPUT SCRIPT %s\n",argv[1]);											
			}
		}
	#endif
//wait:
	fcloseall();

    
	if (!ERROR)
	{
		if (!((!FullRec) && (!TIMESTAMPFAIL)))
			printf("\n *** GAME REBUILD NEEDED ***\n");
	}
   
	if (WADRERUN)
		printf("\n *** NEED TO REWAD LEVELS ***\n");


  

	if ((ERROR) || (!NOPROMPT))
	{
		printf("Press key to continue\n");
		for (;;)
		{
			while (!_kbhit());
				break;
		}
	}
	#if SH4
	   	if (!ERROR)
	   	{
			if (!((!FullRec) && (!TIMESTAMPFAIL)))
				#if ARTIST
				//	system("command /e:8192 /cP:\\UTILS\\ARTCOMP.bat");
					system("P:\\UTILS\\ARTCOMP.bat");
				#else
					if (GAMECOMPILE)
#if GODMODEMACHINE
   						system("command /e:8192 /cP:\\game\\artstrat\\DOGcompdog.bat");
#else
		   //	   	_spawnl (_P_WAIT, "p:\\GAME\\ARTSTRAT\\COMPDOG.BAT","p:\\GAME\\ARTSTRAT\\COMPDOG.BAT", NULL, NULL);
				ERROR = system("command /e:8192 /cP:\\game\\artstrat\\compdog.bat");
  // ERROR = 	_spawnl (_P_WAIT, "c:\\windows\\COMMAND", "/e:8192 /cP:\\GAME\\ARTSTRAT\\COMPDOG.BAT",NULL);

#endif
					if ((WADRERUN) && (CALLWAD))
					{
						//_spawnl (_P_WAIT, "c:\\DREAMCAST\\REDDOG\\WADDER\\RELEASE\\wadder.exe",NULL, NULL);
						printf("\nNOW RUN WADDER AND GDWORKSHOP\n");
  
		
					 //	for (;;)
					 //	{
					 //		while (!_kbhit());
					 //		break;
					 //	}

						
					}
					fcloseall();
					if (!NOPROMPT)
					{
						printf("COMPILATION OVER - Press key to continue\n");
							
					for (;;)
	   
					 {
					 	while (!_kbhit());
					 	break;
					 }
					} 

				#endif
				
		
		}
	#endif
   /*

	if (ERROR)
	{
	  for (;;)
	  { 
		while (!_kbhit());
			break;
	  }


	}
	*/
	printf("DONE \n");

	return(ERROR);
}


#if 0

void ProcStrat(STRATOBJ *strat)
{
	//Uint16 i;
	STRATOBJ *Obj;

	Obj=&strats[0];
	Obj->func(Obj);
}











VECTOR view,NullVec={0.0,0.0,0.0};

void CameraLookAt(VECTOR *look)
{
	VECTOR invtrans;
	float dz,dx,dy;
	double dsq,dd;


	dx=(look->vx-Campos.vx);
	dy=(look->vy-Campos.vy);
	dz=(look->vz-Campos.vz);


	dsq = (double)((dx*dx)+(dz*dz));
	dd = sqrt(dsq);

	CamX = -atan2(dy,dd);
	CamY = -atan2(dx,dz);


	InitMatrix(&camera);
	AddXRotation(&camera,CamX);
	AddYRotation(&camera,CamY);
	AddZRotation(&camera,CamZ);

	invtrans.vx = -Campos.vx;
	invtrans.vy = -Campos.vy;
	invtrans.vz = -Campos.vz;

	RotateVector(&camera,&invtrans,&view,&NullVec);


}


void InitStream (char *Map)
{
	Uint16 i,num;
	STRINDEX *streamptr=&stream[0];		
	FILE *finput;
	char	filename[32];


	sprintf(&filename,"%s%s",MAPDIR,Map);


	if ((finput = fopen(&filename[0], "rb")) == NULL) 
	{
		printf("Can not open map file\n");	
		exit(1);
	}


	InHead		=(STRINDEX*)&stream[0];
	InTail		=(STRINDEX*)&stream[READCACHE-1];
	DispHead		=(STRINDEX*)&stream[READCACHE];
	DispTail		=(STRINDEX*)&stream[READCACHE+DISPAREA-1];
	ReverseHead	=(STRINDEX*)&stream[READCACHE+DISPAREA];
	ReverseTail	=(STRINDEX*)&stream[READCACHE+DISPAREA+REVERSECACHE-1];


	for (i=0;i<STREAMSIZE;i++)
	{
		streamptr->strip= malloc(sizeof(STRIP));
		if (!streamptr->strip)
			{
			 printf("no mem for strip\n");
			 exit(1);
			}

		streamptr->next = (STRINDEX*)(streamptr-1);	
		streamptr->prev = (STRINDEX*)(streamptr+1);	
		streamptr++;
	}

	InHead->next=NULL;
	ReverseTail->prev=NULL;

	for (i=0;i<STREAMSIZE;i++)
	{

		ReadMapMesh(finput,stream[i].strip);

		stream[i].strip->mappos=
					((STRIPWIDTH*(STREAMSIZE-i)));
	}

	fclose(finput);



//	finput = fopen("pot.bin", "rb");
//	finput = fopen("lpot.bin", "rb");
	finput = fopen("cube.bin", "rb");
	ReadMapMesh(finput,&object);
	fclose(finput);

//	finput = fopen("lcube.bin", "rb");
//	finput = fopen("lpot.bin", "rb");
	finput = fopen("cube.bin", "rb");
	ReadMapMesh(finput,&dummy);
	fclose(finput);

	//strats
	//ReadMesh("ghost.asc",&dummy);

	strats[i].Mesh=&dummy;
	strats[i].func=&InitFunction;


}

void	LandscapeDraw()
{
		STRINDEX *DrawIndex = DispTail;

		while (DrawIndex != InTail)
		{
			RenderStrip(DrawIndex->strip);		

			DrawIndex = DrawIndex->next;

		}

}



double DotProduct(VERTEX *a, VERTEX *b)
{	
	return (a->vx*b->vx)+(a->vy*b->vy)+(a->vz*b->vz);
}



MATRIX PlayerMat;

#define COLL_RANGE			200
#define COLL_RANGE2			200



void CollisionStrat(STRIP *obj,STRIP *flr)
{
	int	Quit,i,ii,j,k,no_dot,sub_coll_face,no_vert,no_coll_face;
	VERTEX v,vec;
	VERTEX objv, pc,v1,v2,n,xpv;
	float	dist;
	VERTEX  *Points,Trans;	
	int	*coll_face, *coll_first;

	no_coll_face=0;
  	coll_face=malloc(sizeof(int)*flr->mesh.numpolys);
	coll_first=malloc(sizeof(int)*flr->mesh.numpolys);

	//QUICK VERT TO FLOOR VERT RANGE RADIUS CHECK
	//BUILDING AN ARRAY OF Ith POLY COLLISIONS



	for (i=0; i<flr->mesh.numpolys; i++)
	{
		for (j=0; j<flr->mesh.flist[i].numside; j++)
		{

			//REMOVING THE NEED TO ROTATE THE POINTS COULD HELP OUT HERE

			if ((fabs(flr->vlist[flr->mesh.flist[i].vertices[j]].vx - Player.vx ) < COLL_RANGE) &&
				(fabs( flr->vlist[flr->mesh.flist[i].vertices[j]].vy - Player.vy ) < COLL_RANGE) &&
				(fabs( flr->vlist[flr->mesh.flist[i].vertices[j]].vz - Player.vz + flr->mappos) < COLL_RANGE))
			{
				coll_face[no_coll_face] = i;
				no_coll_face++;
				break;
			}
		}
	}

	// FOR EVERY COLLISION FACE, PLACING THE ALL THE TRANSFORMED VERTS OF
   // THE PLAYER INTO THE FACE'S PLANE EQUATION TO DERIVE WHETHER IT'S ON THE
   // POLY OR BEHIND IT

	//** TRANSFORM THE POINTS ONCE AND THEN CHECK WOULD BE BETTER


  	Points=malloc(sizeof(VERTEX)*obj->numverts);


	//PLAYER'S WORLD POSITION, RELATIVE TO THE COLLISION PIECE

	Trans.vx=Player.vx;
	Trans.vy=Player.vy;
	Trans.vz=Player.vz-flr->mappos;



	for(j=0; j<obj->numverts; j++)
		RotateVector(&PlayerMat, &obj->vlist[j], &Points[j],&Trans);




	for (i=0; i<no_coll_face; i++)
	{
		for(j=0; j<obj->numverts; j++)
		{

			//BREAKS AS SOON AS ANY POINT FROM THE OBJECT COLLIDES

			if (Points[j].vx * flr->mesh.normals[coll_face[i]].vx +
				Points[j].vy * flr->mesh.normals [coll_face[i]].vy +				
				Points[j].vz * flr->mesh.normals [coll_face[i]].vz +
				flr->mesh.d[coll_face[i]] < 0)
					break;
		}

		// FLAG THAT SAYS NONE OF THE VERTS HAVE HIT THE PLANE
		// ONE VERT HITS THEN QUIT


		if (j == obj->numverts)
			coll_face[i] = -1;
		else
			coll_first[i]=j;
	}




	i=0;
	surface=5;

	//BREAK OUT AS SOON AS ONE VERTEX HITS PLANE

	Quit=0;
	while ((i<no_coll_face) && (!Quit))
	{

		//IF THE FACE PLANE IS BEHIND ALL THE POINTS THEN NO NEED FOR FURTHER

		while (coll_face[i] == -1)
			i++;

		if (i==no_coll_face)
			break;

		n.vx = flr->mesh.normals[coll_face[i]].vx;
		n.vy = flr->mesh.normals[coll_face[i]].vy;
		n.vz = flr->mesh.normals[coll_face[i]].vz;



		for (j=coll_first[i]; j<obj->numverts; j++)
		{

			if (Quit)
				break;

			//NOW FIND WHICH EXACT POINTS HAVE HIT THE PLANE
			//STARTING FROM THE FIRST POINT THAT MADE CONTACT


			dist = Points[j].vx * n.vx +
				   Points[j].vy * n.vy +
				   Points[j].vz * n.vz +
				   flr->mesh.d[coll_face[i]];

			if (dist < 0)
			{



				// IF BEHIND FACE'S PLANE FIND THE POINT OF CONTACT AND
				// WHETHER THAT LIES IN THE FACE'S POLY REGION

				//pc.vx = Points[j].vx - n.vx*dist;
				//pc.vy = Points[j].vy - n.vy*dist;
				//pc.vz = Points[j].vz - n.vz*dist;

				pc.vx = -((Points[j].vy*n.vy) + (Points[j].vz*n.vz) - dist + flr->mesh.d[coll_face[i]]) / n.vx;
				pc.vy = -((Points[j].vx*n.vx) + (Points[j].vz*n.vz) - dist + flr->mesh.d[coll_face[i]]) / n.vy;
				pc.vz = -((Points[j].vx*n.vx) + (Points[j].vy*n.vy) - dist + flr->mesh.d[coll_face[i]]) / n.vz;

				no_dot = 0;
					
				no_vert = flr->mesh.flist[coll_face[i]].numside;

			 	for (k=0; k<no_vert; k++)
				{

					// PLANE TO CONTACT VECTOR NORMAL CALCULATED

					//CAN PRE-CALCULATE PER POLY EDGE ? 
					//BASED ON NO ROTATION OF FACE



					v1.vx = flr->vlist[flr->mesh.flist[coll_face[i]].vertices[(k+1) % no_vert]].vx;
					v1.vy = flr->vlist[flr->mesh.flist[coll_face[i]].vertices[(k+1) % no_vert]].vy;
					v1.vz = flr->vlist[flr->mesh.flist[coll_face[i]].vertices[(k+1) % no_vert]].vz;

					v2.vx = flr->vlist[flr->mesh.flist[coll_face[i]].vertices[k]].vx;
					v2.vy = flr->vlist[flr->mesh.flist[coll_face[i]].vertices[k]].vy;
					v2.vz = flr->vlist[flr->mesh.flist[coll_face[i]].vertices[k]].vz;




					v1.vx = v1.vx - v2.vx;
					v1.vy = v1.vy - v2.vy;
					v1.vz = v1.vz - v2.vz;
					v2.vx = v2.vx - pc.vx;
					v2.vy = v2.vy - pc.vy;
					v2.vz = v2.vz - pc.vz;



					xpv.vx = v2.vy*v1.vz-v2.vz*v1.vy;
					xpv.vy = v2.vz*v1.vx-v2.vx*v1.vz;
					xpv.vz = v2.vx*v1.vy-v2.vy*v1.vx;



					if (DotProduct(&xpv, &n) >= 0)
						no_dot++;
					else
						break;
				}


				if (no_dot == no_vert)
				{
					//IF THE CONTACT POINT IS CONTAINED WITHIN THE POLY
					//THEN HAS HIT IT AND NOT JUST ITS PLANE

					Player.vx = Player.vx - n.vx * (dist) ;
					Player.vy = Player.vy - n.vy * (dist);
					Player.vz = Player.vz - n.vz * (dist);

					//vec.vx=speed.vx;vec.vy=speed.vy;
					//vec.vz = speed.vz;


					surface=50;

					//v.vx = n.vx * surface; 
					//v.vy = n.vy * (surface/3); 
					//v.vz = n.vz * surface; 

					speed.vx += n.vx * surface; 
					speed.vy += n.vy * (surface/3); 
					speed.vz += n.vz * surface; 

					//RotateVector(&PlayerMat, &v, &speed,&vec);
				

					Quit=1;
			  	}						

			}
		}

		i++;
	}

	if (!Quit)
	{
		speed.vx -= speed.vx/4;
		speed.vz -= speed.vz/4;
	}

	free(Points);
	free(coll_first);
	free(coll_face);

}



void DrawStrat(STRIP *drawstrip)
{
	VECTOR *transpoints;
	POINT2D	 *projpoints;
	Uint16 i;
	VECTOR   world,trans;
	MATRIX	*objrot,rot;
	VECTOR   Camtran,Vec;
	VECTOR   PlayTrans;


	world.vx=Player.vx;
	world.vy=Player.vy;
	world.vz=Player.vz;

	RotateVector(&camera,&world,&trans,&view);

	objrot=&PlayerMat;


	InitMatrix(objrot);
	AddYRotation(objrot,PlayerY);
	//AddZRotation(objrot,PlayerZ);
	//AddXRotation(objrot,PlayerX);

	Camtran.vz=Player.vz;
	Camtran.vy=Player.vy;
	Camtran.vx=Player.vx;


	PlayTrans.vx=0;
	PlayTrans.vy=0;
	PlayTrans.vz=-1000.5;


  	RotateVector(objrot,&PlayTrans,&Campos,&Camtran);





	InitMatrix(objrot);
	AddYRotation(objrot,PlayerY);
	AddZRotation(objrot,PlayerZ);
	AddXRotation(objrot,PlayerX);



	MulMatrix (objrot,&camera,&rot);


	transpoints=(VERTEX*)malloc(sizeof (VERTEX) * drawstrip->numverts);
	projpoints=(POINT2D*)malloc(sizeof (POINT2D) * drawstrip->numverts);
	
	for (i=0;i<drawstrip->numverts;i++)
	 	RotateVector(&rot,&drawstrip->vlist[i],transpoints+i,&trans);

	for (i=0;i<drawstrip->numverts;i++)
		ProjectPts(transpoints+i,projpoints+i);


	_setcolor(1);


  	PutPolygon(&drawstrip->mesh,projpoints);



	free(projpoints);
	free(transpoints);
}


void RenderStrip(STRIP *drawstrip)
{
	VECTOR *transpoints;
	POINT2D	 *projpoints;
	Uint16 i;
	VECTOR   world,trans;



	world.vx=0;
	world.vy=0;
	world.vz=drawstrip->mappos;

	RotateVector(&camera,&world,&trans,&view);

	//if ((trans.vz >(LNEARZ)) && (trans.vz<(LFARZ)))
	//if ((trans.vz<(LFARZ)))
	//{

		transpoints=(VERTEX*)malloc(sizeof (VERTEX) * drawstrip->numverts);
		projpoints=(POINT2D*)malloc(sizeof (POINT2D) * drawstrip->numverts);
	
		for (i=0;i<drawstrip->numverts;i++)
		 	RotateVector(&camera,&drawstrip->vlist[i],transpoints+i,&trans);

		for (i=0;i<drawstrip->numverts;i++)
			ProjectPts(transpoints+i,projpoints+i);

		_setcolor(7);
 
 		PutPolygon(&drawstrip->mesh,projpoints);

		free(projpoints);
		free(transpoints);
	//}

}

void	PutPolygon(MESH *model,POINT2D *projected)
{
	Uint16 i,pt,Clipped;
	POINT2D point,point2,point3,point4;
//	struct xycoord poly[3];
	POINT2D PointArr[10000];



	for (i=0;i<model->numpolys;i++)
	{

		if (model->flist[i].numside == 3)
		{

			point.x =  projected[model->flist[i].vertices[0]].x;		
			point.y =  projected[model->flist[i].vertices[0]].y;		
			point2.x = projected[model->flist[i].vertices[1]].x;		
			point2.y = projected[model->flist[i].vertices[1]].y;		
			point3.x = projected[model->flist[i].vertices[2]].x;		
			point3.y = projected[model->flist[i].vertices[2]].y;		




		if ((point.x==CLIP) || (point2.x==CLIP) || (point3.x==CLIP))
			continue;

		if (point.x >0)
			if ((point.x > 160) && (point2.x > 160) && (point3.x >160))
				continue;
		else
			if ((point.x < -160) && (point2.x < -160) && (point3.x <-160))
				continue;

		if (point.y >0)
			if ((point.y > 120) && (point2.y > 120) && (point3.y >120))
				continue;
		else
			if ((point.y < -120) && (point2.y < -120) && (point3.y <-120))
				continue;

		if (((point.x-point2.x) * (point3.y-point2.y))+
			 ((point3.x-point2.x)*(point2.y-point.y))
			 < 0)
			continue;




  		_moveto(point.x+160, 120-point.y);
		_lineto(point2.x+160, 120-point2.y);
		_moveto(point2.x+160, 120-point2.y);
		_lineto(point3.x+160, 120-point3.y);
		_moveto(point3.x+160, 120-point3.y);
		_lineto(point.x+160, 120-point.y);
		} 
		else 
		if (model->flist[i].numside == 4)
		{


			point.x =  projected[model->flist[i].vertices[0]].x;		
			point.y =  projected[model->flist[i].vertices[0]].y;		
			point2.x = projected[model->flist[i].vertices[1]].x;		
			point2.y = projected[model->flist[i].vertices[1]].y;		
			point3.x = projected[model->flist[i].vertices[2]].x;		
			point3.y = projected[model->flist[i].vertices[2]].y;		
			point4.x = projected[model->flist[i].vertices[3]].x;		
			point4.y = projected[model->flist[i].vertices[3]].y;		

			if ((point.x==CLIP) || (point2.x==CLIP) || (point3.x==CLIP) || (point4.x==CLIP))
				continue;

			if (point.x >0)
				if ((point.x > 160) && (point2.x > 160) && (point3.x >160) && (point4.x >160))
					continue;
			else
				if ((point.x < -160) && (point2.x < -160) && (point3.x <-160) && (point4.x <-160))
					continue;

			if (point.y >0)
				if ((point.y > 120) && (point2.y > 120) && (point3.y >120) && (point4.y >120))
					continue;
			else
				if ((point.y < -120) && (point2.y < -120) && (point3.y <-120) && (point4.y <-120))
					continue;

			if (((point.x-point2.x) * (point3.y-point2.y))+
				 ((point3.x-point2.x)*(point2.y-point.y))
				 < 0)
				continue;

  
  			_moveto(point.x+160, 120-point.y);
			_lineto(point2.x+160, 120-point2.y);
			_moveto(point2.x+160, 120-point2.y);
			_lineto(point3.x+160, 120-point3.y);
			_moveto(point3.x+160, 120-point3.y);
			_lineto(point4.x+160, 120-point4.y);
			_moveto(point4.x+160, 120-point4.y);
			_lineto(point.x+160, 120-point.y);

		} 
		else
		{

			// n faces

			Clipped=0;

			for (pt=0;pt<model->flist[i].numside;pt++)
			{
				PointArr[pt].x =  projected[model->flist[i].vertices[pt]].x;		
				if (PointArr[pt].x ==CLIP)
				{
					Clipped=1;
					break;
				}
				else
					PointArr[pt].y =  projected[model->flist[i].vertices[pt]].y;		
			}


			if (!Clipped)
			{

				for (pt=0;pt<model->flist[i].numside;pt++)
				{
					if (PointArr[pt].x > 160)
						break;
				}

				if (pt==model->flist[i].numside)
				{

					for (pt=0;pt<model->flist[i].numside;pt++)
					{
						if (PointArr[pt].x < -160)
							break;
					}

					if (pt==model->flist[i].numside)
					{

						for (pt=0;pt<model->flist[i].numside;pt++)
						{
							if (PointArr[pt].y < -120)
								break;
						}


						if (pt==model->flist[i].numside)
						{


							for (pt=0;pt<model->flist[i].numside;pt++)
							{
								if (PointArr[pt].y > 120)
									break;
							}

							if (pt==model->flist[i].numside)
							{

								for (pt=0;pt<model->flist[i].numside;pt++)
								{
									_moveto(PointArr[pt].x+160, 120-PointArr[pt].y);
									if (pt=model->flist[i].numside-1)
										_lineto(PointArr[0].x+160, 120-PointArr[0].y);
									else
										_lineto(PointArr[pt+1].x+160, 120-PointArr[pt+1].y);
								}

							}
						}
					}
				}
			}
		}
  	}
}

void StripBounds()
{
 	int z;

	z=(STREAMSIZE+1)-(((int)(Player.vz/STRIPWIDTH))% STREAMSIZE );


	if ((z-10) > 0)
		InTail=&stream[z-10];
	else
		InTail=&stream[1];

	if ((z+10) < STREAMSIZE)
		DispTail=&stream[z+10];
	else
		DispTail=&stream[STREAMSIZE-1];
}


VECTOR NullTrans={0,0,0};
#if 0
void Move(float amount)
{
	MATRIX   *objrot;
	VECTOR   Trans,Movement;



	NullTrans.vx=0;
	NullTrans.vy=0;
	NullTrans.vz=0;

	Trans.vx=speed.vx;
	Trans.vy=speed.vy;
	Trans.vz=amount;

	RotateVector(&PlayerMat,&Trans,&Movement,&NullTrans);	


	Player.vx+= Movement.vx; 
	Player.vy+= Movement.vy; 
	Player.vz+= Movement.vz; 
  	
}	

#if 0
void MoveX(float amount)
{
	MATRIX   *objrot,obj;

	VECTOR   Trans,Movement;

	InitMatrix(&obj);
	AddYRotation(&obj,PlayerY);


	NullTrans.vx=0;
	NullTrans.vy=0;
	NullTrans.vz=0;

	Trans.vx=amount;
	Trans.vy=0;
	Trans.vz=0;

	RotateVector(&obj,&Trans,&Movement,&NullTrans);	


	Player.vx+= Movement.vx; 
	Player.vy+= Movement.vy; 
	Player.vz+= Movement.vz; 
	
}	
#endif

void MoveStrat(float amount,STRATOBJ *strat)
{
	MATRIX   *objrot;


	objrot=&strat->rotmat;


	strat->vx+= (*objrot)[0][2] * amount; 
	strat->vy+= (*objrot)[1][2] * amount; 
	strat->vz+= (*objrot)[2][2] * amount; 
	
}	

#endif
#endif 



















