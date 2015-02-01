#pragma once

#include <tvector.h>
#include <pstring.h>

typedef unsigned int StringTableIndex;
typedef tvector<char> StringTable;

StringTableIndex st_add(StringTable& st, pstring s);
char* st_get(StringTable& st, StringTableIndex string);
