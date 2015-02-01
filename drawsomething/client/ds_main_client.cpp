#include "tinker_platform.h"
#include "../tinker/gamecode.h"
#include "shell.h"

extern "C" TDLLEXPORT bool GameInitialize(GameData* game_data, int argc, char** args)
{
	g_shell.Initialize(argc, args);

	return 1;
}

extern "C" TDLLEXPORT bool GameFrame(GameData* game_data)
{
	return 1;
}
