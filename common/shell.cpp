#include "shell.h"

#include <cstring>

Shell g_shell;

void Shell::Initialize(int argc, char** args)
{
	m_argc = argc;
	m_args = args;

	strcpy(m_binary_directory, m_args[0]);
	int i;

	for (i = (int)strlen(m_binary_directory) - 1; i >= 0; i--)
	{
		if (m_binary_directory[i] == '\\' || m_binary_directory[i] == '/')
			break;
	}

	m_binary_directory[i + 1] = '\0';
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

