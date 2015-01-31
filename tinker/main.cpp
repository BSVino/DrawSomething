#include "window.h"

#include "shell.h"
#include "tinker_platform.h"

#include "gamecode.h"

#include <direct.h>
#define chdir _chdir

GameCode g_server_code;
GameCode g_client_code;

int main(int argc, char** args)
{
	g_shell.Initialize(argc, args);

	if (g_shell.GetCommandLineSwitchValue("-game"))
		chdir(g_shell.GetCommandLineSwitchValue("-game"));

	Window window;

	window.Open("Tinker", 1280, 720);

	g_server_code.Initialize("drawsomethingserver.dll");
	g_client_code.Initialize("drawsomethingclient.dll");

	double frame_end_time = 0;
	double frame_start_time = 0;

#ifdef _DEBUG
	double target_frame_time = 1.0f/30;
#else
	double target_frame_time = 1.0f/60;
#endif

	bool game_active = true;

	while (window.IsOpen() && game_active)
	{
		frame_end_time = window.GetTime();

		double next_frame_time = frame_start_time + target_frame_time;

		double time_to_sleep_seconds = next_frame_time - frame_end_time;
		if (time_to_sleep_seconds > 0.001)
			SleepMS((size_t)(time_to_sleep_seconds * 1000));

		frame_start_time = window.GetTime();

		game_active = g_server_code.m_game_frame();
		game_active &= g_client_code.m_game_frame();
	}

	return 0;
}
