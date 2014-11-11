@echo off

@setlocal

rem echo NIMEX_link: %1%.obj
rem C:\lcc\bin\lcclnk -o "%1%.mexw32" -dll "C:\Program Files\MATLAB\R2007a\extern\lib\win32\lcc\mexFunction.def" %1%.obj %2% -s libmx.lib libmex.lib libmat.lib NIDAQmx.lib

rem echo %NIMEX_ENV_LINKER% %NIMEX_ENV_LINKER_ARGS% -o %1.mexw32 %1.obj %2 %3 %4 %NIMEX_ENV_LINKER_LIBS%
rem %NIMEX_ENV_LINKER% %NIMEX_ENV_LINKER_ARGS% -o %1.mexw32 %1.obj %2 %3 %4 %NIMEX_ENV_LINKER_LIBS%

rem echo %NIMEX_ENV_LINKER% %NIMEX_ENV_LINKER_ARGS% %1.obj %NIMEX_ENV_SHARED_OBJECT_FILES% %NIMEX_ENV_LINKER_LIBS% -o %1.mexw32
%NIMEX_ENV_LINKER% %NIMEX_ENV_LINKER_ARGS% %1.obj %NIMEX_ENV_SHARED_OBJECT_FILES% %NIMEX_ENV_LINKER_LIBS% %NIMEX_ENV_LINKER_TARGET_PREFIX%%1.mexw32
rem %NIMEX_ENV_LINKER_LIBS%
