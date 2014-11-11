@echo off

echo NIMEX_BuildInfo.bat: Capturing build-time environment...

%NIMEX_ENV_COMPILER% %NIMEX_ENV_COMPILER_ARGS% %NIMEX_ENV_INCLUDES% %NIMEX_ENV_COMPILER_TARGET_PREFIX%NIMEX_BuildInfo.obj NIMEX_BuildInfo.c
if /i %NIMEX_ENV%==cl goto CLLINK
goto GENERICLINK

:CLLINK
 rem  %NIMEX_ENV_LINKER% /nologo /SUBSYSTEM:console NIMEX_BuildInfo.obj "%NIMEX_ENV_MS_SDK%\lib\uuid.lib" "%NIMEX_ENV_VS_HOME%\lib\libcmt.lib" "%NIMEX_ENV_VS_HOME%\lib\kernel32.lib"  "%NIMEX_ENV_VS_HOME%\lib\oldnames.lib" /OUT:NIMEX_BuildInfo.exe
 %NIMEX_ENV_LINKER% /nologo /SUBSYSTEM:console NIMEX_BuildInfo.obj %NIMEX_ENV_LINKER_LIBS% /OUT:NIMEX_BuildInfo.exe
 rem %NIMEX_ENV_CL_BIN%\dumpbin /SYMBOLS /EXPORTS /HEADERS /SUMMARY NIMEX_BuildInfo.exe
 goto EXECUTE_RESULT

:GENERICLINK
 rem %NIMEX_ENV_LINKER% %NIMEX_ENV_LINKER_ARGS% NIMEX_BuildInfo.obj %NIMEX_ENV_LINKER_LIBS% NIMEX_BuildInfo.exe
 %NIMEX_ENV_LINKER% NIMEX_BuildInfo.obj -o NIMEX_BuildInfo.exe
 goto EXECUTE_RESULT

:EXECUTE_RESULT
echo NIMEX_BuildInfo.bat: Generating build-time header file...
NIMEX_BuildInfo.exe %* > NIMEX_BuildInfo.h
rem NIMEX_BuildInfo.exe

del NIMEX_BuildInfo.exe

echo -----------------------
echo NIMEX_BuildInfo.h:
type NIMEX_BuildInfo.h
echo -----------------------