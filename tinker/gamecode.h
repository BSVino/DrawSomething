#pragma once

#include <time.h>

struct GameData
{
	struct WindowData* m_window_data;
	struct ControlData* m_input;

	double m_game_time;
	double m_frame_time;

	void*   m_memory;
	size_t  m_memory_size;
};

typedef size_t(*GetMemorySizeProcedure)();
typedef bool(*GameInitializeProcedure)(GameData* game_data, int argc, char** args);
typedef bool(*GameFrameProcedure)(GameData* game_data);
typedef void(*LibraryLoadedProcedure)();
typedef void(*SetLocalNetworkMemoryProcedure)(GameData* game_data);

struct GameCode
{
	size_t m_binary_handle;
#ifdef _DEBUG
	time_t m_binary_modified_time;
#endif

#define BINARY_NAME_SIZE 1024
	char m_binary_name[BINARY_NAME_SIZE];

	GameData m_game_data;

	GetMemorySizeProcedure m_get_memory_size;
	GameInitializeProcedure m_game_init;
	GameFrameProcedure m_game_frame;
	LibraryLoadedProcedure m_library_loaded;
	SetLocalNetworkMemoryProcedure m_set_local_network_memory;

	void Initialize(const char* binary, struct WindowData* window_data, struct ControlData* input);
	void Load();
	void Refresh();
};
