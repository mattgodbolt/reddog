# Microsoft Developer Studio Project File - Name="Render" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

CFG=Render - Win32 Profile
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Render.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Render.mak" CFG="Render - Win32 Profile"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Render - Win32 Release" (based on "Win32 (x86) External Target")
!MESSAGE "Render - Win32 Debug" (based on "Win32 (x86) External Target")
!MESSAGE "Render - Win32 Profile" (based on "Win32 (x86) External Target")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Dreamcast/RedDog/Render", OBBAAAAA"
# PROP Scc_LocalPath "."

!IF  "$(CFG)" == "Render - Win32 Release"

# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Cmd_Line "NMAKE /f Render.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "Render.exe"
# PROP BASE Bsc_Name "Render.bsc"
# PROP BASE Target_Dir ""
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Cmd_Line "NMAKE /f Render.mak"
# PROP Rebuild_Opt "/a clean"
# PROP Target_File "Render.exe"
# PROP Bsc_Name "Render.bsc"
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "Render - Win32 Debug"

# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Cmd_Line "NMAKE /f Render.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "Render.exe"
# PROP BASE Bsc_Name "Render.bsc"
# PROP BASE Target_Dir ""
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Cmd_Line "NMAKE /f Render.mak _DEBUG="
# PROP Rebuild_Opt "/a clean _DEBUG="
# PROP Target_File "Render.lib"
# PROP Bsc_Name "Render.bsc"
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "Render - Win32 Profile"

# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Render__"
# PROP BASE Intermediate_Dir "Render__"
# PROP BASE Cmd_Line "NMAKE /f Render.mak _DEBUG="
# PROP BASE Rebuild_Opt "/a clean _DEBUG="
# PROP BASE Target_File "Render.lib"
# PROP BASE Bsc_Name "Render.bsc"
# PROP BASE Target_Dir ""
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Render__"
# PROP Intermediate_Dir "Render__"
# PROP Cmd_Line "NMAKE /f Render.mak _PROFILE="
# PROP Rebuild_Opt "/a clean _PROFILE="
# PROP Target_File "Render.lib"
# PROP Bsc_Name "Render.bsc"
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "Render - Win32 Release"
# Name "Render - Win32 Debug"
# Name "Render - Win32 Profile"

!IF  "$(CFG)" == "Render - Win32 Release"

!ELSEIF  "$(CFG)" == "Render - Win32 Debug"

!ELSEIF  "$(CFG)" == "Render - Win32 Profile"

!ENDIF 

# Begin Group "Source code"

# PROP Default_Filter "c;src"
# Begin Source File

SOURCE=.\Animate.c
# End Source File
# Begin Source File

SOURCE=.\Band.c
# End Source File
# Begin Source File

SOURCE=.\BumpMap.c
# End Source File
# Begin Source File

SOURCE=.\Context.c
# End Source File
# Begin Source File

SOURCE=.\GlobalOps.c
# End Source File
# Begin Source File

SOURCE=.\Init.c
# End Source File
# Begin Source File

SOURCE=.\Lighting.c
# End Source File
# Begin Source File

SOURCE=.\LightVerts.src
# End Source File
# Begin Source File

SOURCE=.\Map.c
# End Source File
# Begin Source File

SOURCE=.\Material.c
# End Source File
# Begin Source File

SOURCE=.\Maths.src
# End Source File
# Begin Source File

SOURCE=.\Matrix.c
# End Source File
# Begin Source File

SOURCE=.\Misc.c
# End Source File
# Begin Source File

SOURCE=.\Model.c
# End Source File
# Begin Source File

SOURCE=.\Noise.c
# End Source File
# Begin Source File

SOURCE=.\Object.c
# End Source File
# Begin Source File

SOURCE=.\Profile.c
# End Source File
# Begin Source File

SOURCE=.\Quat.c
# End Source File
# Begin Source File

SOURCE=.\RendSupt.c
# End Source File
# Begin Source File

SOURCE=.\RendSuptA.src
# End Source File
# Begin Source File

SOURCE=.\ScapeModel.c
# End Source File
# Begin Source File

SOURCE=.\Shadow.c
# End Source File
# Begin Source File

SOURCE=.\Sparks.c
# End Source File
# Begin Source File

SOURCE=.\SpecialFX.c
# End Source File
# Begin Source File

SOURCE=.\Text.c
# End Source File
# End Group
# Begin Group "Resources"

# PROP Default_Filter "txt;mak"
# Begin Source File

SOURCE=.\LastSection.src
# End Source File
# Begin Source File

SOURCE=.\Makefile.dep
# End Source File
# Begin Source File

SOURCE=.\Render.mak
# End Source File
# End Group
# Begin Group "Header files"

# PROP Default_Filter "h;inc"
# Begin Source File

SOURCE=.\Animate.h
# End Source File
# Begin Source File

SOURCE=.\Bands.h
# End Source File
# Begin Source File

SOURCE=.\Hardware.h
# End Source File
# Begin Source File

SOURCE=.\Internal.h
# End Source File
# Begin Source File

SOURCE=.\Lighting.h
# End Source File
# Begin Source File

SOURCE=.\Map.h
# End Source File
# Begin Source File

SOURCE=.\Material.h
# End Source File
# Begin Source File

SOURCE=.\Matrix.h
# End Source File
# Begin Source File

SOURCE=.\Model.h
# End Source File
# Begin Source File

SOURCE=.\Object.h
# End Source File
# Begin Source File

SOURCE=.\Quat.h
# End Source File
# Begin Source File

SOURCE=.\Rasterisers.h
# End Source File
# Begin Source File

SOURCE=.\Render.h
# End Source File
# Begin Source File

SOURCE=.\ScapeModel.h
# End Source File
# Begin Source File

SOURCE=.\Shadow.h
# End Source File
# Begin Source File

SOURCE=.\Sparks.h
# End Source File
# Begin Source File

SOURCE=.\SpecialFX.h
# End Source File
# End Group
# Begin Group "Renderers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Clip.h
# End Source File
# Begin Source File

SOURCE=.\COSprep.src
# End Source File
# Begin Source File

SOURCE=.\NewM_nTexA.src
# End Source File
# Begin Source File

SOURCE=.\NewM_nTexAU.src
# End Source File
# Begin Source File

SOURCE=.\NewM_nTexClip.c
# End Source File
# Begin Source File

SOURCE=.\NewM_nTexClipU.c
# End Source File
# Begin Source File

SOURCE=.\NewM_TexA.src
# End Source File
# Begin Source File

SOURCE=.\NewM_TexAU.src
# End Source File
# Begin Source File

SOURCE=.\NewM_TexClip.c
# End Source File
# Begin Source File

SOURCE=.\NewM_TexClipU.c
# End Source File
# Begin Source File

SOURCE=.\S_nTex.c
# End Source File
# Begin Source File

SOURCE=.\S_Tex.c
# End Source File
# Begin Source File

SOURCE=.\S_TexA.src
# End Source File
# Begin Source File

SOURCE=.\S_TexCA.src
# End Source File
# Begin Source File

SOURCE=.\StripRender.src
# End Source File
# Begin Source File

SOURCE=.\StripRenderClipped.c
# End Source File
# End Group
# End Target
# End Project
