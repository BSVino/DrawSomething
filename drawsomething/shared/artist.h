#pragma once

#include "vector.h"

struct Artist
{
	vec3 m_position;
	eangle m_looking;

	Artist()
		: m_position(0, 0, 0), m_looking(0, 0, 0)
	{
	}

#ifdef DS_CLIENT
	void HandleInput(struct ControlData* input);
#endif
};

#define MAX_ARTISTS 4

