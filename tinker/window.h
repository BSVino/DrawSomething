#pragma once

#include <common.h>

struct WindowData
{
	int m_width;
	int m_height;

	uint8 m_cursor_visible : 1;

	WindowData()
	{
		m_cursor_visible = false;
	}
};

struct Window
{
	struct SDL_Window* m_SDL_window;

	WindowData m_data;

	int m_open : 1;

	void Open(const char* title, int width, int height);

	bool IsOpen();

	double GetTime();

	void PollEvents(struct ControlData* input);

	void SwapBuffers();
};
