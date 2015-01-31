#include "gamecode.h"

#include "tinker_platform.h"

void GameCode::Initialize(char* binary)
{
	m_binary_handle = LoadBinary(binary);

	TAssert(m_binary_handle);
	if (!m_binary_handle)
		return;

	m_game_frame = (GameFrameProcedure)GetProcedureAddress(m_binary_handle, "GameFrame");
}

