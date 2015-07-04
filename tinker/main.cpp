#include "window.h"

#include "shell.h"
#include "tinker_platform.h"

#include "gamecode.h"
#include "input.h"

GameCode g_server_code;
GameCode g_client_code;

void WriteFunData(GameCode* server, GameCode* client);

int main(int argc, char** args)
{
	g_shell.Initialize(argc, args);

#ifdef __APPLE__
	char* last_slash = strrchr(args[0], '/');

	if (strstr(args[0], "Contents/MacOS") == last_slash - 14)
	{
		// We're being run from inside the app bundle. Change our
		// directory to the resources directory. That's where all
		// the stuff is.
		char resources[2048];
		strncpy(resources, args[0], (last_slash - args[0]));
		strcpy(strrchr(resources, '/'), "/Resources");
		SetCurrentDirectory(resources);
	}
#endif

	if (g_shell.HasCommandLineSwitchValue("-game"))
		SetCurrentDirectory(g_shell.GetCommandLineSwitchValue("-game"));

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
		g_server_code.Initialize("server", &window.m_data, &input);

	if (client)
		g_client_code.Initialize("client", &window.m_data, &input);

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

	WriteFunData(&g_server_code, &g_client_code);

	SetLowPeriodScheduler();

	if (server && client)
	{
		g_server_code.m_set_local_network_memory(&g_client_code.m_game_data);
		g_client_code.m_set_local_network_memory(&g_server_code.m_game_data);

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
			g_server_code.m_game_data.m_real_time = frame_start_time;
			g_server_code.m_game_data.m_frame_time = frame_time;
			g_client_code.m_game_data.m_game_time = game_time;
			g_client_code.m_game_data.m_real_time = frame_start_time;
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
			g_server_code.m_game_data.m_real_time = frame_start_time;
			g_server_code.m_game_data.m_frame_time = frame_time;

			game_active = g_server_code.m_game_frame(&g_server_code.m_game_data);

			double frame_end_time = window.GetTime();

			double next_frame_time = frame_start_time + target_frame_time;

			double time_to_sleep_seconds = next_frame_time - frame_end_time;
			if (time_to_sleep_seconds > 0.001)
				SleepMS((size_t)(time_to_sleep_seconds * 1000));
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
			g_client_code.m_game_data.m_real_time = frame_start_time;
			g_client_code.m_game_data.m_frame_time = frame_time;

			game_active = g_client_code.m_game_frame(&g_client_code.m_game_data);

			window.SwapBuffers();

			double frame_end_time = window.GetTime();

			double next_frame_time = frame_start_time + target_frame_time;

			double time_to_sleep_seconds = next_frame_time - frame_end_time;
			if (time_to_sleep_seconds > 0.001)
				SleepMS((size_t)(time_to_sleep_seconds * 1000));
		}
	}

	ClearLowPeriodScheduler();

	return 0;
}

void WriteFunData(GameCode* server, GameCode* client)
{
	FILE* fp = fopen("fundata.txt", "a");
	fprintf(fp, "session {\n");
	fprintf(fp, "\ttimestamp: %d\n", (int)time(0));

#ifdef _WIN64
	fprintf(fp, "\tplatform: win64\n");
#elif __APPLE__
	fprintf(fp, "\tplatform: osx\n");
#else
#error !
#endif

	fprintf(fp, "\tserver_memory: %d\n", (int)server->m_game_data.m_memory_size);
	fprintf(fp, "\tclient_memory: %d\n", (int)client->m_game_data.m_memory_size);

#ifdef _DEBUG
	fprintf(fp, "\tdebug: 1\n");
#else
	fprintf(fp, "\tdebug: 0\n");
#endif

	fprintf(fp, "}\n\n");
	fclose(fp);
}


