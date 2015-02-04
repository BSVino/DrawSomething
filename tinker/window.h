#pragma once

struct WindowData
{
	int m_width;
	int m_height;
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
