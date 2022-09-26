# Microsoft Developer Studio Project File - Name="RDEdit" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=RDEdit - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "RDEdit.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "RDEdit.mak" CFG="RDEdit - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "RDEdit - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "RDEdit - Win32 Hybrid" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "RDEdit - Win32 Artist" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "RDEdit - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/RDEdit", BAAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "RDEdit - Win32 Release"

# PROP BASE Use_MFC 2
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
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /Yu"StdAfx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"c:\3dsmax2\plugins\rdedit.dle"

!ELSEIF  "$(CFG)" == "RDEdit - Win32 Hybrid"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "RDEdit__"
# PROP BASE Intermediate_Dir "RDEdit__"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Hybrid"
# PROP Intermediate_Dir "Hybrid"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD CPP /nologo /MD /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /Yu"StdAfx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"c:\program files\kinetix\3D studio max\plugins\rdedit.dle" /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /dll /incremental:no /debug /machine:I386 /out:"c:\3dsmax2\plugins\rdedit.dle" /pdbtype:sept

!ELSEIF  "$(CFG)" == "RDEdit - Win32 Artist"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "RDEdit__"
# PROP BASE Intermediate_Dir "RDEdit__"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "artist_"
# PROP Intermediate_Dir "artist_"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D ARTIST=1 /FR /Yu"StdAfx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"c:\3dsmax2\plugins\rdedit.dle"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"P:\UTILS\new\rdedit.dle"

!ELSEIF  "$(CFG)" == "RDEdit - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "RDEdit___Win32_Debug"
# PROP BASE Intermediate_Dir "RDEdit___Win32_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /Yu"StdAfx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /dll /incremental:no /debug /machine:I386 /out:"c:\3dsmax2\plugins\rdedit.dle" /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /dll /incremental:no /debug /machine:I386 /out:"c:\3dsmax2\debug\exe\plugins\rdedit.dle" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "RDEdit - Win32 Release"
# Name "RDEdit - Win32 Hybrid"
# Name "RDEdit - Win32 Artist"
# Name "RDEdit - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AnimControl.cpp
# End Source File
# Begin Source File

SOURCE=.\BBox.cpp
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

SOURCE=.\APPLYVC\EvalCol.cpp
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

SOURCE=.\RDEdit.cpp
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
# ADD CPP /Yc"StdAfx.h"
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

SOURCE=.\BBox.h
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

SOURCE=..\Game\Dirs.h
# End Source File
# Begin Source File

SOURCE=.\APPLYVC\EvalCol.h
# End Source File
# Begin Source File

SOURCE=.\HelpBase.h
# End Source File
# Begin Source File

SOURCE=.\IStrip.h
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

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\file.bmp
# End Source File
# Begin Source File

SOURCE=.\RDEdit.def
# End Source File
# Begin Source File

SOURCE=.\RDEdit.rc
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\resource.hm
# End Source File
# End Group
# Begin Group "3DS Libraries"

# PROP Default_Filter "lib"
# Begin Source File

SOURCE=..\..\..\Maxsdk\Lib\Bmm.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\Maxsdk\Lib\Core.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\Maxsdk\Lib\Geom.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\Maxsdk\Lib\Gfx.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\Maxsdk\Lib\Mesh.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\Maxsdk\Lib\Mnmath.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\Maxsdk\Lib\Util.lib
# End Source File
# End Group
# End Target
# End Project
