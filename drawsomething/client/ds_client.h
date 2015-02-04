#pragma once

#include "client/renderer/shaders.h"
#include "renderer/ds_renderer.h"

struct ClientData
{
	ShaderLibrary m_shaders;
	DSRenderer    m_renderer;

	ClientData(struct WindowData* window_data)
		: m_renderer(&m_shaders, window_data)
	{
	}
};

extern ClientData* g_client_data;

