#pragma once

// A Pascal-style string, specified by start and length. You can't usually count on it being null-terminated.
struct pstring
{
	pstring() {}

	// Assumes a null-terminated input string!
	pstring(char* s)
	{
		start = s;
		length = strlen(s);
	}

	size_t length;
	char* start;
};

