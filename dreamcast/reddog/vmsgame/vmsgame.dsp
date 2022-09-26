# Microsoft Developer Studio Project File - Name="VMSGame" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

CFG=VMSGame - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "VMSGame.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "VMSGame.mak" CFG="VMSGame - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "VMSGame - Win32 Release" (based on "Win32 (x86) External Target")
!MESSAGE "VMSGame - Win32 Debug" (based on "Win32 (x86) External Target")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Dreamcast/RedDog/VMSGame", WKCAAAAA"
# PROP Scc_LocalPath "."

!IF  "$(CFG)" == "VMSGame - Win32 Release"

# PROP BASE Use_MFC
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Cmd_Line "NMAKE /f VMSGame.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "VMSGame.exe"
# PROP BASE Bsc_Name "VMSGame.bsc"
# PROP BASE Target_Dir ""
# PROP Use_MFC
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Cmd_Line "nmake /f "VMSGame.mak""
# PROP Rebuild_Opt "/a"
# PROP Target_File "VMSGame.hex"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "VMSGame - Win32 Debug"

# PROP BASE Use_MFC
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Cmd_Line "NMAKE /f VMSGame.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "VMSGame.exe"
# PROP BASE Bsc_Name "VMSGame.bsc"
# PROP BASE Target_Dir ""
# PROP Use_MFC
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Cmd_Line "nmake /f "VMSGame.mak""
# PROP Rebuild_Opt "/a"
# PROP Target_File "VMSGame.hex"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "VMSGame - Win32 Release"
# Name "VMSGame - Win32 Debug"

!IF  "$(CFG)" == "VMSGame - Win32 Release"

!ELSEIF  "$(CFG)" == "VMSGame - Win32 Debug"

!ENDIF 

# Begin Group "Assembler files"

# PROP Default_Filter "asm"
# Begin Source File

SOURCE=.\GHead.asm
# End Source File
# Begin Source File

SOURCE=.\RedDog.asm
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe;mak"
# Begin Source File

SOURCE=.\VMSGame.mak
# End Source File
# Begin Source File

SOURCE=.\GDummy.obj
# End Source File
# End Group
# End Target
# End Project
