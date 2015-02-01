#include "gamecode.h"

#include "shell.h"

#include "tinker_platform.h"

void GameCode::Initialize(char* binary)
{
	m_binary_name = binary;

	Load();
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

	tstring dbg = binary_directory + tstring(m_binary_name).substr(0, strlen(m_binary_name) - 3) + "pdb";
	tstring dbg_copy = tmp_binary_name.substr(0, tmp_binary_name.length() - 3) + "pdb";
	CopyFileTo(dbg, dbg_copy);

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
}

void GameCode::Refresh()
{
#ifdef _DEBUG
	tstring binary_directory(g_shell.m_binary_directory);
	if (GetFileModificationTime((binary_directory + m_binary_name).c_str()) > m_binary_modified_time)
		Load();
#endif
}
