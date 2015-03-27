@echo off
setlocal

:: Activate the x86 -> x64 cross compiler environment.
call "%VS120COMNTOOLS%/../../VC/vcvarsall.bat" x86_amd64

set arg1=%1

set FullBuild="0"
if /i "%arg1%"=="full" goto full_build
goto end_build_option
:full_build
set FullBuild="1"
:end_build_option

set CleanBuild="0"
if /i "%arg1%"=="clean" goto clean_build
goto end_clean_option
:clean_build
set CleanBuild="1"
:end_clean_option

set ProjectDir=%cd%
set OutputDir=x64/Debug/
set ProjectOutputDir=%cd%/x64/Debug/
set CommonPreprocs=-DNOMINMAX -D_CRT_SECURE_NO_WARNINGS -D_WINDLL -D_HAS_EXCEPTIONS=0 -D_CRT_NON_CONFORMING_SWPRINTFS
set CommonFlagsBase=-MTd -nologo -fp:fast -Gm -Od -WX- -Zi -GR- -EHa- -Oi -FC -Fd"%OutputDir%/vc120.pdb" -Fo"%OutputDir%" -Fp"%OutputDir%"
set CommonFlags=%CommonFlagsBase% -W4
set CommonExtDepFlags=%CommonFlagsBase% -W3
set CommonLinkerFlags=-nologo -incremental -debug -LIBPATH:"%ProjectOutputDir%" user32.lib gdi32.lib winmm.lib shell32.lib Ws2_32.lib Opengl32.lib

set CommonInclude=/I"%cd%\common" /I"%cd%\common\math" /I"%cd%/tinker"

if /i "%arg1%"=="run" goto run_game
goto end_run_game
:run_game
WDExpress.exe "%ProjectOutputDir%\Tinker2.exe"
exit /b 0
:end_run_game

if %CleanBuild%=="0" goto skip_clean
echo Deleting intermediate files...
rmdir /s /q "%ProjectDir%/tinker/%OutputDir%"
rmdir /s /q "%ProjectDir%/playroom/%OutputDir%"
rmdir /s /q "%ProjectDir%/tinker/enet/%OutputDir%"
rmdir /s /q "%ProjectDir%/drawsomething/client/%OutputDir%"
rmdir /s /q "%ProjectDir%/drawsomething/server/%OutputDir%"
rmdir /s /q "%ProjectOutputDir%"
exit /b 0
:skip_clean

if not exist "%ProjectOutputDir%" mkdir "%ProjectOutputDir%"

rmdir /s /q "%ProjectOutputDir%/tmp"

:: BUILD TINKERLIB
cl %CommonFlags% %CommonPreprocs% %CommonInclude%^
    common/data.cpp common/platform_win32.cpp common/shell.cpp common/stringtable.cpp common/stb.cpp^
    common/math/color.cpp common/math/matrix.cpp common/math/quaternion.cpp common/math/vector.cpp^
    /c
if %errorlevel% neq 0 exit /b %errorlevel%

pushd "%ProjectOutputDir%"
lib -nologo data.obj platform_win32.obj shell.obj stringtable.obj stb.obj^
    color.obj matrix.obj quaternion.obj vector.obj /out:"%ProjectOutputDir%/TinkerLib.lib"
if %errorlevel% neq 0 exit /b %errorlevel%
popd

:: BUILD ENET
if %FullBuild% == "0" goto skip_enet

set ENetDirectory="%ProjectDir%/../ext-deps/enet/"
pushd "tinker/enet"
if not exist "%OutputDir%" mkdir "%OutputDir%"

set ENetInclude=/I"%ProjectDir%\..\ext-deps\enet\include" %CommonInclude%
cl %CommonExtDepFlags% %CommonPreprocs% %ENetInclude% -W3 %ENetDirectory%/callbacks.c %ENetDirectory%/compress.c %ENetDirectory%/host.c %ENetDirectory%/list.c %ENetDirectory%/packet.c %ENetDirectory%/peer.c %ENetDirectory%/protocol.c %ENetDirectory%/win32.c /c
if %errorlevel% neq 0 exit /b %errorlevel%
popd

pushd "%ProjectOutputDir%"
set ENetObjsDirectory=%ProjectDir%/tinker/enet/%OutputDir%
lib -nologo %ENetObjsDirectory%/callbacks.obj %ENetObjsDirectory%/compress.obj %ENetObjsDirectory%/host.obj %ENetObjsDirectory%/list.obj %ENetObjsDirectory%/packet.obj %ENetObjsDirectory%/peer.obj %ENetObjsDirectory%/protocol.obj %ENetObjsDirectory%/win32.obj /out:"%ProjectOutputDir%/enet.lib"
if %errorlevel% neq 0 exit /b %errorlevel%
popd

:skip_enet

:: BUILD TINKER
pushd tinker
if not exist "%OutputDir%" mkdir "%OutputDir%"

set SDLLibs="%ProjectDir%\..\ext-deps\SDL2-2.0.2\lib\x64\SDL2.lib" "%ProjectDir%\..\ext-deps\SDL2-2.0.2\lib\x64\SDL2main.lib"
set TinkerLibInclude=/I"%ProjectDir%\..\ext-deps\SDL2-2.0.2\include"
set TinkerInclude=/I"%ProjectDir%\..\ext-deps\SDL2-2.0.2\include" %CommonInclude%
cl %CommonFlags% %CommonPreprocs% %TinkerInclude% gamecode.cpp main.cpp window.cpp /link %CommonLinkerFlags% TinkerLib.lib %SDLLibs% /OUT:"%ProjectOutputDir%\Tinker2.exe" /PDB:"%ProjectOutputDir%\Tinker2.pdb" 
:: Error here is OK if we're rebuilding for debug.
::if %errorlevel% neq 0 exit /b %errorlevel%
popd

:: BUILD PLAYROOM
if %FullBuild% == "0" goto skip_playroom
pushd "tools/playroom"
if not exist "%OutputDir%" mkdir "%OutputDir%"

cl %CommonFlags% %CommonPreprocs% %CommonInclude% playroom.cpp /link %CommonLinkerFlags% TinkerLib.lib /OUT:"%ProjectOutputDir%\playroom.exe" /PDB:"%ProjectOutputDir%\playroom.pdb"
if %errorlevel% neq 0 exit /b %errorlevel%
popd

:: RUN PLAYROOM!

call "%ProjectOutputDir%\playroom" -game "%ProjectDir%\install" -header "%ProjectDir%\tinker\assets.h" -source "%ProjectDir%\tinker\assets.cpp"
if %errorlevel% neq 0 exit /b %errorlevel%

:skip_playroom

set GameInclude=/I"%ProjectDir%\drawsomething" /I"%ProjectDir%\..\ext-deps\enet\include" /I"%ProjectDir%\drawsomething\shared" %CommonInclude%

:: BUILD SERVER
pushd "drawsomething/server"
if not exist "%OutputDir%" mkdir "%OutputDir%"

set ServerInclude=/I"%ProjectDir%\drawsomething\server" %GameInclude%

cl %CommonFlags% %CommonPreprocs% %ServerInclude%^
	ds_main_server.cpp ../shared/buckets.cpp server_buckets.cpp^
	../shared/net_ds.cpp ../../tinker/shared/net_shared.cpp ../../tinker/server/net_host.cpp^
	game/s_artist.cpp^
	/link %CommonLinkerFlags% -dll TinkerLib.lib enet.lib /OUT:"%ProjectOutputDir%\DrawSomethingServer.dll" /PDB:"%ProjectOutputDir%\DrawSomethingServer.pdb" 
if %errorlevel% neq 0 exit /b %errorlevel%
popd

:: BUILD CLIENT
pushd "drawsomething/client"
if not exist "%OutputDir%" mkdir "%OutputDir%"

set ClientInclude=/I"%ProjectDir%\..\viewback\server" /I"%ProjectDir%\..\ext-deps\gl3w\include" /I"%ProjectDir%\drawsomething\client" /I"%ProjectDir%\drawsomething\client" %GameInclude%
set ClientPreprocs=-DUSE_SKYBOX -DCLIENT_LIBRARY -DVIEWBACK_TIME_DOUBLE %CommonPreprocs%

cl %CommonExtDepFlags% %ClientInclude% %ProjectDir%\..\ext-deps\gl3w\src\gl3w.c /c
cl %CommonFlags% %ClientPreprocs% %ClientInclude%^
    ds_main_client.cpp ../../tinker/assets.cpp^
    game/c_artist.cpp ../shared/artist.cpp^
    renderer/ds_renderer.cpp ../../tinker/client/renderer/renderer.cpp ../../tinker/client/renderer/context.cpp ../../tinker/client/renderer/shaders.cpp ../../tinker/client/renderer/skybox.cpp^
    ../shared/net_ds.cpp ../../tinker/shared/net_shared.cpp ../../tinker/client/net_client.cpp^
    %ProjectDir%\..\viewback\server\viewback.c^
    /link %CommonLinkerFlags% -dll TinkerLib.lib enet.lib %OutputDir%\gl3w.obj /OUT:"%ProjectOutputDir%\DrawSomethingClient.dll" /PDB:"%ProjectOutputDir%\DrawSomethingClient.pdb" 
if %errorlevel% neq 0 exit /b %errorlevel%
popd

echo Copying SDL DLL into %ProjectOutputDir%
copy "%ProjectDir%\..\ext-deps\SDL2-2.0.2\lib\x64\SDL2.dll" "%ProjectOutputDir%"

