#pragma once

#include <time.h>

struct GameData
{
	void* m_memory;
	size_t m_memory_size;
};

typedef bool(*GameInitializeProcedure)(GameData* game_data, int argc, char** args);
typedef bool(*GameFrameProcedure)(GameData* game_data);

struct GameCode
{
	size_t m_binary_handle;
	time_t m_binary_modified_time;
	char*  m_binary_name;

	GameData m_game_data;

	GameInitializeProcedure m_game_init;
	GameFrameProcedure m_game_frame;

	void Initialize(char* binary, size_t memory_size);
	void Load();
	void Refresh();
};
