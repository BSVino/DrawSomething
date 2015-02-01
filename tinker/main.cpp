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

	g_server_code.Initialize("drawsomethingserver.dll", 1024*1024);
	g_client_code.Initialize("drawsomethingclient.dll", 1024*1024);

	double frame_end_time = 0;
	double frame_start_time = 0;

#ifdef _DEBUG
	double target_frame_time = 1.0f/30;
#else
	double target_frame_time = 1.0f/60;
#endif

	bool game_active = true;

	// TODO: Make some callbacks into the main tinker2 binary to eliminate duplicate code.
	if (!g_server_code.m_game_init(&g_server_code.m_game_data, argc, args))
		return 1;

	if (!g_client_code.m_game_init(&g_client_code.m_game_data, argc, args))
		return 1;

	SetLowPeriodScheduler();

	while (window.IsOpen() && game_active)
	{
		g_server_code.Refresh();
		g_client_code.Refresh();

		frame_end_time = window.GetTime();

		double next_frame_time = frame_start_time + target_frame_time;

		double time_to_sleep_seconds = next_frame_time - frame_end_time;
		if (time_to_sleep_seconds > 0.001)
			SleepMS((size_t)(time_to_sleep_seconds * 1000));

		frame_start_time = window.GetTime();

		game_active = g_server_code.m_game_frame(&g_server_code.m_game_data);
		game_active &= g_client_code.m_game_frame(&g_client_code.m_game_data);
	}

	ClearLowPeriodScheduler();

	return 0;
}
