# Microsoft Developer Studio Project File - Name="dispatcher" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=dispatcher - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "dispatcher.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dispatcher.mak" CFG="dispatcher - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dispatcher - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "dispatcher - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "dispatcher - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DISPATCHER_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DISPATCHER_EXPORTS" /YX /FD /c
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

!ELSEIF  "$(CFG)" == "dispatcher - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DISPATCHER_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DISPATCHER_EXPORTS" /YX /FD /GZ /c
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

# Name "dispatcher - Win32 Release"
# Name "dispatcher - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\src\dispatcher\default_reply_handler.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\dispatcher\dispatcher_context.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\dispatcher\managed_sender.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\dispatcher\managed_sender_table.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\dispatcher\send_queue.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\dispatcher\send_thread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\dispatcher\send_thread_pool.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\dispatcher\sender.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\dispatcher\sender_group.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\dispatcher\sender_table.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\dispatcher\unmanaged_sender.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\src\dispatcher\unmanaged_sender_table.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\src\dispatcher\default_reply_handler.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\dispatcher\dispatcher.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\dispatcher\dispatcher_context.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\dispatcher\dispatcher_log.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\dispatcher\managed_sender.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\dispatcher\managed_sender_table.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\dispatcher\send_queue.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\dispatcher\send_thread.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\dispatcher\send_thread_pool.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\dispatcher\sender.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\dispatcher\sender_group.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\dispatcher\sender_table.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\dispatcher\unmanaged_sender.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\dispatcher\unmanaged_sender_table.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\src\dispatcher\Makefile.am.in
# End Source File
# End Target
# End Project
