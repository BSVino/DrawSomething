#!/bin/bash

set -o nounset
set -e

ProjectDir=`pwd`
OutputDir="Debug/"
ProjectOutputDir="${ProjectDir}/${OutputDir}"
CommonInclude="-I${ProjectDir}/common -I${ProjectDir}/common/math -I${ProjectDir}/tinker"
CommonFlags="-Werror -std=c++11 -g"
CommonExtDepsFlags=""
CommonLinkerFlags="-L${ProjectOutputDir}"


# BUILD TINKERLIB
echo "Building libtinker..."
mkdir -p $ProjectOutputDir/o/tinker

pushd $ProjectOutputDir/o/tinker > /dev/null
clang $CommonInclude $CommonFlags -c \
	$ProjectDir/common/data.cpp $ProjectDir/common/platform_osx.cpp \
	$ProjectDir/common/shell.cpp $ProjectDir/common/stringtable.cpp \
	$ProjectDir/common/stb.cpp $ProjectDir/common/math/color.cpp \
	$ProjectDir/common/math/matrix.cpp $ProjectDir/common/math/quaternion.cpp \
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
	$ENetDirectory/callbacks.c $ENetDirectory/compress.c \
	$ENetDirectory/host.c $ENetDirectory/list.c \
	$ENetDirectory/packet.c $ENetDirectory/peer.c \
	$ENetDirectory/protocol.c $ENetDirectory/unix.c \

cd ../..
libtool -static -o libenet.a o/enet/*.o
popd > /dev/null


# BUILD TINKER
echo "Building tinker..."
mkdir -p $OutputDir

SDLLibs="-framework SDL2"
TinkerInclude="-I/System/Library/Frameworks/SDL2.framework/Headers $CommonInclude"
clang++ $SDLLibs $CommonFlags $TinkerInclude \
	tinker/main.cpp tinker/gamecode.cpp tinker/window.cpp \
	-o $OutputDir/tinker $CommonLinkerFlags -ltinker


# BUILD IGOR
echo "Building igor..."

clang++ $CommonFlags $CommonInclude tools/igor/igor.cpp \
	-o $OutputDir/igor $CommonLinkerFlags -ltinker


# RUN IGOR!
echo "Running igor..."
$ProjectOutputDir/igor -game "$ProjectDir/install" -header "$ProjectDir/tinker/assets.h" -source "$ProjectDir/tinker/assets.cpp"


# BUILD SERVER
echo "Building server..."

GameInclude="-I$ProjectDir/drawsomething -I$ProjectDir/../ext-deps/enet/include -I$ProjectDir/drawsomething/shared $CommonInclude"

mkdir -p $OutputDir

ServerInclude="-I$ProjectDir/drawsomething/server $GameInclude"
clang++ $CommonFlags $ServerInclude \
	drawsomething/server/ds_main_server.cpp \
	drawsomething/shared/buckets.cpp \
	drawsomething/server/server_buckets.cpp \
	drawsomething/shared/net_ds.cpp \
	tinker/shared/net_shared.cpp \
	tinker/server/net_host.cpp \
	drawsomething/server/game/s_artist.cpp \
	-dynamiclib -o $OutputDir/server.dylib $CommonLinkerFlags -ltinker -lenet


# BUILD CLIENT
echo "Building client..."

ClientInclude="-I$ProjectDir/../Viewback/server -I$ProjectDir/drawsomething/client $GameInclude"
ClientPreprocs="-DUSE_SKYBOX -DCLIENT_LIBRARY -DVIEWBACK_TIME_DOUBLE"

clang++ $CommonFlags $ClientInclude $ClientPreprocs \
	drawsomething/client/ds_main_client.cpp \
	tinker/assets.cpp \
	drawsomething/client/game/c_artist.cpp \
	drawsomething/shared/artist.cpp \
	drawsomething/client/renderer/ds_renderer.cpp \
	tinker/client/renderer/renderer.cpp \
	tinker/client/renderer/context.cpp \
	tinker/client/renderer/shaders.cpp \
	tinker/client/renderer/skybox.cpp \
	drawsomething/shared/net_ds.cpp \
	tinker/shared/net_shared.cpp \
	tinker/client/net_client.cpp \
	drawsomething/client/viewback.cpp \
	-dynamiclib -o $OutputDir/client.dylib $CommonLinkerFlags -ltinker -lenet -framework OpenGL

