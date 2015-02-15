
#pragma once

struct Shell
{
	int    m_argc;
	char** m_args;
	char   m_binary_directory[256];

	void Initialize(int argc, char** args);
	bool HasCommandLineSwitchValue(const char* swtch);
	const char* GetCommandLineSwitchValue(const char* swtch);
};

extern Shell g_shell;
