target:		RedDog.elf
!INCLUDE Rules.mak

CFILES		= SbInit.c Memory.c RDDebug.c View.c Stream.c RDInit.c SndUtls.c\
			  Main.c Input.c Game.c FrontEnd.c Loading.c Coll.c Collspec.c Specfx.c Camera.c\
			  command.c pad.c Draw.c DebugDraw.c Process.c Procspec.c Level.c hud.c\
			  Strat.c StratPhysic.c stratway.c Player.c backup.c Weapon.c MPlayer.c Version.c
CPPFILES	= FEBase.cpp Menus.cpp MiscScreens.cpp MPFrontEnd.cpp VMUManager.cpp MPWorkAround.cpp\
			  MPMoreWorkAround.cpp SinglePlayer.cpp FEDistract.cpp

!IFDEF _ARTIST
!INCLUDE C:\STRATS\STRAT.LST
!ELSE
!INCLUDE STRAT.LST
!ENDIF
ASMFILES	= MGMemcpy.src
STRATOBJS	= $(SFILES:.c=.obj)
!IFDEF SFILES2
STRATOBJS2	= $(SFILES2:.c=.obj)
!ENDIF
OBJECTS		= $(CFILES:.c=.obj) $(ASMFILES:.src=.obj) $(CPPFILES:.cpp=.obj)
DWARFFILES	= $(OBJECTS:.obj=.dwf)

!IFDEF _ARTIST
STRATLIB	= C:\STRATS\STRAT.LIB
STRATLIB2	= C:\STRATS\STRAT2.LIB
!ELSE
STRATLIB	= Strat.lib
STRATLIB2	= Strat2.LIB
!ENDIF
LIBS		= Render\Render.lib

!IFNDEF _RELEASE
!IFNDEF _ARTIST
LIBS		= $(LIBS) $(DREAMCAST)\SHC\LIB\LIBCRS.LIB
!ENDIF
!ENDIF

LIBS		= $(LIBS)

all:	RedDog.elf

clean: cleanup all

!IFDEF _DEBUG
!IFDEF _ARTIST
DBG		= _DEBUG= _ARTIST=
!ELSE
DBG		= _DEBUG=
!ENDIF
!ELSEIFDEF _PROFILE
DBG		= _PROFILE=
!ELSE
DBG		= _RELEASE=
!ENDIF

!IFDEF _WARNING
DBG		= $(DBG) _WARNING=
!ENDIF

cleanup:
   	cd Render
	nmake /f Render.mak $(DBG) clean
	cd ..
	$(DEL) *.obj
	$(DEL) RedDog.elf $(TEMP_FILES)

depend:	$(GAMEDEP)

$(GAMEDEP): $(CFILES) $(SFILES)
	$(MKDEP) $(CFILES) $(CPPFILES) $(SFILES) /O$(GAMEDEP) > NUL
!IFDEF _ARTIST
	copy p:\dreamcast\reddog\_strat.h c:\strats\pcode\strat.h
	copy p:\dreamcast\reddog\_BASICRedDog.h c:\strats\pcode\BASICRedDog.h
!ENDIF

Render\Render.lib: dummy
	cd Render
	nmake /f Render.mak $(DBG)
	cd ..

$(STRATLIB): $(STRATOBJS)
#	$(DEL) $*.lib
	$(AR) -sub=<<stratlk.sub
create $*.lib
add $(STRATOBJS: =^
add )
exit
<<KEEP

!IFDEF SFILES2
$(STRATLIB2): $(STRATOBJS2)
#	$(DEL) $*.lib 
	$(AR) -sub=<<stratlk.sub
create $*.lib
add $(STRATOBJS2: =^
add )
exit
<<KEEP
!ENDIF

dummy:

# ---------------- Static dependencies
LOGOFILES	= rd00.bmp rd01.bmp rd02.bmp rd03.bmp rd04.bmp rd05.bmp rd06.bmp rd07.bmp rd08.bmp rd09.bmp rd10.bmp rd11.bmp rd12.bmp rd13.bmp rd14.bmp SaveFail.bmp SaveOk.bmp
LOGOS		= P:\GRAPHICS\VMS\$(LOGOFILES: = P:\GRAPHICS\VMS\)
.bmp.h:
	P:\UTILS\VMSTool.exe $< $@


Input.obj: p:\graphics\vms\RDLogo.bmp $(LOGOS) $(LOGOS:.bmp=.h)
MGMemcpy.obj: Reddog.pre
RedDog.pre: RedDog.inc Localdefs.h
	cl /FILocaldefs.h /EP RedDog.inc > RedDog.pre

FrontEnd.obj: p:\game\texmaps\frontend\ccontrol.h\
				p:\game\texmaps\frontend\vms.h

MULTIPLAYERMAPS = acidbath.h blackice.h blokfort.h circus.h dod(expert).h dod.h dune.h ganymede.h industrialZone.h medieval.h moltern.h ruined.h spacestation.h stormdrain.h wasted.h intelligence.h random.h helterskelter.h
ICONSPRITES = bomb.h electro.h health.h homing.h lockon.h mine.h stealth.h stormer.h ultra.h vulcan.h bigx.h
FEBase.cpp:  p:\game\texmaps\FRONTEND\noise0001.h\
			 p:\game\texmaps\frontend\ccontrol.h\
			 p:\game\texmaps\FRONTEND\SEGALOGO.h\
			 p:\game\texmaps\FRONTEND\ARGOLOGO.h\
			 p:\game\texmaps\frontend\vms.h\
			 p:\game\texmaps\frontend\testzoom.h\
			 p:\game\texmaps\frontend\testcard1.h\
			 p:\game\texmaps\frontend\tank.h\
			 p:\game\texmaps\frontend\tankinside.h\
			 p:\game\texmaps\frontend\titlepage.h\
			 p:\game\texmaps\frontend\pad.h\
			 p:\game\texmaps\frontend\RedDogTech.h\
			 p:\game\texmaps\frontend\MPMaps\$(MULTIPLAYERMAPS: = p:\game\texmaps\frontend\MPMaps\)\
			 p:\game\texmaps\frontend\IconSprites\$(ICONSPRITES: = p:\game\texmaps\frontend\IconSprites\)\
			 p:\game\texmaps\frontend\volcano\volcano01.h\
			 p:\game\texmaps\frontend\ice\ice00.h\
			 p:\game\texmaps\frontend\canyon\canyon00.h\
			 p:\game\texmaps\frontend\city\city01.h\
			 p:\game\texmaps\frontend\hydro\hydro01.h\
			 p:\game\texmaps\frontend\alienbase\Alienbase0001.h\
			 p:\game\texmaps\frontend\challenges\1\one1.h\
			 p:\game\texmaps\frontend\challenges\2\two1.h\
			 p:\game\texmaps\frontend\challenges\3\three1.h\
			 p:\game\texmaps\frontend\challenges\4\four1.h\
			 p:\game\texmaps\frontend\challenges\5\five1.h\
			 p:\game\texmaps\frontend\challenges\6\six1.h\
			 p:\game\texmaps\frontend\challenges\7\seven00.h


# ---------------- Head graphics!
Backup.obj:		GameSave.h
#Backup.obj:		MattHead.h p:\graphics\vms\MattHead.bmp \
#				NikLHead.h p:\graphics\vms\NikLHead.bmp \
#				NikCHead.h p:\graphics\vms\NikCHead.bmp \
#				DaveHead.h p:\graphics\vms\DaveHead.bmp \
#				MatPHead.h p:\graphics\vms\MatPHead.bmp \
#				LeonHead.h p:\graphics\vms\LeonHead.bmp

Game.obj:		p:\game\texmaps\HUD\Multiplaya\AmBomb.h\
				p:\game\texmaps\HUD\Multiplaya\AmKing.h\
				p:\game\texmaps\HUD\Multiplaya\AmFlag.h\
				p:\game\texmaps\HUD\Multiplaya\Predator0000.h\
				p:\game\texmaps\HUD\Multiplaya\Exclam0000.h\
				p:\game\texmaps\HUD\YouWin.h\
				p:\game\texmaps\HUD\NEWHUD\MPHudAll.h\
				p:\game\texmaps\hud\secondaryweapon1.h\
				p:\game\texmaps\hud\secondaryweapon2.h\
				p:\game\texmaps\hud\secondaryweapon3.h\
				p:\game\texmaps\hud\secondaryweapon4.h\
				p:\game\texmaps\hud\secondaryweapon5.h\
				p:\game\texmaps\hud\secondaryweapon6.h\
				p:\game\texmaps\hud\secondaryweapon7.h\
				p:\game\texmaps\hud\secondaryweapon9.h\
				p:\game\texmaps\hud\secondaryweapon10.h\
				p:\game\texmaps\hud\secondaryweapon11.h\
				p:\game\texmaps\hud\secondaryweapon12.h\
				p:\game\texmaps\hud\secondaryweapon13.h

!IFDEF _ARTIST
Game.obj:		dummy
!ENDIF

#MattHead.h: p:\graphics\vms\MattHead.bmp
#	P:\UTILS\VMSTool.exe /c p:\graphics\vms\MattHead.bmp MattHead.h

#NikLHead.h: p:\graphics\vms\NikLHead.bmp
#	P:\UTILS\VMSTool.exe /c p:\graphics\vms\NikLHead.bmp NikLHead.h

#NikCHead.h: p:\graphics\vms\NikCHead.bmp
#	P:\UTILS\VMSTool.exe /c p:\graphics\vms\NikCHead.bmp NikCHead.h

#DaveHead.h: p:\graphics\vms\DaveHead.bmp
#	P:\UTILS\VMSTool.exe /c p:\graphics\vms\DaveHead.bmp DaveHead.h

#MatPHead.h: p:\graphics\vms\MatPHead.bmp
#	P:\UTILS\VMSTool.exe /c p:\graphics\vms\MatPHead.bmp MatPHead.h

#LeonHead.h: p:\graphics\vms\LeonHead.bmp
#	P:\UTILS\VMSTool.exe /c p:\graphics\vms\LeonHead.bmp LeonHead.h

GameSave.h: p:\graphics\vms\gamesave.bmp
	P:\UTILS\VMSTool.exe /c p:\graphics\vms\GameSave.bmp GameSave.h

# Make an amalgam lib
$(AMALGAMLIB): $(DEF_LIBS)
	$(DEL) $(AMALGAMLIB)
	$(AR) -create=$(AMALGAMLIB) -add=$(DEF_LIBS: =,)

# ---------------- End of dependencies

STRATLIBS = $(STRATLIB)
!IFDEF SFILES2
STRATLIBS = $(STRATLIBS) $(STRATLIB2)
!ENDIF

RedDog.elf:	RedDog.pre $(GAMEDEP) $(OBJECTS) $(AMALGAMLIB) $(STRATLIBS) $(LIBS)
	@SET SHC_INC=$(INCLUDE: =,)
	@SET SHC_LIB=$(SHC)\bin
	@SET SHC_TMP=C:
!IFDEF OPTIMIZE_LINK
	@SET PATH=%PATH%;C:\DREAMCAST\SHC\BIN;c:\dreamcast\bin
	C:\DREAMCAST\SHC\BIN\OPTlnkSH.EXE -inf -op=safe -sub=<<ld.sub
INPUT $(DEF_OBJS: =^
INPUT )
INPUT $(IP_OBJS: =^
INPUT )
INPUT $(OBJECTS: =^
INPUT )
LIBRARY $(AMALGAMLIB)
LIBRARY $(STRATLIBS: =^
LIBRARY )
LIBRARY $(DREAMCAST)\shc\lib\SH4NLFZZ.LIB
LIBRARY $(LIBS: =^
LIBRARY )
$(LDFLAGS:-= )
ALIGN_SECTION
ELF
PRINT REDDOG.MAP
FSYMBOL P,PRASTERISERS,PRENDSUPT
FORM A
ENTRY SG_SEC
START IP(8C008000),DSGLH(8C010000)
OUTPUT REDDOG.ELF
EXIT
<<KEEP
!ELSE
	@SET HLNK_LIBRARY1=$(DREAMCAST)\shc\lib\SH4NLFZZ.LIB
	$(LD) $(DEF_OBJS) $(IP_OBJS) $(OBJECTS) -library=$(AMALGAMLIB),$(STRATLIBS: =,),$(LIBS: =,),$(LBASE)\ginsu.lib -align_section\
			$(LDFLAGS)\
			-elf\
			-print=RedDog.map \
			-fsymbol=P,PRASTERISERS,PRENDSUPT \
			-form=a\
			-entry=SG_SEC\
			-start=IP(8C008000),DSGLH(8C010000)\
!IFDEF _ARTIST
			-output=D:\DREAMCAST\RedDog.elf
			p:\utils\symrpl.exe RedDog.map d:\dreamcast\RedDog.elf _kmiGlobalWork _usrGlobalWork
			p:\utils\symrpl.exe RedDog.map d:\dreamcast\RedDog.elf _kmiPurgeMemory _usrPurgeMemory
!ELSE
			-output=RedDog.elf
	p:\utils\symrpl.exe RedDog.map RedDog.elf _kmiGlobalWork _usrGlobalWork
	p:\utils\symrpl.exe RedDog.map RedDog.elf _kmiPurgeMemory _usrPurgeMemory
	P:\utils\incbuild.bat
!ENDIF
!ENDIF

RDDemo.elf:	RedDog.pre $(GAMEDEP) $(OBJECTS) $(AMALGAMLIB) $(STRATLIBS) $(LIBS)
	@SET SHC_INC=$(INCLUDE: =,)
	@SET HLNK_LIBRARY1=$(DREAMCAST)\shc\lib\SH4NLFZZ.LIB
	$(LD) $(DEF_OBJS) $(IP_OBJS) $(OBJECTS) -library=$(AMALGAMLIB),$(STRATLIBS: =,),$(LIBS: =,),$(LBASE)\ginsu.lib,$(DREAMCAST)\SHC\LIB\LIBCRS.LIB -align_section\
			$(LDFLAGS)\
			-elf\
			-print=RDDemo.map \
			-fsymbol=P,PRASTERISERS,PRENDSUPT \
			-form=a\
			-entry=SG_SEC\
			-start=IP(8C008000),DSGLH(8C010000)\
			-output=RDDemo.elf
	p:\utils\symrpl.exe RDDemo.map RDDemo.elf _kmiGlobalWork _usrGlobalWork
	p:\utils\symrpl.exe RDDemo.map RDDemo.elf _kmiPurgeMemory _usrPurgeMemory

# ----------------------- Final build
1st_read.bin: $(GAMEDEP) $(OBJECTS) $(AMALGAMLIB) $(STRATLIBS) $(LIBS)
	@SET SHC_INC=$(INCLUDE: =,)
	@SET HLNK_LIBRARY1=$(DREAMCAST)\shc\lib\SH4NLFZZ.LIB
	$(LD) $(DEF_OBJS) $(OBJECTS) -library=$(AMALGAMLIB),$(STRATLIBS: =,),$(LIBS: =,) -align_section\
			$(LDFLAGS)\
			-print=1st_read.map \
			-fsymbol=P \
			-form=a\
			-start=DSGLH(8C010000)\
			-output=1st_read.abs
	p:\utils\symrpl.exe 1st_read.map 1st_read.abs _kmiGlobalWork _usrGlobalWork
	p:\utils\symrpl.exe 1st_read.map 1st_read.abs _kmiPurgeMemory _usrPurgeMemory
	$(ROF2BIN) -v=0 1st_read.abs

# Dependencies - generated by mkdep :
!IF EXIST($(GAMEDEP))
!INCLUDE $(GAMEDEP)
!ENDIF
