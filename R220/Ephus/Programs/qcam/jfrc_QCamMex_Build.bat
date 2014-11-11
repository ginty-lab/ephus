@echo off

:CL
rem         ::: CL (MSVC) settings. :::
set QCAMMEX_ENG=CL

call %DEV_TOOL_HOME%\jfrc_setDevelopmentEnvironmentVariables.bat
set "QCAMMEX_MATLAB_PATH=%MATLAB_HOME%"

set QCAMMEX_VS_HOME=%VS_HOME%

set QCAMMEX_MS_SDK=%MS_SDK_HOME%
set QCAMMEX_COMPILER=%VS_COMPILER%
set QCAMMEX_LINKER=%VS_LINKER%

set QCAMMEX_COMPILER_ARGS_NON_MEX=/c /O2 /nologo -DQCAMMEX_LOCALTIME_DEPRECATED -DQCAMMEX_COMPILER_CL -DQCAMMEX_VERBOSE_4 -DQMX_WAIT_TRACING
set QCAMMEX_COMPILER_ARGS=%QCAMMEX_COMPILER_ARGS_NON_MEX% -DMATLAB_MEX_FILE

rem NOTE: See jfrc_setDevelopmentEnvironmentVariables.bat for details about the order for QCAM_INCLUDES
set QCAMMEX_INCLUDES=/I. %QCAM_INCLUDES% %MATLAB_INCLUDES% %WIN_INCLUDES%

set QCAMMEX_COMPILER_TARGET_PREFIX=/Fo
rem set QCAMMEX_LINKER_TARGET_PREFIX=/OUT:

set QCAMMEX_LINKER_ARGS_NON_MEX=/nologo /SUBSYSTEM:console
set QCAMMEX_LINKER_ARGS=%QCAMMEX_LINKER_ARGS_NON_MEX% /EXPORT:mexFunction /DLL

set QCAMMEX_LINKER_LIBS=%MATLAB_LIBS% %MS_SDK_LIBS% %QCAM_LIBS%

rem %QCAMMEX_CL_BIN%\dumpbin QCamDriver.dll /EXPORTS
echo QCAMMEX_INCLUDES: %QCAMMEX_INCLUDES%
:COMPILE_EXE
echo Compiling executable...
%QCAMMEX_COMPILER% %QCAMMEX_COMPILER_ARGS_NON_MEX% %QCAMMEX_INCLUDES% %QCAMMEX_COMPILER_TARGET_PREFIX%qcammex.obj qcammex.c

:LINK_EXE
echo Linking executable...
%QCAMMEX_LINKER% %QCAMMEX_LINKER_ARGS_NON_MEX% qcammex.obj %QCAMMEX_LINKER_LIBS% /OUT:qcammex.exe

:COMPILE_MEX
echo Compiling mex file...
%QCAMMEX_COMPILER% %QCAMMEX_COMPILER_ARGS% %QCAMMEX_INCLUDES% %QCAMMEX_COMPILER_TARGET_PREFIX%qcammex.obj qcammex.c

:LINK_MEX
echo Linking mex file...
%QCAMMEX_LINKER% %QCAMMEX_LINKER_ARGS% qcammex.obj %QCAMMEX_LINKER_LIBS% /OUT:qcammex.mexw32

:CLEAN
echo Cleaning...
del *.obj
del qcammex.exp
del qcammex.lib

:TEST
rem qcammex

goto EOF

:EOF