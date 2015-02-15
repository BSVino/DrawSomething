#include "window.h"

#include <SDL.h>

#include "input.h"
#include "keys.h"

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

	SDL_SetRelativeMouseMode(SDL_TRUE);
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

tinker_key_t MapScancode(SDL_Scancode c)
{
	switch (c)
	{
	case SDL_SCANCODE_AC_BACK:
		return TINKER_KEY_APP_BACK;

	case SDL_SCANCODE_MENU:
		return TINKER_KEY_APP_MENU;

	case SDL_SCANCODE_ESCAPE:
		return TINKER_KEY_ESCAPE;

	case SDL_SCANCODE_F1:
		return TINKER_KEY_F1;

	case SDL_SCANCODE_F2:
		return TINKER_KEY_F2;

	case SDL_SCANCODE_F3:
		return TINKER_KEY_F3;

	case SDL_SCANCODE_F4:
		return TINKER_KEY_F4;

	case SDL_SCANCODE_F5:
		return TINKER_KEY_F5;

	case SDL_SCANCODE_F6:
		return TINKER_KEY_F6;

	case SDL_SCANCODE_F7:
		return TINKER_KEY_F7;

	case SDL_SCANCODE_F8:
		return TINKER_KEY_F8;

	case SDL_SCANCODE_F9:
		return TINKER_KEY_F9;

	case SDL_SCANCODE_F10:
		return TINKER_KEY_F10;

	case SDL_SCANCODE_F11:
		return TINKER_KEY_F11;

	case SDL_SCANCODE_F12:
		return TINKER_KEY_F12;

	case SDL_SCANCODE_UP:
		return TINKER_KEY_UP;

	case SDL_SCANCODE_DOWN:
		return TINKER_KEY_DOWN;

	case SDL_SCANCODE_LEFT:
		return TINKER_KEY_LEFT;

	case SDL_SCANCODE_RIGHT:
		return TINKER_KEY_RIGHT;

	case SDL_SCANCODE_LSHIFT:
		return TINKER_KEY_LSHIFT;

	case SDL_SCANCODE_RSHIFT:
		return TINKER_KEY_RSHIFT;

	case SDL_SCANCODE_LCTRL:
		return TINKER_KEY_LCTRL;

	case SDL_SCANCODE_RCTRL:
		return TINKER_KEY_RCTRL;

	case SDL_SCANCODE_LALT:
		return TINKER_KEY_LALT;

	case SDL_SCANCODE_RALT:
		return TINKER_KEY_RALT;

	case SDL_SCANCODE_TAB:
		return TINKER_KEY_TAB;

	case SDL_SCANCODE_RETURN:
		return TINKER_KEY_ENTER;

	case SDL_SCANCODE_BACKSPACE:
		return TINKER_KEY_BACKSPACE;

	case SDL_SCANCODE_INSERT:
		return TINKER_KEY_INSERT;

	case SDL_SCANCODE_DELETE:
		return TINKER_KEY_DEL;

	case SDL_SCANCODE_PAGEUP:
		return TINKER_KEY_PAGEUP;

	case SDL_SCANCODE_PAGEDOWN:
		return TINKER_KEY_PAGEDOWN;

	case SDL_SCANCODE_HOME:
		return TINKER_KEY_HOME;

	case SDL_SCANCODE_END:
		return TINKER_KEY_END;

	case SDL_SCANCODE_KP_0:
		return TINKER_KEY_KP_0;

	case SDL_SCANCODE_KP_1:
		return TINKER_KEY_KP_1;

	case SDL_SCANCODE_KP_2:
		return TINKER_KEY_KP_2;

	case SDL_SCANCODE_KP_3:
		return TINKER_KEY_KP_3;

	case SDL_SCANCODE_KP_4:
		return TINKER_KEY_KP_4;

	case SDL_SCANCODE_KP_5:
		return TINKER_KEY_KP_5;

	case SDL_SCANCODE_KP_6:
		return TINKER_KEY_KP_6;

	case SDL_SCANCODE_KP_7:
		return TINKER_KEY_KP_7;

	case SDL_SCANCODE_KP_8:
		return TINKER_KEY_KP_8;

	case SDL_SCANCODE_KP_9:
		return TINKER_KEY_KP_9;

	case SDL_SCANCODE_KP_DIVIDE:
		return TINKER_KEY_KP_DIVIDE;

	case SDL_SCANCODE_KP_MULTIPLY:
		return TINKER_KEY_KP_MULTIPLY;

	case SDL_SCANCODE_KP_MINUS:
		return TINKER_KEY_KP_SUBTRACT;

	case SDL_SCANCODE_KP_PLUS:
		return TINKER_KEY_KP_ADD;

	case SDL_SCANCODE_KP_PERIOD:
		return TINKER_KEY_KP_DECIMAL;

	case SDL_SCANCODE_KP_EQUALS:
		return TINKER_KEY_KP_EQUAL;

	case SDL_SCANCODE_KP_ENTER:
		return TINKER_KEY_KP_ENTER;

	default:
		break;
	}

	if (c >= SDL_SCANCODE_A && c <= SDL_SCANCODE_Z)
		return (tinker_key_t)('A' + c - SDL_SCANCODE_A);

	if (c >= SDL_SCANCODE_1 && c <= SDL_SCANCODE_9)
		return (tinker_key_t)('1' + c - SDL_SCANCODE_1);

	if (c == SDL_SCANCODE_0)
		return (tinker_key_t)'0';

	if (c == SDL_SCANCODE_SPACE)
		return (tinker_key_t)' ';

	return TINKER_KEY_UNKNOWN;
}

void UpdateAction(ControlAction* action, Uint8 state)
{
	action->m_down = state;

	if (state)
		action->m_pressed = 1;
	else
		action->m_released = 1;
}

void Window::PollEvents(ControlData* input)
{
	input->m_mouse_dx = input->m_mouse_dy = 0;

	for (int k = 0; k < sizeof(input->m_actions) / sizeof(input->m_actions[0]); k++)
		input->m_actions[k].m_pressed = input->m_actions[k].m_released = 0;

	SDL_Event e;

	while (SDL_PollEvent(&e))
	{
		switch (e.type)
		{
		case SDL_QUIT:
			m_open = false;
			break;

		case SDL_MOUSEMOTION:
			if (SDL_GetWindowFlags(m_SDL_window)&SDL_WINDOW_INPUT_FOCUS)
			{
				input->m_mouse_dx = e.motion.xrel;
				input->m_mouse_dy = e.motion.yrel;
			}
			break;

		case SDL_KEYDOWN:
		case SDL_KEYUP:
		{
			tinker_key_t key = MapScancode(e.key.keysym.scancode);
			switch (key)
			{
			default:
				break;

			case 'A':
				UpdateAction(&input->m_left, e.key.state);
				break;

			case 'D':
				UpdateAction(&input->m_right, e.key.state);
				break;

			case 'S':
				UpdateAction(&input->m_back, e.key.state);
				break;

			case 'W':
				UpdateAction(&input->m_forward, e.key.state);
				break;
			}
			break;
		}

		default:
			break;
		}
	}
}

void Window::SwapBuffers()
{
	SDL_GL_SwapWindow(m_SDL_window);
}


