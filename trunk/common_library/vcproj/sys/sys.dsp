# Microsoft Developer Studio Project File - Name="sys" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=sys - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sys.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sys.mak" CFG="sys - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sys - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "sys - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sys - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JIAN_SYS_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JIAN_SYS_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386

!ELSEIF  "$(CFG)" == "sys - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JIAN_SYS_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JIAN_SYS_EXPORTS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "sys - Win32 Release"
# Name "sys - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\src\sys\datetime_util.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\sys\dynamic_linking_loader.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\sys\event.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\sys\file_util.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\sys\fs_util.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\sys\lock.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\sys\logger.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\sys\main_template.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\sys\mmap.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\sys\pool_thread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\sys\read_write_lock.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\sys\sys_info.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\sys\sys_util.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\sys\syscall_exception.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\sys\thread.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\include\sys\atomic.h
# End Source File
# Begin Source File

SOURCE=..\..\include\sys\atomic_asm.h
# End Source File
# Begin Source File

SOURCE=..\..\include\sys\atomic_gcc.h
# End Source File
# Begin Source File

SOURCE=..\..\include\sys\atomic_gcc8.h
# End Source File
# Begin Source File

SOURCE=..\..\include\sys\close_helper.h
# End Source File
# Begin Source File

SOURCE=..\..\include\sys\compiler.h
# End Source File
# Begin Source File

SOURCE=..\..\include\sys\config_file.h
# End Source File
# Begin Source File

SOURCE=..\..\include\sys\datetime_util.h
# End Source File
# Begin Source File

SOURCE=..\..\include\sys\db.h
# End Source File
# Begin Source File

SOURCE=..\..\include\sys\dynamic_linking_loader.h
# End Source File
# Begin Source File

SOURCE=..\..\include\sys\event.h
# End Source File
# Begin Source File

SOURCE=..\..\include\sys\event_queue.h
# End Source File
# Begin Source File

SOURCE=..\..\include\sys\file_util.h
# End Source File
# Begin Source File

SOURCE=..\..\include\sys\fs_util.h
# End Source File
# Begin Source File

SOURCE=..\..\include\sys\lock.h
# End Source File
# Begin Source File

SOURCE=..\..\include\sys\log.h
# End Source File
# Begin Source File

SOURCE=..\..\include\sys\logger.h
# End Source File
# Begin Source File

SOURCE=..\..\include\sys\main_template.h
# End Source File
# Begin Source File

SOURCE=..\..\include\sys\mmap.h
# End Source File
# Begin Source File

SOURCE=..\..\include\sys\pool_thread.h
# End Source File
# Begin Source File

SOURCE=..\..\include\sys\read_write_lock.h
# End Source File
# Begin Source File

SOURCE=..\..\include\sys\ref_countable.h
# End Source File
# Begin Source File

SOURCE=..\..\include\sys\sys_config.h
# End Source File
# Begin Source File

SOURCE=..\..\include\sys\sys_info.h
# End Source File
# Begin Source File

SOURCE=..\..\include\sys\sys_util.h
# End Source File
# Begin Source File

SOURCE=..\..\include\sys\syscall_exception.h
# End Source File
# Begin Source File

SOURCE=..\..\include\sys\thread.h
# End Source File
# Begin Source File

SOURCE=..\..\include\sys\thread_pool.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\src\sys\Makefile.am.in
# End Source File
# End Target
# End Project
