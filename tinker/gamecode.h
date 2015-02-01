#pragma once

#include <time.h>

typedef bool (*GameFrameProcedure)();

struct GameCode
{
	size_t m_binary_handle;
	time_t m_binary_modified_time;
	char*  m_binary_name;

	GameFrameProcedure m_game_frame;

	void Initialize(char* binary);
	void Load();
	void Refresh();
};
