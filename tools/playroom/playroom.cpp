#include <stdio.h>

#include <time.h>

#include "shell.h"
#include "tinker_platform.h"
#include "data.h"

int main(int argc, char** args)
{
	g_shell.Initialize(argc, args);

	if (g_shell.GetCommandLineSwitchValue("-h"))
	{
		printf("playroom crawls the assets of a game and outputs source code containing information about the tree that can be compiled into the binary.\n");
		return 1;
	}

	if (!g_shell.GetCommandLineSwitchValue("-game"))
	{
		printf("PLAYROOM: You must specify a game directory.\n");
		return 1;
	}

	if (!g_shell.GetCommandLineSwitchValue("-header"))
	{
		printf("PLAYROOM: You must specify a header to write.\n");
		return 1;
	}

	if (!g_shell.GetCommandLineSwitchValue("-source"))
	{
		printf("PLAYROOM: You must specify a source file to write.\n");
		return 1;
	}

	const char* game_directory = g_shell.GetCommandLineSwitchValue("-game");

	if (!IsDirectory(game_directory))
	{
		printf("PLAYROOM: It seems that game directory doesn't exist.\n");
		return 1;
	}

	tstring binary_name = args[0];
	if (!IsFile(binary_name))
		binary_name = binary_name + ".exe";

	time_t output_time = time(0);
	time_t input_time = GetFileModificationTime(binary_name.c_str());

	const char* header = g_shell.GetCommandLineSwitchValue("-header");
	const char* source = g_shell.GetCommandLineSwitchValue("-source");

	output_time = tmin(GetFileModificationTime(header), output_time);
	output_time = tmin(GetFileModificationTime(source), output_time);

	tstring header_text, source_text;

	header_text = "#pragma once\n\n// This file is automatically generated by playroom DO NOT MODIFY!\n// Command line:";
	for (int i = 0; i < argc; i++)
		header_text += tsprintf(" %s", args[i]);
	header_text += tsprintf("\n\n");

	source_text = tsprintf("#include \"%s\"\n\n// This file is automatically generated by playroom DO NOT MODIFY!\n// Command line:", header);
	for (int i = 0; i < argc; i++)
		source_text += tsprintf(" %s", args[i]);
	source_text += tsprintf("\n\n");

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

	header_text += tsprintf("typedef enum {\n\tSHADER_INVALID = -1,\n");
	source_text += tsprintf("// This is guaranteed to be sorted by name.\nAssetShader asset_shaders[] = {\n");

	for (size_t i = 0; i < shaders.size(); i++)
	{
		input_time = tmax(GetFileModificationTime((shaders_directory + "/" + shaders[i]).c_str()), input_time);
		FILE* fp = tfopen(shaders_directory + "/" + shaders[i], "r");

		KVData data;
		data.ReadData(fp);

		fclose(fp);

		source_text += tsprintf("\t{ \"%s\", \"shaders/%s.vs\", \"shaders/%s.fs\", \"u_projection\", \"u_view\", \"u_global\", },\n", data.FindChildValueString(TInvalid(KVEntryIndex), "Name"), data.FindChildValueString(TInvalid(KVEntryIndex), "Vertex"), data.FindChildValueString(TInvalid(KVEntryIndex), "Fragment"));
		header_text += tsprintf("\tSHADER_%s = %d,\n", tstring(data.FindChildValueString(TInvalid(KVEntryIndex), "Name")).toupper().c_str(), i);
	}

	header_text += tsprintf("\tMAX_SHADERS = %d,\n} ShaderIndex;\n\n", shaders.size());

	header_text += tsprintf("typedef enum {\n\tUNIFORM_INVALID = -1,\n\t// Common uniforms\n\tUNIFORM_PROJECTION = 0,\n\tUNIFORM_VIEW = 1,\n\tUNIFORM_GLOBAL = 2,\n\tMAX_UNIFORMS = 3,\n} UniformIndex;\n\n");

	source_text += tsprintf("};\n\n");

	header_text += tsprintf("struct AssetShader {\n\tconst char* name;\n\tconst char* vertex_file;\n\tconst char* fragment_file;\n\tconst char* uniform_names[MAX_UNIFORMS];\n};\n// This is guaranteed to be sorted by name.\nextern AssetShader asset_shaders[];\n\n");

	if (input_time < output_time)
	{
		printf("PLAYROOM: No asset files have changed. Skipping code generation.\n");
		return 0;
	}

	FILE* fp_header = tfopen(header, "w");
	FILE* fp_source = tfopen(source, "w");

	if (!fp_header)
	{
		printf("PLAYROOM: Couldn't open the header file specified.\n");
		return 1;
	}

	if (!fp_source)
	{
		printf("PLAYROOM: Couldn't open the source file specified.\n");
		return 1;
	}

	fprintf(fp_header, header_text.c_str());
	fprintf(fp_source, source_text.c_str());

	fclose(fp_header);
	fclose(fp_source);

	printf("PLAYROOM: Assets source '%s' and header '%s' written successfully.\n", source, header);

	return 0;
}

