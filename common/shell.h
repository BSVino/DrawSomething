
#pragma once

struct Shell
{
	int    m_argc;
	char** m_args;

	void Initialize(int argc, char** args);
	const char* GetCommandLineSwitchValue(const char* swtch);
};

extern Shell g_shell;
