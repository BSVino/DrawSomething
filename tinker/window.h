#pragma once

#include <common.h>

struct WindowData
{
	// Read only variables:
	int m_width;
	int m_height;
	int m_drawable_width;
	int m_drawable_height;
	float m_render_scale;

	// x, y location of the mouse in [0, width]x[0, height], only valid if mouse is visible.
	int m_mouse_x;
	int m_mouse_y;

	// Modifiable variables:
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
