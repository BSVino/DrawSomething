#include "gamecode.h"

#include "tinker_platform.h"

#include "shell.h"
#include "window.h"

void GameCode::Initialize(char* binary, size_t memory_size, struct WindowData* window_data)
{
	m_binary_name = binary;

	Load();

	m_game_data.m_window_data = window_data;
	m_game_data.m_memory = malloc(memory_size);
	m_game_data.m_memory_size = memory_size;
}

void GameCode::Load()
{
	FreeBinary(m_binary_handle);

#ifdef _DEBUG
	tstring binary_directory(g_shell.m_binary_directory);
	CreateDirectoryNonRecursive(binary_directory + "tmp" T_DIR_SEP);
	tstring tmp_binary_name(binary_directory + "tmp" T_DIR_SEP + m_binary_name);
	if (!CopyFileTo(binary_directory + m_binary_name, tmp_binary_name))
		return;

	tstring pdb = binary_directory + tstring(m_binary_name).substr(0, strlen(m_binary_name) - 3) + "pdb";
	tstring pdb_copy = tmp_binary_name.substr(0, tmp_binary_name.length() - 3) + "pdb";
	CopyFileTo(pdb, pdb_copy);

	m_binary_handle = LoadBinary(tmp_binary_name.c_str());
#else
	m_binary_handle = LoadBinary(m_binary_name);
#endif

	TAssert(m_binary_handle);
	if (!m_binary_handle)
		return;

#ifdef _DEBUG
	m_binary_modified_time = GetFileModificationTime((binary_directory + m_binary_name).c_str());
#endif

	m_game_frame = (GameFrameProcedure)GetProcedureAddress(m_binary_handle, "GameFrame");
	TAssert(m_game_frame);

	m_game_init = (GameInitializeProcedure)GetProcedureAddress(m_binary_handle, "GameInitialize");
	TAssert(m_game_init);

	m_game_load = (GameLoadProcedure)GetProcedureAddress(m_binary_handle, "GameLoad");
	TAssert(m_game_load);

	m_game_load();
}

void GameCode::Refresh()
{
#ifdef _DEBUG
	tstring binary_directory(g_shell.m_binary_directory);
	if (GetFileModificationTime((binary_directory + m_binary_name).c_str()) > m_binary_modified_time)
	{
		tstring pdb = binary_directory + tstring(m_binary_name).substr(0, strlen(m_binary_name) - 3) + "pdb";
		if (IsFile(binary_directory + m_binary_name) && IsFile(pdb))
		{
			SleepMS(200); // Let the compiler finish writing stuff. Otherwise, crash.
			Load();
		}
	}
#endif
}
