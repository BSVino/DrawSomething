#pragma once

// A Pascal-style string, specified by start and length. You can't usually count on it being null-terminated.
struct pstring
{
	size_t length;
	char* start;
};

