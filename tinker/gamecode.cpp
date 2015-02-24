#include "gamecode.h"

#include "tinker_platform.h"

#include "shell.h"
#include "window.h"

void GameCode::Initialize(char* binary, struct WindowData* window_data, struct ControlData* input)
{
	m_binary_name = binary;

	Load();

	m_game_data.m_window_data = window_data;
	m_game_data.m_input = input;
	m_game_data.m_memory_size = m_get_memory_size();
	m_game_data.m_memory = malloc(m_game_data.m_memory_size);
}

void GameCode::Load()
{
	FreeBinary(m_binary_handle);

#ifdef _DEBUG
	srand((unsigned int)time(0));

	tstring binary_directory(g_shell.m_binary_directory);
	
	CreateDirectoryNonRecursive(binary_directory + "tmp");

	tstring tmp_dir = tsprintf("tmp/%d", rand());
	CreateDirectoryNonRecursive(binary_directory + tmp_dir + T_DIR_SEP);
	tstring tmp_binary_name(binary_directory + tmp_dir + T_DIR_SEP + m_binary_name);
	while (!CopyFileTo(binary_directory + m_binary_name, tmp_binary_name))
		SleepMS(100); // Let the compiler finish writing stuff. Otherwise, crash.

	tstring pdb = binary_directory + tstring(m_binary_name).substr(0, strlen(m_binary_name) - 3) + "pdb";
	tstring pdb_copy = tmp_binary_name.substr(0, tmp_binary_name.length() - 3) + "pdb";
	while (!CopyFileTo(pdb, pdb_copy))
		SleepMS(100); // Let the compiler finish writing stuff. Otherwise, crash.

	m_binary_handle = LoadBinary(tmp_binary_name.c_str());
#else
	m_binary_handle = LoadBinary(m_binary_name);
#endif

	TCheck(m_binary_handle);
	if (!m_binary_handle)
		return;

#ifdef _DEBUG
	m_binary_modified_time = GetFileModificationTime((binary_directory + m_binary_name).c_str());
#endif

	m_get_memory_size = (GetMemorySizeProcedure)GetProcedureAddress(m_binary_handle, "GetMemorySize");
	TAssert(m_get_memory_size);

	m_game_frame = (GameFrameProcedure)GetProcedureAddress(m_binary_handle, "GameFrame");
	TAssert(m_game_frame);

	m_game_init = (GameInitializeProcedure)GetProcedureAddress(m_binary_handle, "GameInitialize");
	TAssert(m_game_init);

	m_library_loaded = (LibraryLoadedProcedure)GetProcedureAddress(m_binary_handle, "LibraryLoaded");
	TAssert(m_library_loaded);

	m_library_loaded();
}

void GameCode::Refresh()
{
#ifdef _DEBUG
	tstring binary_directory(g_shell.m_binary_directory);
	if (GetFileModificationTime((binary_directory + m_binary_name).c_str()) > m_binary_modified_time)
	{
		tstring pdb = binary_directory + tstring(m_binary_name).substr(0, strlen(m_binary_name) - 3) + "pdb";
		if (IsFile(binary_directory + m_binary_name) && IsFile(pdb))
			Load();
	}
#endif
}
