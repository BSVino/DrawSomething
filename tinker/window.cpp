#include "window.h"

#include <SDL.h>

#include <GL3/gl3w.h>


void Window::Open(const char* title, int width, int height)
{
	int init_mode = SDL_INIT_VIDEO | SDL_INIT_TIMER;

#ifdef _DEBUG
	init_mode |= SDL_INIT_NOPARACHUTE;
#endif

	SDL_Init(init_mode);

	int window_mode = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE;

	if (NULL == (m_SDL_window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, window_mode)))
	{
		SDL_Quit();
		return;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

	int buffers_result = SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	int samples_result = SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

	SDL_GL_CreateContext(m_SDL_window);

	SDL_GL_SetSwapInterval(1);

#if defined(__gl3w_h_)
	GLenum err = gl3wInit();
	if (0 != err)
		exit(0);
#endif

	SDL_ShowCursor(true);
}
