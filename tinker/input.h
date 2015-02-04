#pragma once

struct ControlData
{
	ControlData()
	{
		mouse_dx = mouse_dy = 0;
	}

	int mouse_dx, mouse_dy;
};
