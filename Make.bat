@rem Mingw makefile for vgmConv
@call egcc.bat
setlocal
@echo off

rem iterate command line args
for %%x in (%*) do (
if "%%x"=="zlib" set has_zlib=-lz
if "%%x"=="wide" set unicode=yes
if "%%x"=="install" set install=yes
)

rem install library
if defined install (
	copy src\vgmConv.h %prefix%\include
	copy bin\libVgmConv.a %prefix%\lib
	copy bin\libVgmConvW.a %prefix%\lib
	exit /B
)

rem generate config.h
echo #ifndef _CONFIG_H_ > config.h
echo #define _CONFIG_H_ >> config.h
if defined has_zlib (
	echo #define HAS_ZLIB >> config.h
)
if defined unicode (
	echo #define UNICODE >> config.h
	echo #define _UNICODE >> config.h
)
echo #endif >> config.h

rem output names
if defined unicode (
	set APPNAME=vgmConvW.exe
	set LIBNAME=libVgmConvW.a
) else (
	set APPNAME=vgmConv.exe
	set LIBNAME=libVgmConv.a
)

rem files and flags
set SRCS=lib\*.cpp src\*.cpp src\vgx\*.cpp src\codec\*.cpp
set CFLAGS=%CFLAGS2% -I./lib -I./.
set LFLAGS=%LFLAGS% %has_zlib%

rem build vgmConv
echo // vgmConv.cc > obj\vgmConv.cc
for %%f in (%SRCS%) do (
	echo #include "%%f" >> obj\vgmConv.cc
)
gcc %CFLAGS% obj\vgmConv.cc -c -o obj\vgmConv.o
ar -rcs bin\%LIBNAME% obj\vgmConv.o
gcc %CFLAGS% src\app\*.cpp bin\%LIBNAME% -o bin\%APPNAME% %LFLAGS%





:end
endlocal
