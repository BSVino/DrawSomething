#include "window.h"

#include <SDL.h>

void Window::Open(const char* title, int width, int height)
{
	int init_mode = SDL_INIT_VIDEO | SDL_INIT_TIMER;

#ifdef _DEBUG
	init_mode |= SDL_INIT_NOPARACHUTE;
#endif

	SDL_Init(init_mode);

	m_data.m_width = width;
	m_data.m_height = height;

	int window_mode = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE;

	if (NULL == (m_SDL_window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, window_mode)))
	{
		SDL_Quit();
		return;
	}

	m_open = true;

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

	SDL_GL_CreateContext(m_SDL_window);

	SDL_GL_SetSwapInterval(1);

	SDL_ShowCursor(true);
}

bool Window::IsOpen()
{
	if (!m_open)
		return false;

	if (m_SDL_window)
		return (!!(SDL_GetWindowFlags(m_SDL_window)&SDL_WINDOW_SHOWN));
	else
		return true;
}

double Window::GetTime()
{
	return (double)(SDL_GetTicks()) / 1000;
}

void Window::PollEvents()
{
	SDL_Event e;

	while (SDL_PollEvent(&e))
	{
		switch (e.type)
		{
		case SDL_QUIT:
			m_open = false;
			break;

		default:
			break;
		}
	}
}

void Window::SwapBuffers()
{
	SDL_GL_SwapWindow(m_SDL_window);
}


