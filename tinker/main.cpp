#include "window.h"

#include "shell.h"
#include "tinker_platform.h"

#include "gamecode.h"
#include "input.h"

#include <direct.h>
#define chdir _chdir

GameCode g_server_code;
GameCode g_client_code;

int main(int argc, char** args)
{
	g_shell.Initialize(argc, args);

	if (g_shell.HasCommandLineSwitchValue("-game"))
		chdir(g_shell.GetCommandLineSwitchValue("-game"));

	bool server = true;
	bool client = true;

	if (g_shell.HasCommandLineSwitchValue("-server"))
		client = false;

	if (g_shell.HasCommandLineSwitchValue("-connect"))
		server = false;

	Window window;

	if (client)
		window.Open("Tinker", 1280, 720);

	ControlData input;

	if (server)
		g_server_code.Initialize("drawsomethingserver.dll", &window.m_data, &input);

	if (client)
		g_client_code.Initialize("drawsomethingclient.dll", &window.m_data, &input);

#ifdef _DEBUG
	double target_frame_time = 1.0f/30;
#else
	double target_frame_time = 1.0f/60;
#endif

	bool game_active = true;

	// TODO: Make some callbacks into the main tinker2 binary to eliminate duplicate code.
	if (server && !g_server_code.m_game_init(&g_server_code.m_game_data, argc, args))
		return 1;

	if (client && !g_client_code.m_game_init(&g_client_code.m_game_data, argc, args))
		return 1;

	SetLowPeriodScheduler();

	if (server && client)
	{
		double last_frame_start_time = 0;
		double game_time = 0;
		while (window.IsOpen() && game_active)
		{
			double frame_start_time = window.GetTime();

			double frame_time = frame_start_time - last_frame_start_time;
			last_frame_start_time = frame_start_time;

			if (frame_time > 1.0f / 8)
				frame_time = 1.0f / 8;

			game_time += frame_time;

			g_server_code.Refresh();
			g_client_code.Refresh();

			g_server_code.m_game_data.m_game_time = game_time;
			g_server_code.m_game_data.m_frame_time = frame_time;
			g_client_code.m_game_data.m_game_time = game_time;
			g_client_code.m_game_data.m_frame_time = frame_time;

			window.PollEvents(&input);

			game_active = g_server_code.m_game_frame(&g_server_code.m_game_data);
			game_active &= g_client_code.m_game_frame(&g_client_code.m_game_data);

			window.SwapBuffers();

			double frame_end_time = window.GetTime();

			double next_frame_time = frame_start_time + target_frame_time;

			double time_to_sleep_seconds = next_frame_time - frame_end_time;
			if (time_to_sleep_seconds > 0.001)
				SleepMS((size_t)(time_to_sleep_seconds * 1000));
			else
				TCheck(false);
		}
	}
	else if (server)
	{
		double last_frame_start_time = 0;
		double game_time = 0;
		while (game_active)
		{
			double frame_start_time = window.GetTime();

			double frame_time = frame_start_time - last_frame_start_time;
			last_frame_start_time = frame_start_time;

			if (frame_time > 1.0f / 8)
				frame_time = 1.0f / 8;

			game_time += frame_time;

			g_server_code.Refresh();

			g_server_code.m_game_data.m_game_time = game_time;
			g_server_code.m_game_data.m_frame_time = frame_time;

			game_active = g_server_code.m_game_frame(&g_server_code.m_game_data);

			double frame_end_time = window.GetTime();

			double next_frame_time = frame_start_time + target_frame_time;

			double time_to_sleep_seconds = next_frame_time - frame_end_time;
			if (time_to_sleep_seconds > 0.001)
				SleepMS((size_t)(time_to_sleep_seconds * 1000));
			else
				TCheck(false);
		}
	}
	else // client only
	{
		double last_frame_start_time = 0;
		double game_time = 0;
		while (window.IsOpen() && game_active)
		{
			double frame_start_time = window.GetTime();

			double frame_time = frame_start_time - last_frame_start_time;
			last_frame_start_time = frame_start_time;

			if (frame_time > 1.0f / 8)
				frame_time = 1.0f / 8;

			game_time += frame_time;

			g_client_code.Refresh();

			window.PollEvents(&input);

			g_client_code.m_game_data.m_game_time = game_time;
			g_client_code.m_game_data.m_frame_time = frame_time;

			game_active = g_client_code.m_game_frame(&g_client_code.m_game_data);

			window.SwapBuffers();

			double frame_end_time = window.GetTime();

			double next_frame_time = frame_start_time + target_frame_time;

			double time_to_sleep_seconds = next_frame_time - frame_end_time;
			if (time_to_sleep_seconds > 0.001)
				SleepMS((size_t)(time_to_sleep_seconds * 1000));
			else
				TCheck(false);
		}
	}

	ClearLowPeriodScheduler();

	return 0;
}
