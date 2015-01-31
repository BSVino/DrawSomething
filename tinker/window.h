#pragma once

struct Window
{
	struct SDL_Window* m_SDL_window;

	void Open(const char* title, int width, int height);
};
