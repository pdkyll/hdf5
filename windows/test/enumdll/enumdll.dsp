# Microsoft Developer Studio Project File - Name="enumdll" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103
# TARGTYPE "Win32 (ALPHA) Console Application" 0x0603

CFG=enumdll - Win32 AlphaDbg
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "enumdll.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "enumdll.mak" CFG="enumdll - Win32 AlphaDbg"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "enumdll - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "enumdll - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "enumdll - Win32 AlphaDbg" (based on "Win32 (ALPHA) Console Application")
!MESSAGE "enumdll - Win32 AlphaRel" (based on "Win32 (ALPHA) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "enumdll - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../test/enumdll/Release"
# PROP Intermediate_Dir "../../../test/enumdll/Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE F90 /include:"Release/"
# ADD F90 /include:"Release/"
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\..\src" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "_HDF5USEDLL_" /D "_HDF5TESTUSEDLL_" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib WS2_32.lib /nologo /subsystem:console /machine:I386 /include:"_gethostname@8"

!ELSEIF  "$(CFG)" == "enumdll - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../test/enumdll/Debug"
# PROP Intermediate_Dir "../../../test/enumdll/Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE F90 /include:"Debug/"
# ADD F90 /include:"Debug/"
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\..\src" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "_HDF5USEDLL_" /D "_HDF5TESTUSEDLL_" /YX /FD /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib WS2_32.lib /nologo /subsystem:console /debug /machine:I386 /include:"_gethostname@8" /pdbtype:sept

!ELSEIF  "$(CFG)" == "enumdll - Win32 AlphaDbg"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "enumdll_"
# PROP BASE Intermediate_Dir "enumdll_"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /Gt0 /W3 /GX /Zi /Od /I "..\..\src" /I "..\..\test" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "_HDF5USEDLL_" /YX /FD /c
# ADD CPP /nologo /Gt0 /W3 /GX /Zi /Od /I "..\..\src" /I "..\..\test" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "_HDF5USEDLL_" /YX /FD /c
F90=df.exe
# ADD BASE F90 /compile_only /debug:full /include:"enumdll_/" /nologo /warn:nofileopt
# ADD F90 /compile_only /debug:full /include:"Debug/" /nologo /warn:nofileopt
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /debug /machine:ALPHA /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /debug /machine:ALPHA /pdbtype:sept

!ELSEIF  "$(CFG)" == "enumdll - Win32 AlphaRel"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "enumdll0"
# PROP BASE Intermediate_Dir "enumdll0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /Gt0 /W3 /GX /O2 /I "..\..\src" /I "..\..\test" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "_HDF5USEDLL_" /YX /FD /c
# ADD CPP /nologo /Gt0 /W3 /GX /O2 /I "..\..\src" /I "..\..\test" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "_HDF5USEDLL_" /YX /FD /c
F90=df.exe
# ADD BASE F90 /compile_only /include:"enumdll0/" /nologo /warn:nofileopt
# ADD F90 /compile_only /include:"Release/" /nologo /warn:nofileopt
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /machine:ALPHA
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /machine:ALPHA

!ENDIF 

# Begin Target

# Name "enumdll - Win32 Release"
# Name "enumdll - Win32 Debug"
# Name "enumdll - Win32 AlphaDbg"
# Name "enumdll - Win32 AlphaRel"
# Begin Source File

SOURCE=..\..\..\test\enum.c

!IF  "$(CFG)" == "enumdll - Win32 Release"

!ELSEIF  "$(CFG)" == "enumdll - Win32 Debug"

!ELSEIF  "$(CFG)" == "enumdll - Win32 AlphaDbg"

!ELSEIF  "$(CFG)" == "enumdll - Win32 AlphaRel"

!ENDIF 

# End Source File
# End Target
# End Project
