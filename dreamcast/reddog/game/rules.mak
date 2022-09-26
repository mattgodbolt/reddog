_PAL		= 1

# Definitions - directories
!IFDEF _ARTIST
DREAMCAST	= p:\DREAMCAST
!IF EXIST(C:\DREAMCAST\SHC\BIN\SHC.EXE)
SHC			= C:\DREAMCAST\SHC
!ELSE
SHC			= $(DREAMCAST)\SHC
!ENDIF
!ELSE
DREAMCAST	= c:\DREAMCAST
SHC			= $(DREAMCAST)\shc
!ENDIF
LBASE		= $(DREAMCAST)\SHINOBI\LIB
INCLUDE		= $(DREAMCAST)\SHINOBI\INCLUDE \
			  .. \
			  ..\..

!IFDEF _PAL
INCLUDE		= $(DREAMCAST)\SHINOBI\PAL\ $(INCLUDE)
!ENDIF

# Definitions - executables
CC			=	$(SHC)\bin\shc
CPP			=	$(SHC)\bin\shcpp
LD			=	$(SHC)\bin\lnk
AS			=	$(SHC)\bin\asmsh
ELFCONV		=	$(SHC)\bin\cnvs
BINCONV		=	$(SHC)\bin\s2bin
TOELF		=	$(DREAMCAST)\bin\dwfcnv
MOVE		=	move
AR			=	$(SHC)\bin\lbr
DEL			=	del
COPY        =   copy
MKDEP		=	p:\utils\mkdep.exe /Q /I.. /I..\..
SED			=	p:\utils\sed.exe
ROF2BIN		=	$(DREAMCAST)\bin\rof2bin
BINADJ		=	$(DREAMCAST)\bin\binadj
!IFDEF _ARTIST
ARTIST_THINGS = C:\STRATS\PCODE\STRAT.H C:\STRATS\PCODE\BASICREDDOG.H
MKDEP		= $(MKDEP) /IC:\STRATS\PCODE /IC:\STRATS\PCODE\SHCIOP
!ELSE
ARTIST_THINGS =
!ENDIF

# Defintions - flags
CFLAGS		=	-cpu=sh4 -division=cpu -endian=little -fpu=single -pic=0 -macsave=0 -string=const -comment=nonest -sjis 
CFLAGS		=	$(CFLAGS) -section=p=P,c=C,d=D,b=B -extra=a=400 -show=obj,source,expansion,w=80,l=0 -align16
CFLAGS		=	$(CFLAGS) -inline=8 -nestinline=8 -loop -G
!IFDEF _WARNING
CFLAGS		=	$(CFLAGS) -message
!ENDIF
# C Defines
DEFINES		=	__SHC__,__SET4__,__SET5__,_CLX_,_CLX2_,__KATANA__,_STRICT_UNION_
!IFDEF _PAL
DEFINES		=	$(DEFINES),_PAL
!ENDIF

!IF EXIST(C:\DREAMCAST\artist.inc)
DEFINES		= $(DEFINES),LEVEL_HACK
!ENDIF

!IFDEF _DEBUG
# Debug flags lurk herein :
CFLAGS		= $(CFLAGS) -debug 

# Artists get a bastardised debug
!IFDEF _ARTIST
CFLAGS		= $(CFLAGS) -optimize=1 -speed
CFLAGS		= $(CFLAGS) -include=c:\strats\pcode,c:\strats\pcode\shciop
!ELSE
CFLAGS		= $(CFLAGS) -optimize=0
!ENDIF

DEFINES		= $(DEFINES),_DEBUG
LDFLAGS		=	-debug
ASFLAGS		=	-private -cpu=sh4 -endian=little -debug=DWARF -define=DEBUG=on
!ELSEIFDEF _PROFILE
# Profile flags lurk herein :
CFLAGS		= $(CFLAGS) -optimize=1 -speed -debug
DEFINES		= $(DEFINES),NDEBUG,_PROFILE
LDFLAGS		=	-debug
ASFLAGS		=	-private -cpu=sh4 -endian=little -debug=DWARF -define=DEBUG=on
!ELSE
# Release flags lurk herein :
CFLAGS		= $(CFLAGS) -code=asmcode -optimize=1 -speed
DEFINES		= $(DEFINES),NDEBUG,_RELEASE,INLINEASM
LDFLAGS		=	
ASFLAGS		=	-private -cpu=sh4 -endian=little
_RELEASE	=
!ENDIF

# Artist defines
!IFDEF _ARTIST
DEFINES		= $(DEFINES),_ARTIST
!ENDIF

# Add defines
CFLAGS		= $(CFLAGS) -define=$(DEFINES)

# Default libraries and objects to link with
!IFDEF _PROFILE
DEF_LIBS	= $(LBASE)\SHIN_PROF.LIB
AMALGAMLIB	= C:\DREAMCAST\Sega_Prof.lib
!ELSE
DEF_LIBS	= $(LBASE)\SHINOBI.LIB
!IFDEF _ARTIST
AMALGAMLIB	= D:\DREAMCAST\Sega.lib
!ELSE
AMALGAMLIB	= C:\DREAMCAST\Sega.lib
!ENDIF
!ENDIF
DEF_LIBS	= $(DEF_LIBS)\
			  $(LBASE)\SG_SD.LIB\
			  $(LBASE)\AUDIO64.LIB
!IFNDEF _PAL
DEF_LIBS	= $(DEF_LIBS) $(LBASE)\NINJA.LIB
!ELSE
!IFDEF _ARTIST
DEF_LIBS	= $(DEF_LIBS) P:\DREAMCAST\SHINOBI\PAL\KAMUI.LIB
!ELSE
DEF_LIBS	= $(DEF_LIBS) c:\DREAMCAST\SHINOBI\PAL\KAMUI.LIB
!ENDIF
!ENDIF
DEF_OBJS	= $(LBASE)\strt1.obj $(LBASE)\strt2.obj
IP_OBJS		= $(LBASE)\systemid.obj\
			$(LBASE)\toc.obj\
			$(LBASE)\sg_sec.obj\
			$(LBASE)\sg_arejp.obj\
			$(LBASE)\sg_areus.obj\
			$(LBASE)\sg_areec.obj\
			$(LBASE)\sg_are00.obj\
			$(LBASE)\sg_are01.obj\
			$(LBASE)\sg_are02.obj\
			$(LBASE)\sg_are03.obj\
			$(LBASE)\sg_are04.obj\
			$(LBASE)\sg_ini.obj\
			$(LBASE)\aip.obj\
			$(LBASE)\zero.obj

# Temporary files to delete
TEMP_FILES	= Makefile.dep *.tm* *.mot *.dwf mkdep.log *.lct *.ctr 

TOELFFLAGS	=
MOVEFLAGS	=	

# Suffixes
.SUFFIXES : .c .src .obj .lib .elf .bin .gbx .pvr .bmp

# Make rules:

# Compiling a source file
.c.obj:
	@SET SHC_INC=$(INCLUDE: =,),$(SHC)\include
	@SET SHC_LIB=$(SHC)\bin
	@SET SHC_TMP=C:
	@SET HLNK_LIBRARY1=$(SHC)\lib\SH4NLFZZ.LIB
!IF DEFINED(_RELEASE)
	$(CC) $< -ob=$*.src -sub=<<shc.sub
!ELSE
	$(CC) $< -ob=$@ -sub=<<shc.sub
!ENDIF
	$(CFLAGS)
<<KEEP
!IF DEFINED(_RELEASE)
	$(AS) $*.src -O=$*.obj $(ASFLAGS)
!ELSE
	$(TOELF) $(TOELFFLAGS) -sysrof=$*.o $*.obj > NUL
	copy $*.o $*.obj > NUL
	$(DEL) $*.o > NUL
!ENDIF

.cpp.obj:
	@SET SHC_INC=$(INCLUDE: =,),$(SHC)\include
	@SET SHC_LIB=$(SHC)\bin
	@SET SHC_TMP=C:
	@SET HLNK_LIBRARY1=$(SHC)\lib\SH4NLFZZ.LIB
!IF DEFINED(_RELEASE)
	$(CPP) $< -ob=$*.src -sub=<<shc.sub
!ELSE
	$(CPP) $< -ob=$@ -sub=<<shc.sub
!ENDIF
	$(CFLAGS) -B
<<KEEP
!IF DEFINED(_RELEASE)
	$(AS) $*.src -O=$*.obj $(ASFLAGS)
!ELSE
	$(TOELF) $(TOELFFLAGS) -browser=cppdtb\$*.dtb -sysrof=$*.o $*.obj > NUL
	copy $*.o $*.obj > NUL
	$(DEL) $*.o > NUL
!ENDIF

# Assembling a source file
.src.obj:
	$(AS) $< -O=$@ $(ASFLAGS) 

# Generating a GBIX
.pvr.h:
	p:\utils\getgbix.exe $<

# Dependency file name
!IFDEF _ARTIST
GAMEDEP		=	D:\DREAMCAST\GAME.DEP
RENDERDEP	=	D:\DREAMCAST\RENDER.DEP
!ELSE
GAMEDEP		=	MAKEFILE.DEP 
RENDERDEP	=	MAKEFILE.DEP
!ENDIF

C:\STRATS\PCODE\STRAT.H: P:\DREAMCAST\REDDOG\_STRAT.H
	copy P:\DREAMCAST\REDDOG\_STRAT.H C:\STRATS\PCODE\STRAT.H
C:\STRATS\PCODE\BASICREDDOG.H: P:\DREAMCAST\REDDOG\_BASICREDDOG.H
	copy P:\DREAMCAST\REDDOG\_BASICREDDOG.H C:\STRATS\PCODE\BASICREDDOG.H
