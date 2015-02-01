#include <stdio.h>

#include "shell.h"
#include "tinker_platform.h"
#include "data.h"

int main(int argc, char** args)
{
	g_shell.Initialize(argc, args);

	if (g_shell.GetCommandLineSwitchValue("-h"))
	{
		printf("asset2header crawls the assets of a game and outputs a header file containing information about the tree that can be compiled into the binary.\n");
		return 1;
	}

	if (!g_shell.GetCommandLineSwitchValue("-game"))
	{
		printf("You must specify a game directory.\n");
		return 1;
	}

	if (!g_shell.GetCommandLineSwitchValue("-header"))
	{
		printf("You must specify a header to write.\n");
		return 1;
	}

	if (!g_shell.GetCommandLineSwitchValue("-source"))
	{
		printf("You must specify a source file to write.\n");
		return 1;
	}

	const char* game_directory = g_shell.GetCommandLineSwitchValue("-game");

	if (!IsDirectory(game_directory))
	{
		printf("It seems that directory doesn't exist.\n");
		return 1;
	}

	const char* header = g_shell.GetCommandLineSwitchValue("-header");
	const char* source = g_shell.GetCommandLineSwitchValue("-source");

	FILE* fp_header = tfopen(header, "w");
	FILE* fp_source = tfopen(source, "w");

	if (!fp_header)
	{
		printf("Couldn't open the header file specified.\n");
		return 1;
	}

	if (!fp_source)
	{
		printf("Couldn't open the source file specified.\n");
		return 1;
	}

	fprintf(fp_header, "#pragma once\n\n// This file is automatically generated by asset2header DO NOT MODIFY!\n// Command line:");
	for (int i = 0; i < argc; i++)
		fprintf(fp_header, " %s", args[i]);
	fprintf(fp_header, "\n\n");

	fprintf(fp_source, "#include \"%s\"\n\n// This file is automatically generated by asset2header DO NOT MODIFY!\n// Command line:", header);
	for (int i = 0; i < argc; i++)
		fprintf(fp_source, " %s", args[i]);
	fprintf(fp_source, "\n\n");

	// Count shaders.
	tvector<tstring> shaders;
	tstring shaders_directory = game_directory + tstring("/shaders");
	tvector<tstring> shaders_files = ListDirectory(shaders_directory.c_str(), false);

	for (size_t i = 0; i < shaders_files.size(); i++)
	{
		tstring shader_file = shaders_files[i];
		if (!shader_file.endswith(".txt"))
			continue;

		shaders.push_back(shader_file);
	}

	shaders.sort();

	KVData data;

	fprintf(fp_header, "struct AssetShader {\n\tconst char* vertex_file;\n\tconst char* fragment_file;\n};\n\n// This is guaranteed to be sorted.\nextern AssetShader asset_shaders[];\n");
	fprintf(fp_source, "// This is guaranteed to be sorted.\nAssetShader asset_shaders[] = {\n");

	for (size_t i = 0; i < shaders.size(); i++)
	{
		FILE* fp = tfopen(shaders_directory + "/" + shaders[i], "r");

		KVData data;
		data.ReadData(fp);

		fclose(fp);

		fprintf(fp_source, "\t{\"shaders/%s.vs\", \"shaders/%s.fs\" },\n", data.FindChildValueString(-1, "Vertex"), data.FindChildValueString(-1, "Fragment"));
	}

	fprintf(fp_header, "#define MAX_SHADERS %d\n\n", shaders.size());
	fprintf(fp_source, "};\n\n");

	fclose(fp_header);
	fclose(fp_source);

	printf("Assets source '%s' and header '%s' written successfully.\n", source, header);

	return 0;
}

