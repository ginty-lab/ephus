@echo off

cls

echo ************************************
echo *** MEX file compilation routine ***
echo ************************************

call NIMEX_buildEnv_mem %*
set NIMEX_ENV_BUILD_ARGS=%*

rem Load environment variables (only if they exist) into the header BuildInfo.h file.
rem set buildInfoArgs=""
rem if defined NIMEX_VERSION set "buildInfoArgs=%buildInfoArgs% %NIMEX_VERSION%"
rem if defined NIMEX_MAJOR_VERSION set "buildInfoArgs=%buildInfoArgs% %NIMEX_MAJOR_VERSION%"
rem if defined NIMEX_MINOR_VERSION set "buildInfoArgs=%buildInfoArgs% %NIMEX_MINOR_VERSION%"
rem if defined NIMEX_PROGRAMMER set "buildInfoArgs=%buildInfoArgs% %NIMEX_PROGRAMMER%"
rem if /i buildInfoArgs=="" (
rem   call NIMEX_BuildInfo.bat
rem ) else (
rem   call NIMEX_BuildInfo.bat %buildInfoArgs%
rem )
call NIMEX_BuildInfo.bat "%NIMEX_VERSION%" "%NIMEX_MAJOR_VERSION%" "%NIMEX_MINOR_VERSION%" "%NIMEX_PROGRAMMER%"

echo Compiling...
goto COMPILE
:COMPILE_RETURN

echo Linking...
goto LINK
:LINK_RETURN

echo Deploying...
goto DEPLOY
:DEPLOY_RETURN

echo Cleaning...
goto CLEAN
:CLEAN_RETURN

echo Build complete.
goto EOF

:COMPILE
rem echo COMPILE...

rem Shuffle variables around, because of different types of compilation.
rem call NIMEX_compile.bat mexTest
set NIMEX_ENV_COMPILER_ARGS2=%NIMEX_ENV_COMPILER_ARGS%
set NIMEX_ENV_COMPILER_ARGS=%NIMEX_ENV_COMPILER_ARGS_NON_MEX%

call NIMEX_compile.bat memwatch
call NIMEX_compile.bat NIMEX_memManagement
call NIMEX_compile.bat NIMEX_genericStructures
call NIMEX_compile.bat NIMEX_utilities
call NIMEX_compile.bat NIMEX_Objects
call NIMEX_compile.bat NIMEX_Callbacks

set NIMEX_ENV_COMPILER_ARGS=%NIMEX_ENV_COMPILER_ARGS2%
call NIMEX_compile.bat NIMEX_createTask
call NIMEX_compile.bat NIMEX_addAnalogInputChannel
call NIMEX_compile.bat NIMEX_setChannelProperty
call NIMEX_compile.bat NIMEX_startTask
call NIMEX_compile.bat NIMEX_readAnalogF64
call NIMEX_compile.bat NIMEX_stopTask
call NIMEX_compile.bat NIMEX_addAnalogOutputChannel
call NIMEX_compile.bat NIMEX_writeAnalogF64
call NIMEX_compile.bat NIMEX_sendTrigger
call NIMEX_compile.bat NIMEX_setTaskProperty
call NIMEX_compile.bat NIMEX_getTaskProperty
call NIMEX_compile.bat NIMEX_getChannelProperty
rem call NIMEX_compile.bat NIMEX_bindEveryNCallback
rem call NIMEX_compile.bat NIMEX_bindDoneCallback
call NIMEX_compile.bat NIMEX_bindEventListener
call NIMEX_compile.bat NIMEX_readDigitalU32
call NIMEX_compile.bat NIMEX_writeDigitalU32
call NIMEX_compile.bat NIMEX_addDigitalOutputChannel
call NIMEX_compile.bat NIMEX_addDigitalInputChannel
call NIMEX_compile.bat NIMEX_display
call NIMEX_compile.bat NIMEX_deleteTask
call NIMEX_compile.bat NIMEX_registerSamplesAcquiredPreprocessor
call NIMEX_compile.bat NIMEX_registerSamplesAcquiredListener
call NIMEX_compile.bat NIMEX_updateCounterOutput
call NIMEX_compile.bat NIMEX_addCOFrequency
call NIMEX_compile.bat NIMEX_addCOTime
call NIMEX_compile.bat NIMEX_registerOutputDataPreprocessor
call NIMEX_compile.bat NIMEX_putSample
call NIMEX_compile.bat NIMEX_getSample
call NIMEX_compile.bat NIMEX_acquireLock
call NIMEX_compile.bat NIMEX_releaseLock
call NIMEX_compile.bat NIMEX_commitTask
call NIMEX_compile.bat NIMEX_updateDataSourceByCallback
rem call NIMEX_compile.bat NIMEXEng_getDeviceNames

del NIMEX_BuildInfo.h

goto COMPILE_RETURN

:LINK
rem echo LINK...
@setlocal

echo Building module of shared components...
%NIMEX_ENV_LINKER% %NIMEX_ENV_LINKER_ARGS_NON_MEX% NIMEX_memManagement.obj NIMEX_utilities.obj NIMEX_genericStructures.obj NIMEX_Objects.obj NIMEX_Callbacks.obj memwatch.obj %NIMEX_ENV_LINKER_LIBS% %NIMEX_ENV_LINKER_TARGET_PREFIX%NIMEX_shared.dll /DEF:NIMEX_shared.def

echo Linking with %NIMEX_ENV_SHARED_OBJECT_FILES% against %NIMEX_ENV_LINKER_LIBS%...
rem call NIMEX_link.bat mexTest
call NIMEX_link.bat NIMEX_createTask
call NIMEX_link.bat NIMEX_addAnalogInputChannel
call NIMEX_link.bat NIMEX_setChannelProperty
call NIMEX_link.bat NIMEX_startTask
call NIMEX_link.bat NIMEX_readAnalogF64
call NIMEX_link.bat NIMEX_stopTask
call NIMEX_link.bat NIMEX_addAnalogOutputChannel
call NIMEX_link.bat NIMEX_writeAnalogF64
call NIMEX_link.bat NIMEX_sendTrigger
call NIMEX_link.bat NIMEX_setTaskProperty
call NIMEX_link.bat NIMEX_getTaskProperty
call NIMEX_link.bat NIMEX_getChannelProperty
rem call NIMEX_link.bat NIMEX_bindEveryNCallback
rem call NIMEX_link.bat NIMEX_bindDoneCallback
call NIMEX_link.bat NIMEX_bindEventListener
call NIMEX_link.bat NIMEX_readDigitalU32
call NIMEX_link.bat NIMEX_writeDigitalU32
call NIMEX_link.bat NIMEX_addDigitalOutputChannel
call NIMEX_link.bat NIMEX_addDigitalInputChannel
call NIMEX_link.bat NIMEX_display
call NIMEX_link.bat NIMEX_deleteTask
call NIMEX_link.bat NIMEX_registerSamplesAcquiredPreprocessor
call NIMEX_link.bat NIMEX_registerSamplesAcquiredListener
call NIMEX_link.bat NIMEX_updateCounterOutput
call NIMEX_link.bat NIMEX_addCOFrequency
call NIMEX_link.bat NIMEX_addCOTime
call NIMEX_link.bat NIMEX_registerOutputDataPreprocessor
call NIMEX_link.bat NIMEX_putSample
call NIMEX_link.bat NIMEX_getSample
call NIMEX_link.bat NIMEX_acquireLock
call NIMEX_link.bat NIMEX_releaseLock
call NIMEX_link.bat NIMEX_commitTask
call NIMEX_link.bat NIMEX_updateDataSourceByCallback
rem call NIMEX_link.bat NIMEXEng_getDeviceNames

goto LINK_RETURN

:CLEAN
rem echo CLEAN...
del NIMEX_*.obj
del NIMEX_*.exp
del NIMEX_*.lib
rem del NIMEXEng_*.obj
rem del NIMEXEng_*.exp
rem del NIMEXEng_*.lib

goto CLEAN_RETURN

:DEPLOY
rem echo DEPLOY...
rem move NIMEX_memManagement.mexw32 ..\NIMEX_memManagement.mex

move NIMEX_*.mexw32 %NIMEX_ENV_TARGET_PATH%\.
move NIMEX_*.dll %NIMEX_ENV_TARGET_PATH%\.
rem move NIMEXEng_*.mexw32 ..\@nimexEngine\private\.
rem move *test*.mexw32 ..\.


goto DEPLOY_RETURN

:EOF
echo EOF
rem echo Subroutine %0 complete.