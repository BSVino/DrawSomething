#include "stringtable.h"

StringTableIndex st_add(StringTable& st, pstring string)
{
	// First a linear search to see if the symbol is already here.
	// This could probably be accelerated somehow.
	// We need this so that identical strings have identical addresses,
	// some algorithms require that (ie any algorithm that uses map<const char*, x>)
	if (st.size())
	{
		char* s = &st[0];
		size_t size = st.size();
		char* end = s + size;
		while (s < end)
		{
			if (strncmp(s, string.start, string.length + 1) == 0)
				return (StringTableIndex)(s - &st[0]);
			s += strlen(s) + 1;
		}
	}

	StringTableIndex r = (StringTableIndex)st.size();
	st.resize(r + string.length + 1);
	strncpy(&st[r], string.start, string.length);

	return r;
}

char* st_get(StringTable& st, StringTableIndex string)
{
	return &st.data()[string];
}

StringTableIndex st_find(StringTable& st, char* s)
{
	size_t st_size = st.size();
	const char* p = st.data();
	do
	{
		if (strcmp(p, s) == 0)
			return (StringTableIndex)(p - st.data());

		p += strlen(p)+1;
	} while ((size_t)(p - st.data()) < st_size);

	return (StringTableIndex)~0;
}


