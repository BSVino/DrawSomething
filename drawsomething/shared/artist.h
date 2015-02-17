#pragma once

#include "vector.h"

struct Artist
{
	vec3 m_position;
	eangle m_looking;

	uint8 m_active : 1;

	Artist()
		: m_position(0, 0, 1.6f), m_looking(0, 0, 0)
	{
		m_active = 0;
	}

#ifdef CLIENT_LIBRARY
	void HandleInput(struct ControlData* input);
#endif
};

#define MAX_ARTISTS 4

