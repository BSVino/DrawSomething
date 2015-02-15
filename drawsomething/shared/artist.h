#pragma once

#include "vector.h"

struct Artist
{
	Vector m_position;
	Vector m_eye_height;
	EAngle m_looking;

#ifdef DS_CLIENT
	void HandleInput(struct ControlData* input);
#endif
};

#define MAX_ARTISTS 4

