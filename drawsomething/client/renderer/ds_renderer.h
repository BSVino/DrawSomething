#pragma once

#include "client/renderer/renderer.h"

struct DSRenderer
{
	Renderer base;

	// TODO: REMOVE ME
	float m_yaw, m_pitch;

	DSRenderer(ShaderLibrary* shaders, struct WindowData* window_data)
		: base(shaders, window_data)
	{
		m_yaw = m_pitch = 0;
	}

	void Draw();
};

