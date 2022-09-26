# Microsoft Developer Studio Project File - Name="Final" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

CFG=Final - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Final.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Final.mak" CFG="Final - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Final - Win32 Release" (based on "Win32 (x86) External Target")
!MESSAGE "Final - Win32 Ginsu Demo" (based on "Win32 (x86) External Target")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Dreamcast/RedDog/Final", QGCAAAAA"
# PROP Scc_LocalPath "."

!IF  "$(CFG)" == "Final - Win32 Release"

# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Cmd_Line "NMAKE /f Final.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "Final.exe"
# PROP BASE Bsc_Name "Final.bsc"
# PROP BASE Target_Dir ""
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Cmd_Line "NMAKE /f Final.mak FINAL="
# PROP Rebuild_Opt "/a"
# PROP Target_File "reddog.elf"
# PROP Bsc_Name "Final.bsc"
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "Final - Win32 Ginsu Demo"

# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Final___Win32_Ginsu_Demo"
# PROP BASE Intermediate_Dir "Final___Win32_Ginsu_Demo"
# PROP BASE Cmd_Line "NMAKE /f Final.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "Final.exe"
# PROP BASE Bsc_Name "Final.bsc"
# PROP BASE Target_Dir ""
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Final___Win32_Ginsu_Demo"
# PROP Intermediate_Dir "Final___Win32_Ginsu_Demo"
# PROP Cmd_Line "NMAKE /f Final.mak DEMO="
# PROP Rebuild_Opt "/a DEMO="
# PROP Target_File "Final.exe"
# PROP Bsc_Name "Final.bsc"
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "Final - Win32 Release"
# Name "Final - Win32 Ginsu Demo"

!IF  "$(CFG)" == "Final - Win32 Release"

!ELSEIF  "$(CFG)" == "Final - Win32 Ginsu Demo"

!ENDIF 

# Begin Source File

SOURCE=.\convert.bat
# End Source File
# Begin Source File

SOURCE=.\Final.mak
# End Source File
# End Target
# End Project
