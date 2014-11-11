@echo off

@setlocal

set name=%1
shift
shift

echo NIMEX_compile: "%name%.c"

rem echo %NIMEX_ENV_COMPILER% %NIMEX_ENV_COMPILER_ARGS% %NIMEX_ENV_INCLUDES% %NIMEX_ENV_TARGET_PREFIX%%name%.obj %name%.c
%NIMEX_ENV_COMPILER% %NIMEX_ENV_COMPILER_ARGS% %NIMEX_ENV_INCLUDES% %NIMEX_ENV_COMPILER_TARGET_PREFIX%%name%.obj %name%.c