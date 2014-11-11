@echo off

echo Making NiMex project...

:ENV
call %DEV_TOOL_HOME%\jfrc_setDevelopmentEnvironmentVariables.bat
rem call %DEV_TOOL_HOME%\BuildInfo.bat
set "NIMEX_COMPILE_NON_MEX=%VS_COMPILER% /Zp4 /O2 /nologo /I. /I..\src\c /c -DNIMEX_LOCALTIME_DEPRECATED -DNIMEX_COMPILER_CL %WIN_INCLUDES% %NIDAQMX_INCLUDES% %GLIB_INCLUDES% %MATLAB_INCLUDES%"
rem set "NIMEX_COMPILE_NON_MEX=%VS_COMPILER% /nologo /I. /I..\src\c /c -DNIMEX_DEBUG_LOG -DNIMEX_VERBOSE_4 -DNIMEX_DEBUG_MEM_MANAGEMENT -DNIMEX_LOCALTIME_DEPRECATED -DNIMEX_COMPILER_CL %WIN_INCLUDES% %NIDAQMX_INCLUDES% %GLIB_INCLUDES% %MATLAB_INCLUDES%"
set "NIMEX_COMPILE_MEX=%NIMEX_COMPILE_NON_MEX% -DMATLAB_MEX_FILE"
set "NIMEX_LINK_NON_MEX=%VS_LINKER% /MACHINE:x86 /nologo /SUBSYSTEM:console %WIN_LIBS% %NIDAQMX_LIBS% %GLIB_LIBS% %MATLAB_LIBS% /DLL"
set "NIMEX_LINK_MEX=%NIMEX_LINK_NON_MEX% NIMEX_shared.lib /EXPORT:mexFunction"
set TARGET_PATH=..
set TARGET_PATH_ENGINE=..\..\..\@nimexEngine\private
set ORIGINAL_PATH=%PATH%


echo Generating build-time header file...
%NIMEX_COMPILE_NON_MEX% /FoNIMEX_BuildInfo.obj ..\src\c\NIMEX_BuildInfo.c
%VS_LINKER% /MACHINE:x86 /nologo /SUBSYSTEM:console %WIN_LIBS% %NIDAQMX_LIBS% %GLIB_LIBS% %MATLAB_LIBS% NIMEX_BuildInfo.obj /OUT:NIMEX_BuildInfo.exe
rem %VS_LINKER% /nologo /SUBSYSTEM:console %WIN_LIBS% %NIDAQMX_LIBS% %GLIB_LIBS% %MATLAB_LIBS% NIMEX_BuildInfo.obj /OUT:NIMEX_BuildInfo.exe
rem %NIMEX_LINK_NON_MEX% %MATLAB_LIBS% NIMEX_BuildInfo.obj /OUT:NIMEX_BuildInfo.exe
set "PATH=%PATH%;%MATLAB_HOME%\bin\win32;%VS_HOME%\Common7\IDE"
NIMEX_BuildInfo.exe %* > NIMEX_BuildInfo.h
del NIMEX_BuildInfo.obj
del NIMEX_BuildInfo.exe
echo -----------------------
echo NIMEX_BuildInfo.h:
type NIMEX_BuildInfo.h
echo -----------------------

:BUILD
echo Compiling shared components...
%NIMEX_COMPILE_NON_MEX% /FoNIMEX_memManagement.obj ..\src\c\NIMEX_memManagement.c
%NIMEX_COMPILE_NON_MEX% /FoNIMEX_genericStructures.obj ..\src\c\NIMEX_genericStructures.c
%NIMEX_COMPILE_NON_MEX% /FoNIMEX_utilities.obj ..\src\c\NIMEX_utilities.c
%NIMEX_COMPILE_NON_MEX% /FoNIMEX_Objects.obj ..\src\c\NIMEX_Objects.c
%NIMEX_COMPILE_NON_MEX% /FoNIMEX_Callbacks.obj ..\src\c\NIMEX_Callbacks.c
echo Linking shared components...
%NIMEX_LINK_NON_MEX% NIMEX_memManagement.obj NIMEX_genericStructures.obj NIMEX_utilities.obj NIMEX_Objects.obj NIMEX_Callbacks.obj /OUT:NIMEX_shared.dll /DEF:..\src\c\NIMEX_shared.def

echo Compiling MEX files...
%NIMEX_COMPILE_MEX% /FoNIMEX_createTask.obj ..\src\c\NIMEX_createTask.c
%NIMEX_COMPILE_MEX% /FoNIMEX_addAnalogInputChannel.obj ..\src\c\NIMEX_addAnalogInputChannel.c
%NIMEX_COMPILE_MEX% /FoNIMEX_setChannelProperty.obj ..\src\c\NIMEX_setChannelProperty.c
%NIMEX_COMPILE_MEX% /FoNIMEX_startTask.obj ..\src\c\NIMEX_startTask.c
%NIMEX_COMPILE_MEX% /FoNIMEX_readAnalogF64.obj ..\src\c\NIMEX_readAnalogF64.c
%NIMEX_COMPILE_MEX% /FoNIMEX_stopTask.obj ..\src\c\NIMEX_stopTask.c
%NIMEX_COMPILE_MEX% /FoNIMEX_addAnalogOutputChannel.obj ..\src\c\NIMEX_addAnalogOutputChannel.c
%NIMEX_COMPILE_MEX% /FoNIMEX_writeAnalogF64.obj ..\src\c\NIMEX_writeAnalogF64.c
%NIMEX_COMPILE_MEX% /FoNIMEX_sendTrigger.obj ..\src\c\NIMEX_sendTrigger.c
%NIMEX_COMPILE_MEX% /FoNIMEX_setTaskProperty.obj ..\src\c\NIMEX_setTaskProperty.c
%NIMEX_COMPILE_MEX% /FoNIMEX_getTaskProperty.obj ..\src\c\NIMEX_getTaskProperty.c
%NIMEX_COMPILE_MEX% /FoNIMEX_getChannelProperty.obj ..\src\c\NIMEX_getChannelProperty.c
%NIMEX_COMPILE_MEX% /FoNIMEX_bindEventListener.obj ..\src\c\NIMEX_bindEventListener.c
%NIMEX_COMPILE_MEX% /FoNIMEX_readDigitalU32.obj ..\src\c\NIMEX_readDigitalU32.c
%NIMEX_COMPILE_MEX% /FoNIMEX_writeDigitalU32.obj ..\src\c\NIMEX_writeDigitalU32.c
%NIMEX_COMPILE_MEX% /FoNIMEX_addDigitalOutputChannel.obj ..\src\c\NIMEX_addDigitalOutputChannel.c
%NIMEX_COMPILE_MEX% /FoNIMEX_addDigitalInputChannel.obj ..\src\c\NIMEX_addDigitalInputChannel.c
%NIMEX_COMPILE_MEX% /FoNIMEX_display.obj ..\src\c\NIMEX_display.c
%NIMEX_COMPILE_MEX% /FoNIMEX_registerSamplesAcquiredPreprocessor.obj ..\src\c\NIMEX_registerSamplesAcquiredPreprocessor.c
%NIMEX_COMPILE_MEX% /FoNIMEX_registerSamplesAcquiredListener.obj ..\src\c\NIMEX_registerSamplesAcquiredListener.c
%NIMEX_COMPILE_MEX% /FoNIMEX_updateCounterOutput.obj ..\src\c\NIMEX_updateCounterOutput.c
%NIMEX_COMPILE_MEX% /FoNIMEX_addCOFrequency.obj ..\src\c\NIMEX_addCOFrequency.c
%NIMEX_COMPILE_MEX% /FoNIMEX_addCOTime.obj ..\src\c\NIMEX_addCOTime.c
%NIMEX_COMPILE_MEX% /FoNIMEX_registerOutputDataPreprocessor.obj ..\src\c\NIMEX_registerOutputDataPreprocessor.c
%NIMEX_COMPILE_MEX% /FoNIMEX_putSample.obj ..\src\c\NIMEX_putSample.c
%NIMEX_COMPILE_MEX% /FoNIMEX_getSample.obj ..\src\c\NIMEX_getSample.c
%NIMEX_COMPILE_MEX% /FoNIMEX_acquireLock.obj ..\src\c\NIMEX_acquireLock.c
%NIMEX_COMPILE_MEX% /FoNIMEX_releaseLock.obj ..\src\c\NIMEX_releaseLock.c
%NIMEX_COMPILE_MEX% /FoNIMEX_updateDataSourceByCallback.obj ..\src\c\NIMEX_updateDataSourceByCallback.c
%NIMEX_COMPILE_MEX% /FoNIMEX_clearBuffer.obj ..\src\c\NIMEX_clearBuffer.c
%NIMEX_COMPILE_MEX% /FoNIMEX_isOutput.obj ..\src\c\NIMEX_isOutput.c
%NIMEX_COMPILE_MEX% /FoNIMEX_commitTask.obj ..\src\c\NIMEX_commitTask.c
%NIMEX_COMPILE_MEX% /FoNIMEX_deleteTask.obj ..\src\c\NIMEX_deleteTask.c
%NIMEX_COMPILE_MEX% /FoNIMEXEng_connectTerms.obj ..\src\c\NIMEXEng_connectTerms.c
%NIMEX_COMPILE_MEX% /FoNIMEXEng_disconnectTerms.obj ..\src\c\NIMEXEng_disconnectTerms.c
%NIMEX_COMPILE_MEX% /FoNIMEXEng_tristateOutputTerm.obj ..\src\c\NIMEXEng_tristateOutputTerm.c
%NIMEX_COMPILE_MEX% /FoNIMEXEng_resetDevice.obj ..\src\c\NIMEXEng_resetDevice.c
%NIMEX_COMPILE_MEX% /FoNIMEXEng_getDeviceNames.obj ..\src\c\NIMEXEng_getDeviceNames.c

echo Linking MEX files...
%NIMEX_LINK_MEX% NIMEX_createTask.obj /OUT:NIMEX_createTask.mexw32
%NIMEX_LINK_MEX% NIMEX_addAnalogInputChannel.obj /OUT:NIMEX_addAnalogInputChannel.mexw32
%NIMEX_LINK_MEX% NIMEX_setChannelProperty.obj /OUT:NIMEX_setChannelProperty.mexw32
%NIMEX_LINK_MEX% NIMEX_startTask.obj /OUT:NIMEX_startTask.mexw32
%NIMEX_LINK_MEX% NIMEX_readAnalogF64.obj /OUT:NIMEX_readAnalogF64.mexw32
%NIMEX_LINK_MEX% NIMEX_stopTask.obj /OUT:NIMEX_stopTask.mexw32
%NIMEX_LINK_MEX% NIMEX_addAnalogOutputChannel.obj /OUT:NIMEX_addAnalogOutputChannel.mexw32
%NIMEX_LINK_MEX% NIMEX_writeAnalogF64.obj /OUT:NIMEX_writeAnalogF64.mexw32
%NIMEX_LINK_MEX% NIMEX_sendTrigger.obj /OUT:NIMEX_sendTrigger.mexw32
%NIMEX_LINK_MEX% NIMEX_setTaskProperty.obj /OUT:NIMEX_setTaskProperty.mexw32
%NIMEX_LINK_MEX% NIMEX_getTaskProperty.obj /OUT:NIMEX_getTaskProperty.mexw32
%NIMEX_LINK_MEX% NIMEX_getChannelProperty.obj /OUT:NIMEX_getChannelProperty.mexw32
%NIMEX_LINK_MEX% NIMEX_bindEventListener.obj /OUT:NIMEX_bindEventListener.mexw32
%NIMEX_LINK_MEX% NIMEX_readDigitalU32.obj /OUT:NIMEX_readDigitalU32.mexw32
%NIMEX_LINK_MEX% NIMEX_writeDigitalU32.obj /OUT:NIMEX_writeDigitalU32.mexw32
%NIMEX_LINK_MEX% NIMEX_addDigitalOutputChannel.obj /OUT:NIMEX_addDigitalOutputChannel.mexw32
%NIMEX_LINK_MEX% NIMEX_addDigitalInputChannel.obj /OUT:NIMEX_addDigitalInputChannel.mexw32
%NIMEX_LINK_MEX% NIMEX_display.obj /OUT:NIMEX_display.mexw32
%NIMEX_LINK_MEX% NIMEX_registerSamplesAcquiredPreprocessor.obj /OUT:NIMEX_registerSamplesAcquiredPreprocessor.mexw32
%NIMEX_LINK_MEX% NIMEX_registerSamplesAcquiredListener.obj /OUT:NIMEX_registerSamplesAcquiredListener.mexw32
%NIMEX_LINK_MEX% NIMEX_updateCounterOutput.obj /OUT:NIMEX_updateCounterOutput.mexw32
%NIMEX_LINK_MEX% NIMEX_addCOFrequency.obj /OUT:NIMEX_addCOFrequency.mexw32
%NIMEX_LINK_MEX% NIMEX_addCOTime.obj /OUT:NIMEX_addCOTime.mexw32
%NIMEX_LINK_MEX% NIMEX_registerOutputDataPreprocessor.obj /OUT:NIMEX_registerOutputDataPreprocessor.mexw32
%NIMEX_LINK_MEX% NIMEX_putSample.obj /OUT:NIMEX_putSample.mexw32
%NIMEX_LINK_MEX% NIMEX_getSample.obj /OUT:NIMEX_getSample.mexw32
%NIMEX_LINK_MEX% NIMEX_acquireLock.obj /OUT:NIMEX_acquireLock.mexw32
%NIMEX_LINK_MEX% NIMEX_releaseLock.obj /OUT:NIMEX_releaseLock.mexw32
%NIMEX_LINK_MEX% NIMEX_updateDataSourceByCallback.obj /OUT:NIMEX_updateDataSourceByCallback.mexw32
%NIMEX_LINK_MEX% NIMEX_clearBuffer.obj /OUT:NIMEX_clearBuffer.mexw32
%NIMEX_LINK_MEX% NIMEX_isOutput.obj /OUT:NIMEX_isOutput.mexw32
%NIMEX_LINK_MEX% NIMEX_commitTask.obj /OUT:NIMEX_commitTask.mexw32
%NIMEX_LINK_MEX% NIMEX_deleteTask.obj /OUT:NIMEX_deleteTask.mexw32
%NIMEX_LINK_MEX% NIMEXEng_connectTerms.obj /OUT:NIMEXEng_connectTerms.mexw32
%NIMEX_LINK_MEX% NIMEXEng_disconnectTerms.obj /OUT:NIMEXEng_disconnectTerms.mexw32
%NIMEX_LINK_MEX% NIMEXEng_tristateOutputTerm.obj /OUT:NIMEXEng_tristateOutputTerm.mexw32
%NIMEX_LINK_MEX% NIMEXEng_resetDevice.obj /OUT:NIMEXEng_resetDevice.mexw32
%NIMEX_LINK_MEX% NIMEXEng_getDeviceNames.obj /OUT:NIMEXEng_getDeviceNames.mexw32

:DEPLOY
echo Deploying to '%TARGET_PATH%' and '%TARGET_PATH_ENGINE%'...
move *.dll %TARGET_PATH%\.
move *NIMEXEng*.mexw32 %TARGET_PATH_ENGINE%\.
move *.mexw32 %TARGET_PATH%\.

:CLEAN
echo Cleaning...
del *.obj
del *.lib
del *.exp
del NIMEX_BuildInfo.h

:END
:DONE
set "PATH=%ORIGINAL_PATH%"
echo DONE