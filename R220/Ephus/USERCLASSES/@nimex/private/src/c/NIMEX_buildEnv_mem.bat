echo NIMEX_buildEnv: Configuring build environment...

rem set "PATH=C:\WINDOWS\system32;C:\WINDOWS;C:\WINDOWS\system32\wbem;c:\program files\ati technologies\ati control panel;C:\Program Files\MATLAB\R2007a\bin;C:\Program Files\MATLAB\R2007a\bin\win32;c:\program files\matlab704\bin\win32;c:\program files\common files\gtk\2.0\bin;c:\program files\quicktime\qtsystem\;c:\program files\ivi\bin;c:\vxipnp\winnt\bin;c:\program files\subversion\bin;C:\Program Files\Microsoft Visual Studio 8\Common7\IDE;"

rem         ::: Universal settings. :::
set NIMEX_ENV_MATLAB_PATH=C:\Program Files\MATLAB\R2007a
rem set NIMEX_ENV_MATLAB_PATH=C:\Program Files\MATLAB704
set NIMEX_ENV_LINKER_ARGS=-dll "%NIMEX_ENV_MATLAB_PATH%\extern\lib\win32\lcc\mexFunction.def"
set NIMEX_ENV_BUILD_ARGS=
set NIMEX_ENV_NI_PATH=C:\Program Files\National Instruments\NI-DAQ\DAQmx ANSI C Dev
rem set NIMEX_ENV_SHARED_OBJECT_FILES=NIMEX_memManagement.obj NIMEX_utilities.obj NIMEX_genericStructures.obj NIMEX_Objects.obj NIMEX_Callbacks.obj
set NIMEX_ENV_SHARED_OBJECT_FILES=NIMEX_shared.lib
rem set NIMEX_ENV_SHARED_OBJECT_FILES=NIMEX_memManagement.obj NIMEX_utilities.obj
rem set NIMEX_ENV_SHARED_OBJECT_FILES=
rem set NIMEX_ENV_NIMEX_BUILD_PATH=%NIMEX_ENV_MATLAB_PATH%\work\nimex
set NIMEX_ENV_NIMEX_BUILD_PATH=C:\dev\sandbox\Matlab\nimex
set NIMEX_ENV_GLIB_PATH=C:\dev\glib-dev-2.12.11
set NIMEX_ENV_TARGET_PATH=..\svobodalab\USERCLASSES\@nimex\private\.

rem         ::: Select compiler specific environment here... :::
set NIMEX_DEFAULT_ENV=cl
if (%1)==() goto %NIMEX_DEFAULT_ENV%
if /i %1==lcc  goto LCC
if /i %1==gcc goto GCC
if /i %1==cl goto CL
if /i %1==msvc goto CL
if /i %1==msvc++ goto CL
if /i %1==vc goto CL
if /i %1==vc++ goto CL
echo NIMEX_buildEnv: Using default environment %NIMEX_DEFAULT_ENV%...
goto %NIMEX_DEFAULT_ENV%


:LCC
rem         ::: LCC settings. :::
echo NIMEX_buildEnv: Using lcc settings...
set NIMEX_ENV=LCC
set NIMEX_ENV_LCC_HOME=G:\lcc
set NIMEX_ENV_LCC_BIN=%NIMEX_ENV_LCC_HOME%\bin
set NIMEX_ENV_COMPILER=%NIMEX_ENV_LCC_BIN%\lcc
set NIMEX_ENV_LINKER=%NIMEX_ENV_LCC_BIN%\lcclnk
set NIMEX_ENV_COMPILER_ARGS=-c -Zp8 -noregistrylookup -DNDEBUG -DMATLAB_MEX_FILE -DNIMEX_COMPILER_LCC
set NIMEX_ENV_INCLUDES=-I"%NIMEX_ENV_NIMEX_BUILD_PATH%" -I"%NIMEX_ENV_MATLAB_PATH%\extern\include" -I"%NIMEX_ENV_LCC_HOME%\include" -I"%NIMEX_ENV_NI_PATH%\include" -I"%NIMEX_ENV_GLIB_PATH%\include\glib-2.0" -I"%NIMEX_ENV_GLIB_PATH%\lib\glib-2.0\include"
rem set NIMEX_ENV_LINKER_ARGS=-s -dll "%NIMEX_ENV_MATLAB_PATH%\extern\lib\win32\lcc\mexFunction.def"
set NIMEX_ENV_LINKER_ARGS=-s -dll mexFunction.def
set NIMEX_ENV_LINKER_TARGET_PREFIX=-o 
set NIMEX_ENV_COMPILER_TARGET_PREFIX=-Fo
set NIMEX_ENV_LINKER_LIBS=libmx.lib libmex.lib libmat.lib NIDAQmx_lcc.lib glib2lccWrapper_lcc.lib

goto EOF


:GCC
rem         ::: GCC settings. :::
echo NIMEX_buildEnv: Using gcc settings...
set NIMEX_ENV=GCC
set NIMEX_ENV_GCC_BIN=C:\msys\1.0\mingw\bin
set NIMEX_ENV_COMPILER_ARGS=-c -O -std=c99 -mtune=pentium -malign-double -fno-exceptions
set NIMEX_ENV_COMPILER_TARGET_PREFIX=-o 
set NIMEX_ENV_COMPILER=%NIMEX_ENV_GCC_BIN%\gcc
set NIMEX_ENV_LINKER=%NIMEX_ENV_GCC_BIN%\gcc
set NIMEX_ENV_LINKER_TARGET_PREFIX=-o 
set NIMEX_ENV_LINKER_ARGS=-shared -L"%NIMEX_ENV_NIMEX_BUILD_PATH%" --library-path="%NIMEX_ENV_NIMEX_BUILD_PATH%" -DNDEBUG -DMATLAB_MEX_FILE -DNIMEX_COMPILER_GCC
set NIMEX_ENV_LINKER_LIBS=-llibmx -llibmex -llibmat -lNIDAQmx
set NIMEX_ENV_INCLUDES=-I"%NIMEX_ENV_NIMEX_BUILD_PATH%" -I"C:\msys\1.0\mingw\include" -I"%NIMEX_ENV_MATLAB_PATH%\extern\include" -I"%NIMEX_ENV_NIMEX_BUILD_PATH%"

goto EOF


:CL
rem         ::: CL (MSVC) settings. :::
echo NIMEX_buildEnv: Using cl (MSVC++) settings...
set NIMEX_ENV=CL
rem set NIMEX_ENV_VS_HOME=C:\Progra~1\Micros~2\VC
set NIMEX_ENV_VS_HOME=C:\Progra~1\MID05A~1\VC
rem set NIMEX_ENV_MS_SDK=C:\Program Files\Microsoft Platform SDK for Windows Server 2003 R2
set NIMEX_ENV_MS_SDK=C:\Program Files\Microsoft Platform SDK
set NIMEX_ENV_CL_BIN=%NIMEX_ENV_VS_HOME%\bin
set NIMEX_ENV_COMPILER=%NIMEX_ENV_CL_BIN%\cl
set NIMEX_ENV_LINKER=%NIMEX_ENV_CL_BIN%\link
rem set NIMEX_ENV_COMPILER_ARGS=/arch:SSE2 /O2 /fp:precise /c /nologo -DNIMEX_LOCALTIME_DEPRECATED -DNDEBUG -DMATLAB_MEX_FILE -DNIMEX_VERBOSE_0 -DNIMEX_COMPILER_CL 
set NIMEX_ENV_COMPILER_ARGS_NON_MEX=/c /O2 /nologo -DNIMEX_LOCALTIME_DEPRECATED -DNIMEX_COMPILER_CL -DNIMEX_VERBOSE_0 -DNDEBUG -DMEMWATCH -DMEMWATCH_STDIO -D_CRT_SECURE_NO_DEPRECATE 
set NIMEX_ENV_COMPILER_ARGS=%NIMEX_ENV_COMPILER_ARGS_NON_MEX% -DMATLAB_MEX_FILE 
set NIMEX_ENV_INCLUDES=/I. /I%NIMEX_ENV_VS_HOME%\include /I"%NIMEX_ENV_MATLAB_PATH%\extern\include" /I"%NIMEX_ENV_NI_PATH%\include" /I"%NIMEX_ENV_MS_SDK%\Include" /I"%NIMEX_ENV_GLIB_PATH%\include\glib-2.0" /I"%NIMEX_ENV_GLIB_PATH%\lib\glib-2.0\include"
set NIMEX_ENV_COMPILER_TARGET_PREFIX=/Fo
set NIMEX_ENV_LINKER_TARGET_PREFIX=/OUT:
set NIMEX_ENV_LINKER_ARGS_NON_MEX=/DLL /nologo /SUBSYSTEM:console
set NIMEX_ENV_LINKER_ARGS=%NIMEX_ENV_LINKER_ARGS_NON_MEX% /EXPORT:mexFunction
set NIMEX_ENV_LIB_PATH=%NIMEX_ENV_MATLAB_PATH%\extern\lib\win32\microsoft
set NIMEX_ENV_LINKER_LIBS="%NIMEX_ENV_LIB_PATH%\libmx.lib" "%NIMEX_ENV_LIB_PATH%\libmex.lib" "%NIMEX_ENV_LIB_PATH%\libmat.lib" "%NIMEX_ENV_NI_PATH%\lib\msvc\NIDAQmx.lib" "%NIMEX_ENV_MS_SDK%\lib\uuid.lib" "%NIMEX_ENV_VS_HOME%\lib\libcmt.lib" "%NIMEX_ENV_MS_SDK%\lib\kernel32.lib" "%NIMEX_ENV_VS_HOME%\lib\oldnames.lib" glib-2.0.lib gmodule-2.0.lib gobject-2.0.lib gthread-2.0.lib "%NIMEX_ENV_MS_SDK%\lib\WinMM.lib" "%NIMEX_ENV_MS_SDK%\lib\User32.lib"

goto EOF

:EOF