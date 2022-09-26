target:	Render.lib
!INCLUDE ..\Rules.mak


CFILES		= Init.c Matrix.c GlobalOps.c Context.c Model.c ScapeModel.c Map.c Text.c Misc.c\
			  Material.c RendSupt.c Object.c Lighting.c Noise.c SpecialFX.c Profile.c\
			  Shadow.c Animate.c quat.c BumpMap.c Band.c Sparks.c\
			  S_nTex.c S_Tex.c M_Tex.c M_nTex.c\
			  NewM_TexClip.c NewM_nTexClip.c NewM_TexClipU.c NewM_nTexClipU.c
ASMFILES	= Maths.src S_TexA.src S_TexCA.src COSPrep.src RendSuptA.src\
			  LightVerts.src NewM_TexA.src NewM_nTexA.src NewM_TexAU.src NewM_nTexAU.src
ASMOBJS		= $(ASMFILES:.src=.obj)
OBJECTS		= $(CFILES:.c=.obj) $(ASMOBJS)
DWARFFILES	= $(OBJECTS:.obj=.dwf)

all:	Render.lib

clean: cleanup all

cleanup:
	$(DEL) *.obj 
   $(DEL) $(TEMP_FILES)
#	$(DEL) Render.lib $(TEMP_FILES)

depend: $(RENDERDEP)

$(RENDERDEP):	$(CFILES) $(ARTIST_THINGS)
	$(MKDEP) $(CFILES) /O$(RENDERDEP) > NUL

#########################################
# The pre-processed renderers live here #
#########################################

WARNING		= "***** THIS FILE IS GENERATED AUTOMATICALLY - DO NOT EDIT *****"
CWARNING	= "/***** THIS FILE IS GENERATED AUTOMATICALLY - DO NOT EDIT *****/"

NewM_TexA.src: StripRender.src Render.mak
	$(SED) -e 's/!LIT//g' -e 's/!UNL/;/g' -e 's/!UNT/;/g' -e 's/!TEX//g' -e 's/ROUTINE/_texMStripDraw/g' StripRender.src > $*.src

NewM_nTexA.src: StripRender.src Render.mak
	$(SED) -e 's/!LIT//g' -e 's/!UNL/;/g' -e 's/!UNT//g' -e 's/!TEX/;/g' -e 's/ROUTINE/_MStripDraw/g' StripRender.src > $*.src

NewM_TexAU.src: StripRender.src Render.mak
	$(SED) -e 's/!LIT/;/g' -e 's/!UNL//g' -e 's/!UNT/;/g' -e 's/!TEX//g' -e 's/ROUTINE/_texMStripDrawU/g' StripRender.src > $*.src

NewM_nTexAU.src: StripRender.src Render.mak
	$(SED) -e 's/!LIT/;/g' -e 's/!UNL//g' -e 's/!UNT//g' -e 's/!TEX/;/g' -e 's/ROUTINE/_MStripDrawU/g' StripRender.src > $*.src

NewM_TexClip.c: StripRenderClipped.c Render.mak
	$(SED) -e 's/!LIT//g' -e 's/!UNL/\/\//g' -e 's/!UNT/\/\//g' -e 's/!TEX//g' -e 's/ROUTINE/texMStripDrawC/g' StripRenderClipped.c > $*.c

NewM_nTexClip.c: StripRenderClipped.c Render.mak
	$(SED) -e 's/!LIT//g' -e 's/!UNL/\/\//g' -e 's/!UNT//g' -e 's/!TEX/\/\//g' -e 's/ROUTINE/MStripDrawC/g' StripRenderClipped.c > $*.c

NewM_TexClipU.c: StripRenderClipped.c Render.mak
	$(SED) -e 's/!LIT/\/\//g' -e 's/!UNL//g' -e 's/!UNT/\/\//g' -e 's/!TEX//g' -e 's/ROUTINE/texMStripDrawCU/g' StripRenderClipped.c > $*.c

NewM_nTexClipU.c: StripRenderClipped.c Render.mak
	$(SED) -e 's/!LIT/\/\//g' -e 's/!UNL//g' -e 's/!UNT//g' -e 's/!TEX/\/\//g' -e 's/ROUTINE/MStripDrawCU/g' StripRenderClipped.c > $*.c

################################
# The rendering library itself #
################################

Render.lib:	$(RENDERDEP) $(OBJECTS)
	$(COPY) $*.mak $*.lib
   	$(DEL) $*.lib 
	$(AR) -create=$*.lib -add=$(OBJECTS: =,)

##########################################################################################################
# Static dependencies - add text.obj: <pvr name> here for each pvr included (must live in specialfx dir) #
##########################################################################################################
PVR_FILES	=	flares\flare1.h decals\gunhit01.h decals\gunhit01.h decals\gunhit02.h\
				decals\gunhit04.h skidmark01\skidmark02.h\
				LenzFlareComponents\Flz00.h LenzFlareComponents\Flz01.h LenzFlareComponents\Flz02.h LenzFlareComponents\Flz03.h\
				LenzFlareComponents\Flz03a.h LenzFlareComponents\Flz04.h LenzFlareComponents\Flz05.h LenzFlareComponents\Flz06.h\
				LenzFlareComponents\SunFlare.h
HUD_FILES	=	healthcolour1.h healthcolour2.h healthcolour3.h healthFrame1.h healthFrame2.h healthFrame3.h minitank.h\
				sheildcolour.h sheildframe.h adrenalincolour.h numbers.h
FE_FILES	=	hud\chargeup1.h hud\Commander.h hud\DropshipPilot.h hud\Hero.h hud\intelligenceofficer04.h\
				reddog\static1.h hud\health1.h hud\redBar.h hud\orangeBar.h hud\hudcharge2.h\
				hud\SPsecondaryweapon1.h\
				hud\SPsecondaryweapon2.h hud\SPsecondaryweapon3.h hud\SPsecondaryweapon4.h hud\SPsecondaryweapon6.h\
				hud\SPsecondaryweapon9.h hud\SPsecondaryweapon10.h\
				hud\SPsecondaryweapon11.h hud\SPsecondaryweapon12.h hud\SPsecondaryweapon13.h\
				hud\LockOnMessage.h HUD\PANEL_BORDER_NEW.H hud\PANEL_INSIDE_NEW.H HUD\NORMALAIM.h HUD\ShootMe.h\
				HUD\MISSILEAIM1.h HUD\MISSILEAIM2.h HUD\MISSILEAIM3.h HUD\NORMALAIMFIRE.h\
				hud\youwin.h hud\NoTargetMessage.h hud\TargettingMessage.h\
				FrontEnd\PressStart.h FrontEnd\PressStartBlur.h hud\ArgoLogo.h FrontEnd\abet(dave)1.h\
				HUD\Multiplaya\Bomm1.h MISC\SNOW.H HUD\NEWHUD\BossHealth.h HUD\greenBar.h HUD\Charging.h

Text.obj:	P:\GAME\TEXMAPS\SPECIALFX\$(PVR_FILES: = P:\GAME\TEXMAPS\SPECIALFX\) P:\GAME\TEXMAPS\$(FE_FILES: = P:\GAME\TEXMAPS\)\
			P:\GAME\TEXMAPS\HUD\NEWHUD\$(HUD_FILES: = P:\GAME\TEXMAPS\HUD\NEWHUD\)

# All assembler files are dependant on ..\RedDog.pre
$(ASMOBJS) : ..\RedDog.pre
..\RedDog.pre: ..\Localdefs.h
	cd ..
	nmake /fGame.mak RedDog.pre
	cd Render

dummy:

# Dependencies - generated by mkdep :
!IF EXIST($(RENDERDEP))
!INCLUDE $(RENDERDEP)
!ENDIF
