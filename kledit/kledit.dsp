# Microsoft Developer Studio Project File - Name="KLEDIT" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=KLEDIT - Win32 Hybrid
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "KLEDIT.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "KLEDIT.mak" CFG="KLEDIT - Win32 Hybrid"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "KLEDIT - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "KLEDIT - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "KLEDIT - Win32 Hybrid" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/KLEDIT", BPDAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "KLEDIT - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KLEDIT_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KLEDIT_EXPORTS" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /fo"Release/KLEdit.res" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /dll /machine:I386 /def:".\klEdit.def" /out:"E:\3dsmax3\plugins\KLEDIT.DLE"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "KLEDIT - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KLEDIT_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KLEDIT_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "KLEDIT - Win32 Hybrid"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "KLEDIT___Win32_Hybrid"
# PROP BASE Intermediate_Dir "KLEDIT___Win32_Hybrid"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Hybrid"
# PROP Intermediate_Dir "Hybrid"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KLEDIT_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MD /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KLEDIT_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "KLEDIT - Win32 Release"
# Name "KLEDIT - Win32 Debug"
# Name "KLEDIT - Win32 Hybrid"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AnimControl.cpp
# End Source File
# Begin Source File

SOURCE=.\BBox.cpp
# End Source File
# Begin Source File

SOURCE=.\BoundRegion.cpp
# End Source File
# Begin Source File

SOURCE=.\BoundSpline.cpp
# End Source File
# Begin Source File

SOURCE=.\CamControl.cpp
# End Source File
# Begin Source File

SOURCE=.\CampingPoint.cpp
# End Source File
# Begin Source File

SOURCE=.\CollPoint.cpp
# End Source File
# Begin Source File

SOURCE=.\CoM.cpp
# End Source File
# Begin Source File

SOURCE=.\Applyvc\EvalCol.cpp
# End Source File
# Begin Source File

SOURCE=.\KLedit.cpp
# End Source File
# Begin Source File

SOURCE=.\KLeditspline.CPP
# End Source File
# Begin Source File

SOURCE=.\Levelflags.cpp
# End Source File
# Begin Source File

SOURCE=.\Material.cpp
# End Source File
# Begin Source File

SOURCE=.\MatLib.cpp
# End Source File
# Begin Source File

SOURCE=.\MeshSupt.cpp
# End Source File
# Begin Source File

SOURCE=.\ModelSave.cpp
# End Source File
# Begin Source File

SOURCE=.\Normals.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjFlags.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjLoad.cpp
# End Source File
# Begin Source File

SOURCE=.\plugoldfuncs.cpp
# End Source File
# Begin Source File

SOURCE=.\Preview.cpp
# End Source File
# Begin Source File

SOURCE=.\RDObject.cpp
# End Source File
# Begin Source File

SOURCE=.\RDScape.cpp
# End Source File
# Begin Source File

SOURCE=.\RDStratMap.cpp
# End Source File
# Begin Source File

SOURCE=.\ShadBox.cpp
# End Source File
# Begin Source File

SOURCE=.\ShadModel.cpp
# End Source File
# Begin Source File

SOURCE=.\splinemod.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Strip.cpp
# End Source File
# Begin Source File

SOURCE=.\util.cpp
# End Source File
# Begin Source File

SOURCE=.\VertLib.cpp
# End Source File
# Begin Source File

SOURCE=.\VisBox.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\animcontrol.h
# End Source File
# Begin Source File

SOURCE=.\Applyvc\ApplyVC.h
# End Source File
# Begin Source File

SOURCE=.\BBox.h
# End Source File
# Begin Source File

SOURCE=.\BoundRegion.h
# End Source File
# Begin Source File

SOURCE=.\BoundSpline.h
# End Source File
# Begin Source File

SOURCE=.\BSP.h
# End Source File
# Begin Source File

SOURCE=.\camcontrol.h
# End Source File
# Begin Source File

SOURCE=.\camppoint.h
# End Source File
# Begin Source File

SOURCE=.\CollPoint.h
# End Source File
# Begin Source File

SOURCE=.\CoM.h
# End Source File
# Begin Source File

SOURCE=.\Applyvc\EvalCol.h
# End Source File
# Begin Source File

SOURCE=.\GenericSFX.h
# End Source File
# Begin Source File

SOURCE=.\HelpBase.h
# End Source File
# Begin Source File

SOURCE=.\IStrip.h
# End Source File
# Begin Source File

SOURCE=.\Kleditspline.h
# End Source File
# Begin Source File

SOURCE=.\LevelFlags.h
# End Source File
# Begin Source File

SOURCE=.\Library.h
# End Source File
# Begin Source File

SOURCE=.\Material.h
# End Source File
# Begin Source File

SOURCE=.\MatLib.h
# End Source File
# Begin Source File

SOURCE=.\MeshSupt.h
# End Source File
# Begin Source File

SOURCE=.\ModelSave.h
# End Source File
# Begin Source File

SOURCE=.\Ninja.h
# End Source File
# Begin Source File

SOURCE=.\Normals.h
# End Source File
# Begin Source File

SOURCE=.\ObjFlags.h
# End Source File
# Begin Source File

SOURCE=.\ObjLoad.h
# End Source File
# Begin Source File

SOURCE=.\Preview.h
# End Source File
# Begin Source File

SOURCE=.\RDEdit.h
# End Source File
# Begin Source File

SOURCE=.\RDHelper.h
# End Source File
# Begin Source File

SOURCE=.\RDMap.h
# End Source File
# Begin Source File

SOURCE=.\RDObject.h
# End Source File
# Begin Source File

SOURCE=.\RDScape.h
# End Source File
# Begin Source File

SOURCE=.\RDStratMap.h
# End Source File
# Begin Source File

SOURCE=.\RedDog.h
# End Source File
# Begin Source File

SOURCE=.\SelfAlloc.h
# End Source File
# Begin Source File

SOURCE=.\ShadBox.h
# End Source File
# Begin Source File

SOURCE=.\splinemod.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\Strip.h
# End Source File
# Begin Source File

SOURCE=.\util.h
# End Source File
# Begin Source File

SOURCE=.\vertcomp.h
# End Source File
# Begin Source File

SOURCE=.\VertLib.h
# End Source File
# Begin Source File

SOURCE=.\VisBox.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\RDEdit\kledit.def

!IF  "$(CFG)" == "KLEDIT - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KLEDIT - Win32 Debug"

!ELSEIF  "$(CFG)" == "KLEDIT - Win32 Hybrid"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RDEdit.rc
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# End Group
# Begin Group "maxlibs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=F:\Maxsdk\lib\zlibdll.lib
# End Source File
# Begin Source File

SOURCE=F:\Maxsdk\lib\bmm.lib
# End Source File
# Begin Source File

SOURCE=F:\Maxsdk\lib\client.lib
# End Source File
# Begin Source File

SOURCE=F:\Maxsdk\lib\core.lib
# End Source File
# Begin Source File

SOURCE=F:\Maxsdk\lib\edmodel.lib
# End Source File
# Begin Source File

SOURCE=F:\Maxsdk\lib\expr.lib
# End Source File
# Begin Source File

SOURCE=F:\Maxsdk\lib\Flilibd.lib
# End Source File
# Begin Source File

SOURCE=F:\Maxsdk\lib\Flilibh.lib
# End Source File
# Begin Source File

SOURCE=F:\Maxsdk\lib\Flilibr.lib
# End Source File
# Begin Source File

SOURCE=F:\Maxsdk\lib\flt.lib
# End Source File
# Begin Source File

SOURCE=F:\Maxsdk\lib\gcomm.lib
# End Source File
# Begin Source File

SOURCE=F:\Maxsdk\lib\geom.lib
# End Source File
# Begin Source File

SOURCE=F:\Maxsdk\lib\gfx.lib
# End Source File
# Begin Source File

SOURCE=F:\Maxsdk\lib\gup.lib
# End Source File
# Begin Source File

SOURCE=F:\Maxsdk\lib\helpsys.lib
# End Source File
# Begin Source File

SOURCE=F:\Maxsdk\lib\Maxscrpt.lib
# End Source File
# Begin Source File

SOURCE=F:\Maxsdk\lib\maxutil.lib
# End Source File
# Begin Source File

SOURCE=F:\Maxsdk\lib\mesh.lib
# End Source File
# Begin Source File

SOURCE=F:\Maxsdk\lib\MNMath.lib
# End Source File
# Begin Source File

SOURCE=F:\Maxsdk\lib\Paramblk2.lib
# End Source File
# Begin Source File

SOURCE=F:\Maxsdk\lib\particle.lib
# End Source File
# Begin Source File

SOURCE=F:\Maxsdk\lib\tessint.lib
# End Source File
# Begin Source File

SOURCE=F:\Maxsdk\lib\viewfile.lib
# End Source File
# Begin Source File

SOURCE=F:\Maxsdk\lib\acap.lib
# End Source File
# End Group
# End Target
# End Project
