#include "window.h"
#include "shell.h"

#include "gamecode.h"

#include <direct.h>
#define chdir _chdir

GameCode g_gamecode;

int main(int argc, char** args)
{
	g_shell.Initialize(argc, args);

	if (g_shell.GetCommandLineSwitchValue("-game"))
		chdir(g_shell.GetCommandLineSwitchValue("-game"));

	Window window;

	window.Open("Tinker", 1280, 720);

	g_gamecode.Initialize("drawsomething.dll");

	while (true)
	{
		g_gamecode.m_game_frame();
	}

	return 0;
}
