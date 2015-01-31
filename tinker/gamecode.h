#pragma once

typedef bool (*GameFrameProcedure)();

struct GameCode
{
	size_t m_binary_handle;

	GameFrameProcedure m_game_frame;

	void Initialize(char* binary);
};
