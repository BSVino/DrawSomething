#pragma once

#include <time.h>

struct GameData
{
	struct WindowData* m_window_data;

	void*   m_memory;
	size_t  m_memory_size;
};

typedef bool(*GameInitializeProcedure)(GameData* game_data, int argc, char** args);
typedef bool(*GameFrameProcedure)(GameData* game_data);
typedef void(*GameLoadProcedure)();

struct GameCode
{
	size_t m_binary_handle;
#ifdef _DEBUG
	time_t m_binary_modified_time;
#endif
	char*  m_binary_name;

	GameData m_game_data;

	GameInitializeProcedure m_game_init;
	GameFrameProcedure m_game_frame;
	GameLoadProcedure m_game_load;

	void Initialize(char* binary, size_t memory_size, struct WindowData* window_data);
	void Load();
	void Refresh();
};
