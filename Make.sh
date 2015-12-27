#! /bin/bash
# linux makefile for vgmConv
# Yes I should use a real makefile
# but fuck that shit

if [ "$1" == "install" ]; then
	# install the library
	cp src/vgmConv.h $prefix/include
	cp bin/libVgmConv.a $prefix/lib
	exit
fi

# generate config.h
echo "#ifndef _CONFIG_H_" > config.h
echo "#define _CONFIG_H_" >> config.h
if [ "$1" == "zlib" ]; then
	echo "#define HAS_ZLIB" >> config.h
	has_zlib="-lz"
fi
echo "#endif" >> config.h

# files and flags
APPNAME="vgmConv"
LIBNAME="libVgmConv.a"
SRCS="src/codec/*.cpp src/vgx/*.cpp src/*.cpp lib/*.cpp"
CFLAGS="-Os -fomit-frame-pointer -fno-rtti -fno-exceptions"
CFLAGS+=" -I./lib -I./."
LFLAGS=$has_zlib

# build vgmConv
gcc $CFLAGS $SRCS -o vgmConv.o -r -nostdlib
strip --strip-unneeded vgmConv.o
objcopy vgmConv.o -w --keep-global-symbols=globals.txt
ar -rcs bin/$LIBNAME vgmConv.o
gcc $CFLAGS src/app/*.cpp bin/$LIBNAME -o bin/$APPNAME $LFLAGS
strip bin/$APPNAME
