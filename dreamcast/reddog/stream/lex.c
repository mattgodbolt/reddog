#include "ext.h"
#include "lex.h"
#include "defs.h"
#if SH4
	#include <process.h>
#endif

#define BASESFXNUM	128

//look for build heads for rdo file update on script head warez

//LATEST BUILD:- MIGHTYDWIGHTYYORKE FOR SINGLEMAP BUILDS

//ENSURED ONLY FILES THAT ARE IN ERROR GET RECOMPILED
//INVALIDATED TIME STAMPS ON FILES THAT FAIL PREPROCESSING
//LOOK FOR OLE GUNNAR

//NOTES TO GET THEM WORKING AGAIN
//SEARCH FOR MANUTD ADDED AND REMOVE, SEARCH FOR MANUTD DELETED AND ADD BACK IN


#define ASIZE(a) (sizeof(a)/sizeof((a)[0]))

char		CurrentLevel[MAX_LINE];


char *suppname;
int NotSpecifiedMap = 0;
int LOOPNEST;
STATE statename[MAX_STATE];
int WADRERUN;	




DEFINITION Defs[MAX_DEFS];
DEFINITION IVars[MAX_DEFS];
DEFINITION FVars[MAX_DEFS];
short CHARACTERSET=0; //FLAG TO SAY WHETHER A CHAR MOD WAD NEEDS TO BE BUILT
short CharUsedArray[MAXGAMECHARS];

int SMALAMOUNT=0;
int AMALAMOUNT=0;
int STMALAMOUNT=0;
int MALAMOUNT=0;
int SINGLEMAP=0;
MODREC ModelTable[MAX_MODELS];
int NumMods=0;
SOUNDREC SoundTable[512];
int NumSounds=0;
int BaseSound=0;
ANIMREC AnimTable[512];
int NumAnims=0;
int DomeSet=0;
char DomeName[256];

STRATFIELD StratList[MAX_STRATS];
STRATFIELD StratFile[MAX_STRATS];
int FullRec=0;
int anyvar =0;
int floatvar =0;
int line=0;

int line2=0;
int EndSubBlock[MAX_NESTS];		
int stratlabel[MAX_NESTS];	//Used for GOTO <STRATNAME>stratlabel
int InIf[MAX_NESTS];					
int elselabel[MAX_NESTS];	//Used for GOTO <STRATNAME>stratlabel
int elselabelused[MAX_NESTS];	//Used for GOTO <STRATNAME>stratlabel
SCRIPTLINE *elselabelline[MAX_NESTS];
SCRIPTLINE	*EvalLine[MAX_NESTS];	//FIXED THE STATEMENT..ENDWHILE PROBLEM

#define LINEMAX 512
int Indents[LINEMAX];
int Indents2[LINEMAX];
SCRIPTLINE *lines[LINEMAX];
SCRIPTLINE *lines2[LINEMAX];


int	JumpLabel=0;				//Used for jump backs in loop immediates
char ImmediateLabel[128];		//Global label for the current loop immediate block

enum {CONDITION,EVALUATE};

int WhileIn=0;
int KillMe=0;		
int NEST=0;
int Brackets=0;
int SAVENEST=0;
int Label=0;			//Incs on defining states to address the next symbol entry per state
int CurrentLabel=0;  //Current state being used from state symbol list
int Indent=0;			//The current indentation level for the state being processed
int Global=0;			//Whether the current symbol being processed is global or local
int Error=0;
int NestLevel=0;
int WhileLevel=0;
int IfLevel=0;
int ElseLevel=0;
int fatal = 0;
int Defined = 0;
int IVarNum = 0;
int FVarNum=0;
int Procedures=NUMPROC;
int Reserved=NUMRES;
int Operation=NUMOPER;
int ElseMet[MAX_NESTS];
int Imm[MAX_NESTS];
int DefinedStrats=0;
int DefinedFiles=0;
int InternalGrab=0;
int PROCESSED = 0;

SCRIPTLINE *CurrentLine;



char *errorlog;
char *script;
char *currentstrat;

char HEADER		[256];
char CINCLUDE	[256];
FILE *Header;
FILE *Cinclude;
int SolvedCurrent;
int GetTotStrats();
void LogErrorFile(int fail);
void KEYBREAK();
char CurrentStrat[128];

void CleanStratOpen(int mode)
{
	int i;

	if (!mode)
	{
		for (i=0;i<MAX_STRATS;i++)
		{
			StratFile[i].name = NULL;

		}
		DefinedFiles = 0;
	}
	else
	{

		for (i=0;i<MAX_STRATS;i++)
		{  
			if (StratFile[i].name)
			{
				free(StratFile[i].name);
				StratFile[i].name = NULL;
			}
			DefinedFiles = 0;

		}

	}


}


void ResetVars()
{
	int i;
	anyvar = 0;
	floatvar = 0;
	WhileIn=0;
 	KillMe=0;		
 	NEST=0;
 	SAVENEST=0;
 	Brackets=0;
 	Label=0;			
	CurrentLabel=0;
 	Indent=0;			
 	Global=0;			
 	Error=0;
 	NestLevel=0;
 	WhileLevel=0;
 	IfLevel=0;
 	ElseLevel=0;
 	fatal = 0;
 	Defined = 0;
	FVarNum=0;
	IVarNum=0;


	#if ALLRESIDENT
//		NumMods=0;
//		NumAnims=0;
	#endif

	for (i=0;i<LINEMAX;i++)
	{
	  	lines[i]=NULL;
	  	lines2[i]=NULL;
		Indents[i]=0;
		Indents2[i]=0;
		Indents[i]=0;
		Indents2[i]=0;


	}


	for (i=0;i<MAX_NESTS;i++)
	{
		stratlabel[i]=0;
		elselabel[i]=0;
		elselabelused[i]=0;
		InIf[i]=0;
		Imm[i] = 0;
		ElseMet[i] = 0;
		EvalLine[i]='\0';
		
	}

   

	line=0;
	line2=0;
	CurrentLine=NULL;
	JumpLabel=0;
	DomeSet = 0;
	Clear(DomeName,256);
}

//CLEAN UP OF MEMORY USED

void	CleanMe()
{

	int i;

	for (i=0;i<NumAnims;i++)
	{
	   
		AMALAMOUNT -= strlen(AnimTable[i].ModelName) + 1;
		free(AnimTable[i].ModelName);
		AnimTable[i].ModelName = NULL;
		AMALAMOUNT -= strlen(AnimTable[i].Name) + 1;
		free(AnimTable[i].Name);
		AnimTable[i].Name = NULL;
	}

	for (i=0;i<NumMods;i++)
	{
		MALAMOUNT -= strlen(ModelTable[i].Name) + 1;
		free(ModelTable[i].Name);
		ModelTable[i].Name = NULL;
	}
	for (i=0;i<NumSounds;i++)
	{
		SMALAMOUNT -= strlen(SoundTable[i].Name) + 1;
		free(SoundTable[i].Name);
		SoundTable[i].Name = NULL;
	}

	for (i=0;i<DefinedStrats;i++)
	{
		STMALAMOUNT -= strlen(StratList[i].addr) + 1;
		free (StratList[i].addr);
		StratList[i].addr = NULL;
		STMALAMOUNT -= strlen(StratList[i].name) + 1;
		free (StratList[i].name);
		StratList[i].name = NULL;
	}
}

int DemosChanged(char *levelname)
{
	struct	_stat buf;
	int LogChanged=0,CamChanged=0;
	char logscript[128];
	int len,oldlen,i;
	char *timestamp,*oldtime;
	FILE *LogFile,*timfp;

	levelname+=1;
	sprintf(logscript,"%s%s%s",GAMEDIR,levelname,".LOG");
	LogFile = fopen(logscript,"rb");
	if (LogFile)
	{
		_stat(logscript,&buf);
		//READ THE TIME OF LAST FILE MODIFIED INTO A BUFFER		

		len=strlen(ctime(&buf.st_mtime))+1;
		timestamp = (char*)malloc(len);
		sprintf (timestamp,ctime(&buf.st_mtime));


		//NOW OPEN AND READ THE PREVIOUS RECORD..IF NEW THEN CREATE
		sprintf(logscript,"%s%s%s",INTDIR,levelname,".TIL");
		if (timfp = fopen(logscript,"rb"))
		{
			fread(&oldlen,sizeof(int),1,timfp);
			if (len == oldlen)
			{

				oldtime = (char*)malloc(oldlen);
				fread(oldtime,sizeof(char),oldlen,timfp);
				for (i=0;i<oldlen;i++)
				{
					if (oldtime[i] != timestamp[i])
					{
						LogChanged = 1;
						break;
					}
				}
				free (oldtime);
			}
			else
				LogChanged =1;
			fclose(timfp);
		}
		else
			LogChanged =1;

		if (LogChanged)
			printf("demo has changed %s\n",levelname);


		//AND OUTPUT THE FILE
		sprintf(logscript,"%s%s%s",INTDIR,levelname,".TIL");
		timfp = fopen(logscript,"wb");

		fwrite(&len,sizeof(int),1,timfp);
		fwrite(timestamp,sizeof(char),len,timfp);

		fclose(timfp);

		free(timestamp);

		fclose(LogFile);
	}
	else
	   LogChanged = 0;

	sprintf(logscript,"%s%s%s",GAMEDIR,levelname,".CAM");
	LogFile = fopen(logscript,"r");
	if (LogFile)
	{
		_stat(logscript,&buf);
		//READ THE TIME OF LAST FILE MODIFIED INTO A BUFFER		

		len=strlen(ctime(&buf.st_mtime))+1;
		timestamp = (char*)malloc(len);
		sprintf (timestamp,ctime(&buf.st_mtime));

		//NOW OPEN AND READ THE PREVIOUS RECORD..IF NEW THEN CREATE
		sprintf(logscript,"%s%s%s",INTDIR,levelname,".CIL");
		if (timfp = fopen(logscript,"rb"))
		{
			fread(&oldlen,sizeof(int),1,timfp);
			if (len == oldlen)
			{

				oldtime = (char*)malloc(oldlen);
				fread(oldtime,sizeof(char),oldlen,timfp);
				for (i=0;i<oldlen;i++)
				{
					if (oldtime[i] != timestamp[i])
					{
						CamChanged = 1;
						break;
					}
				}
				free (oldtime);
			}
			else
				CamChanged =1;
			fclose(timfp);
		}
		else
			CamChanged =1;

		if (CamChanged)
			printf("cam has changed %s\n",levelname);


		//AND OUTPUT THE FILE
		sprintf(logscript,"%s%s%s",INTDIR,levelname,".CIL");
		timfp = fopen(logscript,"wb");

		fwrite(&len,sizeof(int),1,timfp);
		fwrite(timestamp,sizeof(char),len,timfp);

		fclose(timfp);

		free(timestamp);

		fclose(LogFile);
	}
	else
	   CamChanged = 0;

	return(CamChanged | LogChanged);
}

int ScriptChanged(char *levelname)
{
	struct	_stat buf;
	int ScriptChanged=0;
	char logscript[128];
	int len,oldlen,i;
	char *timestamp,*oldtime;
	FILE *LogFile,*timfp;

	levelname+=1;
	sprintf(logscript,"%s%s%s",GAMEDIR,levelname,".DSC");
	LogFile = fopen(logscript,"rb");
	if (LogFile)
	{
		_stat(logscript,&buf);
		//READ THE TIME OF LAST FILE MODIFIED INTO A BUFFER		

		len=strlen(ctime(&buf.st_mtime))+1;
		timestamp = (char*)malloc(len);
		sprintf (timestamp,ctime(&buf.st_mtime));


		//NOW OPEN AND READ THE PREVIOUS RECORD..IF NEW THEN CREATE
		sprintf(logscript,"%s%s%s",INTDIR,levelname,".TIS");
		if (timfp = fopen(logscript,"rb"))
		{
			fread(&oldlen,sizeof(int),1,timfp);
			if (len == oldlen)
			{

				oldtime = (char*)malloc(oldlen);
				fread(oldtime,sizeof(char),oldlen,timfp);
				for (i=0;i<oldlen;i++)
				{
					if (oldtime[i] != timestamp[i])
					{
			   			WADRERUN++;
						ScriptChanged = 1;
						break;
					}
				}
				free (oldtime);
			}
			else
			{
			 	WADRERUN++;
				ScriptChanged =1;
			}
			fclose(timfp);
		}
		else
		{
		   	WADRERUN++;
			ScriptChanged =1;
		}
		if (ScriptChanged)
			printf("SCRIPT has changed %s\n",levelname);


		//AND OUTPUT THE FILE
		sprintf(logscript,"%s%s%s",INTDIR,levelname,".TIS");
		timfp = fopen(logscript,"wb");

		fwrite(&len,sizeof(int),1,timfp);
		fwrite(timestamp,sizeof(char),len,timfp);

		fclose(timfp);

		free(timestamp);

		fclose(LogFile);
	}
	else
	   ScriptChanged = 0;

	return(ScriptChanged);
}







enum {SINGLE,SCRIPT,RESIDENT};
int TIMESTAMPFAIL=0;

enum {UPDATE,SKIP};


//OLE GUNNAR NEW ROUTINE
//INVALIDATES A STRAT'S TIMESTAMP INFO. TO ENSURE THAT IT WILL RECOMPILE FOLLOWING
//AN ERROR
void DeleteTimeStamp(char *filename)
{
	int len;
	char StratFile[128];
	FILE *timfp;

 	sprintf(StratFile,"%s%s%s",INTDIR,filename,".TIM");
	if (timfp = fopen(StratFile,"wb"))
	{
		len = -1;
	   	fwrite(&len,sizeof(int),1,timfp);
		fclose(timfp);
	}
}


void TimeStampCheck(char* StratFile, char *filename,int MODE)
{
	struct	_stat buf;
	int len,oldlen,i;
	char *timestamp,*oldtime;
	FILE *timfp;

	//MATT MODS TO UPDATE THE TIME STAMPS EVERY TIME
//	if (!TIMESTAMPFAIL)
//	{
		_stat(StratFile,&buf);
				
				
		//READ THE TIME OF LAST FILE MODIFIED INTO A BUFFER		

		len=strlen(ctime(&buf.st_mtime))+1;
		timestamp = (char*)malloc(len);
		sprintf (timestamp,ctime(&buf.st_mtime));

	

		//NOW OPEN AND READ THE PREVIOUS RECORD..IF NEW THEN CREATE
		sprintf(StratFile,"%s%s%s",INTDIR,filename,".TIM");
		if (timfp = fopen(StratFile,"rb"))
		{
			fread(&oldlen,sizeof(int),1,timfp);
			if (len == oldlen)
			{

			//	printf("old time %s, new time %s\n",oldtime,timestamp);				
				oldtime = (char*)malloc(oldlen);
				fread(oldtime,sizeof(char),oldlen,timfp);
				for (i=0;i<oldlen;i++)
				{
					if (oldtime[i] != timestamp[i])
					{
					  //	printf("failed time stamp %s\n",StratFile);

						TIMESTAMPFAIL = 1;
						break;
					}
				}
				free (oldtime);
			}
			else
			{
			   //	printf("failed time stamp %s\n",StratFile);

				TIMESTAMPFAIL =1;
			}
			fclose(timfp);
		}
		else
		{
		  //	printf("failed time stamp %s\n",StratFile);
			


			TIMESTAMPFAIL =1;
		}
		//if (TIMESTAMPFAIL)
		//	printf("failed time stamp %s\n",StratFile);

	   	if (MODE == UPDATE)
		{
			//AND OUTPUT THE FILE
		 	sprintf(StratFile,"%s%s%s",INTDIR,filename,".TIM");
		 	timfp = fopen(StratFile,"wb");

			fwrite(&len,sizeof(int),1,timfp);
			fwrite(timestamp,sizeof(char),len,timfp);
			fclose(timfp);

		}
		#if DEBUG_TIMESTAMP
			printf ("%d\n",len);
			printf ("%s\n",timestamp);
		#endif
		free(timestamp);
  //	}		

}

int ProcessModel(char *line)
{
	int  ModelEntry;
	char ModelName[128];
	char Xclude[128];
	
	Clear(ModelName,128);

	ReadToEndUpper(ModelName,line);
	if (!(strncmp("NONE",ModelName,strlen(ModelName))))
	{
		sprintf(ModelName,"NULL");
		return(-1);
	}

	line = line + strlen(ModelName);
 
	//READ SOME MORE TEXT IN HERE
	line=WhiteSpaceRead(line);
	ReadToEnd(Xclude,line);


	if ( (strlen(Xclude)) && (!(strnicmp(CurrentLevel,Xclude,strlen(Xclude))) ))
		ModelEntry = -1;
	else
		ModelEntry = GetModelIndex(ModelName);
	
	return (ModelEntry);
}

int ProcessSound(char *line)
{							   
	int  SoundEntry;
	char SoundName[128];
	char SoundFile[128];
	FILE *OSSFILE;
	
	Clear(SoundName,128);

	ReadToEndUpper(SoundName,line);

	sprintf(SoundFile,"%s%s%s",SFXDIR,SoundName,".OSS");
	OSSFILE = fopen(SoundFile,"r");
  	if (OSSFILE)
	{
		SoundEntry = GetSoundIndex(SoundName);
	   	fclose(OSSFILE);
		
	}
	else
   	{
   		printf("SFX FILE %s AS REFERENCED IN %s DOES NOT EXIST \n",SoundFile,CurrentStrat);
   		KEYBREAK();
   	}


	return (SoundEntry);
}

	
int ProcessAnim(char *line)
{
	int  AnimEntry;
	char AnimName[128];
	
	Clear(AnimName,128);
	ReadToEndUpper(AnimName,line);
	if (!(strncmp("NONE",AnimName,strlen(AnimName))))
	{
		sprintf(AnimName,"NULL");
		return(-1);
	}

	
	AnimEntry = GetAnimIndex(AnimName);
	return (AnimEntry);
}




//ALL REFERENCED MAPS WILL BE APPENDED TO A MAP DATABASE FILE
//IF THEY ARE NOT ALREADY IN THERE. THIS IS TO FACILITATE THE
//BUILDING OF THE GLOBAL STRAT TABLE

typedef struct mapfile
{
	int size;
	char file[128];
	int	del;
}MAPFILE;

void MakeWadTxt(char *mapname)
{
	int nummaps=0;
	int loop;
	char DBFile[MAX_LINE];
	char line[MAX_LINE];
	MAPFILE Maps[32];
	FILE *fp;
	FILE *wadfile;
	FILE *sysfile;


	sprintf(DBFile,"%s%s%s",WADDIR,"WAD",".TXT");	
	if (!(wadfile=fopen(DBFile, "w")))
		return;


	//BRING ACROSS THE SYSTEM ESSENTIALS INTO OUR WAD
	sprintf(DBFile,"%s%s%s",SYSWADDIR,"SYSWAD",".TXT");	
	if ((sysfile=fopen(DBFile, "r")))
	{
		for (;;)
		{
			if (!(fgets(line,MAX_LINE,sysfile)))
				break;

			fputs(line,wadfile);
		}
		fclose(sysfile);
	}

	#if ARTIST
		sprintf(DBFile,"%s%s%s",INTDIR,"GAMEMAPS",".MDB");	
	#else
		sprintf(DBFile,"%s%s%s",STRATDIR,"GAMEMAPS",".MDB");	
	#endif	
		
	if (fp=fopen(DBFile, "rb"))
	{
		fread(&nummaps,sizeof(int),1,fp);

		for (loop=0;loop<nummaps;loop++)
		{

			fread(&Maps[loop].size,sizeof(int),1,fp);
			fread(&Maps[loop].file,sizeof(char),Maps[loop].size,fp);

			
			if ((SINGLEMAP) && (strlen(Maps[loop].file) != strlen(mapname)))
				continue;

			if ((SINGLEMAP) && (strnicmp(Maps[loop].file,mapname,strlen(mapname))))
				continue;

			
			
			sprintf(DBFile,"\n%s\t%s\t%s\n",
				"CreateWad",
				Maps[loop].file,
				Maps[loop].file);
			fputs(DBFile,wadfile);


			//QUICK DOME HACK, EVENTUALLY THIS SHOULD GO IN THE LEVEL MAP WAD FILE ITSELF
		   //	sprintf(DBFile,"\t%s\n",
		   //		"ADDOBJECT	MISC\\DOME_LEVEL2");
		   //	fputs(DBFile,wadfile);

			sprintf(DBFile,"\t%s\t%s\n",
				"AddMap",
				Maps[loop].file);
			fputs(DBFile,wadfile);

			sprintf(DBFile,"CloseWad\n");
			fputs(DBFile,wadfile);

			

		}

		fclose(fp);

	}
	fclose(wadfile);

}


int CheckMapDB(char *filename)
{
	int quit;
	int nummaps=0,len;
	int found=0,loop;
	char DBFile[MAX_LINE];
	MAPFILE Maps[32];
	
	FILE *fp;

	//ENSURE THE @ IS REMOVED FROM THE FILENAME
	filename ++;

	//if @@ used then we will DELETE the specified map from the database
	if (*filename == '@')
	{
		quit=1;
		filename++;
	}
	else
		quit=0;


	#if ARTIST
		sprintf(DBFile,"%s%s%s",INTDIR,"GAMEMAPS",".MDB");	
	#else
		sprintf(DBFile,"%s%s%s",STRATDIR,"GAMEMAPS",".MDB");	
	#endif
	


	if (fp=fopen(DBFile, "a+b"))
	{
		fread(&nummaps,sizeof(int),1,fp);
  
		
		for (loop=0;loop<nummaps;loop++)
		{
			fread(&Maps[loop].size,sizeof(int),1,fp);
			fread(&Maps[loop].file,sizeof(char),Maps[loop].size,fp);
			if (quit)
			{
				if (!(strnicmp(Maps[loop].file,filename,strlen(filename)+1)))
				{
					printf("deleting %s\n",Maps[loop].file);
					Maps[loop].del++;
					found++;
				}
			}
			else
				Maps[loop].del = 0;
		}


		//read all the maps in so check

		if (!quit)
		{
			for (loop=0;loop<nummaps;loop++)
			{	
				if (!(strnicmp(Maps[loop].file,filename,strlen(filename)+1)))
				{
					found ++;
					break;
				}
			}

		}


		if ((!found) || (quit))
		{
			if (!quit)
				nummaps++;

			fclose(fp);
			fp=fopen(DBFile, "wb");
			if (quit)
			{
				found = nummaps - found;
				fwrite(&found,sizeof(int),1,fp);
			}
			else
				fwrite(&nummaps,sizeof(int),1,fp);
			for (loop=0;loop<nummaps-1;loop++)
			{
				if ((!Maps[loop].del))
				{
					fwrite(&Maps[loop].size,sizeof(int),1,fp);
					fwrite(&Maps[loop].file,sizeof(char),Maps[loop].size,fp);
					printf ("written %s\n",Maps[loop].file);
				}
			}

			if (!quit)
			{
				len = strlen(DBFile)+1;
				fwrite (&len,sizeof(int),1,fp);
				fwrite (filename,sizeof(char),strlen(filename)+1,fp);
				printf ("written %s\n",filename);
			}
		}

		fclose (fp);
	}
	else
	{
		printf("cannot open %s for writing\n",DBFile);
		quit=1;

	}


	#if SH4
		//CREATE THE WAD.TXT FILE
		MakeWadTxt(filename);

	#endif


	return(quit);
}



FILE *OpenFile(char *filename,int Mode)
{
	char StratFile[MAX_LINE];
	FILE *fp;



	if (Mode==SCRIPT)
	{
	//	#if !ALLRESIDENT
			//TACK OFF THE @ IF NEED BE (STRAT @LEVEL1)

			filename+=1;
	//	#endif
		#if ARTIST
			sprintf(StratFile,"%s%s%s",STRATMAPDIR,filename,".SCR");	
		#else
			sprintf(StratFile,"%s%s%s",STRATDIR,filename,".SCR");	
		#endif
	}
	else 
	if (Mode == RESIDENT)
	{
		#if ARTIST
			sprintf(StratFile,"%s%s%s",STRATMAPDIR,filename,".SCR");	
		#else
			sprintf(StratFile,"%s%s%s",STRATDIR,filename,".SCR");	
		#endif
	}
	else
		sprintf(StratFile,"%s%s%s",STRATDIR,filename,".DST");	

	#if DEBUG_TIMESTAMP
		printf("now checking the stamp on %s\n",StratFile);
	#endif	

			
	if (fp=fopen(StratFile, "ra"))
	{
		//UTDREM
		if ((Mode != SCRIPT) && (Mode != RESIDENT))
			TimeStampCheck(StratFile,filename,SKIP);
		
		
		return (fp);
	}
	else
	{
		printf ("file error %s\n",StratFile);
		return (NULL);

	}

}


FILE *OpenMap(char *filename,int RedoMap)
{
	char StratFile[MAX_LINE];
	FILE *fp;

	if (!RedoMap)
		return(NULL);

	filename+=1;
	sprintf(StratFile,"%s%s%s",MAPDIR,filename,".MAP");	

	if (fp=fopen(StratFile, "wb"))
		return (fp);
	else
	{
		printf ("MAP file error %s\n",StratFile);
		return (NULL);
	}
}


// OUTPUT THE MODEL LIST
FILE *OpenModelList(char *filename)
{
	char StratFile[MAX_LINE];
	FILE *fp;

	filename+=1;
	sprintf(StratFile,"%s%s%s",MAPDIR,filename,".MOD");	

//#if WAD
//	if (fp=fopen(StratFile, "wb"))
//#else
	if (fp=fopen(StratFile, "wa"))
//#endif
		return (fp);
	else
	{
		printf ("MODEL file error %s\n",StratFile);
		return (NULL);

	}
}

//OPEN THE ANIM LIST
FILE *OpenAnimList(char *filename)
{
	char StratFile[MAX_LINE];
	FILE *fp;

	filename+=1;
	sprintf(StratFile,"%s%s%s",MAPDIR,filename,".ANI");	

	if (fp=fopen(StratFile, "wa"))
		return (fp);
	else
	{
		printf ("ANIM file error %s\n",StratFile);
		return (NULL);

	}
}

//CHECK INT MODEL LIST
void CheckModelDep(char *filename)
{
	char StratFile[MAX_LINE];
	int i,len,OldMods;
	FILE *fp;
	char *modname;

	sprintf(StratFile,"%s%s%s",INTDIR,filename+1,".MIN");	

//	printf("opening %s models\n",StratFile);
	
	if (fp=fopen(StratFile, "rb"))
	{
		fread(&OldMods,sizeof(int),1,fp);
		if (OldMods != NumMods)
		{
			FullRec =1;
//			TIMESTAMPFAIL =1;
			WADRERUN =1;
		}
		else
		{
			for (i=0;i<NumMods;i++)
			{
				fread(&len,sizeof(int),1,fp);
				modname=(char*)malloc(len);

				fread(modname,sizeof(char),len,fp);
				//printf("model %d is %s \n",i,modname);
			  	//TREBCUPS took out	THE NEXT 3 LINES, REPLACED WITH NEW CHECK
				//	for (loop=0;loop<len;loop++)
			  //	{
				   //	if ((modname[loop]) != (ModelTable[i].Name[loop]))
					
					
					if ( 
						strnicmp(modname,ModelTable[i].Name,strlen(modname))
					  && 
						strnicmp(modname,ModelTable[i].Name,strlen(ModelTable[i].Name))
						)
					{
						free(modname);
						FullRec =1;
						//TIMESTAMPFAIL =1;
					   	WADRERUN = 1;
						break;
					}
					//TREBCUPS TOOK THIS OUT
			  //	}
				 free(modname);
				
			}			
		}
		fclose(fp);
	}
	else
		FullRec = 1;

	fp=fopen(StratFile, "wb");
	fwrite(&NumMods,sizeof(int),1,fp);		
	for (i=0;i<NumMods;i++)
	{
		len=strlen(ModelTable[i].Name)+1;
		fwrite(&len,sizeof(int),1,fp);
		//printf("%s of length %d \n",ModelTable[i].Name,len);
		fwrite(ModelTable[i].Name,sizeof(char),len,fp);
	}
	fclose(fp);

}

//CHECK INT SOUND LIST
void CheckSoundDep(char *filename)
{
	char StratFile[MAX_LINE];
	int i,len,OldMods;
	FILE *fp;
	char *modname;

	sprintf(StratFile,"%s%s%s",INTDIR,filename+1,".SIN");	

	
	if (fp=fopen(StratFile, "rb"))
	{
		fread(&OldMods,sizeof(int),1,fp);
		if (OldMods != NumSounds)
		{
			FullRec =1;
			WADRERUN =1;
		}
		else
		{
			for (i=0;i<NumSounds;i++)
			{
				fread(&len,sizeof(int),1,fp);
				modname=(char*)malloc(len);

				fread(modname,sizeof(char),len,fp);
					
				if ( 
					strnicmp(modname,SoundTable[i].Name,strlen(modname))
				  && 
					strnicmp(modname,SoundTable[i].Name,strlen(SoundTable[i].Name))
					)
					{
						free(modname);
						FullRec =1;
					   	WADRERUN = 1;
						break;
					}
				 free(modname);
				
			}			
		}
		fclose(fp);
	}
	else
		FullRec = 1;

	fp=fopen(StratFile, "wb");
	fwrite(&NumSounds,sizeof(int),1,fp);		
	for (i=0;i<NumSounds;i++)
	{
		len=strlen(SoundTable[i].Name)+1;
		fwrite(&len,sizeof(int),1,fp);
		fwrite(SoundTable[i].Name,sizeof(char),len,fp);
	}
	fclose(fp);

}



//CHECK INT ANIM LIST
void CheckAnimDep(char *filename)
{
	char StratFile[MAX_LINE];
	int i,len,OldAnims;
	//int loop;
	FILE *fp;
	char *modname;

	sprintf(StratFile,"%s%s%s",INTDIR,filename+1,".AIN");	

//	printf("opening %s models\n",StratFile);
	
	if (fp=fopen(StratFile, "rb"))
	{
		fread(&OldAnims,sizeof(int),1,fp);
		if (OldAnims != NumAnims)
		{
			FullRec =1;
//			TIMESTAMPFAIL =1;
			WADRERUN = 1;
		}
		else
		{
			for (i=0;i<NumAnims;i++)
			{
				fread(&len,sizeof(int),1,fp);
				modname=(char*)malloc(len);
				AMALAMOUNT += len;

				fread(modname,sizeof(char),len,fp);
				//TREBLE COMING CHECKS
				//printf("model %d is %s \n",i,modname);
				 if ( 
						strnicmp(modname,AnimTable[i].Name,strlen(modname))
					  && 
						strnicmp(modname,AnimTable[i].Name,strlen(AnimTable[i].Name))
						)
					{
						free(modname);
						FullRec =1;
						//TIMESTAMPFAIL =1;
					   	WADRERUN = 1;
						break;
					}
				 free(modname);

				 /*DELETED BLOCK FOR TREBLE COMING CHECKS*/
#if 0
				 for (loop=0;loop<len;loop++)
				{
										if ((modname[loop]) != (AnimTable[i].Name[loop]))
					{
						WADRERUN = 1;
						FullRec =1;
			//			TIMESTAMPFAIL =1;
						break;
					}
				}
#endif
			}			
		}
		fclose(fp);
	}
	else
		FullRec = 1;

	fp=fopen(StratFile, "wb");
	fwrite(&NumAnims,sizeof(int),1,fp);		
	for (i=0;i<NumAnims;i++)
	{
		len=strlen(AnimTable[i].Name)+1;
		fwrite(&len,sizeof(int),1,fp);
		//printf("%s of length %d \n",ModelTable[i].Name,len);
		fwrite(AnimTable[i].Name,sizeof(char),len,fp);
	}
	fclose(fp);

}


				    


FILE *OpenTable(char *filename)
{
	char StratFile[MAX_LINE];
	FILE *fp;

	filename+=1;
	sprintf(StratFile,"%s%s%s",SCRIPTDIR,filename,".C");	

	if (fp=fopen(StratFile, "wa"))
		return (fp);
	else
	{
		printf ("STRAT TABLE file error %s\n",StratFile);
		return (NULL);

	}


}

#if SH4
	FILE *OpenMakeFile(char *filename)
	{
		char StratFile[MAX_LINE];
		FILE *fp;

		filename+=1;
		sprintf(StratFile,"%s%s",MAKEFILEDIR,"STRAT.LST");	

		if (fp=fopen(StratFile, "wa"))
			return (fp);
		else
		{
			printf ("MAKEFILE file error %s\n",StratFile);
			return (NULL);
	   	}
      }
#endif

void Initialise()
{
	
	int loop;

//	return;
	for (loop = 0; loop < 512; loop ++)
	{
		ModelTable[loop].used = SoundTable[loop].used = AnimTable[loop].used = 0;

	}
}

int	GetModelIndex(char *model)
{
	int i=0;


//	printf("Num Mods %d\n",NumMods);	

	if (!(strnicmp(model,"P:\\GAME\\OBJECTS\\",16)))
		model += 16;
	if (!(strnicmp(model,"P:\\GAME\\NEWOBJECTS\\",19)))
		model += 19;



	for (i=0;i<NumMods;i++)
	{
		if ((!strnicmp(model,ModelTable[i].Name,strlen(model)+1))
		  &&(!strnicmp(ModelTable[i].Name,model,strlen(ModelTable[i].Name)+1)))	
		
		{
			#if DEBUGMOD
				printf("FOUND DUP MODEL name %s reques name %s entry %d\n",ModelTable[i].Name, model,i);
			#endif
	#if ALLRESIDENT
		if (SolvedCurrent)
			ModelTable[i].used=1;
	#else
		ModelTable[i].used=1;
	#endif
			return (i);	
		}
	}

	ModelTable[i].Name = malloc(strlen(model)+1);
	MALAMOUNT += strlen(model) + 1;

	strcpy (ModelTable[i].Name,model);

	#if ALLRESIDENT
		if (SolvedCurrent)
			ModelTable[i].used=1;
	#else
		ModelTable[i].used=1;
	#endif


	#if DEBUGMOD
		printf("allocated model %s at entry %d\n",ModelTable[i].Name,i);		
		printf("first model %s ",ModelTable[0].Name);		
	#endif
	i++;
	NumMods = i;
	if (NumMods > MAX_MODELS)
	{
   		printf("NUMBER OF GAME MODELS HAS EXCEEDED MAX ALLOWED %d \n",MAX_MODELS);
   		KEYBREAK();
	}
	return(i-1);
}

char *singlenames[]=
{
	"MATTPLEVEL",
	"SAVLEVEL",
	"NICKC",
	"DES",
	"MOOG",
	"HYDRO",
	"CANYON",
	"VOLCANO",
	"CITY",
	"ICE",
	"ALIEN",








};

char *multnames[]=
{
	"POWERUP1",
	"POWERUP2",
	"POWERUP3",
	"POWERUP4",
	"POWERUP5",
	"POWERUP6",
	"POWERUP7",
	"POWERUP8",
	"POWERUP9",
	"POWERUP10",
	"POWERUP11",








};




int SoundCheck (char *sfx)
{
	int loop,loop2,fail;
	fail = 0;

	for (loop = 0; loop < 11;loop++)
	{
		if (!(strnicmp(CurrentLevel,singlenames[loop],strlen(CurrentLevel))))
		{
			for (loop2 = 0; loop2 < 11; loop2 ++)
			{
				if (!(strnicmp(sfx,multnames[loop2],strlen(sfx))))
				{
					fail = 1;
				//	printf("IGNORING %s\n",sfx);

				}

			}

		}


	}

	return(!fail);

}



int	GetSoundIndex(char *sfx)
{
	int i=0;



	if (!(strnicmp(sfx,"P:\\SOUND\\SFX\\",13)))
		sfx += 13;

	//first check for entry into generic bank



	//sound check here

//	if (SoundCheck(sfx))
//	{

	for (i=0;i<NumSounds;i++)
	{
		if (!strnicmp(sfx,SoundTable[i].Name,strlen(SoundTable[i].Name)+1))
		{
			#if DEBUGMOD
				printf("FOUND DUP MODEL name %s reques name %s entry %d\n",ModelTable[i].Name, model,i);
			#endif
			if (SoundCheck(sfx))
			{		
		
		
			#if ALLRESIDENT
				if (SolvedCurrent)
					SoundTable[i].used=1;
			#else
				SoundTable[i].used=1;
			#endif
			}
			
			return (i);	
		}
	}


	SoundTable[i].Name = malloc(strlen(sfx)+1);
	SMALAMOUNT += strlen(sfx) + 1;

	strcpy (SoundTable[i].Name,sfx);

	if (SoundCheck(sfx))
	{

	#if ALLRESIDENT
		if (SolvedCurrent)
			SoundTable[i].used=1;
	#else
		SoundTable[i].used=1;
	#endif
	}

	i++;
	NumSounds = i;
  //	if (NONGENERIC)
  //		i = i + BASESFXNUM;
//   	}
	return(i-1);
}

char *month[12] =
{
	"jan",
	"feb",
	"mar",
	"apr",
	"may",
	"jun",
	"jul",
	"aug",
	"sep",
	"oct",
	"nov",
	"dec"
};


int MonthGreat(char *dmon,char *smon)
{
	int i,loop, found;

	found = 0;

	//go to point of src month in array
	for (loop=0;loop<12;loop++)
	{
		if (!strnicmp(month[loop],smon,3))
			break;

	}

	for (i = loop; i < 12; i++)
	{
		if (!strnicmp(month[i],dmon,3))
		{
		   	if (i > loop)
				found ++;
			break;
		}

	}


	return(found);
}


int DateGreat(char *dest,char *src)
{
	char *dpt;
	char *spt;
	int dlen;
	int slen;
	int dtime;
	int stime;
	char dname [4];
	char sname [4];
	dlen = strlen(dest);
	slen = strlen(src);
	dpt = dest + dlen - 5;
	spt = src + slen - 5;
	dtime = atoi(dpt);
	stime = atoi(spt);

	//year check
	if (stime != dtime)
		return(1);

	//month check
	dpt = dest + 4;
	spt = src + 4;
	Clear(dname,4);
	Clear(sname,4);
	strncpy(dname,dpt,3);
	strncpy(sname,spt,3);

	if (MonthGreat(sname,dname))
		return(1);

	//MONTH OF SRC IS LESS THAN OR EQUAL TO THE MONTH OF DST
	//IF EQUAL, CHECK FURTHER

	if (!strnicmp(sname,dname,3))
	{
	//day check
	dpt = dest + 8;
	spt = src + 8;
	
	Clear(dname,4);
	Clear(sname,4);
	strncpy(dname,dpt,2);
	strncpy(sname,spt,2);
	dtime = atoi(dname);
	stime = atoi(sname);

	if (stime > dtime)
		return(1);

	if (stime == dtime)
	{
		//hour check
		dpt = dest + 11;
		spt = src + 11;
	
		Clear(dname,4);
		Clear(sname,4);
		strncpy(dname,dpt,2);
		strncpy(sname,spt,2);
		dtime = atoi(dname);
		stime = atoi(sname);

		if (stime > dtime)
			return(1);

		if (stime == dtime)
		{
			//min check
			dpt = dest + 14;
			spt = src + 14;
	
			Clear(dname,4);
			Clear(sname,4);
			strncpy(dname,dpt,2);
			strncpy(sname,spt,2);
			dtime = atoi(dname);
			stime = atoi(sname);

			if (stime > dtime)
				return(1);
		}

	}
	}
	return(0);


}

void ReadGenericSounds()
{
	char GenericFile[128];
	char OSSFILENAME[128];
	char GenericOSSFileName[128];
	char name[64];
	FILE *OSSFILE;
	FILE *GENERICOSSFILE;
	char line[MAX_LINE];
	FILE *SoundFile;
	int index;
	struct _stat txtbuf,katbuf;


  
	sprintf(GenericOSSFileName, "%sGENERICSFX.OSS",SFXDIR);
	GENERICOSSFILE = fopen(GenericOSSFileName,"w");

	sprintf(name,"BankFileName P:\\SOUND\\GENERICSOUNDFX.Kat\n");
	fputs(name,GENERICOSSFILE);

	sprintf(GenericFile, "%sGENERICSFXBANK.TXT",SFXDIR);
	SoundFile = fopen(GenericFile,"r");
	if (SoundFile)
	{
		for (;;)
		{
			if (!(fgets(line,MAX_LINE,SoundFile)))
				break;
			Clear(name,64);
			ReadToEnd(name,line);

			if (name[0] == 0)
				break;

		   	if (strlen(name) > 2);
				index = GetSoundIndex(name);
		   //	Clear(name,64);
		   //	ReadToEnd(name,line);

			sprintf(OSSFILENAME,"%s%s.OSS",SFXDIR,name);
			OSSFILE = fopen(OSSFILENAME, "r");
			if ((OSSFILE) && (GENERICOSSFILE))
			{
				for (;;)
				{
					if (!(fgets(line,MAX_LINE,OSSFILE)))
						break;
					fputs(line,GENERICOSSFILE);

				}
				
			} 
			else 
			{
				printf("missing SOUNDFILE %s\n", OSSFILENAME);
   				KEYBREAK();
			}
			if (OSSFILE)
				fclose(OSSFILE);


		}
		if (GENERICOSSFILE)
			fclose(GENERICOSSFILE);

		fclose(SoundFile);
	}

	//set up the base generic internal table

	for (index=NumSounds;index<BASESFXNUM;index++)
	{
		SoundTable[index].Name = malloc(7);
		SMALAMOUNT += 7;

		strcpy (SoundTable[index].Name,"DUMMY\n");

	   //	SoundTable[index].Name = NULL;


	}


    NumSounds = BASESFXNUM;


	BaseSound = BASESFXNUM;
	
	//check date of generic txt against generic kat
	sprintf(name, "P:\\SOUND\\GENERICSOUNDFX.KAT");
	_stat(name,&katbuf);
	_stat(GenericFile,&txtbuf);
	Clear(name,64);
	Clear(GenericFile,64);


	sprintf(name,ctime(&katbuf.st_mtime));
	sprintf(GenericFile,ctime(&txtbuf.st_mtime));
	if (DateGreat(name,GenericFile))
	//run the compressor on the generic bank
	_spawnl (_P_WAIT, "p:\\sound\\MKBANK","P:\\SOUND\\MKBANK", GenericOSSFileName, NULL);
}


void CheckAnimName(char *anim,char *animmodel,char *animname)
{
	int i,index=0;
	int validanim=0;
	int len=strlen(anim);

	for (i=0;i<len;i++)
	{

		if (anim[i] == '>')
		{	
		 	//HAVE WE ALREADY READ THE ANIM NAME ?
			//IF SO, BREAK OUT
			if (validanim)
				break;
			validanim++;
			index=0;
			continue;
		}

		if (validanim)
		{
			animname[index]=toupper(anim[i]);
			index++;
		}
		else
		{
			animmodel[index]=toupper(anim[i]);
			index++;
		}
	}

	if (!(validanim))
	{
		printf("invalid anim %s\n",anim);
		printf("Press key to continue\n");

		for (;;)
		{

			if getchar()
				break;
		}

		exit(1);
	}
}

int	GetAnimIndex(char *anim)
{
	char AnimModel[256];
	char AnimName[256];
	char String[512];
	int i=0;

	Clear(AnimModel,256);
	Clear(AnimName,256);
	ReadToEnd(String,anim);
	CheckAnimName(String,AnimModel,AnimName);

	for (i=0;i<NumAnims;i++)
	{
		if ((!strncmp(AnimName,AnimTable[i].Name,strlen(AnimTable[i].Name)+1)) &&
			(!strncmp(AnimModel,AnimTable[i].ModelName,strlen(AnimTable[i].ModelName)+1)))
		{
			#if DEBUGANIM
				printf("FOUND DUP Anim model %s name %s reques name %s entry %d\n",AnimTable[i].ModelName,AnimTable[i].Name, anim,i);
			#endif
	#if ALLRESIDENT
		if (SolvedCurrent)
			AnimTable[i].used=1;
	#else
		AnimTable[i].used=1;
	#endif
			return (i);	
		}
	}

	AnimTable[i].Name = malloc(strlen(AnimName)+1);
	AMALAMOUNT += strlen(AnimName) + 1;

	strcpy (AnimTable[i].Name,AnimName);

	AnimTable[i].ModelName = malloc(strlen(AnimModel)+1);
	AMALAMOUNT += strlen(AnimModel) + 1;
	strcpy (AnimTable[i].ModelName,AnimModel);

	#if ALLRESIDENT
		if (SolvedCurrent)
			AnimTable[i].used=1;
	#else
		AnimTable[i].used=1;
	#endif


	#if DEBUGANIM
		printf("allocated model %s anim %s at entry %d\n",AnimTable[i].ModelName,AnimTable[i].Name,i);		
	#endif
	i++;
	NumAnims = i;
	return(i-1);
}


char currentstratfile[MAX_LINE];


//WILL GO THROUGH THE MAP FILE SEARCHING FOR KEYWORD STRAT AND OPENING THE STRAT'S
//FILE IN ORDER TO PROCESS IT

FILE *GetNextEntry(FILE *fp)
{
	char line[MAX_LINE];
	char *linest=line;
	char *filest=currentstratfile;

	Clear(currentstratfile,MAX_LINE);


	for (;;)
	{
		
		if (!(fgets(line,MAX_LINE,fp)))
			return(NULL);
		else
		{

			if (linest=WhiteSpaceRead(line))	
			{		
				if (!strnicmp("Object :",linest,strlen("Object")))
				{
					linest +=strlen("Object :");
					linest = WhiteSpaceRead(linest);
					ProcessModel(linest);

					//printf("entry %d\n",ProcessModel(linest));
				}					

				if(!strnicmp("STRATEND",linest,strlen("STRATEND")))
				{
					
					return(NULL);
				}					
				if(!strnicmp("STRAT",linest,strlen("STRAT")))
				{
					linest += strlen("STRAT");
					linest = WhiteSpaceRead(linest);

					while (*linest != '\n')
					{

						*filest=*linest;
						linest++;
						filest++;
					}
					//printf("opening %s\n",currentstratfile);
					return(OpenFile(currentstratfile,SINGLE));
				}

			}
		}
	}
}

char *Path (char *name)
{

	while (*name != '\0')
	{
		name++;

	}
	while (*name != '\\')
	{
		name--;
	}
	name++;
	return (name);
}	

void InsertModelChange(char *line)
{
	char modelfile[128];
	char Xclude[128];
	int	ModelEntry;	
	char scriptstring[512];
	char tab[32];
	char value[128];

	Clear(scriptstring,512);
	Clear(value,128);

	//GET THE BASE SCRIPT LINE TO USE FOR THE BLOCK

	StatementBlockStructure(scriptstring,tab);

	
	line=WhiteSpaceRead(line);
	ReadToEnd(modelfile,line);
	line = line + strlen(modelfile);
	#if DEBUGMOD
		printf("model file is %s\n",modelfile);
	#endif

	//READ SOME MORE TEXT IN HERE
	line=WhiteSpaceRead(line);
	ReadToEnd(Xclude,line);


	if ( (strlen(Xclude)) && (!(strnicmp(CurrentLevel,Xclude,strlen(Xclude))) ))
		ModelEntry = 0;
	else
		ModelEntry = GetModelIndex(modelfile);

	Indentation(tab);

		sprintf(scriptstring,"%s%s%d%s\n",
				tab,
				"SetModel(strat,",
				ModelEntry,
				");");

	AddLine(scriptstring);

	//PUSH THE LINE AS THE START OF THE NEXT BLOCK
	PushLine();


	if (!EndSubBlock[NEST])
	{

		EndSubBlock[NEST]=1;
		KillMe=NEST;
	}

		PushLine2();


	//STACK THE START OF THE NEXT BLOCK
	//IF AT BASE NEST

	if (!NEST)
	{
		statename[CurrentLabel].indent=Indent-1;
		statename[CurrentLabel].line=CurrentLine;
	}
}

void KEYBREAK()
{
 	for (;;)
   	{
   		while (!_kbhit());
   			break;
   	}
	LogErrorFile(1);
	//free up any reserved memory
	CleanModel();
	CleanSound();				
	CleanAnim();
	//ensure all files are closed
	fcloseall();
 	exit(1);
}

void InsertSoundChange(char *line)
{
	char modelfile[128];
	char Params[128];
	char Params2[128];
	char scriptstring[512];
	char tab[32];
	char value[128];
	char *origline = line;
	float x,y,z,Volume;
	int Channel;
	int index;

	Clear(scriptstring,512);
	Clear(value,128);
	Clear(Params,128);
	Clear(Params2,128);



	//GET THE BASE SCRIPT LINE TO USE FOR THE BLOCK

	StatementBlockStructure(scriptstring,tab);

	
	line=WhiteSpaceRead(line);
	if (line)
	{
		ReadToEnd(modelfile,line);
		line = line + strlen(modelfile);
		#if DEBUGMOD
			printf("model file is %s\n",modelfile);
		#endif
		index = GetSoundIndex(modelfile);
	}
	else
	{
		printf("missing SOUNDFILE details in %s\n",origline);
		strcpy(modelfile,"DUMMY");
		KEYBREAK();
	}

	//READ THE CHANNEL
	line=WhiteSpaceRead(line);
	if (line)
	{
		ReadToEnd(Params,line);
		Channel = atoi(Params);
		line = line + strlen(Params);
	}
	else
	{
		printf("missing CHANNEL details in %s\n",origline);
		Channel = 0;
		KEYBREAK();
	}

	//READ THE VOLUME
	line=WhiteSpaceRead(line);
	if (line)
	{
		ReadToEnd(Params,line);
		line = line + strlen(Params);
		ParseExpression(Params2,Params,1);
	  //	Volume = (float)atof(Params);
	  //	line = line + strlen(Params);
	}
	else
	{
		printf("missing VOLUME details in %s\n",origline);
	   	sprintf(Params2,"0.0f");
		Volume = 0.0f;
		KEYBREAK();
	}

	//READ THE X
	line=WhiteSpaceRead(line);
	if (line)
	{
		ReadToEnd(Params,line);
		x = (float)atof(Params);
		line = line + strlen(Params);
   	}
	else
	{
		printf("missing X details in %s\n",origline);
		x = 0.0f;
		KEYBREAK();
	}

	//READ THE Y
	line=WhiteSpaceRead(line);
	if (line)
	{
		ReadToEnd(Params,line);
		y = (float)atof(Params);
		line = line + strlen(Params);
   	}
	else
	{
		printf("missing Y details in %s\n",origline);
		KEYBREAK();
	}



	//READ THE Z
	line=WhiteSpaceRead(line);
	if (line)
	{
		ReadToEnd(Params,line);
		z = (float)atof(Params);
	  	line = line + strlen(Params);
	}
	else
	{
		printf("missing Z details in %s\n",origline);
		KEYBREAK();
	}

	//READ THE FLAG
	line=WhiteSpaceRead(line);
   	if (line)
	{
	  //	ReadToEnd(Params,line);
	  	Clear(Params,128);
		if (!ParseExpression(Params,line,CONDITION))
	   {
			printf("invalid FLAG details in %s\n",origline);
			KEYBREAK();
	   }
	   


	}
	else
	{
		printf("missing FLAG details in %s\n",origline);
		KEYBREAK();
	}

	Indentation(tab);
   //	sprintf(scriptstring,"%sPlaySound(strat,%d,%d,%f,%f,%f,%f,%s);\n",tab,Channel,index,Volume,x,y,z,Params);
	sprintf(scriptstring,"%sPlaySound(strat,%d,%d,%s,%f,%f,%f,%s);\n",tab,Channel,index,Params2,x,y,z,Params);



  /*		sprintf(scriptstring,"%s%s%d%s\n",
				tab,
				"SetModel(strat,",
				ModelEntry,
				");");
	*/
	AddLine(scriptstring);

	//PUSH THE LINE AS THE START OF THE NEXT BLOCK
	PushLine();


	if (!EndSubBlock[NEST])
	{

		EndSubBlock[NEST]=1;
		KillMe=NEST;
	}

		PushLine2();


	//STACK THE START OF THE NEXT BLOCK
	//IF AT BASE NEST

	if (!NEST)
	{
		statename[CurrentLabel].indent=Indent-1;
		statename[CurrentLabel].line=CurrentLine;
	}
	
}



void InsertAnimChange(char *line)
{
	char modelfile[128];
	int	ModelEntry;	
	char scriptstring[512];
	char tab[32];
	char value[128];

//	return;
	Clear(scriptstring,512);
	Clear(tab,32);
	Clear(value,128);
//	printf("anim line %s\n",line);

	//GET THE BASE SCRIPT LINE TO USE FOR THE BLOCK




	StatementBlockStructure(scriptstring,tab);

//	printf("anim line done1 %s\n",line);

	
	line=WhiteSpaceRead(line);
	ReadToEnd(modelfile,line);

	#if DEBUGMOD
		printf("model file is %s\n",modelfile);
	#endif

	ModelEntry = GetAnimIndex(modelfile);

	Indentation(tab);

		sprintf(scriptstring,"%s%s%d%s\n",
				tab,
				"SetAnim(strat,",
				ModelEntry,
				");");
	AddLine(scriptstring);

	//PUSH THE LINE AS THE START OF THE NEXT BLOCK
	PushLine();


	if (!EndSubBlock[NEST])
	{

		EndSubBlock[NEST]=1;
		KillMe=NEST;
	}

	PushLine2();


	//STACK THE START OF THE NEXT BLOCK
	//IF AT BASE NEST

	if (!NEST)
	{
		statename[CurrentLabel].indent=Indent-1;
		statename[CurrentLabel].line=CurrentLine;
	}

#if DEBUG_ANIM
	printf("anim line done %s\n",line);
#endif
}

int NewColFormat;


/*
 * Load a model from the given stream
 */
Model *LoadModel (FILE *fp)
{
	ModelHeader header;
	Model *retVal;
	int nPolys, nEdges, temp,mat;
	UV *uv;

	/* Set the material type accordingly */
   //	rLoadContext = &objContext;

	/* Read the header */
	fread (&header, sizeof (header),1, fp);

	/* Read the model itself, after allocating it */
	retVal = malloc (sizeof (Model));
	if (header.version < 0x140) {
		fread (retVal, sizeof (Model)-8,1, fp);
	} else if (header.version < 0x150) {
		fread (retVal, sizeof (Model)-4,1, fp);
	} else {
		fread (retVal, sizeof (Model),1, fp);
	}
	/* Read the material array */
	if (retVal->nMats)
	{
	  	retVal->material = malloc (sizeof (Material) * retVal->nMats);
		for (mat = 0; mat < retVal->nMats; ++mat)
		{
			char buffer[1024];
			fread (&retVal->material[mat], sizeof (Material),1, fp);
			if (retVal->material[mat].surf.GBIX)
				fread (buffer, retVal->material[mat].surf.GBIX, 1, fp);
			if (retVal->material[mat].pasted.surf.GBIX)
				fread (buffer, retVal->material[mat].pasted.surf.GBIX, 1, fp);
		}
		free(retVal->material);
	}

	/* Read in the vertices and vertex normal array */
	retVal->vertex			= malloc (sizeof (Point3) * retVal->nVerts);
	retVal->vertexNormal	= malloc (sizeof (Vector3) * retVal->nVerts);

	fread (retVal->vertex, sizeof (Point3), retVal->nVerts, fp);
	free(retVal->vertex);

	fread (retVal->vertexNormal, sizeof (Point3), retVal->nVerts, fp);
	free(retVal->vertexNormal);



	/* Derive some data */
	nPolys = retVal->nQuads + retVal->nTris + retVal->nStripPolys;
	nEdges = (retVal->nQuads * 4) + (retVal->nTris * 3) + (retVal->nStrips * 2) + retVal->nStripPolys;

	/* Read in the plane equations */
 	if (retVal->plane)
	{
		retVal->plane = malloc (sizeof (Plane) * nPolys);
		fread (retVal->plane, sizeof (Plane),nPolys, fp);
		free(retVal->plane);
	}

	/* Read in the UV values (which must be aligned) */
	retVal->uv = malloc (sizeof (UV) * nEdges + 16);
	uv = (UV *)(((int)retVal->uv + 15) &~15);
	fread (uv, sizeof (UV),nEdges, fp);
	free(retVal->uv);


	/* Finally, read in the strip info */
	temp = (int)retVal->strip;
	retVal->strip = malloc (sizeof (VertRef) * temp);
	fread (retVal->strip, sizeof (VertRef),temp, fp);
	free(retVal->strip);


	return retVal;
}

 /*
 * Recursively load an object	*/

void RecurseLoadObject (Object *retVal,FILE *fp)
{
	int child;

	/* Read in the base structure */
	fread (retVal, sizeof (Object),1,fp);

	/* Read in the associated collision points */
	if (retVal->ncp)
	{
		int i;
		retVal->cp = malloc (sizeof (CP2) * retVal->ncp);
		retVal->ocpt = malloc (sizeof (Point3) * retVal->ncp);
		if (NewColFormat)
		{
			for (i = 0; i < retVal->ncp; ++i) {
				fread (&retVal->cp[i].centre, sizeof (Point3), 1, fp);
				fread (&retVal->cp[i].radius, sizeof (float), 1, fp);
				retVal->cp[i].worldPos.x = retVal->cp[i].worldPos.y = retVal->cp[i].worldPos.z = 0.0f;
			}
		}
		else 
		{
			for (i = 0; i < retVal->ncp; ++i) {
				fread (&retVal->cp[i], sizeof (Point3),1, fp);
				retVal->cp[i].radius = 0.f;
			}
		}

	   	free(retVal->cp);
	   	free(retVal->ocpt);
	
	}
	/* Now to read in the model */
	if (retVal->model)
	{
		retVal->model = LoadModel (fp);
	  	free(retVal->model);
	}

	/* Recursively load children */
	if (retVal->no_child)
		retVal->child = malloc (sizeof (Object) * retVal->no_child);
	for (child = 0; child < retVal->no_child; ++child)
		RecurseLoadObject (&retVal->child[child], fp);

	if (retVal->child)
		free(retVal->child);
}

int	PNODEREAD(FILE *fp)
{
	ObjectHeader hdr;
	Object *obj;
   	int animflag=0;
	int NewObjFormat;

	/* Read in the header */
	fread (&hdr, sizeof (hdr),1, fp);

	if (hdr.tag != 0x544a424f)
		return(0);

	/* Check for the new collision point stuff */
	NewColFormat = (hdr.version >= 0x190);

	NewObjFormat = (hdr.version >= 0x191);

	/* Now recursively load in the objects */
	//numSub = 0;
	obj = malloc (sizeof (Object));
	RecurseLoadObject (obj,fp);
	
	free(obj);

	if (NewObjFormat)
		/*READ IN THE FLAG THAT SAYS ANIM ATTACHED/NON-ATTACHED*/
		fread(&animflag,sizeof(int),1,fp);

	if (!animflag)
		return(0);

	
	return(1);	

}


short GetModelAnimEntry(FILE *fp,char *name)
{
   	int numframes,numsubmods,animtype;
	int i,found=0;
	short maxkey;
	short numkeyframes;
	KEYJUMP jumptable;


  	/*READ IN THE ANIM TYPE*/
  	fread (&animtype, sizeof (int),1, fp);

  	/*READ IN THE NUMBER OF FRAMES*/
  	fread (&numframes, sizeof (int),1, fp);

	/*READ IN THE NUMBER OF MODELS PER FRAME*/
	fread (&numsubmods, sizeof (int),1, fp);

	

	/*READ INT THE NUMBER OF SPECIFIED KEYFRAME JUMPTABLE ENTRIES*/
	fread (&numkeyframes, sizeof (short),1,fp);

	for	(i=0;i<numkeyframes;i++)
	{
		fread(&jumptable.name,sizeof(char),32,fp);	//READ THE NAME

		fread(&jumptable.offset,sizeof(short),1,fp);	//THE OFFSET

		fread(&jumptable.numframes,sizeof(short),1,fp);	//AND THE FRAMES
	   
		fread(&maxkey,sizeof(short),1,fp);	//MAX KEYS


		jumptable.key = malloc(sizeof(short)*maxkey);

	  	fread(jumptable.key,sizeof(short),maxkey,fp);   //AND THE KEYFRAMES

		free(jumptable.key);
	  //	fread(&jumptable.key,sizeof(short),16,fp);   //AND THE KEYFRAMES


		if (strlen(name) == strlen(jumptable.name))
		{
			if (!(strnicmp(jumptable.name,name,strlen(name))))
			{
				found++;
				break;
			}
		}
	}
	
	if (!found)
		return(-1);
	else
		return(i);
	 //	return(jumptable.offset);
	

}



int GetModelAnimIndex(char *anim)
{
	char AnimModel[256];
	char AnimName[256];
	char String[512];
	FILE *ModFile;
	int entry;
	int ERROR=0;
	
	Clear(AnimModel,256);
	Clear(AnimName,256);
	ReadToEnd(String,anim);
	CheckAnimName(String,AnimModel,AnimName);

//	printf("MODEL IS %s ANIM IS %s\n",AnimModel,AnimName);


	sprintf(String,"%s%s",NEW_REDDOG_OBJECTS,AnimModel);

	ModFile = fopen(String,"rb");

	if (ModFile)
	{
		//READ UP THE MODEL WAREZ
		if (PNODEREAD(ModFile))
		{
			entry = GetModelAnimEntry(ModFile,AnimName);

			if (entry<0)
			{
				ERROR++;
				printf("****MODEL FILE %s DOES NOT CONTAIN ANIM %s\n",String,AnimName);	
			}
		}
		else
		{
			printf("****MODEL FILE %s DOES NOT CONTAIN ANIM %s\n",String,AnimName);	
			ERROR++;
			return(-1);


		}



	}
	else
	{
		printf("****MODEL FILE %s DOES NOT EXIST\n",String);	
		ERROR++;
		return(-1);
	}

	fclose(ModFile);

	if (ERROR)
		return(-1);
	else
		return(entry);
}


//reads modelname>animname>
//at the end, anim will point to the flag value
void GetModelAnimFlag(char *anim,char *value)
{
   	char *string=anim;
	int i,index=0;
	int validanim=0;
	int len=strlen(anim);

	for (i=0;i<len;i++)
	{

		if (anim[i] == '>')
		{	
		 	//HAVE WE ALREADY READ THE ANIM NAME ?
			//IF SO, BREAK OUT
			if (validanim)
			{
			 	i++;
				break;
			}
			validanim++;
			continue;
		}

	}
	anim = WhiteSpaceRead(&anim[i]);

	if (!anim)
	{
		printf("\n***ANIM FLAG MISSING %s\n",string);
		printf("Press key to continue\n");

		for (;;)
		{
			while (!_kbhit());
				break;
		}

		exit(1);


	}
	else
	{
	   //	value=value;
	   	ParseExpression(value,anim,CONDITION);
	}


}							  



int InsertModelAnimChange(char *line)
{
	char modelfile[128];
	int	ModelEntry;	
	char scriptstring[512];
	char tab[32];
	char value[128];

	Clear(scriptstring,512);
	Clear(tab,32);
	Clear(value,128);
	//GET THE BASE SCRIPT LINE TO USE FOR THE BLOCK




	StatementBlockStructure(scriptstring,tab);

//	printf("anim line done1 %s\n",line);

	
	line=WhiteSpaceRead(line);
	ReadToEnd(modelfile,line);

	#if DEBUGMOD
		printf("model file is %s\n",modelfile);
	#endif

	ModelEntry = GetModelAnimIndex(modelfile);
	if (ModelEntry<0)
		return(0);

	GetModelAnimFlag(modelfile,value);


	Indentation(tab);

	sprintf(scriptstring,"%s%s%d%s%s%s\n",
				tab,
				"SetModelAnim(strat,",
				ModelEntry,
				",",
				value,
	   		");");
	AddLine(scriptstring);

	//PUSH THE LINE AS THE START OF THE NEXT BLOCK
	PushLine();


	if (!EndSubBlock[NEST])
	{

		EndSubBlock[NEST]=1;
		KillMe=NEST;
	}

	PushLine2();


	//STACK THE START OF THE NEXT BLOCK
	//IF AT BASE NEST

	if (!NEST)
	{
		statename[CurrentLabel].indent=Indent-1;
		statename[CurrentLabel].line=CurrentLine;
	}

	return(1);
}



#define INVALID -1

int GetParams(FILE *fp,FILE *MapFile,int mode)
{
	char cursor[64];
	char line[MAX_LINE];
	char *linest=line;
	char *filest=currentstratfile;
	int token,model;
	int i,end=0,wayvar=0;
	short numpath,numways,startway;
	int numlights;
	int stratflag;
	float posvar=0;

	Clear(currentstratfile,MAX_LINE);

	do
//	for (;;)
	{
		if (!(fgets(line,MAX_LINE,fp)))
			return(0);
		else
		{

			linest=WhiteSpaceRead(line);
			
			if (!linest)
				continue;
			if ((strnicmp(linest,"//",2)) && ((strnicmp(linest,"/*",2))))
			{

				token=INVALID;	
				for(i=0; i< ASIZE(MapFields); i++)
				{
					
					if(!strncmp(MapFields[i].word,linest,MapFields[i].length))
					{
						token = MapFields[i].token;
						linest += MapFields[i].length;
						break;
					}		
				}
	 		   //	if (token==INVALID)
			   //		printf("%s %d\n",linest,token);

				

				if (token != INVALID)
				{
					switch (token)
					{
						case (POS):
						case DIR:
						case POINT3VAL:
					//	case (RELVEL):
					//	case (ABSVEL):
						case (WAYPOS):
						case (NODEVERTEX):
						case (TRIGVERTEX):
						case (ACTVERT):
						case (ROT):
						case (SCALE):
							for (i=0;i<3;i++)
							{
								Clear(cursor,64);
								linest = WhiteSpaceRead(linest);
								ReadToEnd(cursor,linest);
								linest += strlen(cursor);
								posvar = (float)atof(cursor);
								fwrite(&posvar,sizeof(float),1,MapFile);
							}
							break;

						case (AREAVERTEX):

						case (SUBAREAVERTEX):
							for (i=0;i<2;i++)
							{
								Clear(cursor,64);
								linest = WhiteSpaceRead(linest);
								ReadToEnd(cursor,linest);
								linest += strlen(cursor);
								posvar = (float)atof(cursor);
								//printf("FLOATY %f\n",posvar);
								fwrite(&posvar,sizeof(float),1,MapFile);
							}
							break;

						//case (HEALTH):

						case FLOATVAL:
						case FOGVAL:
						case ACTRAD:
						case NEAR:
						case FAR:
						case MINVAL:
						case (BOXMAXX):
						case (BOXMINX):
						case (BOXMAXY):
						case (BOXMINY):
						case MAXVAL:
						case (AREARADIUS):
						case(SUBAREARADIUS):
						case (ELLIPSELENGTH):
						case (ELLIPSEWIDTH):
						case (TRIGRADIUS):
							Clear(cursor,64);
							linest = WhiteSpaceRead(linest);
							ReadToEnd(cursor,linest);
							linest += strlen(cursor);
							posvar = (float)atof(cursor);
							fwrite(&posvar,sizeof(float),1,MapFile);
							break;

						// 1 - the value set in the editor
						case (ARMOUR):
						//	Clear(cursor,64);
						//	linest = WhiteSpaceRead(linest);
						//	ReadToEnd(cursor,linest);
						//	linest += strlen(cursor);
						//	posvar = 1.0f - (float)atof(cursor);
						//	fwrite(&posvar,sizeof(float),1,MapFile);
							break;

						case (MODEL):
							linest = WhiteSpaceRead(linest);
							model = ProcessModel(linest);
							#if DEBUGMOD
								printf("id %d\n",model);
							#endif
							fwrite(&model,sizeof(short),1,MapFile);
							break;

						case (FLAG):
							linest = WhiteSpaceRead(linest);
							Clear(cursor,64);
							linest = WhiteSpaceRead(linest);
							ReadToEnd(cursor,linest);
							stratflag = atoi(cursor);
							//printf("cursor = %s\n", cursor);
							//if (!(strcmp(cursor,"4096")))
							//	flagvar = STRAT_ENEMY;
							//else
							//	flagvar = STRAT_FRIENDLY;
							#if DEBUG_MAPFLAGS
								printf("flag %d\n", flagvar);
							#endif
							fwrite(&stratflag,sizeof(int),1,MapFile);
							break;

						case (STARTWAY):
						case (STARTPATH):
						case (TRIGGERFLAG):
						case (TRIGGERACTIVATECOUNT):
							linest = WhiteSpaceRead(linest);
							Clear(cursor,64);
							linest = WhiteSpaceRead(linest);
							ReadToEnd(cursor,linest);
							startway = atoi(cursor);
							fwrite(&startway,sizeof(short),1,MapFile);
							break;

						case (INTVAL):
						case (WAYPATH):
						case (NODEMASK):
							Clear(cursor,64);
							linest = WhiteSpaceRead(linest);
							ReadToEnd(cursor,linest);
							wayvar = atoi(cursor);
							fwrite(&wayvar,sizeof(int),1,MapFile);
							#if DEBUG_WAY
								printf("strat waypoint = %d\n",wayvar);
							#endif
							break;


						case (SHORTVAL):
						case (PATHAREA):
						case (NUMPATH):
						case (NUMACT):
						case (ACTPOINT):
						case (NUMTRIG):
						case (TRIGPATH):
						case (ATTACHED):
						case (MAPENTRY):
						case (NUMAREAPOINTS):
						case (NUMAREAS):
						case  (NUMBOXES):
						case (NUMSHAPEAREAS):
						case (NUMEXTSPLINEAREAS):
						case (NUMSIGHTAREAS):
						case (SUBSUBAREAS):
						case (NUMCIRCLEAREAS):
						case (NUMELLIPSEAREAS):
						case (WAYTYPE):
						case (NETNODENUM):
							Clear(cursor,64);
							linest = WhiteSpaceRead(linest);		
							ReadToEnd(cursor,linest);
							numpath = (short)atoi(cursor);
							fwrite(&numpath,sizeof(short),1,MapFile);
							#if DEBUG_WAY
								printf("numpath = %d\n",numpath);
							#endif  //if token == numareapoints breakpoint and check from there 
							if (token==NUMAREAS)
								token=token;								
								
								
								break;

						case (BYTEVAL):
						case (MAPVARS):
						case (LIGHTCOL):
						case (NUMPATROLROUTES):
						case (NETPATHNODE):
						case (NUMBERNETWAY):
							Clear(cursor,64);
							linest = WhiteSpaceRead(linest);		
							ReadToEnd(cursor,linest);
							numpath = (unsigned char)atoi(cursor);
							fwrite(&numpath,sizeof(unsigned char),1,MapFile);
							#if DEBUG_WAY
								printf("numpath = %d\n",numpath);
							#endif  //if token == numareapoints breakpoint and check from there 
							if (token==NUMAREAS)
								token=token;								
								
								
								break;
						case (NUMWAYS):
	  						Clear(cursor,64);
							linest = WhiteSpaceRead(linest);
							ReadToEnd(cursor,linest);
							numways = (short)atoi(cursor);
							fwrite(&numways,sizeof(short),1,MapFile);
							#if DEBUG_WAY
								printf("numways = %d\n",numways);
							#endif
							break;

						case (DOMEVAL):
//							printf("DOME FOUND\n");
							Clear(cursor,64);
							linest = WhiteSpaceRead(linest);
							ReadToEnd(cursor,linest);
							if (!(strnicmp(cursor,"NO_DOME",7)))
								DomeSet =0;
							else
							{
								DomeSet = 1;
								strcpy(DomeName,cursor);
							
							}

							break;
							
						case NUMLIGHTS:
							Clear(cursor,64);
							linest = WhiteSpaceRead(linest);
							ReadToEnd(cursor,linest);
							numlights = (short)atoi(cursor);
							fwrite(&numlights,sizeof(short),1,MapFile);
							break;
							
						case COLOUR:
							for (i=0;i<3;i++)
							{
								char colour;
								Clear(cursor,64);
								linest = WhiteSpaceRead(linest);
								ReadToEnd(cursor,linest);
								linest += strlen(cursor);
								colour = (char)atoi(cursor);
								fwrite (&colour,sizeof(char),1,MapFile);
							}
							break;
							
						case SPOTLIGHT:
							{
								char one = 1;
								fwrite (&one, 1, 1, MapFile);
							}
							break;
						case OMNILIGHT:
							{
								char zero = 0;
								fwrite (&zero, 1, 1, MapFile);
							}
							break;
						case (ENDLIGHTS):
							end++;
							break;
						case (ENDTRIG):
							end++;
							break;
						case (ENDACT):
							end++;
							break;
						case (ENDEVENT):
							end++;
							break;
						case (ENDSTRAT):
							if (!mode)
							 end++;
							break;
						case (ENDAREA):
							end++;
							break;
						case (ENDPATHS):
							end++;
							break;
						case (ENDDEFAULT):
							end++;
							break;
					}			
				}
			}		
			
		}
	}	while (!end);
	return(1);
}









FILE *GetNextEntryInt()
{
	int found=0;
	char line[MAX_LINE];
	char *linest=line;
	char *filest=currentstratfile;

	Clear(currentstratfile,MAX_LINE);


	if (InternalGrab >= DefinedStrats)
		return (NULL);
	else
	{
		
		strcpy (currentstratfile,StratList[InternalGrab].name);

		InternalGrab++;
	}


		return(OpenFile(currentstratfile,SINGLE));
}


int CheckStrat(char *Strat,int Mode)
{
	int i,found=0;
	char InitAddress[64];


	if (!PROCESSED)
	{
	Clear(InitAddress,64);


//	printf("checking %s \n",Strat);



	for (i=0;i< DefinedStrats;i++)
	{

		//case insensitive
		if (!(strnicmp(Strat,StratList[i].name,strlen(StratList[i].name)+1)))
		{
			if (SolvedCurrent)
				StratList[i].currentlevel = 1;

			#if DEBUG_STRATDUPS
				printf("duplicate strat found %s\n",Strat);
			#endif
			found++;
			break;
		}
			
	}

	if (!found)
	{
		
			StratList[DefinedStrats].name=malloc(strlen(Strat)+1);
			STMALAMOUNT += strlen(Strat) + 1;
			strcpy(StratList[DefinedStrats].name,Strat);

			sprintf(InitAddress,"%s%s",Strat,"Init");
			StratList[DefinedStrats].addr=malloc(strlen(InitAddress)+1);
		    STMALAMOUNT += strlen(InitAddress) + 1;
			strcpy(StratList[DefinedStrats].addr,InitAddress);
		//	StratList[DefinedStrats].opened = 1;

			//FLAG TO SAY THAT THE STRAT IS COMING FROM THE CURRENT LEVEL
			if (SolvedCurrent)
				StratList[DefinedStrats].currentlevel = 1;

			#if DEBUG_INT
				printf("address %s\n",StratList[DefinedStrats].addr);
			#endif
			if (DefinedStrats >= MAX_STRATS)
			{
   				printf("NUMBER OF GAME STRATS HAS EXCEEDED MAX ALLOWED %d \n",MAX_STRATS);
   				KEYBREAK();
			}
			DefinedStrats++;
		
	  //	 printf("opening %s\n",Strat);
		return(0);
	}
	else
		return(1);
	}
	else
	{

	for (i=0;i< DefinedFiles;i++)
	{

		//case insensitive
		if (!(strnicmp(Strat,StratFile[i].name,strlen(StratFile[i].name)+1)))
		{
			found++;
			break;
		}
			
	}

	if (!found)
	{
		
			StratFile[DefinedFiles].name=malloc(strlen(Strat)+1);
			STMALAMOUNT += strlen(Strat) + 1;
			strcpy(StratFile[DefinedFiles].name,Strat);

			DefinedFiles++;
		//	printf("opening %s\n",Strat);
		return(0);
	}
	else
		return(1);
 //		return(1);

	
	}	
}

int GetStratEntry(char *Strat)
{
	int i,maxstrats=0;


	for (i=0;i<DefinedStrats;i++)
	{

		if ((!(strnicmp(Strat,StratList[i].name,strlen(Strat)+1))) &&
			(!(strnicmp(StratList[i].name,Strat,strlen(StratList[i].name))))
			)
			
			break;

		if ((SINGLEMAP) && (StratList[i].currentlevel))
			maxstrats++;

	}

	if ((SINGLEMAP) && (StratList[i].currentlevel))
		return(maxstrats);
	else
		return(i);
}


int GetStratEntryReal(char *Strat)
{
	int i,maxstrats=0;


	for (i=0;i<DefinedStrats;i++)
	{
		if ((!(strnicmp(Strat,StratList[i].name,strlen(Strat)+1))) &&
			(!(strnicmp(StratList[i].name,Strat,strlen(StratList[i].name))))
			)
			
			break;

	   //	if (!(strnicmp(Strat,StratList[i].name,strlen(Strat)+1)))
	   //		break;

	}

		return(i);
}


void SetStratTableEntry(char *Strat)
{
	int entry;

	entry=GetStratEntryReal(Strat);

	StratList[entry].floatnum=FVarNum;
	StratList[entry].intnum=IVarNum;

}


//GIVEN P:\\..\\MODELNAME will extract the MODELNAME only

void ExtractModelName(char *model,char *file)
{
	int len,oldlen;
	char *start = file;
	len=strlen(file);
	file += len;


	oldlen = len;




	while ((oldlen) && (*file != '\\'))
	{
		oldlen--;
		file--;
	}
	
	//while (*file != '\\')
	//	file--;

	file ++;

	while (file != start+len)
	{
		*model=*file;
		model++;
		file++;
	}
}

int ValidAnim(int entry)
{
	int valid = -1,i;	
	char model[64];

	for (i=0;i<NumMods;i++)
	{
		Clear(model,64);
		ExtractModelName(model,ModelTable[i].Name);
		#if DEBUGANIM
			printf("checking model name %s of model entry %d \n",model,i);
		#endif
		if (!(strncmp(AnimTable[entry].ModelName,model,strlen(AnimTable[entry].ModelName))))
		{
			#if DEBUGANIM
				printf("model name %s of model entry %d is valid \n",model,i);
			#endif
			valid=i;
			break;
		}
	}

	return(valid);
}



void	CleanModel()
{
	int i;
	for (i=0;i<NumMods;i++)
	{
		if (ModelTable[i].used)
		{
			ModelTable[i].used = 0;				

			#if !ALLRESIDENT
	   			free(ModelTable[i].Name);
	   			ModelTable[i].Name = NULL;
				#endif
		}
	}
}

void	CleanSound()
{
	int i;
	for (i=0;i<NumSounds;i++)
	{
		if (SoundTable[i].used)
		{
			SoundTable[i].used = 0;				

			#if !ALLRESIDENT
		   		free(SoundTable[i].Name);
				SoundTable[i].Name = NULL;
		   	#endif
		}
	}
}


void	CleanAnim()
{
	int i;
	for (i=0;i<NumAnims;i++)
	{
		if (AnimTable[i].used)
		{
			#if !ALLRESIDENT
				free(AnimTable[i].ModelName);
				AnimTable[i].ModelName = NULL;
				free(AnimTable[i].Name);
				AnimTable[i].Name = NULL;
			#endif
			AnimTable[i].used = 0;				

		}
	}
}

//VERSION 2, OUTPUT A LIST OF MODELS IN THE GAME, SEND THEM TO MATT'S WADDER, GRAB THE
//RESULTING FILE AND MUNGE INTO THE MAP WAD

int ModelListBuild(char *strat,int UsedAmount,int fullproc)
{
	struct _stat buf;
	int i,error=0;
 	char filename[512];
	FILE *ModFile;

	
	if ((fullproc) && (ModFile = OpenModelList(strat)))
	{

		fprintf(ModFile,"%d\n",UsedAmount);
		for (i=0;i<NumMods;i++)
		{
			if (ModelTable[i].used)
			{
				if (!strnicmp (ModelTable[i].Name, "P:\\GAME\\NEWOBJECTS\\", 19)) //{
					sprintf (filename, NEW_REDDOG_OBJECTS "%s.RDO", &ModelTable[i].Name[19]);
			  	else
				if (!strnicmp (ModelTable[i].Name, "P:\\GAME\\OBJECTS\\", 16)) //{
					sprintf (filename, NEW_REDDOG_OBJECTS "%s.RDO", &ModelTable[i].Name[16]);
				else
					sprintf(filename,NEW_REDDOG_OBJECTS "%s.RDO",ModelTable[i].Name);

				if (_stat(filename,&buf) == -1)
				{

					printf("** MODEL RDO FILE DOES NOT EXIST %s\n",filename);
					fprintf(ModFile,"%s\n",filename);
					#if !ALLRESIDENT
		   				free(ModelTable[i].Name);
		   			#endif
					error++;
					break;
				}
   				else
				{	
					fprintf(ModFile,"%s\n",filename);
					fprintf(ModFile,"%d\n",i);
					
				   	ModelTable[i].used = 0;				

					//fridays addition
					#if !ALLRESIDENT
		   				free(ModelTable[i].Name);
		   			#endif

				}
				
			}
		}
		fclose(ModFile);
	}
	else
	{
		for (i=0;i<NumMods;i++)
		{
			if (ModelTable[i].used)
			{
				ModelTable[i].used = 0;				

				#if !ALLRESIDENT
		   			free(ModelTable[i].Name);
		   		#endif
			}
		}
	}

	return(error);
}


//MATTP VERSION TO JUST OUTPUT THE BINARY MODEL FILES		
void WadModList(char *strat,int UsedAmount,FILE *stratfp,int fullproc)
{
	struct _stat buf;
	int i,mem,totmem;
 	char filename[512];
	FILE *ModFile;
	char *modspace, *base;


	if (fullproc)
	{

		//1ST PASS
		//RUN THROUGH MODEL FILES ALLOCATING SIZE
	   
		mem = 0;
		for (i=0;i<NumMods;i++)
		{
			if (ModelTable[i].used)
			{
				if (!strnicmp (ModelTable[i].Name, "P:\\GAME\\OBJECTS\\", 16)) //{
					sprintf (filename, REDDOG_OBJECTS "%s.RDO", &ModelTable[i].Name[16]);
				else
					sprintf(filename,REDDOG_OBJECTS "%s.RDO",ModelTable[i].Name);
	
				if (_stat(filename,&buf) == -1)
				{
		
					printf("** FATAL ERROR TRYING TO OPEN %s\n",filename);
				}
				else
				{
					//ALLOCATE ENOUGH STORAGE SPACE FOR MODEL AND A ENTRY LIST INT VALUE
					#if ALLRESIDENT
						mem += buf.st_size;
						mem += 4;
					#else
						mem += buf.st_size;
					#endif

				}
				
			}
		}

		//SECOND PASS

		//ALLOCATE THE STORAGE AND START READING IN	DATA
		base = modspace = (char *)malloc(mem);


		printf(" ADD %x\n",modspace + mem);

		if (!modspace)
		{
			printf("** FATAL ERROR ON ALLOCATING WAD MODEL SPACE\n ");
			return;
		}

		totmem = mem;

		for (i=0;i<NumMods;i++)
		{
			if (ModelTable[i].used)
			{
				if (!strnicmp (ModelTable[i].Name, "P:\\GAME\\OBJECTS\\", 16)) //{
					sprintf (filename, REDDOG_OBJECTS "%s.RDO", &ModelTable[i].Name[16]);
				else
					sprintf(filename,REDDOG_OBJECTS "%s.RDO",ModelTable[i].Name);

				ModFile = fopen(filename,"rb");
				//utd rule
				_stat(filename,&buf);
			
				fread(modspace,1,buf.st_size,ModFile);

				modspace+=buf.st_size;

				#if ALLRESIDENT
					//output the entry name
					*(int*)modspace = i;
					modspace+=4;
				#endif	

   
				ModelTable[i].used = 0;				

	   			#if !ALLRESIDENT
	   				free(ModelTable[i].Name);
	   			#endif

				fclose(ModFile);
			}
		}


		//NOW HAVE THE DATA IN MEMORY SO DUMP IT OUT INTO OUR MODEL FILE

		modspace -= mem;

		//OUTPUT THE NUMBER OF MODELS
		fwrite(&UsedAmount,sizeof(int),1,stratfp);

		fwrite(base,1,mem,stratfp);

		free(modspace);

	}
	else
	{
		for (i=0;i<NumMods;i++)
		{
			if (ModelTable[i].used)
			{
				ModelTable[i].used = 0;				

				#if !ALLRESIDENT
		   			free(ModelTable[i].Name);
		   		#endif
			}
		}
	}
}
		

void WadAnimList(char *strat,int UsedAmount,FILE *stratfp,int fullproc)
{
	int i,MapEntry,len;

	if (fullproc)
	{
		//OUTPUT THE NUMBER OF ANIMS
		fwrite(&UsedAmount,sizeof(int),1,stratfp);

		for (i=0;i<NumAnims;i++)
		{
			if (AnimTable[i].used)
			{
				MapEntry = ValidAnim(i);
			
				if (MapEntry >=0 ) 
				{			
					#if DEBUGANIM
						printf("anim entnum %d \n",MapEntry);
					#endif
					fwrite(&MapEntry,sizeof(int),1,stratfp);

					len=strlen(AnimTable[i].Name)+1;
					fwrite(&len,sizeof(int),1,stratfp);
				 
					fwrite(AnimTable[i].Name,sizeof(char),len,stratfp);

					//OUTPUT THE ENTRY IN THE GLOBAL ANIM LIST
					#if ALLRESIDENT
						fwrite(&i,sizeof(int),1,stratfp);
					#endif
					//fridays addition			
					#if !ALLRESIDENT
						free(AnimTable[i].ModelName);
						free(AnimTable[i].Name);
					#endif
				}
				else
				{
					printf("****** invalid model referenced %s\n",AnimTable[i].ModelName);
					free(AnimTable[i].ModelName);
					free(AnimTable[i].Name);
				}
				AnimTable[i].used = 0;				

			}
		}
	}
	else
	{

		for (i=0;i<NumAnims;i++)
		{
			if (AnimTable[i].used)
			{
					#if !ALLRESIDENT
						free(AnimTable[i].ModelName);
						free(AnimTable[i].Name);
					#endif
				AnimTable[i].used = 0;				

			}
		}
	}
}
	

int ProcessScript(char *strat,FILE *stratfp,int mode);

//CHECKS TO SEE IF A *.LOG FILE EXISTS FOR THE LEVEL, IF SO, IT WILL BE TACKED TO
//THE END OF THE MAP FILE IF IN NON-SH4 MODE
//SH4 MODE WILL SEE THIS INFO BEING LOGGED IN A *.NFO FILE, KEPT IN THE SAME PLACE AS THE
//.MOD FILES. THE WADDER SHOULD USE THIS TO BRING ACROSS THE RELEVANT .LOG FILES AFTER THE MODELS
//HAVE BEEN BROUGHT ACROSS	

//IN ADDITION, .CAM FILES, IF EXISTING, ARE PROCESSED INTO .CAP FILES, AGAIN RESIDING IN THE
//SAME DIRECTORY AS .MOD FILES. AGAIN, THE .NFO FILE SHOULD RECORD THIS FOR THE WADDER, OR IT WILL BE
//DIRECTLY TACKED ONTO THE END OF THE WAD FILE IN NON-SH4 MODE


void ProcessRecorded(char *strat,FILE *stratfp,int fullproc)
{
	FILE	*LogFile;
	FILE    *SFXFile;
	char	logscript[128];
	char	SFXFileName[128];
	char	line[MAX_LINE];
	char    cursor[64];
	char	*linest;
	int		ID,usedSFX;
	int		numcommands;
	int		token;
	int		KATSIZE;
	int		i,LogExists=0,CamExists=0,ScriptExists=0;
  	struct	_stat buf;
	CAMCOMMAND *CamBuffer, *CamBufferBase;
	#if !SH4
		INPUT *InputBuffer;
	#endif

	strat+=1;
	sprintf(logscript,"%s%s%s",GAMEDIR,strat,".LOG");
	LogFile = fopen(logscript,"rb");
	if (LogFile)
	{
		LogExists = 1;
		//.LOG FILE EXISTS SO READ IT IN AND THEN SQUIRT IT OUT AGAIN

		#if !SH4
			InputBuffer = (INPUT*)malloc(sizeof(INPUT)*MAXKEYS);
			fread(InputBuffer,sizeof(INPUT),MAXKEYS,LogFile);
			fwrite(InputBuffer,sizeof(INPUT),MAXKEYS,stratfp);
			free(InputBuffer);
			fclose(LogFile);
		#endif
	}

	
	sprintf(logscript,"%s%s%s",GAMEDIR,strat,".CAM");
	LogFile = fopen(logscript,"ra");
	if (LogFile)
	{
		CamExists = 1;
		//.CAM FILE EXISTS SO LETS PROCESS IT
		numcommands = 0;
		
		while (fgets(line,MAX_LINE,LogFile))
			numcommands++;

		if (numcommands)
		{
		
			CamBufferBase = CamBuffer = (CAMCOMMAND*)malloc(sizeof(CAMCOMMAND)*numcommands);

			rewind(LogFile);

			while (fgets(line,MAX_LINE,LogFile))
			{
				linest=WhiteSpaceRead(line);
				if (!linest)
					continue;

				token=INVALID;	
				for(i=0; i< ASIZE(CamFields); i++)
				{
					if(!strnicmp(CamFields[i].word,linest,CamFields[i].length))
					{
						token = CamFields[i].token;
						linest += CamFields[i].length;
						break;
					}		
				}
			
				if (token != INVALID)
				{
					switch (token)
					{
					 	case CAMERASCR:
							Clear(cursor,64);
							linest = WhiteSpaceRead(linest);
							ReadToEnd(cursor,linest);
							CamBuffer->camnum = (short)atoi(cursor);
							linest += strlen(cursor);
					   //		break;
					   //	case FRAMES:
							Clear(cursor,64);
							linest = WhiteSpaceRead(linest);
											
							ReadToEnd(cursor,linest);
			   				CamBuffer->numframes = (short)atoi(cursor);
							CamBuffer++;
							break;
					}
			 	}
			}
		}
		fclose(LogFile);

		CamBuffer = CamBufferBase;

#if		SH4
		//OUR CAMERA SCRIPT IS SET UP IN A BUFFER
		//SO OUTPUT THE .CAP FILE

		/****** TO DO *************/

		sprintf(logscript,"%s%s%s",MAPDIR,strat,".CAP");
		LogFile = fopen(logscript,"wb");

		//WRITE THE NUMBER OF COMMANDS

		fwrite(&numcommands,sizeof(short),1,LogFile);
		//NOW THE COMMANDS THEMSELVES
		fwrite(CamBuffer,sizeof(CAMCOMMAND),numcommands,LogFile);
		fclose(LogFile);


#else
		//OUR CAMERA SCRIPT IS SET UP IN A BUFFER
		//SO TACK IT TO THE LEVELS END

		//WRITE THE NUMBER OF COMMANDS
		fwrite(&numcommands,sizeof(short),1,stratfp);
		//NOW THE COMMANDS THEMSELVES
		fwrite(CamBuffer,sizeof(CAMCOMMAND),numcommands,stratfp);
#endif

		if (CamBuffer)
			free(CamBuffer);

	}

   	#if SH4
		sprintf(logscript,"%s%s%s",MAPDIR,strat,".NFO");
		LogFile = fopen(logscript,"wa");

		//if ((!CamExists) && (!LogExists))
		//	fprintf(LogFile,"NO EXTRAS\n");

		if (DomeSet)
	   		fprintf(LogFile,"%s%s\n",DOMEDIR,DomeName);
		else
	   		fprintf(LogFile,"SKIP_NODOME\n");



	   	if (LogExists)
	   		fprintf(LogFile,"%s%s%s\n",GAMEDIR,strat,".LOG");
		else
	   		fprintf(LogFile,"SKIP_NODEMOLOG\n");



	   	if (CamExists)
	   		fprintf(LogFile,"%s%s%s\n",MAPDIR,strat,".CAP");
		else
	   		fprintf(LogFile,"SKIP_NODEMOCAM\n");


		ScriptExists = ProcessScript(strat,stratfp,1);

	   	if (ScriptExists)
	   		fprintf(LogFile,"%s%s%s\n",MAPDIR,strat,".DSO");
		else
	   		fprintf(LogFile,"SKIP_NOSCRIPT\n");

		//IS THERE A CHARACTER WAD TXT FILE ?
		if (CHARACTERSET)
		{
			for (i=0;i<MAXGAMECHARS;i++)
			{
			
		   		if (!CharUsedArray[i])
					fprintf(LogFile,"SKIP_NOCSW\n");
				else
				{
					#if BUILDHEADS
						switch (i)
						{
							default:
								fprintf(LogFile,NEW_REDDOG_OBJECTS "%s.RDO\n","HUD\\HERO");
						   //	fprintf(LogFile,"\t%s%s%s\n",MAPDIR,strat,".CSW");
							break;

						}
					#else
						fprintf(LogFile,"SKIP_NOCSW\n");

					#endif

				}

			}
		}
		else
		{
	   	 	for (i=0;i<MAXGAMECHARS;i++)
				fprintf(LogFile,"SKIP_NOCSW\n");

		}

		//now lets see what sounds go out..if any
		//..so we first check that we are using unique non-generic sounds

		//if (NumSounds != BaseSound)
		//{
			//get the size of the kat
			sprintf(SFXFileName,"%s%s.KAT",MAPDIR,strat);
			SFXFile = fopen(SFXFileName,"rb");
			if (SFXFile)
			{
			  	_stat(SFXFileName,&buf);
			  	KATSIZE = buf.st_size;
				fclose(SFXFile);
			}
		  
			//..build an sfx file containing the number of unique sounds
			//..plus their offset remap
			//..and let the wad know about it
			sprintf(SFXFileName,"%s%s.SFX",MAPDIR,strat);
		   	fprintf(LogFile,"%s\n",SFXFileName);

//			if (fullproc)
//			{
			SFXFile = fopen(SFXFileName,"wb");
			if (SFXFile)
			{
				//output the number of unique sounds

			   //	UniqueSFX = NumSounds - BaseSound;
			   //	fwrite(&UniqueSFX,sizeof(int),1,SFXFile);
		   		usedSFX=0;

				for (i=BaseSound;i<NumSounds;i++)
				{
					//see if the sound is used
					if (SoundTable[i].used)
					{
						usedSFX++;

					}

				}

				//num to read
			    fwrite(&usedSFX,sizeof(int),1,SFXFile);
			  

		   		usedSFX=0;
				for (i=BaseSound;i<NumSounds;i++)
				{
					//see if the sound is used
					if (SoundTable[i].used)
					{
						//a unique
						//so output true entry in big sound table
						fwrite(&i,sizeof(int),1,SFXFile);
						//and now its address offset
						//different depending on whether its generic or unique
						if (i<BaseSound)
							ID = i;
						  //	ID = BaseSound + usedSFX;
						else
							ID = usedSFX;// + 128;
						fwrite(&ID,sizeof(int),1,SFXFile);
						usedSFX++;

						if (i>= BaseSound)
						{
							SoundTable[i].used = 0;
					   	  // 	free (SoundTable[i].Name);
						}
					}


				}

				//tack on the byte size of the .KATFILE
				fwrite(&KATSIZE,sizeof(int),1,SFXFile);
				fclose(SFXFile);
			
			}
//			}
			//Now let the WAD know about the .KAT file
			sprintf(SFXFileName,"%s%s.KAT",MAPDIR,strat);
		   	fprintf(LogFile,"%s\n",SFXFileName);

			
			
	   //	}
	   //	else
	   //	   	fprintf(LogFile,"SKIP_NOCSW\n");




		fclose(LogFile);

		
   	#endif

}


//GIVEN AN ENTRY OF TEXTSTART --- blabla -- TEXTEND, THIS WILL RETURN THE AMOUNT OF blabla CHARS

int SumScriptText(int entryline,FILE *File, int *numtextlines)
{
	char	line[MAX_LINE];
	int textsize=0;
	char	*linest;
	int		token,end=0;
	int i,LineNum=0;

	while ((!end) && (fgets(line,MAX_LINE,File)))
	{

		linest=WhiteSpaceRead(line);
		if (!linest)
			continue;

		token=INVALID;	
		for(i=0; i< ASIZE(ScriptFields); i++)
		{
			if(!strnicmp(ScriptFields[i].word,linest,ScriptFields[i].length))
			{
				token = ScriptFields[i].token;
				linest += ScriptFields[i].length;
				break;
			}		
		}
			
		if (token != INVALID)
		{
			switch (token)
			{
					case TEXTEND:
						end++;
						break;
					case TEXTENTRY:
					  //	Clear(cursor,64);
					  //	linest = WhiteSpaceRead(linest);
					   //	sscanf(linest,"%s\n",cursor);
					   //	ReadToEnd(cursor,linest);
					   //	linest++;
				 
						*(numtextlines) = *(numtextlines)+1;					
						textsize += strlen(linest);
						break;
					case NOTEXT:
					  //	*(numtextlines) = 1;					
						textsize = 0;
						end++;
						break;

					default:
						break;
			}
		}

	}

	//ERROR IF NO TEXTEND RECORD HAS BEEN FOUND
	if (!end)
		textsize=-1;
	else
	{
		//PUT THE FILE BACK AS WE FOUND IT
	  	rewind(File);

		while ((LineNum != entryline) && (fgets(line,MAX_LINE,File)))
		{
			LineNum++;
		}



	}

   return(textsize);


}

int SumScriptScene(int entryline,FILE *File)
{
	char	line[MAX_LINE];
	char	*linest;
	int		token,end=0, error = 0, endfound = 0;
	int i,LineNum=0,NUMLINES=0;

	while ((!end) && (fgets(line,MAX_LINE,File)))
	{

		linest=WhiteSpaceRead(line);
		if (!linest)
			continue;

		token=INVALID;	
		for(i=0; i< ASIZE(ScriptFields); i++)
		{
			if(!strnicmp(ScriptFields[i].word,linest,ScriptFields[i].length))
			{
				token = ScriptFields[i].token;
				linest += ScriptFields[i].length;
				break;
			}		
		}
			
		if (token != INVALID)
		{
			switch (token)
			{
					case SCENEENTRYEND:
					   	NUMLINES++;
						endfound++;
						break;
					case SCENEEND:
						if (!endfound)
							error++;
						end++;
						break;
					
					case SCENESCRIPTENTRY:
				 		NUMLINES++;					
						break;
					default:
						error++;
						break;
			}
		}

	}

	//ERROR IF NO TEXTEND RECORD HAS BEEN FOUND
	if ((!end) || (error))
	{
		NUMLINES=-1;
		printf("\n\t!! ERROR:UNTERMINATED SCENE !!\n");

	}
	else
	{
		//PUT THE FILE BACK AS WE FOUND IT
	  	rewind(File);

		while ((LineNum != entryline) && (fgets(line,MAX_LINE,File)))
		{
			LineNum++;
		}



	}

   return(NUMLINES);


}




int CopyScriptText(int entryline,FILE *out,FILE *File)
{
	char	line[MAX_LINE];
	int textsize=0;
	char	*linest;
	int		token,end=0;
	int i,LineNum=0,len;


	while ((!end) && (fgets(line,MAX_LINE,File)))
	{

		LineNum++;
		linest=WhiteSpaceRead(line);
		if (!linest)
			continue;

		token=INVALID;	
		for(i=0; i< ASIZE(ScriptFields); i++)
		{
			if(!strnicmp(ScriptFields[i].word,linest,ScriptFields[i].length))
			{
				token = ScriptFields[i].token;
				linest += ScriptFields[i].length;
				break;
			}		
		}
			
		if (token != INVALID)
		{
			switch (token)
			{
					case TEXTEND:
						end++;
						break;
		
					case TEXTENTRY:
					 //	linest++;
						

						len = strlen(linest);
						fwrite(&len,sizeof(short),1,out);
						fwrite(linest,sizeof(char),strlen(linest),out);
					  //	strcpy(buf,linest);
					  //	buf+=(strlen(linest)+1);
						break;
					default:
						break;
			}
		}

	}

   return(LineNum);	

}

int CopySceneText(int entryline,FILE *out,FILE *File)
{
	char	line[MAX_LINE];
	int textsize=0;
	char	*linest;
	int		token,end=0;
	int i,LineNum=0;
	char    cursor[64];
	short  entry;


	while ((!end) && (fgets(line,MAX_LINE,File)))
	{

		LineNum++;
		linest=WhiteSpaceRead(line);
		if (!linest)
			continue;

		token=INVALID;	
		for(i=0; i< ASIZE(ScriptFields); i++)
		{
			if(!strnicmp(ScriptFields[i].word,linest,ScriptFields[i].length))
			{
				token = ScriptFields[i].token;
				linest += ScriptFields[i].length;
				break;
			}		
		}
			
		if (token != INVALID)
		{
			switch (token)
			{
		
					case SCENESCRIPTENTRY:
					 
						//READ TO END OUTPUT THE ENTRY SHORT
							Clear(cursor,64);
						 	linest = WhiteSpaceRead(linest);
						 	ReadToEnd(cursor,linest);
							
						 	entry = (short)atoi(cursor);
						   	fwrite(&entry,sizeof(short),1,out);

						break;

					case SCENEENTRYEND:
							end++;
						//OUTPUT NULL TERMINATOR SHORT -1
							entry = -1;
						   	fwrite(&entry,sizeof(short),1,out);


						

						break;
					default:
						break;
			}
		}

	}

   return(LineNum);	

}


short GetTotalScriptEntries(FILE *File)
{
	char	line[MAX_LINE];
	int textsize=0;
	char	*linest;
	int		token,end=0;
	int i,LineNum=0;

	int TIMECHECKMODE=0;
	int error = 0;
	int entry=0;

	while ((fgets(line,MAX_LINE,File)) && (!error))
	{

		linest=WhiteSpaceRead(line);
		if (!linest)
			continue;

		token=INVALID;	
		for(i=0; i< ASIZE(ScriptFields); i++)
		{
			if(!strnicmp(ScriptFields[i].word,linest,ScriptFields[i].length))
			{
				token = ScriptFields[i].token;
				linest += ScriptFields[i].length;
				break;
			}		
		}
			
		if (token != INVALID)
		{
			switch (token)
			{
					case SCRIPTENTRY:
						if (!TIMECHECKMODE)
						{
							entry++;
							TIMECHECKMODE=1;
						}
						break;
					case SCRIPTTIME:
						if (TIMECHECKMODE)
							TIMECHECKMODE=0;
						break;
					case TEXTSTART:
						if (TIMECHECKMODE)
							error++;

						
					default:
						break;
			}
		}

	}
	if (TIMECHECKMODE)
		printf("\n\t!! ERROR:MISSING TIME FIELD IN SCRIPT FILE !!\n");


	return(entry);

}

short GetTotalSceneEntries(FILE *File)
{
	char	line[MAX_LINE];
	int textsize=0;
	char	*linest;
	int		token,end=0;
	int i,LineNum=0;

	int TIMECHECKMODE=0;
	int error = 0;
	int scenes=0;

	while ((fgets(line,MAX_LINE,File)) && (!error))
	{

		linest=WhiteSpaceRead(line);
		if (!linest)
			continue;

		token=INVALID;	
		for(i=0; i< ASIZE(ScriptFields); i++)
		{
			if(!strnicmp(ScriptFields[i].word,linest,ScriptFields[i].length))
			{
				token = ScriptFields[i].token;
				linest += ScriptFields[i].length;
				break;
			}		
		}
			
		if (token != INVALID)
		{
			switch (token)
			{
			  	case SCENESTART:
			  	  	scenes++;
			  		break;
						
			  	default:
			  		break;
			}
		}

	}


	return(scenes);

}


void BuildCSW(char *strat)
{
	struct _stat buf;
	FILE	*LogFile;
	FILE    *ModFile;
	char	logscript[128];
	char	filename[128];
	char    *ModMem;
	int		size;
	short	totchars,i;

	sprintf(logscript,"%s%s%s",MAPDIR,strat,".CSW");
	LogFile = fopen(logscript,"wb");
	if (LogFile)
	{
	  	//first of all get max entries of character models
		//0 - 10

		totchars = 0;
		for (i=0;i<MAXGAMECHARS;i++)
		{
			if (CharUsedArray[i])
				totchars++;	
		}
		
		fwrite(&totchars,sizeof(short),1,LogFile);
		
		
		for (i=0;i<MAXGAMECHARS;i++)
		{
			if (CharUsedArray[i])
			{
				fwrite(&i,sizeof(short),1,LogFile);
			 	switch (i)
				{
					default:
						sprintf(filename,NEW_REDDOG_OBJECTS "%s.RDO","HUD\\HERO");
						break;

				}
				ModFile = fopen(filename,"rb");
				if (ModFile)
				{
				
					//utd rule
					_stat(filename,&buf);

					size  = buf.st_size;
					ModMem = (char*)malloc(size);
					if (ModMem)
					{
						fread(ModMem,sizeof(char),size,ModFile);
						fwrite(ModMem,sizeof(char),size,LogFile);
						free(ModMem);
					}
					//else error on mem allocation
					

					fclose(ModFile);
				}
				// else error on modfile open



			}
			
			//	fprintf(LogFile,"%d\n",i);
			
		}

		//else error on logfile open

		fclose(LogFile);
	}
}



// TO PROCESS .DSC FILES WHICH ARE USED FOR REDDOG IN-GAME SCRIPTS
// mode 0 to check if scripts needed, used in building .NFO
// mode 1 to build up the .DSO and .CSW (CHARACTER MODEL LIST) files

// .NFO CURRENTLY USES MODE 1, AS WE ARE BEING STUPIDLY LAZY AT THE MOMENT
int ProcessScript(char *strat,FILE *stratfp,int mode)
{
	FILE	*LogFile;
	char	logscript[128];
	char	line[MAX_LINE];
	char    cursor[64];
	char	*linest;
	int		numcommands;
	int		token;
	int		i,ScriptExists=0,LINENUM,STARTLINE;
	int		size;
	int     numtextlines;
	short   Scenes,entry,PassageOpen=0;
	FILE    *outfile;
	unsigned char scriptflag;
	signed char	scriptcharacter;
 //	short *CharUsedArray=NULL;

   //	strat+=1;

	
	sprintf(logscript,"%s%s%s",GAMEDIR,strat,".DSC");
  	LogFile = fopen(logscript,"r");
	if (LogFile)
	{

		ScriptExists = 1;
		if (mode)
		{
			sprintf(logscript,"%s%s%s",MAPDIR,strat,".DSO");
			outfile = fopen(logscript,"wb");


			//.DSC FILE EXISTS SO LETS PROCESS IT
			numcommands = 0;
		
			while (fgets(line,MAX_LINE,LogFile))
				numcommands++;

			if (numcommands)
			{


			  //	CharUsedArray = (short*)malloc(sizeof(short) * MAXGAMECHARS);

   				CHARACTERSET = 0;
				for (i=0;i<MAXGAMECHARS;i++)
					CharUsedArray[i]=0;

			//get the total number of scenes
			rewind(LogFile);
			Scenes = GetTotalSceneEntries(LogFile);
		  	fwrite(&Scenes,sizeof(short),1,outfile);
		  			

			//get the total number of script entries
			rewind(LogFile);
			entry = GetTotalScriptEntries(LogFile);
			fwrite(&entry,sizeof(short),1,outfile);


			rewind(LogFile);

			LINENUM=0;

			while (fgets(line,MAX_LINE,LogFile))
			{

				LINENUM++;
			   	linest=WhiteSpaceRead(line);
	   	  	 	if (!linest)
					continue;

				token=INVALID;	
				for(i=0; i< ASIZE(ScriptFields); i++)
				{
					if(!strnicmp(ScriptFields[i].word,linest,ScriptFields[i].length))
					{
						token = ScriptFields[i].token;
						linest += ScriptFields[i].length;
						break;
					}		
				}
			
				if (token != INVALID)
				{
					switch (token)
					{
   						//READ THE PASSAGE ENTRY 
						case SCENESTART:
							STARTLINE = LINENUM;
							Clear(cursor,64);
							linest = WhiteSpaceRead(linest);
							ReadToEnd(cursor,linest);
							
							entry = (short)atoi(cursor);
						   	fwrite(&entry,sizeof(short),1,outfile);

							PassageOpen++;
							size = SumScriptScene(STARTLINE,LogFile);
							if (size > 0)
							{
								 /*KICK OUT THE NUMBER OF SCRIPTS IN THE SCENE*/
								 fwrite(&size,sizeof(short),1,outfile);
								 LINENUM +=  CopySceneText(STARTLINE,outfile,LogFile);
							}
							 		
							break;
						case SCENEEND:
							PassageOpen--;
							break;
							
						
						//OUTPUT THE SCRIPT ENTRY
						case SCRIPTENTRY:
							//RESET THE SCRIPT FLAG
							scriptflag = 0;
						   	//RESET THE SCRIPT CHARACTER
							scriptcharacter = -1;
							
							Clear(cursor,64);
							linest = WhiteSpaceRead(linest);
							ReadToEnd(cursor,linest);
							entry = (short)atoi(cursor);
							fwrite(&entry,sizeof(short),1,outfile);
							break;
						//PROCESS ANY FLAGS
						case SCRIPT_SKIPPABLE:
							scriptflag |= 1;
							break;
						case SCRIPT_STOPSGAME:
							scriptflag |= 2;
							break;
						//PROCESS THE CHARACTERS
						case INTELLIGENCE_OFFICER:
						case HERO:
						case COMMANDER:
						case DROPSHIP_PILOT:
							scriptcharacter = (signed char)(token - INTELLIGENCE_OFFICER);
							//update the array 
							
							break;



						//OUTPUT THE SCRIPT TIME
						case SCRIPTTIME:
							Clear(cursor,64);
							linest = WhiteSpaceRead(linest);
							ReadToEnd(cursor,linest);
							entry = (short)atoi(cursor);
							fwrite(&entry,sizeof(short),1,outfile);
							break;


						case TEXTSTART:
							STARTLINE = LINENUM;


							numtextlines = 0;
							size = SumScriptText(STARTLINE,LogFile,&numtextlines);

							if (size>=0)
							{

							   /*KICK OUT THE NUMBER OF LINES OF TEXT*/
							   fwrite(&numtextlines,sizeof(short),1,outfile);
					 
							   /*KICK OUT THE FLAG*/
							   fwrite(&scriptflag,sizeof(unsigned char),1,outfile);

							   /*KICK OUT THE ASSOCIATED CHARACTER*/
							   fwrite(&scriptcharacter,sizeof(signed char),1,outfile);
						  	   //AND UPDATE THE ARRAY + FLAG WHETHER CHARACTER SET
							   if (scriptcharacter >=0)
							   {
									CharUsedArray[scriptcharacter] = 1;
									CHARACTERSET = 1;
							   }
							   /*GET THE TEXT ITSELF*/
							 LINENUM +=  CopyScriptText(STARTLINE,outfile,LogFile);
							   
							}
							break;
						default:
							break;
					}
			 	}
			}
	  
		  	//free(CharUsedArray);
			
			if (PassageOpen)
				printf("\n\t!! UNTERMINATED PASSAGE IN SCRIPT FILE!!\n");

		
		}
		fclose(outfile);
		}
		fclose(LogFile);
	}

#if 0
	//SEE IF WE NEED TO BUILD A .CSW FILE
	if ((CHARACTERSET) && (mode))
		BuildCSW(strat);
#endif

	return(ScriptExists);
}

void ProcessSounds(char *level)
{
	char OssFileName[128];
	FILE *OSSFILE;
	char name[128];
	char line[128];
	char IndOssFileName[128];
	FILE *INDOSSFILE;
	int i;
	
	//skip the @
	level++;

	//..local oss to be written to local strats\maps
	sprintf(OssFileName,"%s%s.OSS",MAPDIR,level);
	OSSFILE = fopen(OssFileName,"w");
	//.kats to be written to local strats\maps
	sprintf(name,"%s%s",MAPDIR,level);
	sprintf(line,"BankFileName %s.Kat\n",name);
	fputs(line,OSSFILE);

	if (OSSFILE)
	{
		for (i=BaseSound;i<NumSounds;i++)
		{
			//let's check if the sound is needed on this map
			if (SoundTable[i].used)
			{
				//if so, open the oss file
				sprintf(IndOssFileName,"%s%s.OSS",SFXDIR,SoundTable[i].Name);
			    INDOSSFILE = fopen(IndOssFileName,"r");
				if (INDOSSFILE)
			  	{
					//copy the details of the .OSS to the main level .OSS
					for (;;)
					{
						Clear(line,128);
						if (!(fgets(line,MAX_LINE,INDOSSFILE)))
							break;
						fputs(line,OSSFILE);


					}


					fclose(INDOSSFILE);
				}
				//SoundTable[i].used = 0;
				//free (SoundTable[i].name);
			}
			

		}
		fclose(OSSFILE);

	}
	//run the compressor on the level bank
	_spawnl (_P_WAIT, "p:\\sound\\MKBANK","P:\\SOUND\\MKBANK", OssFileName, NULL);

   //now make a .SFX file for internal directory
}



int OutputMap(char *strat)
{
	FILE *Map;

	FILE *stratfp;
	FILE *Script;
	short EntNum;
	int	  i,MapEntry=0,UsedAmount=0, RedoMap,error = 0, Unique;
	#if SH4
   		char convarg[64];
	#endif

	//WE ALWAYS REWAD UP THE MAP WE HAVE ASKED TO EXPORT
	//only need to rebuild wads for other maps in the .MDB if the strat list has changed,
	//the model list has changed or the anim list has changed
   //OR THE DEMO .LOG/.CAM OR .DSC FILES HAVE CHANGED


	if ((!FullRec) && (!TIMESTAMPFAIL) && (NotSpecifiedMap))
		RedoMap = 0;
	else
		RedoMap = 1;

	//IN ADDITION ENSURE TIMESTAMPS ARE KEPT FOR THE .LOG AND .CAM FILES
	//IF THESE CHANGE THEN THIS MAP WILL HAVE TO BE REDONE

	if (DemosChanged(strat))
		RedoMap	= 1;

	if (ScriptChanged(strat))
		RedoMap = 1;

	//else
	//	RedoMap	= 0;

   //without this things are fucked
   //	RedoMap = 1;
	if (Map=OpenMap(strat,RedoMap))
	{
		
		stratfp=OpenFile(strat,SCRIPT);
		Script=stratfp;
		stratfp=GetNextEntry(Script);

		do {
			MapEntry++;
			
			fclose (stratfp);
		} while (stratfp=GetNextEntry(Script));

		fclose(Map);
		fclose(Script);		
		printf("**** %d STRATS PLACED in %s map ****\n",MapEntry,strat+1);


		Unique = GetTotStrats();

		printf("**** %d UNIQUE STRATS used in %s map ****\n",Unique,strat+1);

	

	}


#if WAD
//	if (Map = OpenModelList(strat))

	if (Map = OpenMap(strat,RedoMap))
	{

		
		stratfp=OpenFile(strat,SCRIPT);
		Script=stratfp;
		stratfp=GetNextEntry(Script);
		fwrite(&MapEntry,sizeof(int),1,Map);
		#if DEBUG_OUT
			printf("output %d elements\n",MapEntry);
		#endif

		//GO THROUGH THE MAP UNTIL ENDSTRAT RECORD IS FOUND	
		do {

			EntNum=(short)GetStratEntry(currentstrat);
			#if DEBUG_OUT
				printf("strat %s has entry %d\n",currentstrat,EntNum);
			#endif
			fwrite(&EntNum,sizeof(short),1,Map);
		
			
			if (!GetParams(Script,Map,0))
				printf("FUCKED MAP TERMINATION\n");
			
			fclose (stratfp);
		} while (stratfp=GetNextEntry(Script));

		//NOW PARSE THE TRIGGERS OF THE MAP		
		
		GetParams(Script,Map,1);

		//NOW PARSE THE EVENTS OF THE MAP		
		
		GetParams(Script,Map,1);

		//NOW PARSE THE ACTIVATIONS OF THE MAP		
		
		GetParams(Script,Map,1);

		//NOW PARSE THE WAYPOINTS OF THE MAP		
		
		GetParams(Script,Map,1);
		
		//NOW PARSE THE AREAS OF THE MAP		
		
		GetParams(Script,Map,1);
		
		// Read and parse lights
		GetParams (Script, Map,1);
		
		// Read and parse cam mods
		GetParams (Script, Map,1);
		
//		fclose(Map);
		fclose(Script);
		
		UsedAmount=0;

	   	for (i=0;i<NumAnims;i++)
		{
			if (AnimTable[i].used)
				UsedAmount++;
		}	
		WadAnimList(strat,UsedAmount,Map,RedoMap);

		
		
		UsedAmount=0;

		for (i=0;i<NumMods;i++)
		{
			if (ModelTable[i].used)
				UsedAmount++;
		}

		printf("**** %d MODELS in %s map ****\n",UsedAmount,strat+1);

		#if	SH4
			error = ModelListBuild(strat,UsedAmount,RedoMap);
		#else
			WadModList(strat,UsedAmount,Map,RedoMap);
		#endif

		if (!error)
		{
		//PROCESS THE SOUNDS USED
		UsedAmount=0;
	   //	BaseSound = 0;

		for (i=0;i<NumSounds;i++)
		{
			if (SoundTable[i].used)
				UsedAmount++;
		}
		printf("**** %d SOUNDS in %s map ****\n",UsedAmount,strat+1);
	   
		//build the sound .oss for this level 
	//	if (RedoMap)
		ProcessSounds(strat);

		//TACK ANY RECORDED DEMO INFO ONTO END OF MAP
		ProcessRecorded(strat,Map,RedoMap);

		//PROCESS THE SCRIPT FILE
		ProcessScript(strat+1,Map,1);
		}

		fclose(Map);
		#if SH4
			//NOW CALL THE WADDER FOR THIS LEVEL
			if (RedoMap)
			{

				sprintf(convarg,"%s",strat+1);
	  		/*	_spawnl (_P_WAIT, "c:\\DREAMCAST\\REDDOG\\WADDER\\DEBUG\\wadder.exe","c:\\DREAMCAST\\REDDOG\\WADDER\\DEBUG\\wadder.exe", convarg, NULL);*/
			}
		#endif

	}
	else
	{
		CleanSound();
		CleanModel();
		CleanAnim();

	}


#else


	if (Map=OpenMap(strat,RedoMap))
	{
		
		stratfp=OpenFile(strat,SCRIPT);
		Script=stratfp;
		stratfp=GetNextEntry(Script);
		fwrite(&MapEntry,sizeof(int),1,Map);
		#if DEBUG_OUT
			printf("output %d elements\n",MapEntry);
		#endif

		//GO THROUGH THE MAP UNTIL ENDSTRAT RECORD IS FOUND	
		do {

			EntNum=(short)GetStratEntry(currentstrat);
			#if DEBUG_OUT
				printf("strat %s has entry %d\n",currentstrat,EntNum);
			#endif
			fwrite(&EntNum,sizeof(short),1,Map);
		
			
			if (!GetParams(Script,Map,0))
				printf("FUCKED MAP TERMINATION\n");
			
			fclose (stratfp);
		} while (stratfp=GetNextEntry(Script));

		//NOW PARSE THE TRIGGERS OF THE MAP		
		
		GetParams(Script,Map,1);

		//NOW PARSE THE EVENTS OF THE MAP		
		
		GetParams(Script,Map,1);

		//NOW PARSE THE ACTIVATIONS OF THE MAP		
		
		GetParams(Script,Map,1);

		//NOW PARSE THE WAYPOINTS OF THE MAP		
		
		GetParams(Script,Map,1);
		
		//NOW PARSE THE AREAS OF THE MAP		
		
		GetParams(Script,Map,1);
		
		// Read and parse lights
		GetParams (Script, Map,1);
		
		fclose(Map);
		fclose(Script);
	}


	UsedAmount=0;

	for (i=0;i<NumAnims;i++)
		if (AnimTable[i].used)
			UsedAmount++;


	if (stratfp = OpenAnimList(strat))
	{
		//OUTPUT THE NUMBER OF Anims
		fprintf(stratfp,"%d\n",UsedAmount);

		//NOW OUTPUT EACH ANIM
		//TO BE LOADED WITHIN THE GAME ENGINE
		
		for (i=0;i<NumAnims;i++)
		{
			if (AnimTable[i].used)
			{
				MapEntry = ValidAnim(i);
			
			if (MapEntry >=0 ) 
			{			
				#if DEBUGANIM
					printf("anim entnum %d \n",MapEntry);
				#endif
				fprintf(stratfp,"%d\n",MapEntry);
				fprintf(stratfp,"%s\n",AnimTable[i].Name);
				//OUTPUT THE ENTRY IN THE GLOBAL ANIM LIST
				#if ALLRESIDENT
					fprintf(stratfp,"%d\n",i);
				#endif
				//fridays addition			
				#if !ALLRESIDENT
					free(AnimTable[i].ModelName);
					free(AnimTable[i].Name);
				#endif
				#if DEBUG_ANIM
				printf("output %s\n",outstring);	
				#endif
			}
			else
			{
				printf("****** invalid model referenced %s\n",AnimTable[i].ModelName);
				free(AnimTable[i].ModelName);
				free(AnimTable[i].Name);
			}
							AnimTable[i].used = 0;				

			}
		}

		


		fclose(stratfp);
	}

	
	
	UsedAmount=0;

	for (i=0;i<NumMods;i++)
		if (ModelTable[i].used)
			UsedAmount++;

	printf("**** %d MODELS in %s map ****\n",UsedAmount,strat+1);



	if (stratfp = OpenModelList(strat))
	{
		//OUTPUT THE NUMBER OF MODELS
		fprintf(stratfp,"%d\n",UsedAmount);


		//NOW OUTPUT EACH MODEL
		//TO BE LOADED WITHIN THE GAME ENGINE
		
		for (i=0;i<NumMods;i++)
		{
			
			
			if (ModelTable[i].used)
			{
			
				//Clear(outstring,128);

				printf("MODEL %d %s\n",i+1,ModelTable[i].Name);
				fprintf(stratfp,"%s\n",ModelTable[i].Name);

				//OUTPUT THE ENTRY IN THE GLOBAL MODEL LIST
				#if ALLRESIDENT
					fprintf(stratfp,"%d\n",i);
				#endif
				
				ModelTable[i].used = 0;				

				//fridays addition
				#if !ALLRESIDENT
					free(ModelTable[i].Name);
				#endif
				#if DEBUG_OUT
					printf("output %s\n",outstring);	
				#endif
			}
		}

		


		fclose(stratfp);
	}
#endif
   return(error);

}

int GetTotStrats()
{
	int i,tot=0;
	for (i=0;i<DefinedStrats;i++)	
	{
		if (StratList[i].currentlevel)
		
			tot++;
	}
	return(tot);
}



void OutputStratTable(char *strat)
{
	int   i,count=0;
	int maxstrats;
	FILE *Table;
	char Index[128];
	

	if (SINGLEMAP)
		maxstrats = GetTotStrats();
	else
		maxstrats = DefinedStrats;


	if (Table=OpenTable(strat))
	{
		#if SH4
		   	sprintf(Index,"#include \"basicreddog.h\"\n");
		   	fputs(Index,Table);
			sprintf(Index,"#include \"levelst.h\"\n");
			fputs(Index,Table);
	   	#endif


		//fputs("#include EXT\n",Table);
		sprintf(Index,"%s%d%s\n","STRATENTRY STRATS[",maxstrats,"]= {");
		fputs (Index,Table);

		for (i=0;i<DefinedStrats;i++)
		{
			if ((SINGLEMAP) && (!StratList[i].currentlevel))
				continue;

		
			//printf("%d %s\n",i,StratList[i].addr);
			sprintf(Index,"%s%s%s\n","&",StratList[i].addr,",");
			fputs(Index,Table);
			sprintf(Index,"%d%s\n",StratList[i].floatnum,",");
			fputs(Index,Table);
			sprintf(Index,"%d%s\n",StratList[i].intnum,",");
			fputs(Index,Table);
		}

		fputs ("};",Table);
		fclose(Table);
	}
	#if SH4

	if (Table=OpenMakeFile(strat))
	{

		//fputs("#include EXT\n",Table);
		//if (DefinedStrats > 1)
		{
			sprintf(Index,"%s\t%s%s%s%s\n",
					"SFILES",
					"=",
					CPATH,
					strat+1,
					".c\\");
			fputs (Index,Table);


		
			for (i=0;i<DefinedStrats;i++)
			{
				if ((SINGLEMAP) && (!StratList[i].currentlevel))
					continue;

			   	if (count==(maxstrats-1))
			   //	if (i==(maxstrats-1))
					sprintf(Index,"\t%s%s%s\n",CPATH,StratList[i].name,".c");
				else
					sprintf(Index,"\t%s%s%s\n",CPATH,StratList[i].name,".c\\");
				
				if (i==255)
				{
				sprintf(Index,"%s\t%s%s%s%s\n",
					"SFILES2",
					"=",
					CPATH,
					StratList[i].name,
					".c\\");
				}

				//last before sfiles2 define
				if (i==254)
				{
				sprintf(Index,"\t%s%s%s\n",
					CPATH,
					StratList[i].name,
					".c");
				}	fputs(Index,Table);
				count++;
			}
		}
		/*else
		{
			sprintf(Index,"%s\t%s%s%s\n",
					"SFILES",
					"=",
					StratList[0].addr,
					".c");
			fputs (Index,Table);
		} */

		fclose(Table);
	}

	#endif


}


void OpenAndSolveStrat(char *IncStrat, int Mode);

enum {CREATETYPE,BOIDCREATETYPE,OBJECTTYPE,CREAINDTYPE,ANIMTYPE,MISSFIRETYPE,DESTROYMETYPE,FASTCREATETYPE,SOUNDTYPE};

void SolveStratInc(FILE *stratfile,int Mode)
{
	char line[MAX_LINE];
	char *linest=line,*origline;
	char IncStrat[64];
	char IncSound[64];
	int diff;
	int TYPE;

	for (;;)
	{
		if(fgets(line,MAX_LINE,stratfile) == NULL)
		{
		   //	fclose(stratfile);
			break;			

		}
		else
		{
			linest=WhiteSpaceRead(line);
			if (linest)
			{
				if ((strnicmp(linest,"//",2)) && (strnicmp(linest,"/*",2)))
				{
					origline = linest;
				  	TYPE = -1;
					if(!strnicmp("CREATE",linest,strlen("CREATE")))
						TYPE = CREATETYPE;
					if(!strnicmp("BOIDCREATE",linest,strlen("BOIDCREATE")))
						TYPE = BOIDCREATETYPE;
					if(!strnicmp("OBJECT>",linest,strlen("OBJECT>")))
						TYPE = OBJECTTYPE;
					if(!strnicmp("CREAIND",linest,strlen("CREAIND")))
						TYPE = CREAINDTYPE;
					if(!strnicmp("ANIM>",linest,strlen("ANIM>")))
						TYPE = ANIMTYPE;
					if(!strnicmp("MISSFIRE",linest,strlen("MISSFIRE")))
						TYPE = MISSFIRETYPE;
					if(!strnicmp("DESTROYME",linest,strlen("DESTROYME")))
						TYPE = DESTROYMETYPE;
					if(!strnicmp("FASTCREATE",linest,strlen("FASTCREATE")))
						TYPE = FASTCREATETYPE;
					if(!strnicmp("PLAYSOUND>",linest,strlen("PLAYSOUND>")))
						TYPE = SOUNDTYPE;

		 			switch (TYPE)
					{
						case (CREATETYPE):
						{
							linest += strlen("CREATE");
							linest=WhiteSpaceRead(linest);
							Clear(IncStrat,64);
							ReadToEnd(IncStrat,linest);
										
							if (
								(strnicmp(CurrentStrat,IncStrat,strlen(CurrentStrat)))
								 ||
								(strnicmp(CurrentStrat,IncStrat,strlen(IncStrat)))
								)
								diff = 1;
							else
								diff = 0;
	

							if ( 
								(diff)&& 
							  	((!CheckStrat(IncStrat,Mode)) || (SolvedCurrent && !SINGLEMAP)))
							//	((!CheckStrat(IncStrat,Mode)) || (SolvedCurrent)))
									OpenAndSolveStrat(IncStrat,Mode);
						   	#if DEBUG_STRATDUPS
								else
							 	printf("	already solved %s\n",IncStrat);
							#endif


							break;
						}
						
						case (BOIDCREATETYPE):
						{
							linest += strlen("BOIDCREATE");
							linest=WhiteSpaceRead(linest);
							Clear(IncStrat,64);
							ReadToEnd(IncStrat,linest);

						 	if ((!CheckStrat(IncStrat,Mode)) || (SolvedCurrent && !SINGLEMAP))
						//	if ((!CheckStrat(IncStrat,Mode)) || (SolvedCurrent))
								OpenAndSolveStrat(IncStrat,Mode);
							#if DEBUG_STRATDUPS
							else
								printf("	already solved %s\n",IncStrat);
							#endif
								
							break;
						}
					
					  	case (OBJECTTYPE):
						{
						   	linest += strlen("OBJECT>");
						   	linest=WhiteSpaceRead(linest);
						   	#if DEBUG_OUT
						   		printf("object %s\n",linest);
						   		printf("entry %d\n",ProcessModel(linest));
						   	#endif
						   	ProcessModel(linest);
							break;
						}

					  	case (CREAINDTYPE):
						{
						 	linest += strlen("CREAIND");
						 	linest=WhiteSpaceRead(linest);
						 	Clear(IncStrat,64);
						 	ReadToEnd(IncStrat,linest);

							if (
								(strnicmp(CurrentStrat,IncStrat,strlen(CurrentStrat)))
								 ||
								(strnicmp(CurrentStrat,IncStrat,strlen(IncStrat)))
								)
								diff = 1;
					   		else
					   			diff = 0;
	

							if ( 
								(diff) && 
							  	((!CheckStrat(IncStrat,Mode)) || (SolvedCurrent && !SINGLEMAP)))
						//		((!CheckStrat(IncStrat,Mode)) || (SolvedCurrent)))
								OpenAndSolveStrat(IncStrat,Mode);
						   	break;
						}
					  	case (ANIMTYPE):
						{
						  	linest += strlen("ANIM>");
						  	linest=WhiteSpaceRead(linest);
						  	#if DEBUG_ANIM
						  		printf("ANIM %s\n",linest);
						  		printf("ANIM ENTRY%d\n",Player(linest));
						  	#endif
						  	ProcessAnim(linest);
							break;
						}
					  	case (MISSFIRETYPE):
						{
						  	linest += strlen("MISSFIRE");
						  	linest=WhiteSpaceRead(linest);
						  	Clear(IncStrat,64);
						  	ReadToEnd(IncStrat,linest);
												
						  	if ((!CheckStrat(IncStrat,Mode)) || (SolvedCurrent && !SINGLEMAP))
						//  	if ((!CheckStrat(IncStrat,Mode)) || (SolvedCurrent))
						  		OpenAndSolveStrat(IncStrat,Mode);
						  	#if DEBUG_STRATDUPS
						  	else
						  		printf("	already solved %s\n",IncStrat);
						  	#endif
							break;
						}
					  	case (DESTROYMETYPE):
						{
							linest += strlen("DESTROYME");
							linest=WhiteSpaceRead(linest);
							Clear(IncStrat,64);
							ReadToEnd(IncStrat,linest);
												
						   	if ((!CheckStrat(IncStrat,Mode)) || (SolvedCurrent && !SINGLEMAP))
						//	if ((!CheckStrat(IncStrat,Mode)) || (SolvedCurrent))
								OpenAndSolveStrat(IncStrat,Mode);
							#if DEBUG_STRATDUPS
							else
								printf("	already solved %s\n",IncStrat);
							#endif
							break;
						}
					  	case (FASTCREATETYPE):
						{
						   	linest += strlen("CREATEFAST");
						   	linest=WhiteSpaceRead(linest);
						   	Clear(IncStrat,64);
						   	ReadToEnd(IncStrat,linest);
							
						   	if ((!CheckStrat(IncStrat,Mode)) || (SolvedCurrent && !SINGLEMAP))
						//   	if ((!CheckStrat(IncStrat,Mode)) || (SolvedCurrent))
								OpenAndSolveStrat(IncStrat,Mode);
							#if DEBUG_STRATDUPS
							else
								printf("	already solved %s\n",IncStrat);
							#endif

							break;
						}
					  	case (SOUNDTYPE):
						{
						   	linest += strlen("PLAYSOUND>");
						   	linest=WhiteSpaceRead(linest);
							if (linest)
							{
							 	ReadToEnd(IncSound,linest);	
								ProcessSound(IncSound);


							}
							else
							{
								printf("missing SOUNDFILE details in  %s %s\n",CurrentStrat,origline);
								KEYBREAK();
						   
							}



						 	#if DEBUG_OUT
						   		printf("object %s\n",linest);
						   		printf("entry %d\n",ProcessModel(linest));
						   	#endif
							break;
						}

						default:
							break;
					}
				}
			}
		}
	}
}

void OpenAndSolveStrat(char *IncStrat, int Mode)
{
 	char OpenFile[128];
	FILE *fp;

	Clear(CurrentStrat,128);
	strcpy(CurrentStrat,IncStrat);

	Clear(OpenFile,128);
	sprintf(OpenFile,"%s%s%s",STRATDIR,IncStrat,".DST");
	fp=fopen(OpenFile,"ra");
	if (fp)
	{
		//UTDREM
		TimeStampCheck(OpenFile,IncStrat,SKIP);
		#if SOLVE_DEBUG
			printf("	solving strat %s\n",OpenFile);			
		#endif
	   	CleanStratOpen(0);
		SolveStratInc(fp,Mode);
		CleanStratOpen(1);
		fclose(fp);
	}
	else
   		printf(" \n **** PROBLEM OPENING %s MAY NOT YET EXIST \n\n",OpenFile);
}


void WriteStratTable(FILE *strat)
{
	int i,len;
	int maxstrats;

	if (SINGLEMAP)
		maxstrats = GetTotStrats();
	else
		maxstrats = DefinedStrats;


	fwrite(&maxstrats,sizeof(int),1,strat);

	for (i=0;i<DefinedStrats;i++)
	{
		if ((SINGLEMAP) && (!StratList[i].currentlevel))
			continue;

		len = strlen(StratList[i].name)+1;
		#if DEP_DEBUG
			printf("written %s size %d\n",StratList[i].name,len);
		#endif
		fwrite(&len,sizeof(int),1,strat);
//		fwrite(&StratList[i],sizeof(STRATFIELD),1,strat);	
		fwrite(StratList[i].name,sizeof(char),len,strat);	
	}
}

int NewStrat(char *strat, int loop)
{	int newstrat =1;
	int i;
	
	for (i=0;i<DefinedStrats;i++)
	{
		//TREBLE TROPHY ADDED THE SECOND && CHECK
		 if ((!(strnicmp(strat,StratList[i].name,strlen(StratList[i].name)+1))) && (loop == i))
		 {
			 newstrat = 0;
			 break;
		 }
	}

	return(newstrat);

}
void CompareStrat(FILE *strat)
{
	int defined,i,mem,maxstrats;
	STRATFIELD field;


	if (SINGLEMAP)
		maxstrats = GetTotStrats();
	else
		maxstrats = DefinedStrats;
	
	fread(&defined,sizeof(int),1,strat);
   //	printf("defined %d\n",defined);
   
	printf("\tTOTAL STRATS IN DATABASE %d\n",maxstrats);
	
	  //OLE GUNNAR REPLACED THE LINE BELOW WITH THE LINE BELOW THAT*/
  	//if ((defined != maxstrats) || (defined==-1))
   	 if ((defined != maxstrats))

  		FullRec ++;
	else
	{
		#if DEP_DEBUG
			printf("same again please, further checks needed\n");
		#endif

		for (i=0;i<defined;i++)
		{

			fread(&mem,sizeof(int),1,strat);


			field.name=(char*)malloc(mem);

			fread(field.name,sizeof(char),mem,strat);

			#if DEP_DEBUG
				printf("prev field name %s\n",field.name);
				printf("new field name %s\n",StratList[i].name);
			#endif
		  //TREBLE TROPHY 
		  //	 if (!((SINGLEMAP) && (!StratList[i].currentlevel)))
			 	 if (NewStrat(field.name,i))
					FullRec++;

				 
			  //	 if ((strncmp(field.name,StratList[i].name,strlen(StratList[i].name)+1)))
			  //		FullRec++;

			free (field.name);
		}	
	}
}


//ADDED ON SAT NOV 14TH '98
//	SINGLEMAP MODE - returns 1 if the last build = this build
//	FULL MAPS  - returns 1 for further checking

int LastBuildCheck(char *level)
{
	char	StratFile[128];
	FILE *stratfp;
   	char   *ReadName;
	int res = 0;
	int len;


	if (!SINGLEMAP)
		return(1);

	sprintf(StratFile,"%s%s%s",INTDIR,"GAME",".INT");	
											
	if (stratfp=fopen(StratFile, "rb"))
	{
		fread(&len,sizeof(int),1,stratfp);
		ReadName = malloc(sizeof(char) * len);
		fread(ReadName,sizeof(char),len,stratfp);
		if (!(strnicmp(ReadName,level,strlen(level)+1)))
			res = 1;

		free(ReadName);



		stratfp = fopen(StratFile, "wb");
		len = strlen(level)+1;
		fwrite(&len,sizeof(int),1,stratfp);
	 	fwrite(level,sizeof(char),len,stratfp);

	}
	else
	{
		stratfp = fopen(StratFile, "wb");
		len = strlen(level)+1;
		fwrite(&len,sizeof(int),1,stratfp);
	 	fwrite(level,sizeof(char),len,stratfp);

	}
	fclose(stratfp);
	return(res);


}


void	FatalRec(char *strat)
{
	int		fatval=-1;
	char	StratFile[128];
	FILE *	stratfp;

	/*OLE GUNNAR*/
	return;

//	printf("fatal here\n");

	sprintf(StratFile,"%s%s%s",INTDIR,strat+1,".INT");	

	//printf("%s\n",StratFile);
	if (stratfp=fopen(StratFile, "wb"))
	{
		fwrite(&fatval,sizeof(int),1,stratfp);
		fclose(stratfp);
	}
	
}

void InvalidateMDB(char *currentlevel)
{	char DBFile[MAX_LINE];
	char toss[MAX_LINE];
	char StratFile[MAX_LINE];
//	MAPFILE Maps[32];
	FILE *fp,*file;
	int defined=-1;
	int nummaps,loop,size;

	#if ARTIST
		sprintf(DBFile,"%s%s%s",INTDIR,"GAMEMAPS",".MDB");	
	#else
		sprintf(DBFile,"%s%s%s",STRATDIR,"GAMEMAPS",".MDB");	
	#endif	
		
	if (fp=fopen(DBFile, "rb"))
	{
		fread(&nummaps,sizeof(int),1,fp);

		for (loop=0;loop<nummaps;loop++)
		{

			fread(&size,sizeof(int),1,fp);
			fread(&toss,sizeof(char),size,fp);

			
			//IF MAP NAME NOT THE SAME AS CURRENT LEVEL THEN INVALIDATE IT

			if (strnicmp(toss,currentlevel,strlen(currentlevel)))
			{
				sprintf(StratFile,"%s%s%s",INTDIR,toss,".INT");	
				if (file=fopen(StratFile, "wb"))
				{
					fwrite(&defined,sizeof(int),1,file);
					fclose(file);
				}
			}
		}
		fclose(fp);
	}

}



	char		toss[MAX_LINE];
int StratCollate(char *strat)
{
	char		StratFile[128];
	FILE		*stratfp;
	int         Mode=SCRIPT;
	int   		parse;
	int			quit;
	FILE		*Script;
	int			SolveMode=1;
#if ALLRESIDENT
	FILE        *DB;
	char		DBFile[MAX_LINE];
	char		CurrentLevel[MAX_LINE];
	int			NumMaps;
	int			size;
	int			loop;
#endif


	if ((Mode == SCRIPT) && (!PROCESSED))
		if (CheckMapDB(strat))
			exit(0);

	
#if ALLRESIDENT

	
	strncpy(CurrentLevel,strat,strlen(strat)+1);	
	
	#if ARTIST
		sprintf(DBFile,"%s%s%s",INTDIR,"GAMEMAPS",".MDB");	
	#else
		sprintf(DBFile,"%s%s%s",STRATDIR,"GAMEMAPS",".MDB");	
	#endif
	DB=fopen(DBFile, "rb");
	fread(&NumMaps,sizeof(int),1,DB);
  //	printf("NUM MAPS IN GAME DBASE %d\n",NumMaps);
	printf("******PROCESSING MAP  %s IN GAME DBASE, (%d ENTRIES)******\n",CurrentLevel+1,NumMaps);
	
	for (loop=0;loop<NumMaps;loop++)
	{
	
		fread(&size,sizeof(int),1,DB);
		fread(toss,sizeof(char),size,DB);

		//WE ONLY WISH TO BUILD MODEL AND ANIM LISTS RELEVANT TO THE CURRENTLY
		//SPECIFIED OUTPUT LEVEL, SO THIS IS THE BIT THAT FACILITATES IT

		//MIGHTYDWIGHTYORKE
	   	if ((SINGLEMAP) && (strnicmp(toss,CurrentLevel+1,strlen(CurrentLevel))))
	   		continue;
		if (!(strnicmp(toss,CurrentLevel+1,strlen(CurrentLevel))))
		{
			SolveMode = 1;
			SolvedCurrent = 1;
		}
		else
		{
			SolveMode = 0;
			SolvedCurrent = 0;
		}
#endif
	

#if ALLRESIDENT	
	
	if (!(stratfp=OpenFile(toss,RESIDENT)))
		return(0);
#else
	if (!(stratfp=OpenFile(strat,Mode)))
		return(0);
#endif

	Script=stratfp;
	stratfp=GetNextEntry(Script);
	currentstrat = currentstratfile;

	
	if (!stratfp)
		return(0);

	quit=0;

	parse=0;
	fatal=0;


//	printf("**PARSING MAP %s **\n",toss);
			

	//fridays addition
	#if ALLRESIDENT
			if ((!PROCESSED) || (SolveMode))
		//	if ((!PROCESSED))
			{
	
			//	printf("**PARSING MAP %s **\n",toss);
	#endif
		CleanStratOpen(0);
	do
	{
		CheckStrat(currentstrat,SolveMode);


		SolveStratInc(stratfp,SolveMode);
		fclose(stratfp);
	}while (stratfp=GetNextEntry(Script));

		CleanStratOpen(1);
	//fridays addition
	#if ALLRESIDENT
		}
	#endif

#if	ALLRESIDENT
	fclose(Script);	
	#if DEBUG_INT
		for (i=0;i<DefinedStrats;i++)
			printf("Internal Strat List %s solved %d\n",StratList[i].name,StratList[i].currentlevel);
	#endif
	}
	fclose(DB);
#else
	fclose(Script);	

#endif


	#if DEBUG_INT
		for (i=0;i<DefinedStrats;i++)
			printf("Internal Strat List %s solved %d\n",StratList[i].name,StratList[i].currentlevel);
	#endif	

	

	//WE HAVE COLLATED AN INTERNAL LIST OF STRATS USED IN THE MAP
	//WE NOW CHECK WHETHER IT IS THE SAME AS BEFORE
	//IF SO WE DON'T NEED TO RECOMPILE THE STRATS ONLY UPDATE THE
	//MAPS

	sprintf(StratFile,"%s%s%s",INTDIR,strat+1,".INT");	
	//  TRIPLE
   //	FullRec=0;

	// FIRST CHECK THE LAST BUILD AGAINST THIS BUILD 
	// IF THE NAMES THE SAME THEN FURTHER CHECKS ARE NEEDED
	// OTHERWISE INVALIDATE


	if (!(LastBuildCheck(StratFile)))
	{
		FullRec = 1;
		WADRERUN = 1;
	//	return(1);
	}	
	//  	if (!strnicmp(strat+1,CurrentLevel+1,strlen(CurrentLevel)))
	//	{
	if (stratfp=fopen(StratFile, "rb"))
	{
	   	#if DEBUG_DEP
			printf("STRAT FILE ALREADY %s\n",StratFile);
	   	#endif
		CompareStrat(stratfp);
		fclose(stratfp);
   //TREBLES - REMOVED THIS		
	//	if ((SINGLE) ||	(!strnicmp(CurrentLevel+1,suppname+1,strlen(CurrentLevel))))
	   	//TREBLES ADDED THIS
		if ((SINGLEMAP) &&		(!strnicmp(CurrentLevel+1,suppname+1,strlen(CurrentLevel))))
		{
			InvalidateMDB(CurrentLevel+1);
	 
			
			stratfp = fopen(StratFile, "wb");
	   	#if DEBUG_DEP
	  		printf("STRAT FILE UPDATED %s\n",StratFile);
	   	#endif
	  	WriteStratTable(stratfp);
	  	fclose(stratfp);
		
		}	
		else
		{
	 //		{
		
	  	//if 
			//TREBLES BLOCK START
	  	stratfp = fopen(StratFile, "wb");
	   	#if DEBUG_DEP
	  		printf("STRAT FILE UPDATED %s\n",StratFile);
	   	#endif
	  	WriteStratTable(stratfp);
	  	fclose(stratfp);
		//TREBLES BLOCK END
	 	}
	}
	else
	{
		stratfp = fopen(StratFile, "wb");
		#if DEBUG_DEP
			printf("STRAT FILE CREATED %s\n",StratFile);
		#endif
		WriteStratTable(stratfp);
		fclose(stratfp);
		FullRec=1;
	}	

  //		}
	//SEE IF THE INTERNAL MODEL LIST HAS CHANGED	
	CheckModelDep(strat);

	//SEE IF THE INTERNAL SOUND LIST HAS CHANGED	
	CheckSoundDep(strat);

	//SEE IF THE INTERNAL ANIM LIST HAS CHANGED	
   //	CheckAnimDep(strat);
	return(1);
}	

int IgnoreStratRecomp(char *filename,FILE *stratfile)
{
	char StratFile[128];
	int TEMP;

 //	return(0);
	//IF A FULL RECOMPILATION IS NEEDED, IE: NEW MODELS/STRATS THEN RETURN FALSE
	//FERGIEMAGNIFICO
	if (FullRec)
	{
		TEMP = TIMESTAMPFAIL ;
	 	sprintf(StratFile,"%s%s%s",STRATDIR,filename,".DST");	

		TimeStampCheck(StratFile, filename,UPDATE);

		//	fclose(stratfile);
		TIMESTAMPFAIL = TEMP;
		return(0);
	}
	
	//IF TIME STAMPS HAVE PASSED THEN RETURN FALSE
	if (!TIMESTAMPFAIL)
	{
	   //	fclose(stratfile);
		 return(0);
	}

  	sprintf(StratFile,"%s%s%s",STRATDIR,filename,".DST");	

	TEMP = TIMESTAMPFAIL ;
	TIMESTAMPFAIL = 0;
	TimeStampCheck(StratFile, filename,UPDATE);
	if (!TIMESTAMPFAIL)
	{
	//	fclose(stratfile);
		TIMESTAMPFAIL = TEMP;
		return(1);

	}

	TIMESTAMPFAIL = TEMP;

	return(0);

}


int IGNORE=0;

//outputs the status of any build terminated with errors

void LogErrorFile(int fail)
{
	FILE	*errfp;
	char	logscript[128];	

	sprintf(logscript,"%s%s%s",INTDIR,"ERROR",".LOG");
	errfp = fopen(logscript,"wb");
	if (errfp)
	{
		fwrite(&fail,sizeof(int),1,errfp);
		fwrite(&FullRec,sizeof(int),1,errfp);
		fwrite(&WADRERUN,sizeof(int),1,errfp);
		fwrite(&TIMESTAMPFAIL,sizeof(int),1,errfp);
		fclose(errfp);
	}
}


//reads the status of the last build, and if it was a failure then reads in the corresponding
//build control vars

void ReadLogErrors()
{
	int		fail;
	FILE	*errfp;
	char	logscript[128];	

	sprintf(logscript,"%s%s%s",INTDIR,"ERROR",".LOG");
	errfp = fopen(logscript,"rb");
	if (errfp)
	{
		fread(&fail,sizeof(int),1,errfp);
		if (fail)
		{
			fread(&FullRec,sizeof(int),1,errfp);
			fread(&WADRERUN,sizeof(int),1,errfp);
			fread(&TIMESTAMPFAIL,sizeof(int),1,errfp);
		}
		fclose(errfp);
	}

}


//Conversion takes an input script *.scr, or a single strat file .*.dst
//The former will be parsed and all referenced strats collated into an internal table
//The strats within that table will then be compiled 

int StratConv(char *strat)
{
	FILE *		stratfp;
//	FILE *		Script;
	FILE *		output;
	int         Mode=SINGLE;
	int   		linenum,parse;
	int   		ScriptOn;
	int			quit,Comment;
	int   		i,token;
	char 	   	line[MAX_LINE];
	#if SH4
		char		headername[64];
	#endif
	char 			*line_tail;
	char 			*origin,*o;
	SCRIPTLINE 	*ScriptStart;	


   //	if (!SINGLEMAP)
   //	{
		if (!(strncmp(strat,"@",1)))
		{
			Mode=SCRIPT;	
			printf("\n");
			//	printf("OPENING MAP\n\n");
			if (!StratCollate(strat))
				return(0);

		}	
   //	}	
	
//	FullRec =1;
	/*	TIMESTAMPFAIL = 1;	*/
	if ((!FullRec) && (!TIMESTAMPFAIL))
		printf("\nNO STRAT RECOMP NEEDED\n");


	 //DO WE NEED TO FULLY RECOMPILE THE STRATS ?


	if ((FullRec) || (TIMESTAMPFAIL))	
	{


		//PROCESSED USED TO ONLY COMPILE THE STRATS ONCE
		if (!PROCESSED)
		{


			InternalGrab = 0;

			if (Mode==SCRIPT)
			{
			//	Script=stratfp;
				ScriptOn=1;
				stratfp=GetNextEntryInt();
				currentstrat = currentstratfile;	
			}
			else
			{
				stratfp=OpenFile(strat,Mode);
				currentstrat = strat;
				ScriptOn=0;
			}

			if (!stratfp)
				return(0);
	
			//BIZARRE 
		   	if ((FullRec) || (TIMESTAMPFAIL))
		   	{

				#if ALLRESIDENT
					sprintf(HEADER,"%s%s",HEADERDIR,"LEVELSTRATS.H");	
				#else
					sprintf(HEADER,"%s%s%s",HEADERDIR,strat+1,"ST.H");	
				#endif
				Header=fopen(HEADER, "wa");

				if (!Header)
					printf("fatal%s",HEADER);

				HeaderStart(Header);


				#if ALLRESIDENT
					sprintf(CINCLUDE,"%s%s",SCRIPTDIR,"LEVELSTRATS.C");	
					//		printf("opening for writing %s on level %s\n",CINCLUDE,strat);
				#else
					sprintf(CINCLUDE,"%s%s%s",SCRIPTDIR,strat+1,"ST.C");	
				#endif
				Cinclude=fopen(CINCLUDE, "wa");
				IncludeStart(Cinclude,strat);
		   	}
			IGNORE=0;

			do
			{
				IGNORE = 
				IgnoreStratRecomp(currentstrat,stratfp);
				{
				 //	continue;

				printf("PROCESSING %s\n",currentstrat);

				ResetVars();

				o=origin=errorlog=malloc(32768);

				ScriptStart = InitScript();

				quit=0;

				parse=0;
				fatal=0;

				for (;;)
				{

					linenum=0;

					for (;;)
					{
						/* Pull next line and look at the front to see what it contains */

						if(fgets(line,MAX_LINE,stratfp) == NULL)
							break;			/* EOF */
						else
						{
							token = NOP;
	
							line_tail=WhiteSpaceRead(line);

							if (line_tail)
							{		
								Comment=0;

								for(i=0; i< ASIZE(Keywords); i++)
								{

									if ((strnicmp(line_tail,"//",2)) && (strnicmp(line_tail,"/*",2)))
									{
										if(!strnicmp(Keywords[i].word,line_tail,Keywords[i].length))
										{
											token = Keywords[i].token;
											line_tail += Keywords[i].length;
											break;
										}
									}
									else
									{
										Comment++;
										break;
									}
								}

								if ((token==NOP) && (!Comment))
									token=EVALUATION;
							}

							if (Interpret(parse,token,line_tail,line,linenum))
								break;
						}
						linenum++;
					}

					if ((fatal) || (parse==1) || (StateUnBalanced(line_tail,linenum)))
						break;
					parse++;
				
					//SEEK BACK
					fseek(stratfp,0,0);
				}	


#if LOGWRITEON
				if (output=OpenLog(currentstrat))
				do
				{
					fputc((*origin),output);
					origin++;
				}  while ((*origin) );
				fclose(output);
#endif
				if (Error)
				{
					CleanScript(ScriptStart->next);
				  	//OLE GUNNAR ADDED BELOW
					DeleteTimeStamp(currentstrat);
					LogErrorFile(1);
					printf("TERMINATED WITH %d ERRORS\n",Error);
					fatal ++;
				}
				else
				{
					if (!IGNORE)
					{
	 
						//what was being done before
						if (output=OpenScript(currentstrat))
						{
							printf("\n\t*** RECOMPILING STRAT %s\n",currentstrat);
			
						   
							#if SH4
								sprintf(headername,"#include \"basicreddog.h\"\n");
								fputs(headername,output);
								sprintf(headername,"#include \"c:\\strats\\headers\\%s%s\n",currentstrat,".h\"");
								fputs(headername,output);
							#endif
							 //OLE
							WriteScript(output,ScriptStart->next);
					  		
							if (output=OpenHeader(currentstrat))
								WriteHeader(output);
							else
								printf("WRITE ERROR ON OUTPUT OF SCRIPT HEADER %s\n",currentstrat);
						}
						else
							printf("WRITE ERROR ON OUTPUT OF C SCRIPT %s\n",currentstrat);
					}
				  	else
					{
						if (output=OpenHeader(currentstrat))
							WriteHeader(output);
						else
							printf("WRITE ERROR ON OUTPUT OF SCRIPT HEADER %s\n",currentstrat);

					}
				   		
				}

				free (ScriptStart->text);
				free (ScriptStart);
	
				for (i=0;i<Label;i++)
				{
					free (statename[i].state);
					statename[i].state = NULL;
				}
				for (i=0;i<Defined;i++)
				{
					free (Defs[i].definition);
					Defs[i].definition = NULL;
					free (Defs[i].value);
					Defs[i].value = NULL;
				}
 
				SetStratTableEntry(currentstrat);
		
				for (i=0;i<IVarNum;i++)
				{
					free (IVars[i].definition);
					IVars[i].definition = NULL;
					free (IVars[i].value);
					IVars[i].value = NULL;
				}
  
				for (i=0;i<FVarNum;i++)
				{
					free (FVars[i].definition);
					FVars[i].definition = NULL;
					free (FVars[i].value);
					FVars[i].value = NULL;
				}
  
				free(o);

				fclose(stratfp);
				//fclose(output);
				}
			} while ((stratfp=GetNextEntryInt()) && ScriptOn && (!fatal));

			//BIZARRE 
		   	if ((FullRec) || (TIMESTAMPFAIL))
		   	{
				OutputStratTable(strat);
				fclose(Cinclude);
				HeaderEnd(Header);
				fclose(Header);
		   	}
		}
	}

	PROCESSED = 1;
	if (!fatal)
		fatal = OutputMap(strat);



	//fridays addition
	#if !ALLRESIDENT	
		for (i=0;i<DefinedStrats;i++)
		{
			free (StratList[i].addr);
			StratList[i].addr = NULL;
			free (StratList[i].name);
			StratList[i].name = NULL;
		}
	#endif


	if (fatal)
	{
		FatalRec(strat);
		return(0);
	}

	SolvedCurrent = 0;
	InternalGrab = 0;


	LogErrorFile(0);

	
	return(1);

}



int Interpret(int ParseMode,int token,char *line_tail,char*line,int linenum)
{
	int end=0;
	int RefState=0;

	switch (ParseMode)
	{
		case	FIRSTPASS:
		{

			switch (token)
			{
				case	STATEDEFINE:
					if (line_tail=LeadingSpace(line_tail,line))
					{
						if (!(line_tail = WhiteSpaceRead(line_tail)))
							error(0,line_tail,linenum);
						else
						{
							if (StateUnBalanced(line_tail,linenum))
							{
								fatal++;
								end++;
							}
							
							

							if (!StateLabelParse(line_tail))
								error(0,line_tail,linenum);
							else
							{
									StateDefine();
									NestLevel=0;
							}

						}
					}
					else
						error(0,line_tail,linenum);
					break;

				case DEFINE:
					if (!(AddDefine(line_tail)))
						error(11,line_tail,linenum);
					break;

				case LOCALINT:
					if (!(AddVar(line_tail,LOCALINT)))
						error(11,line_tail,linenum);
					else
					   anyvar++;
					break;

				case LOCALFLOAT:
					if (!(AddVar(line_tail,LOCALFLOAT)))
						error(11,line_tail,linenum);
					else
					   anyvar++;
					break;


				case PARAMINT:
					//INT PARAMS MUST BE MADE BEFORE ANY OTHER VARIABLE TYPE
					if (anyvar || floatvar)
						error(18,line_tail,linenum);
					else
					{
						if (!(AddVar(line_tail,PARAMINT)))
							error(11,line_tail,linenum);
					}
					break;

				case PARAMFLOAT:
					if (anyvar)
						error(19,line_tail,linenum);
					else
					{
						if (!(AddVar(line_tail,PARAMFLOAT)))
							error(11,line_tail,linenum);
						else
						   floatvar++;
					}
					break;


				case LOOPIMM:
				case LOOP :
					if (!(line_tail = WhiteSpaceRead(line_tail)))
							error(4,line_tail,linenum);
					else

						/*** Ensure no more than sensible levels of nesting **/

						if (NestLevel == 3)
						{
							error(7,line_tail,linenum);
							fatal++;
							end++;
						}
						else
							NestLevel++;
					break;

				case ENDIMMLOOP:
				case ENDLOOP:
					/*** Ensure the endloop balances with a loop open **/

					if (NestLevel)
						NestLevel--;
					else
					{
						error(6,line_tail,linenum);
						fatal++;
						end++;

					}
					break;

				case IF:
					IfLevel++;
					break;

				case ENDIF:
					if (IfLevel)
					{
						IfLevel--;

						if (ElseLevel)				
							ElseLevel--;	

					}
					else
					{
						error(13,line_tail,linenum);
						fatal++;
						end++;

					}
					break;

				case ELSE:
					break;

				case WHILE:
					WhileLevel++;
					break;

				case ENDWHILE:
					if (WhileLevel)
						WhileLevel--;
					else
					{
						error(10,line_tail,linenum);
						fatal++;
						end++;

					}
					break;



				case END:
					
					
					end++;
					break;

				default:
					break;

			}
			break;
		}


		case SECONDPASS:	
		{		
			switch (token)
			{
				//SECOND PASS THROUGH SET THE STATE WHEN HIT

				case ENDSTATE:
					break;

				case CREATE:
					ParseSpawn(line_tail,CHILD);
					break;
				case CREATEFAST:
					ParseSpawn(line_tail,CHILDFAST);
					break;
				case DESTROYME:
					ParseExplode(line_tail);
					break;

				case FIRE:
					ParseSpawn(line_tail,MFIRE);
					break;

				case CREATEIND:
					ParseSpawn(line_tail,IND);
					break;

				case BOIDCREATE:
					ParseSpawn(line_tail,BOID);
					break;



				// HITTING THE STATE HEADER BLOCK
				case  STATEDEFINE:
					line_tail = WhiteSpaceRead(line_tail);
					SetState(line_tail);
					break;


				// HITTING THE STATE CHANGE COMMAND

				case	STATECHANGE:
					if (!(line_tail = WhiteSpaceRead(line_tail)))
							error(1,line_tail,linenum);
					else
					{
						if (!(RefState=CheckLabel(line_tail)))
							error(2,line_tail,linenum);
						else
							InsertCondition(RefState-1);

					}
					break;
					
				case	TRIGON:
					if (!(line_tail = WhiteSpaceRead(line_tail)))
							error(1,line_tail,linenum);
					else
					{
						if (!(RefState=CheckLabel(line_tail)))
							error(2,line_tail,linenum);
						else
							InsertTrigger(RefState-1,line_tail);

					}
					break;
					
				case	TRIGOFF:
					if (!(line_tail = WhiteSpaceRead(line_tail)))
							error(1,line_tail,linenum);
					else
					{
						if (!(RefState=CheckLabel(line_tail)))
							error(2,line_tail,linenum);
						else
							RemoveTrigger(RefState-1,line_tail);

					}
					break;
					
				case	TRIGHOLD:
					if (!(line_tail = WhiteSpaceRead(line_tail)))
							error(1,line_tail,linenum);
					else
					{
						if (!(RefState=CheckLabel(line_tail)))
							error(2,line_tail,linenum);
						else
							HoldTrigger(RefState-1,line_tail);

					}
					break;
					
				case	TRIGFREE:
					if (!(line_tail = WhiteSpaceRead(line_tail)))
							error(1,line_tail,linenum);
					else
					{
						if (!(RefState=CheckLabel(line_tail)))
							error(2,line_tail,linenum);
						else
							ReleaseTrigger(RefState-1,line_tail);

					}
					break;
					
				case	TRIGEND:
					EndTrigger();
					break;
					
				case	TRIGFIN:
					FinishTrigger();
					break;
					

				case	TRIGRESET:
					ResetTrigger();
					break;
					

				case SETMODEL:
					InsertModelChange(line_tail);
					break;

				case SETSOUND:
					InsertSoundChange(line_tail);
					break;

				case SETANIM:
					InsertAnimChange(line_tail);
					break;
					
				case SETMODELANIM:
					if (!(InsertModelAnimChange(line_tail)))
					{
					 //	error(12,line_tail,linenum);
						end++;
						fatal++;
					}
					break;
					
				//VERIFYING LOOP VALUES

				case LOOP:
					line_tail = WhiteSpaceRead(line_tail);
					InsertLoop(line_tail);
					break;

				case LOOPIMM:
					line_tail = WhiteSpaceRead(line_tail);
					InsertLoopImmediate(line_tail);
					break;

				case ENDLOOP:
					CloseLoop(line_tail);
					break;

				case ENDIMMLOOP:
					CloseLoopImm(line_tail);
					break;

				case WHILE:
					if (!(InsertWhile(line_tail)))
					{
						error(12,line_tail,linenum);
						end++;
						fatal++;
					}		
					break;

				case ENDWHILE:
					CloseWhile(line_tail);
					break;

				case IF:
					if (!(InsertIf(line_tail)))
					{
						error(15,line_tail,linenum);
						end++;
						fatal++;
					}		
					break;

				case ENDIF:
					CloseIf(line_tail);
					break;

				case ELSE:
					CloseElse(line_tail);
					break;

				case PRINT:
					if (!(InsertPrint(line_tail)))
					{
						error(17,line_tail,linenum);
						end++;
						fatal++;
					}		
					break;

				case EVALUATION:
					if (line_tail=(WhiteSpaceRead(line_tail)))

						if (!InsertEvaluation(line_tail))
						{

							error(17,line_tail,linenum);
							end++;
							fatal++;

						}
					break;	



				case END:
					end++;
					break;

				default:
					break;
			}
			break;
		}

	}

   	//CHECK THE LOOP LEVELS
	if (NEST>=6)
   	{
   		printf("\n*** NEST ERROR found at line num %d \n*** REDUCE THE IF/WHILE/LOOP LEVELS %d\n\n",linenum,NEST);
   		end++;
   		fatal++;
	}

	return (end);

}

int StateUnBalanced(char *line_tail, int linenum)
{
	int UnBalanced = 0;

	/*** Ensure all selections were closed in previous state **/
												
	if (IfLevel)
	{
		printf("UNCLOSED IF statements found in strat \n");
		UnBalanced++;
	}	

	/*** Ensure all loops were closed in previous state **/

	if (NestLevel)	
	{
		error(5,line_tail,linenum);
		UnBalanced++;
	}

	/*** Ensure all whiles were closed in previous state **/

	if (WhileLevel)	
	{
		error(9,line_tail,linenum);
		UnBalanced++;
	}
	if (UnBalanced)
		fatal++;

	return (UnBalanced);
}

int AlreadyDefined(char *symbol)
{
	int i;

	for (i=0;i<Defined;i++)
	{
		if (!Defs[i].definition)
			break;
		if ((strlen(Defs[i].definition)) != (strlen(symbol)))
			continue;

		if (!strncmp(symbol,Defs[i].definition,strlen(Defs[i].definition)))
		{
			printf("label %s is already a define\n",symbol);
			return(1);
		}
	}

	for (i=0;i<IVarNum;i++)
	{
		if (!IVars[i].definition)
			break;
		if ((strlen(IVars[i].definition)) != (strlen(symbol)))
			continue;

		if (!strncmp(symbol,IVars[i].definition,strlen(IVars[i].definition)))
		{
			printf("label %s is already a declared int variable\n",symbol);
			return(1);
		}
	}

	for (i=0;i<FVarNum;i++)
	{
		if (!FVars[i].definition)
			break;
		if ((strlen(FVars[i].definition)) != (strlen(symbol)))
			continue;

		if (!strncmp(symbol,FVars[i].definition,strlen(FVars[i].definition)))
		{
			printf("label %s is already a declared float variable\n",symbol);
			return(1);
		}
	}

	return (0);
}
	
int AddDefinition(char *symbol,char *cursor)
{
	char numbuf[32];
	int res;


	if (AlreadyDefined(symbol))	
		return (0);
			
	Defs[Defined].definition=malloc((strlen(symbol)+1));


	
	strcpy(Defs[Defined].definition,symbol);


	if (!(cursor=WhiteSpaceRead(cursor)))
//	if (!cursor)
		return(0);
	
	Clear(numbuf,32);

	ReadToEnd(numbuf,cursor);

	if (res=NumericCheck(numbuf))
	{
		if (res==FLOAT)
			sprintf(numbuf,"%s%s",numbuf,"f");

		Defs[Defined].value=malloc((strlen(numbuf)+1));
		strcpy(Defs[Defined].value,numbuf);
		//printf("defined %s of %s\n",symbol,numbuf);
		Defined++;
		return(Defined+1);
	}
	else
	  	return (0);


}

int AddIVar(char *symbol,char *cursor)
{

	char numbuf[32];

	if (AlreadyDefined(symbol))	
		return (0);
	
	IVars[IVarNum].definition=malloc((strlen(symbol)+1));


	
	strcpy(IVars[IVarNum].definition,symbol);

	if (!(cursor=WhiteSpaceRead(cursor)))
	{
		//UNINITIALISED INTEGER VARIABLE
		IVarNum++;
		return(IVarNum);
	}

	Clear(numbuf,32);

	ReadToEnd(numbuf,cursor);

	if (NumericIntCheck(numbuf))
	{
		IVars[IVarNum].value=malloc((strlen(numbuf)+1));
		strcpy(IVars[IVarNum].value,numbuf);
		//printf("defined %s of %s\n",symbol,numbuf);
		IVarNum++;
		return(IVarNum);
	}
	else
	{
		printf ("Defined integer variable %s with invalid number %s\n",symbol,numbuf); 
		return (0);
	}

}

int AddFVar(char *symbol,char *cursor)
{

	char numbuf[32];

	if (AlreadyDefined(symbol))	
		return (0);
		
	FVars[FVarNum].definition=malloc((strlen(symbol)+1));
	
	strcpy(FVars[FVarNum].definition,symbol);


	if (!(cursor=WhiteSpaceRead(cursor)))
	{
		//UNINITIALISED FLOAT VARIABLE
		
		FVarNum++;
		return(FVarNum);	
	}

	Clear(numbuf,32);

	ReadToEnd(numbuf,cursor);

	if (NumericFloatCheck(numbuf))
	{
		FVars[FVarNum].value=malloc((strlen(numbuf)+1));
		strcpy(FVars[FVarNum].value,numbuf);
		FVarNum++;
		return(FVarNum);
	}
	else
	{
		printf ("Defined float variable %s with invalid number %s\n",symbol,numbuf); 
		return (0);
	}	  	


}



int ValidDefine(char *symbol,char *cursor)
{
	if (strncmp(symbol,"A",1) < 0)  
		return (0);
	else
		return (AddDefinition(symbol,cursor));
}


int ValidIVarDefine(char *symbol,char *cursor)
{
	if (strncmp(symbol,"A",1) < 0)  
		return (0);
	else
		return(AddIVar(symbol,cursor));
}

int ValidFVarDefine(char *symbol,char *cursor)
{
	if (strncmp(symbol,"A",1) < 0)  
		return (0);
	else
		return(AddFVar(symbol,cursor));
}


int ValidFParamDefine(char *symbol,char *cursor)
{
	char buffer[64];

	Clear(buffer,64);
	ReadToEnd(buffer,symbol);

	if (strncmp(buffer,"A",1) < 0)  
		return (0);
	else
		return(AddFVar(buffer,cursor));
}


int ValidIParamDefine(char *symbol,char *cursor)
{
	char buffer[64];

	Clear(buffer,64);
	ReadToEnd(buffer,symbol);

	if (strncmp(buffer,"A",1) < 0)  
		return (0);
	else
		return(AddIVar(buffer,cursor));
}






int AddDefine(char *cursor)
{
	char symbol[64];
	Clear(symbol,64);

	if (cursor=WhiteSpaceRead(cursor))
	{
		if (cursor=SymbolGet(symbol,cursor))
			return (ValidDefine(symbol,cursor));
		else
			return (0);
		  
	}
	else
		return (0);
}

int AddVar(char *cursor,int mode)
{
	char symbol[64];
	Clear(symbol,64);

	if (cursor=WhiteSpaceRead(cursor))
	{
		if (cursor=SymbolGet(symbol,cursor))
		{
		  
			switch (mode)
			{
				case LOCALINT:
					return (ValidIVarDefine(symbol,cursor));

				case LOCALFLOAT:
					return (ValidFVarDefine(symbol,cursor));
				case PARAMINT:
					return (ValidIParamDefine(symbol,cursor));

				case PARAMFLOAT:
					return (ValidFParamDefine(symbol,cursor));
			}
			
		 //	if (mode == LOCALINT)
		  //		return (ValidIVarDefine(symbol,cursor));
		  //	else
		   //		return (ValidFVarDefine(symbol,cursor));
		}	
		else
			return (0);
		  
	}
	//else
	return (0);
	
}



char *StateLabelParse(char *cursor)
{
	//ENSURE LABEL STARTS WITH ALPHA

	if (strncmp(cursor,"A",1) < 0)  
		return (NULL);
	else
	{
		if (!(CheckLabel(cursor)))
		{
			AddLabel(cursor);
			return(cursor);
		}
		else
			return (NULL);
	}

}


void ReadToEnd(char *dest,char*src)
{
	if (src)
	{
	while	(
			(strncmp(src," ",1)) &&
			(strncmp(src,"\0",1)) &&
			(strncmp(src,"\t",1)) &&
			(strncmp(src,"\n",1)) )
	{

		strncpy(dest,src,1);
	 	src++;
		dest++;
	}
	*dest='\0';
}
}

void ReadToEndUpper(char *dest,char*src)
{
	while	(
			(strncmp(src," ",1)) &&
			(strncmp(src,"\0",1)) &&
			(strncmp(src,"\t",1)) &&
			(strncmp(src,"\n",1)) )
	{

		*dest = toupper(*src);
	//	strncpy(dest,src,1);
	 	src++;
		dest++;
	}
//	*dest='\0';

}

int CheckLabel(char *cursor)
{	
	Uint16 i,found=0;
	char buffer[32];


	for (i=0;i<32;i++)
		buffer[i]='\0';


	ReadToEnd(buffer,cursor);


	for (i=0;i<Label;i++)
	{
		if (!statename[i].state)
			break;

		#if DEBUG
			printf("checking %s %s\n",buffer,statename[i].state);
			printf("checking %d %d\n",strlen(buffer),strlen(statename[i].state));
		#endif

		if ((strlen(statename[i].state)) < (strlen(buffer)))
			continue;



		if (!strncmp(buffer,statename[i].state,strlen(statename[i].state)))
		{
			found++;
			break;

		}
	}
	if (found)
		return(i+1);
	else
		return(0);

}





int ProcedureCheck(char *symbol)
{	
	int i,found=0;

	for (i=0;i<Procedures;i++)
	{
		if (!ProcedureWords[i].symbol)
			break;

		//printf("Proc %s sym %s\n",ProcedureWords[i].symbol,symbol);

		if ((strlen(ProcedureWords[i].symbol)) != (strlen(symbol)))
			continue;


		if (!strncmp(symbol,ProcedureWords[i].symbol,strlen(ProcedureWords[i].symbol)))
		{
			found++;
			break;

		}
	}

 //	printf("Proc %s\n",ProcedureWords[0].symbol);
	if (found)
		return(i+1);
	else
		return(0);

}

int SymbolCheck(char *symbol)
{	
	int i,found=0;

	for (i=0;i<Reserved;i++)
	{
		if (!ReservedWords[i].symbol)
			break;


		if ((strlen(ReservedWords[i].symbol)) != (strlen(symbol)))
			continue;


		if (!strncmp(symbol,ReservedWords[i].symbol,strlen(ReservedWords[i].symbol)))
		{
			found++;
			break;

		}
	}
	if (found)
		return(i+1);
	else
		return(0);

}

int DefinitionCheck(char *symbol)
{	
	int i,found=0;

	for (i=0;i<Defined;i++)
	{
		if (!Defs[i].definition)
			break;


		if ((strlen(Defs[i].definition)) != (strlen(symbol)))
			continue;



		if (!strncmp(symbol,Defs[i].definition,strlen(Defs[i].definition)))
		{
			found++;
			break;

		}
	}
	if (found)
		return(i+1);
	else
		return(0);

}

int FloatVarCheck(char *symbol)
{	
	int i,found=0;
	unsigned int len;

	len = strlen(symbol);

	for (i=0;i<FVarNum;i++)
	{
		if (!FVars[i].definition)
			break;

	

		if ((strlen(FVars[i].definition)) != len)
			continue;


		if (!strncmp(symbol,FVars[i].definition,strlen(FVars[i].definition)))
		{
			found++;
			break;

		}
	}
	if (found)
		return(i+1);
	else
		return(0);

}

int IntVarCheck(char *symbol)
{	
	int i,found=0;
	unsigned int len;

	len = strlen(symbol);

	for (i=0;i<IVarNum;i++)
	{
		if (!IVars[i].definition)
			break;


		if ((strlen(IVars[i].definition)) != len)
			continue;


		if (!strncmp(symbol,IVars[i].definition,strlen(IVars[i].definition)))
		{
			found++;
			break;

		}
	}
	if (found)
		return(i+1);
	else
		return(0);

}


int GlobalDefCheck(char *symbol)
{	
	int i,found=0;

	for (i=0;i<GDefined;i++)
	{
		if (!GDefs[i].definition)
			break;


		if ((strlen(GDefs[i].definition)) != (strlen(symbol)))
			continue;


		if (!strncmp(symbol,GDefs[i].definition,strlen(GDefs[i].definition)))
		{
			found++;
			break;

		}
	}
	if (found)
		return(i+1);
	else
		return(0);

}


int NumericIntCheck(char *symbol)
{	
	unsigned int i,found=1,len;
	char next[1];

	len=strlen(symbol);
	for (i=0;i<len;i++)
	{

		//READ SYMBOL, IF IT'S NOT IN RANGE OF 0-9, -+, THEN EXIT
		//AS IT'S NOT A NUMERIC INTEGER

		next[0]=*symbol;


		if (
				(!(isdigit(next[0]))) && 
				(*next != '-')
			)
		{
			found=0;
			break;
		}

		symbol++;
	}

	return(found);

}

int NumericFloatCheck(char *symbol)
{	
	unsigned int i,found=1,len,pointfound=0,numafter=0;
	char next[1];

	len=strlen(symbol);
	
	for (i=0;i<len;i++)
	{

		//READ SYMBOL, IF IT'S NOT IN RANGE OF 0-9, -+, FLOAT THEN EXIT
		//AS IT'S NOT A NUMERIC FLOAT, ALSO ENSURE THE . IS SPECIFIED
		//AND A DIGIT FOLLOWS IT

		next[0]=*symbol;

		

		if (	(!(isdigit(next[0]))) && 
				(*next != '.') &&
				(*next != '-')
			)
			{
			found=0;
			break;
			}	
		
		if (pointfound)
			numafter=1;
		
		if ((*next) == '.')
			pointfound=1;	
		symbol++;
	}


	return(found & pointfound & numafter);

}




int NumericCheck(char *symbol)
{	
	unsigned int i,len,found=INT;
	char next[1];

	len=strlen(symbol);

	for (i=0;i<len;i++)
	{

		//READ SYMBOL, IF IT'S NOT IN RANGE OF 0-9, -+, FLOAT THEN EXIT
		//AS IT'S NOT A NUMERIC INTEGER

		next[0]=*symbol;


		if (
				(!(isdigit(next[0]))) && 
				(*next != '-') &&
				(*next != '.')
			)
		{
			found=0;
			break;
		}

		if (*next == '.')
			found = FLOAT;

		symbol++;
	}

	return(found);

}


int OperationCheck(char *symbol)
{	
	int i,found=0;


	for (i=0;i<Operation;i++)
	{
		if (!Operations[i].symbol)
			break;



		if ((strlen(Operations[i].symbol)) != (strlen(symbol)))
			continue;

		if (!strncmp(symbol,Operations[i].symbol,strlen(Operations[i].symbol)))
		{
			found++;
			break;

		}
	}
	if (found)
		return(i+1);
	else
		return(0);

}


void CaseInsert()
{
	char scriptstring[128];
	char tab[32];
	Indentation(tab);
 	sprintf( scriptstring,"%s%s%d%s\n",
 				tab,
 				"case (",
 				statename[CurrentLabel].stack[NEST],
 				"):");				
 	AddLine(scriptstring);
 	Indent++;
 	statename[CurrentLabel].stack[NEST]++;
}

SCRIPTLINE *ElseLine;

void CaseInsertElse()
{
	char scriptstring[128];
	char tab[32];
	Indentation(tab);



	ElseLine=CurrentLine;
	
	//ADDED FRIDAY
//	sprintf (scriptstring,"%s%s\n",tab,"break;");
//	AddLine(scriptstring);
	sprintf (scriptstring,"%s%s%d%d%s\n",
					tab,
					currentstrat,
					NEST,
					stratlabel[NEST],
					":");
	AddLine(scriptstring);
	InIf[NEST-1] = 0;

 	sprintf( scriptstring,"%s%s%d%s\n",
 				tab,
 				"case (",
				99,
 				"):");				
 	AddLine(scriptstring);
 	Indent++;
}

void CaseInsert2()
{
	char scriptstring[128];
	char tab[32];
	Indentation(tab);
 	sprintf( scriptstring,"%s%s%d%s\n",
 				tab,
 				"case (",
 				statename[CurrentLabel].stack[NEST],
 				"):");				
 	AddLine(scriptstring);
 	Indent++;
}


void SwitchHeader()
{
	char scriptstring[128];
	char tab[32];
 	Indentation(tab);	
 	sprintf(scriptstring,"\n%s%s%d%s\n",
 				tab,
 				"switch (strat->stack[",
				NEST,
				"])");
 	AddLine(scriptstring);
 	sprintf( scriptstring,"%s%s\n",tab,"{");
 	AddLine(scriptstring);
  	Indent++;	
 	Indentation(tab);	
 	statename[CurrentLabel].stack[NEST]=0;
 	EndSubBlock[NEST]=0;
}

void SwitchFooter()
{
	char scriptstring[128];
	char tab[32];

 	Indentation(tab);	
 	sprintf( scriptstring,"%s%s\n",tab,"default:");
 	AddLine(scriptstring);
 	Indent++;
 	Indentation(tab);	
 	sprintf( scriptstring,"%s%s\n",tab,"Reset(strat);");
 	AddLine(scriptstring);
 	sprintf( scriptstring,"%s%s\n",tab,"break;");
 	AddLine(scriptstring);
 	Indent--;
 	Indent--;
 	Indentation(tab);
 	sprintf( scriptstring,"%s%s\n",tab,"}");
 	AddLine(scriptstring);
	Indent=2;
 	Indentation(tab);
}

void SwitchLoopFooter()
{
	char scriptstring[128];
	char tab[32];

	Indentation(tab);
	sprintf(scriptstring,"%s%s\n",tab,"default:");
	AddLine(scriptstring);
	Indent++;
	Indentation(tab);

	//manutd added 2 lines
   	sprintf(scriptstring,"%s%s%d%s\n",tab,"strat->timer[",NEST-1,"]++;");
   	AddLine(scriptstring);

	sprintf(scriptstring,"%s%s%d%s\n",tab,"strat->timer[",NEST,"]=0;");
	AddLine(scriptstring);


	if (WhileIn)
	{

		sprintf ( scriptstring,"%s%s%d%s%d%s\n",tab,"strat->stack[",
					NEST,
					"]=",0,";");									
		AddLine(scriptstring);

		WhileIn=0;
	}
	else
	{

		if (NEST >0)
		{
			sprintf(scriptstring,"%s%s%d%s\n",tab,"strat->stack[",NEST,"]=0;");
			AddLine(scriptstring);
		}
		else
		{
			sprintf(scriptstring,"%s%s%d%s\n",tab,"strat->stack[",NEST+1,"]=0;");
			AddLine(scriptstring);
		}

	}
//friday
//	sprintf(scriptstring,"%s%s\n",tab,"break;\n");

	if (Imm[NEST])
		sprintf(scriptstring,"%s%s%s%s\n",tab,"goto",ImmediateLabel,";");
	else		
		sprintf(scriptstring,"%s%s\n",tab,"return;\n");
	AddLine(scriptstring);
	Indent--;  //case level

	Indent--;  //switch level
	Indentation(tab);
	sprintf(scriptstring,"%s%s\n",tab,"}");
	AddLine(scriptstring);

}


void SwitchCondFooter()
{
	char scriptstring[128];
	char tab[32];

	Indent++;
	Indentation(tab);
//utd
//	sprintf(scriptstring,"%s%s\n",tab,"break;");
//	AddLine(scriptstring);
	Indent--;
	Indentation(tab);
	sprintf(scriptstring,"%s%s\n",tab,"default:");
	AddLine(scriptstring);
	Indent++;
	Indentation(tab);


	sprintf(scriptstring,"%s%s%d%s\n",tab,"strat->timer[",NEST,"]=0;");
	AddLine(scriptstring);


	if (WhileIn)
	{

		sprintf ( scriptstring,"%s%s%d%s%d%s\n",tab,"strat->stack[",
					NEST,
					"]=",0,";");									
		AddLine(scriptstring);

		WhileIn=0;
	}
	else
	{

		if (NEST >0)
		{
			//manutdadd two lines
			sprintf(scriptstring,"%s%s%d%s\n",tab,"strat->timer[",NEST-1,"]++;");
			AddLine(scriptstring);
			sprintf(scriptstring,"%s%s%d%s\n",tab,"strat->stack[",NEST,"]=0;");
			AddLine(scriptstring);
		}
		else
		{
			//manutd add two lines
			sprintf(scriptstring,"%s%s%d%s\n",tab,"strat->timer[",NEST-1,"]++;");
			AddLine(scriptstring);
			sprintf(scriptstring,"%s%s%d%s\n",tab,"strat->stack[",NEST+1,"]=0;");
			AddLine(scriptstring);
		}
	}
	//FRIDAY
	//	sprintf(scriptstring,"%s%s\n",tab,"break;\n");
	if (Imm[NEST])
		sprintf(scriptstring,"%s%s%s%s\n",tab,"goto",ImmediateLabel,";");
	else		
		sprintf(scriptstring,"%s%s\n",tab,"return;\n");
	AddLine(scriptstring);
	Indent--;  //case level

	Indent--;  //switch level
	Indentation(tab);
	sprintf(scriptstring,"%s%s\n",tab,"}");
	AddLine(scriptstring);

}

void SwitchIfFooter()
{
	
	char tab[32];
	char scriptstring[128];

	Indentation(tab);

	//FRIDAY FOR THE ELSE STUFF
	stratlabel[NEST]++;

	
	
	elselabelused[NEST]=0;
	elselabel[NEST] = stratlabel[NEST];



	sprintf (scriptstring,"%s%s%d%d%s\n",
				tab,
				currentstrat,
				NEST,
				stratlabel[NEST],
				":");
	AddLine(scriptstring);
	elselabelline[NEST]=CurrentLine;

	
	sprintf(scriptstring,"%s%s\n",tab,"default:");
	AddLine(scriptstring);
	Indent++;
	Indentation(tab);


	sprintf(scriptstring,"%s%s%d%s\n",tab,"strat->timer[",NEST,"]=0;");
	AddLine(scriptstring);

	if (WhileIn)
	{

		sprintf ( scriptstring,"%s%s%d%s%d%s\n",tab,"strat->stack[",
					NEST,
					"]=",0,";");									
		AddLine(scriptstring);

		WhileIn=0;
	}
	else
	{
		if (NEST >0)
		{
			sprintf(scriptstring,"%s%s%d%s\n",tab,"strat->stack[",NEST,"]=0;");
			AddLine(scriptstring);
			sprintf(scriptstring,"%s%s%d%s%d%s\n",tab,"strat->stack[",NEST-1,"]=",
			statename[CurrentLabel].stack[NEST-1],";");
			AddLine(scriptstring);
		}
		else
		{
			sprintf(scriptstring,"%s%s%d%s\n",tab,"strat->stack[",NEST+1,"]=0;");
			AddLine(scriptstring);
			sprintf(scriptstring,"%s%s%d%s%d%s\n",tab,"strat->stack[",NEST,"]=",
			statename[CurrentLabel].stack[NEST],";");
			AddLine(scriptstring);
		}
	}
	if (Imm[NEST])
		sprintf(scriptstring,"%s%s%s%s\n",tab,"goto",ImmediateLabel,";");
	else		
		sprintf(scriptstring,"%s%s\n",tab,"break;\n");
//	sprintf(scriptstring,"%s%s\n",tab,"return;\n");

	AddLine(scriptstring);
	Indent--;  //case level

	Indent--;  //switch level
	Indentation(tab);
	sprintf(scriptstring,"%s%s\n",tab,"}");
	AddLine(scriptstring);

}


void SetState (char *cursor)
{
	int i,y=0;
	char buffer[32];

	for (i=0;i<32;i++)
		buffer[i]='\0';


	ReadToEnd(buffer,cursor);

	for (i=0;i<Label;i++)
	{
		if ((strlen(statename[i].state)) < (strlen(buffer)))
			continue;

		if (!strncmp(buffer,statename[i].state,strlen(statename[i].state)))
		{
			Indent=1;
			CurrentLabel=i;
			Global=0;	//Local symbol found so reset global
			statename[i].Internal=-1;
			statename[i].indent=2;
			ResetSwitches();
			SAVENEST=NEST=0;
			break;

		}

	}

	for (i=0;i<MAX_NESTS;i++)
		EndSubBlock[NEST]=0;

	for (i=0;i<LINEMAX;i++)
	{
	  	lines[i]=NULL;
	  	lines2[i]=NULL;
		Indents[i]=0;
		Indents2[i]=0;


	}


	for (i=0;i<MAX_NESTS;i++)
	{
	   	//lines[i]=NULL;
	   	//lines2[i]=NULL;
		//Indents[i]=0;
		//Indents2[i]=0;
		InIf[i]=0;
		Imm[i]=0;
		ElseMet[i]=0;
		EvalLine[i]='\0';
	}
	line=0;
	line2=0;
	KillMe=-1;
  
	CurrentLine=statename[CurrentLabel].line;
	SwitchHeader();
	statename[CurrentLabel].line=CurrentLine;
	SwitchFooter();
	CurrentLine=statename[CurrentLabel].line;


}

void ResetSwitches()
{
	int i,j;
	for (i=0;i<Label;i++)
	{
		for (j=0;j<30;j++)
			statename[i].stack[j]=0;
	}
}

void AddLabel(char *cursor)
{
	int i;
	char buffer[32];
	ReadToEnd(buffer,cursor);

	statename[Label].state=malloc((strlen(cursor)+1));
	strcpy(statename[Label].state,buffer);
	output(0,statename[Label].state);
	CurrentLabel=Label;
	for (i=0;i<30;i++)
		statename[Label].stack[i]=0;


	Label++;

	
}



char  *LeadingSpace(char *cursor, char *line)
{
	if (strncmp(cursor," ",1))
		return(NULL);
	else
	{
		cursor++;	
		return(cursor);
	}
}


char  *WhiteSpaceRead(char *cursor)
{	Uint16 charfound=0;
	int len;
	char *max;

	//SAFETY CHECK TO ENSURE NULL * IS NOT CHECKED

	if (!cursor )
		return (NULL);

	len = strlen(cursor)+1;
	max = cursor+MAX_LINE;



//	for (cursor;cursor < cursor+MAX_LINE;cursor++)
	for (cursor;cursor < max;cursor++)
	{
		if ((strncmp(cursor," ",1)) && (strncmp(cursor,"\t",1)))
		{
			if ((strncmp(cursor,"\n",1))  && (strncmp(cursor,"\0",1)))
				charfound=1;
			break;
		}
	}

	if (charfound)
		return (cursor);
	else
		return (NULL);
}

char *Numeric(char *cursor, char*line)
{
	for (cursor;cursor < &line[MAX_LINE];cursor++)
	{
		if (	!(strncmp(cursor," ",1)) ||
				!(strncmp(cursor,"t ",1)) ||
				!(strncmp(cursor,"\n ",1)) 
			)
			break;

			if ((strncmp(cursor,"0",1) < 0) || (strncmp(cursor,"9",1) >0))
			{
				cursor=NULL;
				break;
			}

	}

	return(cursor);
}

void CopyVal(char *dest,char *cursor)
{
	int i;

	for (i=0;i<32;i++)
		dest[i]='\0';


	while (strcmp(cursor,"\n"))
	{
		if ((strcmp(cursor,"0") > 0) && (strcmp(cursor,"9") <0))
		{
			strncpy(dest,cursor,1);		
			dest++;
		}
		cursor++;
	}

}






void error(int type,char *line,int number)
{
	char errorstring[512];


	if (statename[CurrentLabel].state)
		sprintf(errorstring,"STATE:%s\n %s %d label %s\n",
			statename[CurrentLabel].state,
			errortable[type],number,line);
	else
		sprintf(errorstring,"%s %d label %s\n",
			errortable[type],number,line);

	printf("ERROR %s\n",errorstring);

//	strncpy(errorlog,errorstring,strlen(errorstring));
//	errorlog += strlen(errorstring);
	Error ++;

}


char scripttable[4][50]={
	"void",
	"possible label space error at",
	"Undefined label at"
};


void output(int type,char *line)
{
	char scriptstring[512];

	if (!(strnicmp(line,"init",4)))
		sprintf(scriptstring,"%s %s%s (STRAT *strat)\n",scripttable[type],currentstrat,line);
	else
		sprintf(scriptstring,"%s %s%s (STRAT *strat)\n","static void",currentstrat,line);
	AddLine(scriptstring);

}

FILE *OpenLog(char *filename)
{
	char StratFile[256];
	FILE *fp;


	sprintf(StratFile,"%s%s%s",LOGDIR,filename,".ERR");	

	if (fp=fopen(StratFile, "w"))
		return (fp);

	else
	{
		printf ("file error %s\n",StratFile);
		return (NULL);
	}


}


void IncludeStart(FILE *fp,char *Level)
{	
	char Include[128];
	char StratFile[128];

//	fputs("#define EXT \"ext.h\"\n",fp);
	Level ++;

	sprintf(StratFile,"%s%s%s",SCRIPTDIR,Level,".C");
	sprintf(Include,"%s%s%s\n","#include \"",StratFile,"\"");
	fputs(Include,fp);
}

FILE *OpenScript(char *filename)
{
	char StratFile[256];
	char Include[256];
	FILE *fp;


	//OUTPUT SCRIPT *.C AND ADD ITS INCLUDE TO THE LEVELST.C FILE

	sprintf(StratFile,"%s%s%s",SCRIPTDIR,filename,".C");	

	//BIZARRE 
	if ((FullRec) || (TIMESTAMPFAIL))
	{
		sprintf(Include,"%s%s%s\n","#include \"",StratFile,"\"");

		fputs(Include,Cinclude);
	}

	if (fp=fopen(StratFile, "w"))
		return (fp);
	else
	{
		printf ("file error %s\n",StratFile);
		return (NULL);

	}


}

void HeaderStart(FILE *fp)
{
	fputs("#ifndef LEVELST_H\n",fp);
	fputs("#define LEVELST_H\n",fp);

}

void HeaderEnd(FILE *fp)
{
	//EXTERN THE STRATS SO THEY CAN BE USED ELSEWHERE
	
	fputs("extern STRATENTRY STRATS[];\n",fp);
	fputs("#endif \n",fp);

}


FILE *OpenHeader(char *filename)
{
	char StratFile[256];
	char Include[256];
	FILE *fp;


	//OUTPUT SCRIPT *.H AND ADD ITS INCLUDE TO THE LEVELST.H FILE

	//FIRST SKIP OVER ANY SPACES

	while (*filename==' ')
		filename++;

	sprintf(StratFile,"%s%s%s",HEADERDIR,filename,".H");	

	//BIZARRE 
	if ((FullRec) || (TIMESTAMPFAIL))
	{
		sprintf(Include,"%s%s%s\n","#include \"",StratFile,"\"");

		fputs(Include,Header);
	}

	if (fp=fopen(StratFile, "w"))
		return (fp);
	else
	{
		printf ("file error %s\n",StratFile);
		return (NULL);

	}


}


SCRIPTLINE *InitScript()
{
	int i;

	for (i=0;i<MAX_STATE;i++)
		statename[i].state=NULL;

	for (i=0;i<MAX_DEFS;i++)
		Defs[i].definition=NULL;


	CurrentLine=malloc(sizeof(SCRIPTLINE));
	CurrentLine->prev = CurrentLine->next = NULL;
	CurrentLine->text = NULL;
	return(CurrentLine);
}


//FREE SCRIPT BUFFER WITHOUT WRITING

void CleanScript(SCRIPTLINE *start)
{
	SCRIPTLINE *old=start;

	do
	{
		old=start;
		start=old->next;
		free (old->text);
		free (old);
	}	while (start);


}



//OUTPUT SCRIPT LINES AND FREES

void WriteScript(FILE *scriptfile,SCRIPTLINE *start)
{
	SCRIPTLINE *old=start;

	do
	{
		old=start;
		fputs(old->text,scriptfile);
		start=old->next;
		free (old->text);
		free (old);
	}	while (start);
	fclose(scriptfile);


}

void WriteHeader(FILE *scriptfile)
{
	int i;

	char scriptstring[512];


	for (i=0;i<Label;i++)
	{

		if (!strnicmp(statename[i].state,"init",4))
		sprintf(scriptstring,"%s %s%s %s \n",
									"void",
									currentstrat,
									statename[i].state,
									"(STRAT *strat);");
		else
		sprintf(scriptstring,"%s %s%s %s \n",
									"static void",
									currentstrat,
									statename[i].state,
									"(STRAT *strat);");
		fputs(scriptstring,scriptfile);

	}

	fclose(scriptfile);

}


void AddLine(char *linetext)
{
  //	return;
	SCRIPTLINE *old=CurrentLine;

	#if DEBUG
		printf("adding script line%s\n",linetext);
	#endif

	CurrentLine=malloc(sizeof(SCRIPTLINE));
	if (!CurrentLine)
		printf("allocation error\n");

	CurrentLine->prev = old;
	CurrentLine->next = old->next;	
	old->next=CurrentLine;
	CurrentLine->text = calloc((strlen(linetext)+1),sizeof(char));
	if (!CurrentLine->text)
		printf("allocation error\n");

  	strncpy (CurrentLine->text,linetext,strlen(linetext));
	#if DEBUG
		printf("added script line%s\n",linetext);
  	#endif


}


//INSERT TWO {}s TO OUTPUT SCRIPT
//SET THE FIRST INSERTION LINE FOR ENTRIES TO THAT STATEMENT'S SCRIPT
#define BRACK  "{\n"
#define BRACK2 "}\n\n"
void StateDefine()
{

	char scriptstring[16];

  	sprintf(scriptstring,"%s",BRACK);
	AddLine(scriptstring);

	statename[CurrentLabel].line=CurrentLine;

	sprintf(scriptstring,"%s",BRACK2);
	AddLine(scriptstring);
  
}

void InsertCondition(int RefState)
{
	char scriptstring[128];
	char tab[32];
	int SaveIndent;
	SCRIPTLINE *line;

	Clear(scriptstring,128);

	Indentation(tab);


	//GET THE BASE SCRIPT LINE TO USE FOR THE BLOCK

	if (SAVENEST < NEST)
		GetLine();
	else
		GetLine2();

	//FRIDAY

	if (!EndSubBlock[NEST])
	{

		if (!NEST)
			CaseInsert();
		else
			if (statename[CurrentLabel].stack[NEST])
				CaseInsert();
	}


	IfLabelCheck();

	if ((NEST) && (statename[CurrentLabel].stack[NEST]==0))
	{
		SwitchHeader();
		CaseInsert();
		SaveIndent=Indent;
		Indent--;
		line=CurrentLine;
	  	SwitchCondFooter();
		CurrentLine=line;
		Indent=SaveIndent;
	}

	Indentation(tab);
	sprintf(scriptstring,"%s%s%s%s%s\n",
				tab,
				"ChangeState(strat,&",
				currentstrat,
			  	statename[RefState].state,	
				");");

	AddLine(scriptstring);

	EvalLine[NEST]=CurrentLine;

	sprintf(scriptstring,"%s%s\n",
				tab,
				"return;"
			  );

	AddLine(scriptstring);


	
	//PUSH THE LINE AS THE START OF THE NEXT BLOCK
	PushLine();
	PushLine2();

	
//	statename[CurrentLabel].line=CurrentLine;
		
}

/*THURS 29TH APR, HACK PLACED IN TO ENSURE .0f ALWAYS PLACED ON 'EVERY' TRIGGERS*/

void InsertTrigger(int RefState, char *cursor)
{
	char *labpt;
	char scriptstring[256];
	char label[128];
	char label2[128];
	char tab[32];
	int	 HACK,len,res,SymbolType,len2,len3;

	Clear(scriptstring,256);

	StatementBlockStructure(scriptstring,tab);	

	//READ TO THE END OF THE TRIGGER ADDRESS
	Clear(label,128);
	labpt=label;
	cursor = WhiteSpaceRead(cursor);
	ReadToEnd(labpt,cursor);
	cursor+=strlen(label);



	//NOW GET THE TYPE OF TRIGGER
	Clear(label,128);
	cursor = WhiteSpaceRead(cursor);
	ReadToEnd(labpt,cursor);


	//CHECK TO SEE WHAT TYPE OF TRIGGER IT IS
	
	SymbolType =0;
	HACK = 0;

	if ((!strncmp(labpt,"EVERY",5)) || 
		(!strncmp(labpt,"NEARWAYPOINT",12)) ||
		(!strncmp(labpt,"NEARACTPOINT",12)) ||
		(!strncmp(labpt,"AHEADOFPLAYER",13)) ||
		(SymbolType) ||
		(!strncmp(labpt,"NEARPLAYER",10)))
	{
		if ((!strncmp(labpt,"EVERY",5)))
			HACK = 1;   

		#if DEBUGTRIG		
			printf("param trig found\n");
		#endif
		sprintf(scriptstring,"%s%s%s%s%s%s",
				tab,
				"CreateTriggerParam(strat,&",
				currentstrat,
				statename[RefState].state,	
				",",
				labpt);

		cursor+=strlen(label);

		//grab those parameters
		while (cursor = WhiteSpaceRead(cursor))
		{
			
			Clear(label,128);
//			strncat(label,",",1);
//			labpt=label+1;
			
			strncat(scriptstring,",",1);

			//ReadToEnd(labpt,cursor);
			ReadToEnd(label,cursor);

			len=strlen(label);
			
			if (SymbolType)
			{
				if (res=GlobalDefCheck(labpt))
				{
						res--;
						Clear(label2,128);					
						Copy(label2,GDefs[res].value,strlen(GDefs[res].value));
					//	value = atoi(label2);
					//	value = value &(0xffffffff - STRAT2_PICKUP);
					//	Clear(label2,128);					

					//	itoa(value,label2,10);
						len2=strlen(label2);
					//	strncat(scriptstring,"(float)",7);
						strncat(scriptstring,label2,len2);
				}
				//TODO CLEAN UP + else error message here
			
			}
			else
			{
//				len=strlen(label);
				res = NumericCheck(labpt);
				/*
				if (res==FLOAT)
				{				  */
				if (HACK)
				{
					strncat(label,".0f",3);
					len3 = strlen(label);
					strncat(scriptstring,label,len3);
				}
				else
				{
					//WHAT WAS DONE PRE HACK
					strncat(label,"f",1);
					strncat(scriptstring,label,len+1);
				}
					/*	}
				else
					strncat(scriptstring,label,len);  */
			}
			cursor+=len;
		}
	}
	else
	{
		#if DEBUGTRIG
			printf("non param trig found %s\n",labpt);
		#endif

		sprintf(scriptstring,"%s%s%s%s%s%s%",
				tab,
				"CreateTrigger(strat,&",
				currentstrat,
				statename[RefState].state,	
				",",
				labpt);
	}


	strncat(scriptstring,");\n",3);
	AddLine(scriptstring);
	EvalLine[NEST]=CurrentLine;

	//TREBLE'S NEAR ADDED	
	PushLine();


	if (!EndSubBlock[NEST])
	{

		EndSubBlock[NEST]=1;
		KillMe=NEST;
	}

		PushLine2();


	//STACK THE START OF THE NEXT BLOCK
	//IF AT BASE NEST

	if (!NEST)
	{
		statename[CurrentLabel].indent=Indent-1;
		statename[CurrentLabel].line=CurrentLine;
	}


	//TREBLE'S NEAR

	
	
	//PUSH THE LINE AS THE START OF THE NEXT BLOCK
//	PushLine();
//	PushLine2();
		
}

void RemoveTrigger(int RefState, char *cursor)
{
	char *labpt;
	char scriptstring[256];
	char label[128];
	char tab[32];

	Clear(scriptstring,256);

	StatementBlockStructure(scriptstring,tab);	

	//READ TO THE END OF THE TRIGGER ADDRESS
	Clear(label,128);
	labpt=label;
	cursor = WhiteSpaceRead(cursor);
	ReadToEnd(labpt,cursor);
	cursor+=strlen(label);



	//NOW GET THE TYPE OF TRIGGER
	Clear(label,128);
	cursor = WhiteSpaceRead(cursor);
	ReadToEnd(labpt,cursor);


	sprintf(scriptstring,"%s%s%s%s%s%s%s",
			tab,
			"KillTrigger(strat,&",
			currentstrat,
			statename[RefState].state,	
			",",
			labpt,
			");\n");

	AddLine(scriptstring);
	
	
	EvalLine[NEST]=CurrentLine;

	PushLine();

	if (!EndSubBlock[NEST])
	{

		EndSubBlock[NEST]=1;
		KillMe=NEST;
	}

		PushLine2();


	//STACK THE START OF THE NEXT BLOCK
	//IF AT BASE NEST

	if (!NEST)
	{
		statename[CurrentLabel].indent=Indent-1;
		statename[CurrentLabel].line=CurrentLine;
	}


	//SAVENEST=NEST;
	//NEST++;


	
	//PUSH THE LINE AS THE START OF THE NEXT BLOCK
   //	PushLine();
	PushLine2();		
}

void HoldTrigger(int RefState, char *cursor)
{
	char *labpt;
	char scriptstring[256];
	char label[128];
	char tab[32];

	Clear(scriptstring,256);

	StatementBlockStructure(scriptstring,tab);	

	//READ TO THE END OF THE TRIGGER ADDRESS
	Clear(label,128);
	labpt=label;
	cursor = WhiteSpaceRead(cursor);
	ReadToEnd(labpt,cursor);
	cursor+=strlen(label);



	//NOW GET THE TYPE OF TRIGGER
	Clear(label,128);
	cursor = WhiteSpaceRead(cursor);
	ReadToEnd(labpt,cursor);


	sprintf(scriptstring,"%s%s%s%s%s%s%s",
			tab,
			"HoldTrig(strat,&",
			currentstrat,
			statename[RefState].state,	
			",",
			labpt,
			");\n");

	AddLine(scriptstring);
	
	
	EvalLine[NEST]=CurrentLine;


	
	//PUSH THE LINE AS THE START OF THE NEXT BLOCK
	PushLine();
	PushLine2();		
}

void ReleaseTrigger(int RefState, char *cursor)
{
	char *labpt;
	char scriptstring[256];
	char label[128];
	char tab[32];

	Clear(scriptstring,256);

	StatementBlockStructure(scriptstring,tab);	

	//READ TO THE END OF THE TRIGGER ADDRESS
	Clear(label,128);
	labpt=label;
	cursor = WhiteSpaceRead(cursor);
	ReadToEnd(labpt,cursor);
	cursor+=strlen(label);



	//NOW GET THE TYPE OF TRIGGER
	Clear(label,128);
	cursor = WhiteSpaceRead(cursor);
	ReadToEnd(labpt,cursor);


	sprintf(scriptstring,"%s%s%s%s%s%s%s",
			tab,
			"ReleaseTrig(strat,&",
			currentstrat,
			statename[RefState].state,	
			",",
			labpt,
			");\n");

	AddLine(scriptstring);
	
	
	EvalLine[NEST]=CurrentLine;


	
	//PUSH THE LINE AS THE START OF THE NEXT BLOCK
	PushLine();
	PushLine2();		
}

void EndTrigger()
{
	char scriptstring[256];
	char tab[32];

	Clear(scriptstring,256);

	StatementBlockStructure(scriptstring,tab);	

	sprintf(scriptstring,"%s%s",
			tab,
			"EndTrigger(strat);\n");

	AddLine(scriptstring);
	
	
	EvalLine[NEST]=CurrentLine;

	
	//PUSH THE LINE AS THE START OF THE NEXT BLOCK
	PushLine();
	PushLine2();
			
}

void FinishTrigger()
{
	char scriptstring[256];
	char tab[32];

	Clear(scriptstring,256);

	StatementBlockStructure(scriptstring,tab);	

	sprintf(scriptstring,"%s%s",
			tab,
			"CompleteTrigger(strat);\n");

	AddLine(scriptstring);
	
	
	EvalLine[NEST]=CurrentLine;

	
	//PUSH THE LINE AS THE START OF THE NEXT BLOCK
	PushLine();
	PushLine2();
			
}


void ResetTrigger()
{
	char scriptstring[256];
	char tab[32];

	Clear(scriptstring,256);

	StatementBlockStructure(scriptstring,tab);	

	sprintf(scriptstring,"%s%s",
			tab,
			"ResetTrig(strat);\n");

	AddLine(scriptstring);
	
	
	EvalLine[NEST]=CurrentLine;

	
	//PUSH THE LINE AS THE START OF THE NEXT BLOCK
	PushLine();
	PushLine2();
			
}


//ENSURES CASE(99) always preceeded by the correct Goto label,
//if not already there, when inserting conditions for the ELSE part

void IfLabelCheck()
{
	char scriptstring[512];
	char tab[32];

	if ((NEST && InIf[NEST-1]) && (ElseMet[NEST]))
	{

		Clear(scriptstring,512);
		Indentation(tab);
		//ADDED FRIDAY

		
		sprintf ( scriptstring,"%s%s%s%d%d%s\n",
				tab,
				"goto ",
				currentstrat,
				NEST,
				elselabel[NEST],
				";");
		AddLine(scriptstring);

		elselabelused[NEST]=1;
		

		
		sprintf (scriptstring,"%s%s%d%d%s\n",
					tab,
					currentstrat,
					NEST,
					stratlabel[NEST],
					":");
		AddLine(scriptstring);
		InIf[NEST-1] = 0;

	}
}

void InsertLoop(char *cursor)
{

	char scriptstring[512];
	char tab[32];
	char value[128];
	int SaveIndent;
	SCRIPTLINE *line;

	Clear(scriptstring,512);
	Clear(value,128);
	EndSubBlock[NEST]=0;


	//GET THE BASE SCRIPT LINE TO USE FOR THE BLOCK

	if (SAVENEST < NEST)
		GetLine();
	else
		GetLine2();

	if ((KillMe==NEST) && (statename[CurrentLabel].stack[NEST]))
	{

		Indentation(tab);
		
		sprintf ( scriptstring,"%s%s%d%s%d%s\n",tab,"strat->stack[",
					NEST,"]=",statename[CurrentLabel].stack[NEST],";");									
		AddLine(scriptstring);

		IfLabelCheck();

		Indent--;
		Indentation(tab);
		KillMe=-1;
	}
	else
		IfLabelCheck();

	if (!NEST)
		CaseInsert();
	else
		if (statename[CurrentLabel].stack[NEST])
			CaseInsert();

	Indentation(tab);


	if (((NEST) && (statename[CurrentLabel].stack[NEST]==0)))
	{
		SwitchHeader();
		CaseInsert();
		SaveIndent=Indent;
		Indent--;
		line=CurrentLine;
	  	SwitchLoopFooter();
		CurrentLine=line;
		Indent=SaveIndent;
	}

	Indentation(tab);
	ParseExpression(value,cursor,CONDITION);


	LOOPNEST = NEST;
	sprintf ( scriptstring,"%s%s%d%s%s%s\n",tab,"if (strat->timer[",
				NEST,
				"]<",value,")");									
	AddLine(scriptstring);

	sprintf ( scriptstring,"%s%s\n",tab,"{");
	AddLine(scriptstring);
	Indent++;

	Indentation(tab);

/*	manutd deleted

	sprintf ( scriptstring,"%s%s%d%s\n",tab,"strat->timer[",
				NEST,
				"]++;");									
	AddLine(scriptstring);
  */

	//PUSH THE LINE AS THE START OF THE NEXT BLOCK

	PushLine();

	Indent--;
	Indentation(tab);
	
	 //manutd added 2 lines
  	sprintf ( scriptstring,"%s%s%d%s\n",tab,"strat->timer[",
  				NEST,
  				"]++;");									
  	AddLine(scriptstring);


	
	
	sprintf(	scriptstring,"%s%s\n",tab,"}");
	AddLine(scriptstring);
	sprintf(	scriptstring,"%s%s\n",tab,"else");
	AddLine(scriptstring);
	sprintf(	scriptstring,"%s%s\n",tab,"{");
	AddLine(scriptstring);
	Indent++;
	


	Indentation(tab);
	sprintf ( scriptstring,"%s%s%d%s\n",tab,"strat->timer[",
				NEST,
				"]=0;");									
	AddLine(scriptstring);

	sprintf ( scriptstring,"%s%s%d%s%d%s\n",tab,"strat->stack[",
				NEST,
				"]=",statename[CurrentLabel].stack[NEST],";");									
	AddLine(scriptstring);
	Indent--;
	Indentation(tab);
	sprintf(	scriptstring,"%s%s\n",tab,"}");
	AddLine(scriptstring);
	Indent--;

	PushLine2();

	//STACK THE START OF THE NEXT BLOCK
	//IF AT BASE NEST

	if (!NEST)
	{
		statename[CurrentLabel].indent=Indent-1;
		statename[CurrentLabel].line=CurrentLine;
	}

	
	EvalLine[NEST]='\0';
	SAVENEST=NEST;
	NEST++;



}

void InsertLoopImmediate(char *cursor)
{

	char scriptstring[512];
	char tab[32];
	char value[128];
	int SaveIndent;
	SCRIPTLINE *line;

	Clear(scriptstring,512);
	Clear(value,128);
	EndSubBlock[NEST]=0;


	//GET THE BASE SCRIPT LINE TO USE FOR THE BLOCK

	if (SAVENEST < NEST)
		GetLine();
	else
		GetLine2();


	if ((KillMe==NEST) && (statename[CurrentLabel].stack[NEST]))
	{

		Indentation(tab);
		
		sprintf ( scriptstring,"%s%s%d%s%d%s\n",tab,"strat->stack[",
					NEST,"]=",statename[CurrentLabel].stack[NEST],";");									
		AddLine(scriptstring);

		IfLabelCheck();

		Indent--;
		Indentation(tab);
		KillMe=-1;
	}
	else
		IfLabelCheck();

	if (!NEST)
		CaseInsert();
	else
		if (statename[CurrentLabel].stack[NEST])
			CaseInsert();

	Indentation(tab);

	//UTD
	Clear (ImmediateLabel,128);
	sprintf(ImmediateLabel,"%s%s%s%d",tab,
										currentstrat,
										"imm",
										JumpLabel
										);
	sprintf(scriptstring,"%s%s\n",ImmediateLabel,":");
	AddLine(scriptstring);
	JumpLabel ++;

	if (((NEST) && (statename[CurrentLabel].stack[NEST]==0)))
	{
		SwitchHeader();
		CaseInsert();
		SaveIndent=Indent;
		Indent--;
		line=CurrentLine;
	  	SwitchLoopFooter();
		CurrentLine=line;
		Indent=SaveIndent;
	}

	Indentation(tab);
	ParseExpression(value,cursor,CONDITION);

	sprintf ( scriptstring,"%s%s%d%s%s%s\n",tab,"if (strat->timer[",
				NEST,
				"]<",value,")");									
	AddLine(scriptstring);

	sprintf ( scriptstring,"%s%s\n",tab,"{");
	AddLine(scriptstring);
	Indent++;

	Indentation(tab);

	//MANUTD DEL 2 LINES

	/*sprintf ( scriptstring,"%s%s%d%s\n",tab,"strat->timer[",
				NEST,
				"]++;");									
	AddLine(scriptstring);

	  */
	//PUSH THE LINE AS THE START OF THE NEXT BLOCK

	PushLine();

	Indent--;
	Indentation(tab);
	//MANUTD ADD 2 LINES

	sprintf ( scriptstring,"%s%s%d%s\n",tab,"strat->timer[",
				NEST,
				"]++;");									
	AddLine(scriptstring);

	  
	sprintf(	scriptstring,"%s%s\n",tab,"}");
	AddLine(scriptstring);
	sprintf(	scriptstring,"%s%s\n",tab,"else");
	AddLine(scriptstring);
	sprintf(	scriptstring,"%s%s\n",tab,"{");
	AddLine(scriptstring);
	Indent++;
	
	Indentation(tab);
	sprintf ( scriptstring,"%s%s%d%s\n",tab,"strat->timer[",
				NEST,
				"]=0;");									
	AddLine(scriptstring);

	sprintf ( scriptstring,"%s%s%d%s%d%s\n",tab,"strat->stack[",
				NEST,
				"]=",statename[CurrentLabel].stack[NEST],";");									
	AddLine(scriptstring);
	Indent--;
	Indentation(tab);
	sprintf(	scriptstring,"%s%s\n",tab,"}");
	AddLine(scriptstring);
	Indent--;
	//sprintf(scriptstring,"%s%s%s\n",tab,"goto",ImmediateLabel);
	//AddLine(scriptstring);

	PushLine2();

	//STACK THE START OF THE NEXT BLOCK
	//IF AT BASE NEST

	if (!NEST)
	{
		statename[CurrentLabel].indent=Indent-1;
		statename[CurrentLabel].line=CurrentLine;
	}

	
	EvalLine[NEST]='\0';
	SAVENEST=NEST;
	NEST++;
	Imm[NEST]=1;



}

int InsertWhile(char *cursor)
{

	char scriptstring[512];
	char tab[32];
	char value[128];
	int SaveIndent,valid=0;
	SCRIPTLINE *line;

	Clear(scriptstring,512);
	Clear(value,128);

	EndSubBlock[NEST]=0;


	//GET THE BASE SCRIPT LINE TO USE FOR THE BLOCK

	if (SAVENEST < NEST)
		GetLine();
	else
		GetLine2();



	if ((KillMe==NEST) && (statename[CurrentLabel].stack[NEST]))
	{

		Indentation(tab);
		sprintf ( scriptstring,"%s%s%d%s%d%s\n",tab,"strat->stack[",
			NEST,"]=",statename[CurrentLabel].stack[NEST],";");									
		AddLine(scriptstring);

		IfLabelCheck();

		Indent--;
		Indentation(tab);
		KillMe=-1;
	}
	else
		IfLabelCheck();

	if (!NEST)
		CaseInsert();
	else
		if (statename[CurrentLabel].stack[NEST])
			CaseInsert();
	//mpsat
   //	statename[CurrentLabel].stack[NEST]++;

	Indentation(tab);

	if ((NEST) && (statename[CurrentLabel].stack[NEST]==0))
	{
		SwitchHeader();
		CaseInsert();
		SaveIndent=Indent;
		Indent--;
		line=CurrentLine;
	  	SwitchLoopFooter();
		CurrentLine=line;
		Indent=SaveIndent;
	}

	Indentation(tab);


	if (ParseExpression(value,cursor,CONDITION))
	{

		WhileIn=1;

		//MODIFIED IN DECEMBER TO ENSURE WHILES WORK WITHIN LOOPS
		
	  //	if ((NEST) && (LOOPNEST == (NEST-1)))
	  //	{
	  //		sprintf ( scriptstring,"%s%s%d%s\n",tab,"strat->timer[",
	  //				NEST-1,"]--;");									
	  //		AddLine(scriptstring);
	  //	}

		sprintf ( scriptstring,"%s%s%s%s\n",tab,"if (",value,")");
		AddLine(scriptstring);

		sprintf ( scriptstring,"%s%s\n",tab,"{");
		AddLine(scriptstring);

		//added **
	  //	sprintf ( scriptstring,"%s%s%d%s%d%s\n",tab,"strat->stack[",NEST,"]=",statename[CurrentLabel].stack[NEST],";");
	   //	AddLine(scriptstring);
	   	//added **
	   //	sprintf ( scriptstring,"%s%s%d%s\n",tab,"case(",statename[CurrentLabel].stack[NEST],"):");
	   //	AddLine(scriptstring);


		Indent++;

		Indentation(tab);

		//ADDED MONDAY TO ENSURE WHILES WORK WITHIN LOOPS
		
	   //	if (NEST)
	   //	{
	   //		sprintf ( scriptstring,"%s%s%d%s\n",tab,"strat->timer[",
	   //				NEST-1,"]--;");									
	   //		AddLine(scriptstring);
	   //	}
		valid=1;

	}


	//PUSH THE LINE AS THE START OF THE NEXT BLOCK

	PushLine();

	//ADD FRIDAY
	sprintf(	scriptstring,"%s%s\n",tab,"break;\n");
	AddLine(scriptstring);

	Indent--;
	Indentation(tab);
	sprintf(scriptstring,"%s%s\n",tab,"}");
	AddLine(scriptstring);
	sprintf(scriptstring,"%s%s\n",tab,"else");
	AddLine(scriptstring);
	sprintf(scriptstring,"%s%s\n",tab,"{");
	AddLine(scriptstring);
	Indent++;
	Indentation(tab);
	//december addition	whiles within loops

  //	if ((NEST) && (LOOPNEST == (NEST-1)))
  //	{
   //	   		sprintf ( scriptstring,"%s%s%d%s\n",tab,"strat->timer[",
	//   				NEST-1,"]++;");									
	//   		AddLine(scriptstring);


   //	}

	//added** the -1 bit
	sprintf ( scriptstring,"%s%s%d%s%d%s\n",tab,"strat->stack[",
		 //	NEST,"]=",statename[CurrentLabel].stack[NEST]-1,";");									
			NEST,"]=",statename[CurrentLabel].stack[NEST],";");									
	AddLine(scriptstring);
   	//AUGUST TREBLE
   	sprintf ( scriptstring,"%s%s%d%s%s%s\n",tab,"strat->timer[",
   			NEST,"]=","0",";");									
   	AddLine(scriptstring);

	//added**
   //	statename[CurrentLabel].stack[NEST]++;

	Indent--;
	Indentation(tab);
    sprintf(	scriptstring,"%s%s\n",tab,"}");
	AddLine(scriptstring);



	PushLine2();

	//STACK THE START OF THE NEXT BLOCK
	//IF AT BASE NEST

	if (!NEST)
	{
		statename[CurrentLabel].indent=Indent-1;
		statename[CurrentLabel].line=CurrentLine;
	}

	EvalLine[NEST]='\0';

	SAVENEST=NEST;
	NEST++;



	return(valid);

}

int InsertPrint(char *cursor)
{

	char scriptstring[512];
	char tab[32];
	char value[128];
  	int valid=0;
	/*	int SaveIndent,valid=0;
	SCRIPTLINE *line;
	 */
	Clear(scriptstring,512);
	Clear(value,128);

	StatementBlockStructure(scriptstring,tab);	


	//GET THE BASE SCRIPT LINE TO USE FOR THE BLOCK
  /*	if (SAVENEST < NEST)
		GetLine();
	else
		GetLine2();


	if (!EndSubBlock[NEST])
	{

		if (!NEST)
			CaseInsert();
		else
			if (statename[CurrentLabel].stack[NEST])
				CaseInsert();
	}
	IfLabelCheck();

	if ((NEST) && (statename[CurrentLabel].stack[NEST]==0))
	{
		SwitchHeader();
		CaseInsert();
		SaveIndent=Indent;
		Indent--;
		line=CurrentLine;
	  	SwitchCondFooter();
		CurrentLine=line;
		Indent=SaveIndent;
	}

	Indentation(tab);
	*/

	/*if (SAVENEST < NEST)
		GetLine();
	else
		GetLine2();

	//GOOOEY

	if (!EndSubBlock[NEST])
	{
		if (!NEST)
			CaseInsert();
		else
			if (statename[CurrentLabel].stack[NEST])
				CaseInsert();
	}

	IfLabelCheck();

	if ((NEST) && (statename[CurrentLabel].stack[NEST]==0))
	{
		SwitchHeader();
		CaseInsert();
		SaveIndent=Indent;
		Indent--;
		line=CurrentLine;
	  	SwitchCondFooter();
		CurrentLine=line;
		Indent=SaveIndent;
	}

	Indentation(tab);
*/


	if (ParseExpression(value,cursor,CONDITION))
	{

		sprintf ( scriptstring,"%s%s%d%s\n",
					 tab,"printf(\"%d\\n\",strat->timer[",NEST,"]);");
		AddLine(scriptstring);


		valid=1;

	}



	//PUSH THE LINE AS THE START OF THE NEXT BLOCK
	PushLine();


	if (!EndSubBlock[NEST])
	{

		EndSubBlock[NEST]=1;
		KillMe=NEST;
	}

		PushLine2();


	//STACK THE START OF THE NEXT BLOCK
	//IF AT BASE NEST

	if (!NEST)
	{
		statename[CurrentLabel].indent=Indent-1;
		statename[CurrentLabel].line=CurrentLine;
	}

	//SAVENEST=NEST;
	//NEST++;

	return(valid);

}

void StatementBlockStructure(char *scriptstring, char *tab)
{
  	int SaveIndent,valid=0;
	SCRIPTLINE *line;

		//GET THE BASE SCRIPT LINE TO USE FOR THE BLOCK

	if (SAVENEST < NEST)
		GetLine();
	else
		GetLine2();


	if (!EndSubBlock[NEST])
	{

		if (!NEST)
			CaseInsert();
		else
			if (statename[CurrentLabel].stack[NEST])
				CaseInsert();
	}
	IfLabelCheck();

	if ((NEST) && (statename[CurrentLabel].stack[NEST]==0))
	{
		SwitchHeader();
		CaseInsert();
		SaveIndent=Indent;
		Indent--;
		line=CurrentLine;
	  	SwitchCondFooter();
		CurrentLine=line;
		Indent=SaveIndent;
	}

	Indentation(tab);

}


void	ParseSpawn(char *cursor,int mode)
{
	char *labpt;
	char label[256];
	char scriptstring[256];
	char tab[32];
	int	 entry,len;

	Clear(scriptstring,256);
	Clear(tab,32);
	Clear(label,256);

	StatementBlockStructure(scriptstring,tab);	
	
	
	cursor=WhiteSpaceRead(cursor);

	ReadToEndUpper(label,cursor);

	entry = GetStratEntry(label);


	cursor+=strlen(label);


	//No Params, then just output straight spawn
	if (!(cursor=WhiteSpaceRead(cursor)))
	{
		switch (mode)
		{
			case(BOID):
				sprintf(scriptstring,"%s%s%d%s\n",tab,"BoidSpawn(strat,",
										entry, ");");
				break;
			case(MFIRE):
				sprintf(scriptstring,"%s%s%d%s\n",tab,"MissileSpawn(strat,",
										entry, ");");
				break;
			case(CHILD):
				sprintf(scriptstring,"%s%s%d%s\n",tab,"Spawn(strat,",
												entry, ");");
				break;
			case(CHILDFAST):
				sprintf(scriptstring,"%s%s%d%s\n",tab,"FastSpawn(strat,",
												entry, ");");
				break;
			case(IND):
				sprintf(scriptstring,"%s%s%d%s\n",tab,"SpawnAbs(strat,",
												entry, ");");
				break;
		}


		AddLine(scriptstring);
	}
	else
	{

		switch (mode)
		{
			case(BOID):
				sprintf(scriptstring,"%s%s%d",tab,"BoidSpawn(strat,",entry);
				break;
			case(MFIRE):
				sprintf(scriptstring,"%s%s%d",tab,"MissileSpawn(strat,",entry);
				break;
			case(CHILD):
				sprintf(scriptstring,"%s%s%d",tab,"Spawn(strat,",entry);
				break;
			case(CHILDFAST):
				sprintf(scriptstring,"%s%s%d",tab,"FastSpawn(strat,",entry);
				break;
			case(IND):
				sprintf(scriptstring,"%s%s%d",tab,"SpawnAbs(strat,",entry);
				break;
		}	
		

			
		Clear(label,256);
		strncat(label,",",1);
		labpt=label+1;
		len=strlen(label);
		strncat(scriptstring,label,len);

		Clear(label,256);
		cursor=WhiteSpaceRead(cursor);
			
		ParseExpression(label,cursor,EVALUATION);
		len=strlen(label);
		strncat(scriptstring,label,len);

		strncat(scriptstring,");\n",3);
		AddLine(scriptstring);
	}
 
	 EvalLine[NEST]=CurrentLine;

   	//PUSH THE LINE AS THE START OF THE NEXT BLOCK
	PushLine();


	if (!EndSubBlock[NEST])
	{

		EndSubBlock[NEST]=1;
		KillMe=NEST;
	}

		PushLine2();


	//STACK THE START OF THE NEXT BLOCK
	//IF AT BASE NEST

	if (!NEST)
	{
		statename[CurrentLabel].indent=Indent-1;
		statename[CurrentLabel].line=CurrentLine;
	}

  
}


void	ParseExplode(char *cursor)
{
	char label[128];
	char scriptstring[128];
	char tab[32];
	int	 entry;

	Clear(scriptstring,128);
	Clear(tab,32);
	Clear(label,64);

	StatementBlockStructure(scriptstring,tab);	
	
	
	cursor=WhiteSpaceRead(cursor);

	ReadToEndUpper(label,cursor);

	entry = GetStratEntry(label);


	cursor+=strlen(label);


	//No Params, then just output straight spawn
	if (!(cursor=WhiteSpaceRead(cursor)))
	{
		sprintf(scriptstring,"%s%s%d%s\n",tab,"ExplodeMe(strat,",
							entry, ");");


		AddLine(scriptstring);

	/*sprintf(scriptstring,"%s%s\n",
				tab,
				"return;"
			  );

	AddLine(scriptstring);
	  */
	//	sprintf(scriptstring,"%s%s\n",tab,"return;");						entry, ");");


	 //	AddLine(scriptstring);
	}
 
	EvalLine[NEST]=CurrentLine;

   	//PUSH THE LINE AS THE START OF THE NEXT BLOCK
	PushLine();


	if (!EndSubBlock[NEST])
	{

		EndSubBlock[NEST]=1;
		KillMe=NEST;
	}

		PushLine2();


	//STACK THE START OF THE NEXT BLOCK
	//IF AT BASE NEST

	if (!NEST)
	{
		statename[CurrentLabel].indent=Indent-1;
		statename[CurrentLabel].line=CurrentLine;
	}

  
}




int InsertEvaluation(char *cursor)
{

	char scriptstring[512];
	char tab[32];
	char value[512];
	int	 valid=0;

//	return;

	Clear(scriptstring,512);
	Clear(value,512);



	//GET THE BASE SCRIPT LINE TO USE FOR THE BLOCK

	StatementBlockStructure(scriptstring,tab);



	if (ParseExpression(value,cursor,EVALUATE))
	{

		if (strlen(value))
		{
			sprintf ( scriptstring,"%s%s%s\n",
						 tab,value,";");
			AddLine(scriptstring);
		}


		EvalLine[NEST]=CurrentLine;

		//statename[CurrentLabel].line=CurrentLine;
		valid=1;

	}


	//PUSH THE LINE AS THE START OF THE NEXT BLOCK
	PushLine();


	if (!EndSubBlock[NEST])
	{

		EndSubBlock[NEST]=1;
		KillMe=NEST;
	}

		PushLine2();


	//STACK THE START OF THE NEXT BLOCK
	//IF AT BASE NEST

	if (!NEST)
	{
		statename[CurrentLabel].indent=Indent-1;
		statename[CurrentLabel].line=CurrentLine;
	}


	//SAVENEST=NEST;
	//NEST++;

	return(valid);

}

int InsertIf(char *cursor)
{

	char scriptstring[512];
	char tab[32];
	char value[256];
	int SaveIndent;
	SCRIPTLINE *line;
	int  valid=0;

	if (!(cursor=WhiteSpaceRead(cursor)))
		return (0);

	Clear(scriptstring,512);
	Clear(value,256);
	EndSubBlock[NEST]=0;


	//GET THE BASE SCRIPT LINE TO USE FOR THE BLOCK

	if (SAVENEST < NEST)
		GetLine();
	else
		GetLine2();



	//ADDED BY MATT
	EvalLine[NEST]=0;

	if ((KillMe==NEST) && (statename[CurrentLabel].stack[NEST]))
	{

		Indentation(tab);
		sprintf ( scriptstring,"%s%s%d%s%d%s\n",tab,"strat->stack[",
			NEST,"]=",statename[CurrentLabel].stack[NEST],";");									
		AddLine(scriptstring);

		
		IfLabelCheck();

		Indent--;
		Indentation(tab);
		KillMe=-1;
	}
	else
		IfLabelCheck();



	if (!NEST)
		CaseInsert();
	else
		if (statename[CurrentLabel].stack[NEST])
			CaseInsert();

	if ((NEST) && (statename[CurrentLabel].stack[NEST]==0))
	{
		SwitchHeader();
		CaseInsert();
		SaveIndent=Indent;
		Indent--;
		line=CurrentLine;
	  	SwitchLoopFooter();
		CurrentLine=line;
		Indent=SaveIndent;
	}

	Indentation(tab);


	Indentation(tab);

	NEST++;
	SAVENEST=NEST;


	SwitchHeader();
	CaseInsert();
	SaveIndent=Indent;
	Indent--;
	line=CurrentLine;
  	SwitchIfFooter();

	//MarkIndent[NEST]=Indent;
	//Mark[NEST]=CurrentLine;

	NEST--;
	Indent--;
	PushLine2();
	Indent++;

	 InIf[NEST]=1;


	NEST++;


	CurrentLine=line;
	Indent=SaveIndent;

	Indentation(tab);


	if (ParseExpression(value,cursor,CONDITION))
	{


		Indentation(tab);

		sprintf ( scriptstring,"%s%s%s%s\n",tab,"if (",value,")");
		AddLine(scriptstring);


		Indent++;
		Indentation(tab);

		sprintf ( scriptstring,"%s%s%d%s\n",tab,"strat->stack[",
					NEST,
					"]=1;");									
		AddLine(scriptstring);
		Indent--;
		Indentation(tab);

		sprintf ( scriptstring,"%s%s\n",tab,"else");
		AddLine(scriptstring);
		sprintf ( scriptstring,"%s%s\n",tab,"{");
		AddLine(scriptstring);
		Indent++;
		Indentation(tab);
		sprintf ( scriptstring,"%s%s%d%s\n",tab,"strat->stack[",
					NEST,
					"]=99;");									
		AddLine(scriptstring);

		stratlabel[NEST]++;
		sprintf ( scriptstring,"%s%s%s%d%d%s\n",
					tab,
					"goto ",
					currentstrat,
					NEST,
					stratlabel[NEST],
					";");
		AddLine(scriptstring);



		Indent--;
		Indentation(tab);
		sprintf ( scriptstring,"%s%s\n",tab,"}");
		AddLine(scriptstring);
		Indent--;
		Indentation(tab);
		SAVENEST=NEST;


		PushLine2();


		valid=1;


	}

	return(valid);		
}

void CloseLoop(char *cursor)
{

  	statename[CurrentLabel].stack[NEST]=0;

	NEST--;	
	LOOPNEST = -1;
	SAVENEST=NEST;
	PopLine();


}

void CloseLoopImm(char *cursor)
{

  	statename[CurrentLabel].stack[NEST]=0;
	Imm[NEST]=0;
	NEST--;	
	SAVENEST=NEST;
	PopLine();


}

void CloseWhile(char *cursor)
{
	SCRIPTLINE *SaveLine;
	char scriptstring[128],tab[32];

	Clear(scriptstring,128);

	WhileIn=0;

 	statename[CurrentLabel].stack[NEST]=0;


	//WAS THIS :

	//SWITCH STATE
	//CASE CONDITION
	//  DO EVALUATION
	//	BREAK;
	//DEFAULT : 
	//	RESET WHILE LOOP

	//HACK ENSURES THIS :

	//SWITCH STATE
	//CASE CONDITION
	//	DO EVALUATION	
	//	STATE ++   ********** THIS IS THE FIX
	//	BREAK
	//DEFAULT
	//	RESET WHILE LOOP
	
	
	if (EvalLine[NEST])
	{
		Indentation(tab);
		SaveLine=CurrentLine;
		CurrentLine=EvalLine[NEST];

		sprintf ( scriptstring,"%s%s%d%s%d%s\n",tab,"strat->stack[",
				NEST,
				"]=",statename[CurrentLabel].stack[NEST],";");									
		AddLine(scriptstring);

		
		CurrentLine=SaveLine;

	}

//	statename[CurrentLabel].stack[NEST]++;
	
	NEST--;	
	SAVENEST=NEST;
}

void CloseIf(char *cursor)
{

	GetLine2();
	if (!ElseMet[NEST])
		CaseInsertElse();

	ElseMet[NEST]=0;


	//FRIDAY MAR 13 FIX FOR REMOVING UNUSED VARIABLES
	if (!elselabelused[NEST])
	{
	//	elselabelline[NEST]->next->prev = elselabelline[NEST]->prev;
	//	elselabelline[NEST]->prev->next = elselabelline[NEST]->next;
		CurrentLine->prev->next = elselabelline[NEST]->next;
		free(elselabelline[NEST]); 
	}	
	statename[CurrentLabel].stack[NEST]=0;

	NEST--;	
	SAVENEST=NEST;

	KillMe=-1;

}


void CloseElse(char *cursor)
{

	statename[CurrentLabel].stack[NEST]=99;
	EndSubBlock[NEST]=0;
	ElseMet[NEST]=1;

}


void Clear(char *input,int amount)
{
	// innocent whistling of MattG here...
//	memset (input, 0, amount);
 	
 	int i;
	for (i=0;i<amount;i++)
		input[i]='\0';
	
}



enum {ANY,STRING,OPERATION,PARAMETER,PARAMETERMAIN,POPERATION,CHECKPARAMS,QUIT};

char *SymbolGet(char *symbol,char *in)
{
	char	next[1]={"\n"};

  	Clear(symbol,32);
	

	if (!(in=WhiteSpaceRead(in)))
		return(NULL);

	//RETURN IF CARRIAGE RET AFTER SPACES

	if (strncmp(in,"\n",1))
	{

		next[0]=*in;

		if ((!isalnum(next[0])) && (strncmp(next,".",1) && (strncmp(next,"_",1)))) 
		{
			*symbol=next[0];
			return(in);
		}

		while ((isalnum(next[0])) || (!strncmp(next,".",1)) || (!strncmp(next,"_",1)))
		{
			*symbol=next[0];
			symbol++;
			in++;
			next[0] = *in;

		}
		
		return(in);
	}
	else
		return(NULL);
}



//CHECK CODES AGAINST PREDEFINED

#define CODE 12
char codes[CODE][3]={"+","-","*","|","/","<",">","=","!","&",",","%"};

int iscode(char *code)
{
	int found=0,i;

	for (i=0;i<CODE;i++)
	{

		if (!(strncmp(code,codes[i],1)))
		{
			found++;
			break;
		}

	}

	return (found);

}



char *BracketGetOpen(char *symbol,char *in)
{
	char	next[1]={"\n"};
	char  *symstart=symbol;
	int   found=0;

	Clear(symbol,32);


	
	if (!(in=WhiteSpaceRead(in)))
		return(NULL);

	//RETURN IF CARRIAGE RET AFTER SPACES

	//if (strncmp(in,"\n",1))
	if (*in != '\n')
	{


		do
		{
			
				if (*in == '(')
				{

					*symbol=*in;
					symbol++;
					in++;
 					Brackets++;
				}
				else
				if (*in == ')')
				{
					*symbol=*in;
					symbol++;
					in++;
					Brackets--;
				}
				else
				if (!(strncmp(in," ",1)))
				{
					*symbol=*in;
					symbol++;
					in++;
				}

				else
				if (*in == '-')		
				{
					*symbol=*in;
					symbol++;
					in++;
				}
				else
				if (*in == '!')
				{
					*symbol=*in;
					symbol++;
					in++;
				}
				else
					found++;


		} while (!found);
			
		

		return(in);
	}
	else
		return (NULL);
}


char *BracketClose(char *symbol,char *in)
{
	int found=0;
	char	next[1]={"\n"};
	char  *symstart=symbol;


	Clear(symbol,32);


	if (!(in=WhiteSpaceRead(in)))
		return(NULL);

	//RETURN IF CARRIAGE RET AFTER SPACES

	if (strncmp(in,"\n",1))
	{

		//*symbol=NULL;

		do
		{
				//if (!(strncmp(in,")",1)))
				if (*in == ')')
				{
					*symbol=*in;
					symbol++;
					in++;
					found++;
					Brackets--;
				}
				else
				//if (!(strncmp(in," ",1)))
				if (*in == ' ')
				{
					*symbol=*in;
					symbol++;
					in++;
				}


		} while (
				(!(strncmp(in,")",1))) ||
				(!(strncmp(in," ",1))));

			return(in);
	}
	else
		return (NULL);
}

int Params;

int comma;

char *GetComma(char *symbol,char *in)
{
	int found=0;
	char	next[1]={"\n"};
	char  *symstart=symbol;


	Clear(symbol,32);


	if (!(in=WhiteSpaceRead(in)))
		return(NULL);

	//RETURN IF CARRIAGE RET AFTER SPACES

	if (strncmp(in,"\n",1))
	{

		do
		{
				if (!(strncmp(in,",",1)))
				{
					//comma ++;
					comma=0;
					*symbol=*in;
					symbol++;
					in++;
					//Params++;
					found++;
				}
				else
				if (!(strncmp(in," ",1)))
				{
					*symbol=*in;
					symbol++;
					in++;
				}


		} while (
				(!(strncmp(in,",",1))) ||
				(!(strncmp(in," ",1))));

			return(in);
	}
	else
		return (NULL);
}


//JUST READ ( AND SPACES

char *BracketOpen(char *symbol,char *in)
{
	int   found=0;

	char	next[1]={"\n"};
	char  *symstart=symbol;


	Clear(symbol,32);


	if (!(in=WhiteSpaceRead(in)))
		return(NULL);

	//RETURN IF CARRIAGE RET AFTER SPACES

	if (strncmp(in,"\n",1))
	{


		do
		{
				if (!(strncmp(in,"(",1)))
				{

					*symbol=*in;
					symbol++;
					in++;
 					Brackets++;
					found++;
				}
				else
				if (!(strncmp(in," ",1)))
				{
					*symbol=*in;
					symbol++;
					in++;
					found++;
				}


		} while (
				(!(strncmp(in,"(",1))) ||
				(!(strncmp(in," ",1))));



		if (found)
			return(in);
		else
			return(NULL);
	}
	else
		return (NULL);
}

//JUST READ ( SPACES ! AND -

char *ProcLeaders(char *symbol,char *in)
{

	char	next[1]={"\n"};
	char  *symstart=symbol;


	Clear(symbol,32);


	if (!(in=WhiteSpaceRead(in)))
		return(NULL);

	//RETURN IF CARRIAGE RET AFTER SPACES

	
	if (strncmp(in,"\n",1))
	{
		do
		{
			if (!(strncmp(in,"(",1)))
			{
				*symbol=*in;
				symbol++;
				in++;
				Brackets++;
			}
			else
			if (!(strncmp(in," ",1)))
			{
				*symbol=*in;
				symbol++;
		
				in++;
			}
			else
			if (!(strncmp(in,"-",1)))
			{
				*symbol=*in;
				symbol++;
				in++;
			}
			else
			if (!(strncmp(in,"!",1)))
			{
				*symbol=*in;
				symbol++;
				in++;
			}

			
		} while (
				(!(strncmp(in,"(",1))) ||
				(!(strncmp(in,"!",1))) ||
				(!(strncmp(in,"-",1))) ||
				(!(strncmp(in," ",1))));

			return(in);
	}
	else
		return (NULL);
}

char *LeadingOper(char *symbol,char *in)
{
	char	next[1]={"\n"};
	char  *symstart=symbol;
	int found=0;

	
	Clear(symbol,32);

	if (!(in=WhiteSpaceRead(in)))
		return(NULL);

	//RETURN IF CARRIAGE RET AFTER SPACES

	if (*in != '\n')
	{

		

		do
		{
				if (*in == '(')
				//if (!strncmp(in,"(",1))	
				{

					*symbol=*in;
					symbol++;
					in++;
 					Brackets++;
				}
				else
				//if (!strncmp(in,")",1))
				if (*in == ')')
				{
					*symbol=*in;
					symbol++;
					in++;
					Brackets--;
				}
				else
				//if (!strncmp(in," ",1))
				if (*in == ' ')
				{
					*symbol=*in;
					symbol++;
					in++;
				}
				else
					found=1;

		} while (!found);

				

		//if (!(strncmp(in,"\n",1)))
		//	return(NULL);
		//else
			return(in);
	}
	else
		return (NULL);
}


char *OperationGet(char *symbol,char *in)
{
	char	next[1];

	Clear(symbol,32);



	if (!(in=WhiteSpaceRead(in)))
		return(NULL);


	//RETURN IF CARRIAGE RET AFTER SPACES

	if (strncmp(in,"\n",1))
	{


		next[0]=*in;


		//IF NOT AN ALPHABET AND NOT A CODE QUIT
		//COS' IT'S PROBABLY A BRACKET

		if ((!isalpha(next[0])) && (!iscode(next)))
		{
			
			*symbol=next[0];
			return(in);
		}



		while ((isalpha(next[0])) || (iscode(next)))
		{
			*symbol=*in;
			in++;
			symbol++;
			next[0]=*in;
		}


		return(in);

	}
	else
		return (NULL);
}



char *Copy (char *out, char *in, int size)
{
	int i;

	for (i=0;i<size;i++)
	{
		*out=*in;
		out++;
		in++;
	}
	return(out);
}


char *IntVarInsert(char *out,int varnum)
{
	char string[64];
	char *strpt=string;
	unsigned int len,i;

	Clear(string,64);

	sprintf(string,"%s%d%s ","strat->IVar[",varnum,"]");

	len=strlen(string);

	for (i=0;i<len;i++)
	{	
		*out=*strpt;
		out++;
		strpt++;
	}

	return(out);
}

char *FloatVarInsert(char *out,int varnum)
{
	char string[64];
	char *strpt=string;
	unsigned int len,i;

	Clear(string,64);

	sprintf(string,"%s%d%s ","strat->FVar[",varnum,"]");

	len=strlen(string);

	for (i=0;i<len;i++)
	{	
		*out=*strpt;
		out++;
		strpt++;
	}

	return(out);	
}


int ParseExpression(char *out, char *in,int Mode)
{
	Uint32 ExpState=ANY;
	Uint32 SaveState=ANY;
	char  symbol[512];
	char  *Proc,*ProcMain;
	int code=0,i,ProcBracks,ExpParams,ExpStrat,SaveParams,SaveBrackets;
	int res;
	char stratsym[]={"strat,"};
	char floatapp[]={"f"};

	for (i=0;i<512;i++)
		symbol[i]='\0';

	comma= 0;
	do
	{
		
		switch (ExpState)
		{
			case (ANY):


				SaveState = ANY;

				
				if (!(in=BracketGetOpen(symbol,in))) //READ HEADERS !(-)
						ExpState=QUIT;
				else
				{

					out=Copy(out,symbol,strlen(symbol));
					if (!(in=SymbolGet(symbol,in)))
						ExpState=QUIT;
					else
					{
						if (code=SymbolCheck(symbol)) //CHECK RESERVED WORDS
						{
							code--;
							out=Copy(out,ReservedWords[code].word,strlen(ReservedWords[code].word));
							ExpState=OPERATION;
						}
						else
						{ 
							if (code=IntVarCheck(symbol)) //CHECK AGAINST LOCAL INTS
							{
								out = IntVarInsert(out,code - 1);
								ExpState=OPERATION;
							}
							else
							{
								if (code=FloatVarCheck(symbol))	//CHECK AGAINST LOCAL FLOATS
								{
									out = FloatVarInsert(out,code - 1);
									ExpState=OPERATION;
								}
								else
								{		
									
									if (code=DefinitionCheck(symbol)) //CHECK AGAINST DEFINES
									{
										code--;
										out=Copy(out,Defs[code].value,strlen(Defs[code].value));
										ExpState=OPERATION;	

									}
									else
									{

										if (code=GlobalDefCheck(symbol))
										{
											code--;
									
											out=Copy(out,GDefs[code].value,strlen(GDefs[code].value));
											ExpState=OPERATION;
										
										}
										else
										{
											if (res=NumericCheck(symbol))
											{
												out=Copy(out,symbol,strlen(symbol));
												if (res==FLOAT)
													out=Copy(out,floatapp,strlen(floatapp));
												ExpState=OPERATION;
											}
											else
											{
	
												if (code=ProcedureCheck(symbol))
												{									
													code--;
													out=Copy(out,ProcedureWords[code].symbol,strlen(ProcedureWords[code].symbol));
													ExpState=PARAMETER;
													ExpParams=ProcedureWords[code].params;
				
													ExpStrat=ProcedureWords[code].strat;
													Proc = in;
													ProcMain = ProcedureWords[code].symbol;
												}
												else
												{
													printf("	statement error %s\n",symbol);
													Brackets=1;
													ExpState=QUIT;
												}
											}
										}
									}
								}
							}
						}
					}
				}
				break;

			case (OPERATION):

				Clear (symbol,32);


				if (!(in=LeadingOper(symbol,in)))
					ExpState=QUIT;
				else
				{
					
					 out=Copy(out,symbol,strlen(symbol));

					if (!(in=OperationGet(symbol,in)))
						ExpState=QUIT;
					else						
						if (code=OperationCheck(symbol))
						{
							code--;
							if (Mode==CONDITION)
								out=Copy(out,Operations[code].word,strlen(Operations[code].word));
							else
								out=Copy(out,Operations2[code].word,strlen(Operations2[code].word));
							ExpState=SaveState;
						}
						else
						{
							Brackets = 1;
						
							printf("operation error %s\n",symbol);
							ExpState=QUIT;
						}
				}
				break;




			case (PARAMETER):

				SaveState = PARAMETER;

				//FIRST READ OPENING BRACKET AND FORCE ERROR IF NOT FOUND

				ProcBracks = Brackets;

				//ENSURES NON PARAMETER PROCS ARE CORRECTLY PARSED

	//			if (ExpParams)
	//				Params = 1;
	//			else
					Params = 0;

				comma=0;

				if (!(in=BracketOpen(symbol,in))) 
				{
						Brackets=1;
						ExpState=QUIT;
				}
				else
				{

					
					out=Copy(out,symbol,strlen(symbol));

					if (ExpStrat)
					{	
					

						out=Copy(out,stratsym,strlen(stratsym));
						if (!ExpParams)
							out--;
					}
					ExpState=PARAMETERMAIN;
				}

			case (PARAMETERMAIN):

				SaveState = PARAMETERMAIN;

				
				if (!(in=ProcLeaders(symbol,in))) //READ HEADERS !(-)
							ExpState=CHECKPARAMS;
				else
				{	
				

					if (strlen(symbol))
					out=Copy(out,symbol,strlen(symbol));

 
					SaveBrackets = Brackets;
					in = BracketClose(symbol,in);
					out=Copy(out,symbol,strlen(symbol));
			//utd
				if (((Brackets == ProcBracks))) //  || (!ExpParams))
					
					ExpState=CHECKPARAMS;
					else
					{
			
						if (ExpParams)
						{		
						out=Copy(out,symbol,strlen(symbol));
							
						in=GetComma(symbol,in);
						out=Copy(out,symbol,strlen(symbol));
						}



						if (!(in=SymbolGet(symbol,in)))
						//ExpState=QUIT;
							ExpState=CHECKPARAMS;
						else
						{
									//added

							if (!comma)
							{
								Params ++;
								comma++;
							}

							if (code=SymbolCheck(symbol)) //CHECK RESERVED WORDS
							{
								code--;
								out=Copy(out,ReservedWords[code].word,strlen(ReservedWords[code].word));
								if (!ExpParams)
									ExpState=CHECKPARAMS;
								else
									ExpState=POPERATION;
							}
							else
							{ 
								if (code = FloatVarCheck(symbol))
								{
									out = FloatVarInsert(out,code - 1);
									ExpState=POPERATION;
								}
								else
								{
									if (code = IntVarCheck(symbol))
									{
										out = IntVarInsert(out,code - 1);
										ExpState=POPERATION;
									}
									else
									{
																
										if (code=DefinitionCheck(symbol)) //CHECK AGAINST DEFINES
										{
											code--;
											out=Copy(out,Defs[code].value,strlen(Defs[code].value));
											if (!ExpParams)
												ExpState=CHECKPARAMS;
											else
												ExpState=POPERATION;
										}
										else
										{
											if (code=GlobalDefCheck(symbol))
											{
												code--;
									
												out=Copy(out,GDefs[code].value,strlen(GDefs[code].value));
												if (!ExpParams)
													ExpState=CHECKPARAMS;
												else
 													ExpState=POPERATION;
										
											}
											else
											{
												if (res=NumericCheck(symbol))
												{
													out=Copy(out,symbol,strlen(symbol));
							
													if (res==FLOAT)
														out=Copy(out,floatapp,strlen(floatapp));

													if (!ExpParams)
														ExpState=CHECKPARAMS;
													else
														ExpState=POPERATION;
												}
												else
												{											
													Brackets=1;
													ExpState=QUIT;
													//ExpState=CHECKPARAMS;
												}
											}
										}
									}
								}
							}
						}
					}
				}
				break;

			case (POPERATION):

				Clear (symbol,32);

				SaveBrackets = Brackets;
				
				in = BracketClose(symbol,in);
				
				out=Copy(out,symbol,strlen(symbol));

			  	//thursday change  
				if (((Brackets == ProcBracks))) //  || (!ExpParams))
					
					ExpState=CHECKPARAMS;
				
				//thursday took out
				/*if ((Brackets != SaveBrackets) )
				{
						if (Brackets == ProcBracks)
						{
							
							ExpState=CHECKPARAMS;
						}
						else
						{

							ExpState=PARAMETERMAIN;
						}
				} */
				else
				{

						SaveParams = Params;
						in=GetComma(symbol,in);
						out=Copy(out,symbol,strlen(symbol));
						//if (Params != SaveParams)
						if (!comma)
							ExpState=PARAMETERMAIN;
						else
						{

							if (!(in=OperationGet(symbol,in)))
								ExpState=CHECKPARAMS;
							else
								if (code=OperationCheck(symbol))
								{
									code--;
									if (Mode==CONDITION)
										out=Copy(out,Operations[code].word,strlen(Operations[code].word));
									else
										out=Copy(out,Operations2[code].word,strlen(Operations2[code].word));
									ExpState=SaveState;
								}
								else
								{
									printf("poperation error %s\n",symbol);
									Brackets = 1;
									ExpState=QUIT;
								}
						}
				}
				break;

			case (CHECKPARAMS):

				if (Params != ExpParams)				
				{
					printf("Parameter Error %s%s Expecting %d Params found %d\n",ProcMain,Proc,ExpParams,Params);
					Brackets = 1;
					ExpState = QUIT;
				}
				else
				{
					SaveState = ANY;
					ExpState=OPERATION;
				}
			case (QUIT):
				break;
			}

	}
	while (ExpState!=QUIT);

	if (Brackets)
		return(0);
	else
		return(1);

}



void InsertValue(char *cursor)
{
	int  i=0;
	char buffer[32];
	char scriptstring[128];
	char tab[32];
	

	for (i=0;i<32;i++)
		buffer[i]='\0';


	ReadToEnd(buffer,cursor);



	Indentation(tab);

  	sprintf(scriptstring,"%s%s%s%s\n",tab,"for (",buffer,")");
	AddLine(scriptstring);

	statename[CurrentLabel].line=CurrentLine;
  
}




void Indentation(char *tab)
{
	int i;
	for (i=0;i<16;i++)
 	tab[i]='\0';



	for (i=0;i<Indent;i++)
	{
		sprintf(tab,"\t");
		tab++;
	}

}

void PushLine()
{
	lines[line]=CurrentLine;
	Indents[line]=Indent;

	line++;
}

void PushLine2()
{

   //	lines2[line2]=CurrentLine;
   //	Indents2[line2]=Indent;

	lines2[NEST]=CurrentLine;
	Indents2[NEST]=Indent;

  
	line2++;
}


void GetLine()
{
	if (line)
	{
		CurrentLine=lines[line-1];
		Indent=Indents[line-1];
	 	//dodgy bit
		line--;
	}
	else
	{
		Indent=statename[CurrentLabel].indent;
		CurrentLine=statename[CurrentLabel].line;
	}
//	line --;
}
void GetLine2()
{
	if (line2)
	{
	  // 	CurrentLine=lines2[line2-1];
	 //  	Indent=Indents2[line2-1];
		CurrentLine=lines2[NEST];
		Indent=Indents2[NEST];
	   //	line2--;
		//PopLine2();
	}
	else
	{
		Indent=statename[CurrentLabel].indent;
		CurrentLine=statename[CurrentLabel].line;
	}
  //	line2--;
}

int PopLine()
{
	if (line)
	{
		line--;
		return(1);
	}
	else
		return(0);
}

int PopLine2()
{
	if (line2)
	{
		line2--;
		return(1);
	}
	else
		return(0);
}
