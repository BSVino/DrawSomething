#pragma once

#include "client/renderer/renderer.h"

struct DSRenderer
{
	Renderer base;

	DSRenderer(ShaderLibrary* shaders, struct WindowData* window_data)
		: base(shaders, window_data)
	{
	}

	void Draw();

	void RenderSkybox();
};

