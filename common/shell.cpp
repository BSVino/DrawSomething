#include "shell.h"

#include <cstring>

Shell g_shell;

void Shell::Initialize(int argc, char** args)
{
	m_argc = argc;
	m_args = args;
}

const char* Shell::GetCommandLineSwitchValue(const char* swtch)
{
	// -1 to prevent buffer overrun
	for (size_t i = 0; i < m_argc - 1; i++)
	{
		if (strcmp(m_args[i], swtch) == 0)
			return m_args[i + 1];
	}

	return NULL;
}

