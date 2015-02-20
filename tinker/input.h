#pragma once

#include <string.h> // for memset

#include "common.h"

struct ControlAction
{
	uint8 m_pressed : 1;
	uint8 m_released : 1;
	uint8 m_down : 1;
};

struct ControlData
{
	ControlData()
	{
		m_mouse_dx = m_mouse_dy = 0;

		memset(m_actions, 0, sizeof(m_actions));
	}

	int m_mouse_dx, m_mouse_dy;

	union
	{
		struct
		{
			ControlAction m_draw;

			ControlAction m_forward;
			ControlAction m_back;
			ControlAction m_left;
			ControlAction m_right;
		};
		ControlAction m_actions[5];
	};
};
