#!/bin/bash

ProjectDir=`pwd`
OutputDir="Debug/"
ProjectOutputDir="${ProjectDir}/${OutputDir}"
CommonInclude="-I${ProjectDir}/common -I${ProjectDir}/common/math -I${ProjectDir}/tinker"
CommonFlags="-Werror -std=c++11"
CommonExtDepsFlags=""

# BUILD TINKERLIB
echo "Building libtinker..."
mkdir -p $ProjectOutputDir/o/tinker

pushd $ProjectOutputDir/o/tinker > /dev/null
clang $CommonInclude $CommonFlags -c \
	$ProjectDir/common/data.cpp \
	$ProjectDir/common/platform_osx.cpp \
	$ProjectDir/common/shell.cpp \
	$ProjectDir/common/stringtable.cpp \
	$ProjectDir/common/stb.cpp \
	$ProjectDir/common/math/color.cpp \
	$ProjectDir/common/math/matrix.cpp \
	$ProjectDir/common/math/quaternion.cpp \
	$ProjectDir/common/math/vector.cpp \

cd ../..
libtool -static -o libtinker.a o/tinker/*.o
popd > /dev/null

# BUILD ENET
echo "Building libenet..."
mkdir -p $ProjectOutputDir/o/enet

ENetDirectory="$ProjectDir/../ext-deps/enet"
pushd $ProjectOutputDir/o/enet > /dev/null
mkdir -p "$OutputDir"
ENetInclude="-I$ProjectDir/../ext-deps/enet/include"
clang $CommonExtDepsFlags $ENetInclude -c \
	$ENetDirectory/callbacks.c \
	$ENetDirectory/compress.c \
	$ENetDirectory/host.c \
	$ENetDirectory/list.c \
	$ENetDirectory/packet.c \
	$ENetDirectory/peer.c \
	$ENetDirectory/protocol.c \
	$ENetDirectory/unix.c \

cd ../..
libtool -static -o libenet.a o/enet/*.o
popd > /dev/null

# BUILD TINKER
mkdir -p $OutputDir

SDLLibs="-framework SDL2"
TinkerLibInclude="-I/Library/Frameworks/SDL2.framework/Headers"
TinkerInclude="-I/System/Library/Frameworks/SDL2.framework/Headers $CommonInclude"
clang++ $SDLLibs $CommonFlags $TinkerInclude \
	tinker/main.cpp tinker/gamecode.cpp tinker/window.cpp \
	-o $OutputDir/tinker -L${ProjectOutputDir} -ltinker

