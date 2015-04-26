/*
Copyright (c) 2012, Lunar Workshop, Inc.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software must display the following acknowledgement:
   This product includes software developed by Lunar Workshop, Inc.
4. Neither the name of the Lunar Workshop nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY LUNAR WORKSHOP INC ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL LUNAR WORKSHOP BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "data.h"

#define PARSE_REQUIRE(x, error) \
PRAGMA_WARNING_PUSH \
PRAGMA_WARNING_DISABLE(4127) /* conditional expression is constant */ \
do { \
	int r = (x); \
	if (!r) { \
		/*TMsg("PARSE ERROR: Required a " error ".\n");*/ \
		return r; \
		} \
} while (0) \
PRAGMA_WARNING_POP \

#define PARSE_EAT(x) \
PRAGMA_WARNING_PUSH \
PRAGMA_WARNING_DISABLE(4127) /* conditional expression is constant */ \
do { \
	int r = DataParseEat(x); \
	if (!r) { /*TMsg("PARSE ERROR: Expected " #x ".\n");*/ return 0; }\
} while (0) \
PRAGMA_WARNING_POP \

typedef enum
{
	TOKEN_UNKNOWN = -1,
	TOKEN_EOF = 0,
	TOKEN_EOL,
	TOKEN_TEXT,
	TOKEN_VALUE,
	TOKEN_OPEN_CURLY,
	TOKEN_CLOSE_CURLY,
} data_token;

static char* p;
static char* p_end;

static data_token  token_type;
static pstring     token_string;

int DataLexText(char p)
{
	// These characters are delimiters, not considered text.
	if (p == ':' || p == '{' || p == '}')
		return 0;

	// Any other printable ASCII character is text.
	return (p >= ' ');
}

static data_token DataLexNext()
{
	if (p == p_end)
		return token_type = TOKEN_EOF;

	while ((*p == ' ' || *p == '\r' || *p == '\t') && p <= p_end)
		p++;

	if (*p == '\n')
	{
		token_type = TOKEN_EOL;
		while (*p == '\n' && p <= p_end)
			p++;

		token_string.length = p - token_string.start;

		return token_type;
	}

	if (p == p_end)
		return token_type = TOKEN_EOF;

	token_string.start = p;

	if (DataLexText(*p))
	{
		token_type = TOKEN_TEXT;
		while (DataLexText(*p))
			p++;

		token_string.length = p - token_string.start;

		return token_type;
	}
	else if (*p == ':')
		return p++, token_type = TOKEN_VALUE;
	else if (*p == '{')
		return p++, token_type = TOKEN_OPEN_CURLY;
	else if (*p == '}')
		return p++, token_type = TOKEN_CLOSE_CURLY;

	return token_type = TOKEN_UNKNOWN;
}

static int DataParseEat(data_token t)
{
	if (token_type != t)
		return 0;

	DataLexNext();
	return 1;
}

static int DataParsePeek(data_token t)
{
	return token_type == t;
}

/*
	keyvalue <- text [ ":" text ] EOL
*/
int DataParseKeyValue(pstring* key, pstring* value)
{
	*key = token_string;

	PARSE_EAT(TOKEN_TEXT);

	if (DataParsePeek(TOKEN_VALUE))
	{
		PARSE_EAT(TOKEN_VALUE);

		*value = token_string;

		PARSE_EAT(TOKEN_TEXT);
	}
	else
	{
		value->length = 0;
		value->start = NULL;
	}

	PARSE_EAT(TOKEN_EOL);

	return 1;
}

extern int DataParseBlocks(KVData* data, KVEntryIndex parent);

/*
	block <- keyvalue { EOL } [ "{" EOL blocks "}" EOL ]
*/
int DataParseBlock(KVData* data, KVEntryIndex parent, KVEntryIndex* block)
{
	pstring key, value;

	if (!DataParseKeyValue(&key, &value))
		return 0;

	TAssert(data->m_data.size() < TInvalid(KVEntryIndex));

	KVEntryIndex index = *block = (KVEntryIndex)data->m_data.size();
	KVEntry& entry = data->m_data.push_back();
	entry.parent = parent;
	entry.num_children = 0;

	entry.key = st_add(data->m_strings, key);

	if (value.length)
		entry.value = st_add(data->m_strings, value);
	else
		entry.value = TInvalid(StringTableIndex);

	while (DataParsePeek(TOKEN_EOL))
		PARSE_EAT(TOKEN_EOL);

	if (!DataParsePeek(TOKEN_OPEN_CURLY))
		return 1;

	PARSE_EAT(TOKEN_OPEN_CURLY);
	PARSE_EAT(TOKEN_EOL);

	DataParseBlocks(data, index);

	PARSE_EAT(TOKEN_CLOSE_CURLY);
	PARSE_EAT(TOKEN_EOL);

	return 1;
}

/*
	blocks <- { block { EOF } }
*/
int DataParseBlocks(KVData* data, KVEntryIndex parent)
{
	KVEntryIndex first_block = TInvalid(KVEntryIndex);
	KVEntryIndex last_block = TInvalid(KVEntryIndex);
	KVEntryIndex block;
	KVEntryIndex num_children = 0;

	while (DataParseBlock(data, parent, &block))
	{
		if (last_block == TInvalid(KVEntryIndex))
			first_block = block;
		else
			data->m_data[last_block].next_sibling = block;

		last_block = block;
		num_children++;

		while (DataParsePeek(TOKEN_EOL))
			PARSE_EAT(TOKEN_EOL);
	}

	if (num_children)
		data->m_data[block].next_sibling = TInvalid(KVEntryIndex);

	if (parent != TInvalid(KVEntryIndex))
	{
		data->m_data[parent].first_child = first_block;
		data->m_data[parent].num_children = num_children;
	}

	return 1;
}

void KVData::ReadData(FILE* fp)
{
	if (!fp)
		return;

	tstring file_contents = tfread_file(fp);

	p = (char*)file_contents.c_str();
	p_end = (char*)file_contents.c_str() + file_contents.length();

	// Prime the pump
	DataLexNext();

	DataParseBlocks(this, TInvalid(KVEntryIndex));
}

static void SaveData(FILE* /*fp*/, KVEntry* /*data*/, size_t /*level*/)
{
	TUnimplemented();
	/*tstring sTabs;
	for (size_t i = 0; i < iLevel; i++)
		sTabs += "\t";

	for (size_t i = 0; i < pData->GetNumChildren(); i++)
	{
		CData* pChild = pData->GetChild(i);

		if (pChild->GetValueString().length())
			fputs((sTabs + pChild->GetKey() + ": " + pChild->GetValueString() + "\n").c_str(), fp);
		else
			fputs((sTabs + pChild->GetKey() + "\n").c_str(), fp);

		if (pChild->GetNumChildren())
		{
			fputs((sTabs + "{\n").c_str(), fp);
			SaveData(fp, pChild, iLevel + 1);
			fputs((sTabs + "}\n").c_str(), fp);
		}
	}*/
}

void KVData::SaveData(FILE* fp)
{
	if (!fp)
		return;

	::SaveData(fp, 0, 0);
}

void KVData::Reset()
{
	m_data.clear();
	m_strings.clear();
}

KVEntryIndex KVData::FindEntryIndex(KVEntryIndex parent, const char* key)
{
	KVEntryIndex first = 0;
	if (parent != TInvalid(KVEntryIndex))
		first = m_data[parent].first_child;

	for (KVEntryIndex i = first; i != TInvalid(KVEntryIndex); i = m_data[i].next_sibling)
		if (strcmp(st_get(m_strings, m_data[i].key), key) == 0)
			return i;

	return (KVEntryIndex)~0;
}

const char* KVData::FindChildValueString(KVEntryIndex parent, const char* key, char* def)
{
	const char* const_def = def;

	if (!const_def)
		const_def = "";

	KVEntryIndex entry = FindEntryIndex(parent, key);
	if (entry == TInvalid(KVEntryIndex))
		return const_def;

	return st_get(m_strings, m_data[entry].value);
}

