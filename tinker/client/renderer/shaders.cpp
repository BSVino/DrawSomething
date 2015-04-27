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

#include "shaders.h"

#include <memory>
#include <time.h>

#include "tinker_gl.h"

#include <common.h>
#include <tinker_platform.h>
#include "data.h"

#include "assets.h"

#define TMsg(x) TUnimplemented()
#define TError(x) TUnimplemented()

ShaderLibrary::ShaderLibrary()
{
	m_compiled = false;
}

ShaderLibrary::~ShaderLibrary()
{
	Destroy();
}

void ShaderLibrary::Initialize(int samples)
{
	m_compiled = false;
	m_samples = samples;

	CompileShaders();
}

void ShaderLibrary::Destroy()
{
	for (size_t i = 0; i < MAX_SHADERS; i++)
	{
		Shader* pShader = &m_shaders[i];
		glDetachShader((GLuint)pShader->m_program, (GLuint)pShader->m_vshader);
		glDetachShader((GLuint)pShader->m_program, (GLuint)pShader->m_fshader);
		glDeleteProgram((GLuint)pShader->m_program);
		glDeleteShader((GLuint)pShader->m_vshader);
		glDeleteShader((GLuint)pShader->m_fshader);
	}
}

void ShaderLibrary::CompileShaders()
{
	m_header = new tstring();
	m_functions = new tstring();
	m_main = new tstring();

	tstring functions = "shaders/functions.si";
	FILE* f = tfopen_asset(functions, "r");
	if (f)
		*m_functions = tfread_file(f);
	else
		TMsg(tstring("Warning: Couldn't find shader functions file: ") + functions + "\n");

	fclose(f);

#if defined(TINKER_OPENGLES_3)
	tstring header = "shaders/header_gles3.si";
#elif defined(TINKER_OPENGLES_2)
	tstring header = "shaders/header_gles2.si";
#else
	tstring header = "shaders/header_gl3.si";
#endif

	f = tfopen_asset(header, "r");

	if (f)
		*m_header = tfread_file(f);
	else
		TMsg(tstring("Warning: Couldn't find shader header file: ") + header + "\n");

	fclose(f);

#if defined(TINKER_OPENGLES_3)
	tstring main = "shaders/main_gles3.si";
#elif defined(TINKER_OPENGLES_2)
	tstring main = "shaders/main_gles2.si";
#else
	tstring main = "shaders/main_gl3.si";
#endif

	f = tfopen_asset(main, "r");
	if (f)
		*m_main = tfread_file(f);
	else
		TMsg(tstring("Warning: Couldn't find shader main file: ") + sMain + "\n");

	*m_functions = *m_functions;
	*m_header = *m_header;
	*m_main = *m_main;

	fclose(f);

	TAssert(m_samples != -1);

	ClearLog();

	if (m_compiled)
	{
		// If this is a recompile just blow through them.
		for (size_t i = 0; i < MAX_SHADERS; i++)
			m_shaders[i].Compile((ShaderIndex)i, this);
	}
	else
	{
		bool shaders_compiled = true;
		for (size_t i = 0; i < MAX_SHADERS; i++)
		{
			shaders_compiled &= m_shaders[i].Compile((ShaderIndex)i, this);

			if (!shaders_compiled)
				break;
		}

		if (shaders_compiled)
			m_compiled = true;
	}

	delete m_header;
	delete m_functions;
	delete m_main;
}

void ShaderLibrary::DestroyShaders()
{
	for (size_t i = 0; i < MAX_SHADERS; i++)
		m_shaders[i].Destroy();

	m_compiled = false;
}

ShaderIndex ShaderLibrary::FindShader(char* name)
{
	for (int i = 0; i < MAX_SHADERS; i++)
	{
		if (strcmp(name, asset_shaders[i].name) == 0)
			return (ShaderIndex)i;
	}

	return SHADER_INVALID;
}

void ShaderLibrary::ClearLog()
{
	m_log_needs_clearing = true;
}

void ShaderLibrary::WriteLog(const tstring& /*sFile*/, const char* pszLog, const char* /*pszShaderText*/)
{
	if (!pszLog || strlen(pszLog) == 0)
		return;

	TUnimplemented();
#if 0
	tstring sLogFile = Application()->GetAppDataDirectory("shaders.txt");

	TMsg(tsprintf("Log file location: %s\n", sLogFile.c_str()));

	if (m_log_needs_clearing)
	{
		// Only clear it if we're actually going to write to it so we don't create the file.
		FILE* fp = tfopen(sLogFile, "w");
		fclose(fp);
		m_log_needs_clearing = false;
	}

	FILE* fp = tfopen(sLogFile, "a");
	fprintf(fp, ("Shader compile output for file: " + sFile + " timestamp: %d\n").c_str(), (int)time(NULL));
	fprintf(fp, "%s\n\n", pszLog);
	fprintf(fp, "Shader text follows:\n\n");

	tvector<tstring> asTokens;
	explode(pszShaderText, asTokens, "\n");
	for (size_t i = 0; i < asTokens.size(); i++)
		fprintf(fp, "%d: %s\n", i, asTokens[i].c_str());

	fprintf(fp, "\n\n");

	fclose(fp);
#endif
}

Shader::Shader()
{
	m_vshader = 0;
	m_fshader = 0;
	m_program = 0;
}

bool Shader::Compile(ShaderIndex index, ShaderLibrary* library)
{
	tstring shader_header_text = *library->m_header;

	if (library->m_samples)
		shader_header_text += "#define USE_MULTISAMPLE_TEXTURES 1\n";

	shader_header_text += *library->m_functions;

	tstring vertex_header_text;
	vertex_header_text += shader_header_text;
	vertex_header_text += "#define VERTEX_PROGRAM\n";

	tstring fragment_header_text;
	fragment_header_text += shader_header_text;
	fragment_header_text += "#define FRAGMENT_PROGRAM\n";

	FILE* f = tfopen_asset(asset_shaders[index].vertex_file, "r");

	TCheck(f);
	if (!f)
	{
		TMsg(tstring("Could not open vertex program source: ") + asset_shaders[index].vertex_file + "\n");
		return false;
	}

	tstring vertex_shader_text;
	vertex_shader_text += "uniform mat4x4 u_projection;\n";
	vertex_shader_text += "uniform mat4x4 u_view;\n";
	vertex_shader_text += "uniform mat4x4 u_global;\n";

	vertex_shader_text += "#line 1\n" + tfread_file(f);

	fclose(f);

	f = tfopen_asset(asset_shaders[index].fragment_file, "r");

	TCheck(f);
	if (!f)
	{
		TMsg(tstring("Could not open fragment program source: ") + asset_shaders[index].fragment_file + "\n");
		return false;
	}

	tstring fragment_shader_text;
	fragment_shader_text += "uniform vec3 u_camera;\n";
	fragment_shader_text += "uniform vec3 u_camera_direction;\n";
	fragment_shader_text += "#line 1\n" + tfread_file(f);

	fclose(f);

	tstring full_vertex_shader = vertex_header_text + vertex_shader_text + *library->m_main;
	const char* full_vertex_shader_char = full_vertex_shader.c_str();

	size_t vshader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource((GLuint)vshader, 1, &full_vertex_shader_char, NULL);
	glCompileShader((GLuint)vshader);

	int vertex_compiled;
	glGetShaderiv((GLuint)vshader, GL_COMPILE_STATUS, &vertex_compiled);

	if (vertex_compiled != GL_TRUE)
	{
		int log_length = 0;
		char log[1024];
		glGetShaderInfoLog((GLuint)vshader, 1024, &log_length, log);
		library->WriteLog(asset_shaders[index].vertex_file, log, full_vertex_shader_char);
	}

	tstring full_fragment_shader = fragment_header_text + fragment_shader_text + *library->m_main;
	const char* full_fragment_shader_char = full_fragment_shader.c_str();

	size_t fshader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource((GLuint)fshader, 1, &full_fragment_shader_char, NULL);
	glCompileShader((GLuint)fshader);

	int fragment_compiled;
	glGetShaderiv((GLuint)fshader, GL_COMPILE_STATUS, &fragment_compiled);

	if (fragment_compiled != GL_TRUE)
	{
		int log_length = 0;
		char log[1024];
		glGetShaderInfoLog((GLuint)fshader, 1024, &log_length, log);
		library->WriteLog(asset_shaders[index].fragment_file, log, full_fragment_shader_char);
	}

	size_t program = glCreateProgram();

	glBindAttribLocation((GLuint)program, (GLuint)(library->m_position_attribute = 0), "vert_position");
	glBindAttribLocation((GLuint)program, (GLuint)(library->m_normal_attribute = 1), "vert_normal");
	glBindAttribLocation((GLuint)program, (GLuint)(library->m_tangent_attribute = 2), "vert_tangent");
	glBindAttribLocation((GLuint)program, (GLuint)(library->m_bitangent_attribute = 3), "vert_bitangent");
	glBindAttribLocation((GLuint)program, (GLuint)(library->m_texcoord_attributes[0] = 4), "vert_texcoord0");
	glBindAttribLocation((GLuint)program, (GLuint)(library->m_texcoord_attributes[1] = 5), "vert_texcoord1");
	glBindAttribLocation((GLuint)program, (GLuint)(library->m_color_attribute = 6), "vert_color");

	glAttachShader((GLuint)program, (GLuint)vshader);
	glAttachShader((GLuint)program, (GLuint)fshader);
	glLinkProgram((GLuint)program);

	int iProgramLinked;
	glGetProgramiv((GLuint)program, GL_LINK_STATUS, &iProgramLinked);

	if (iProgramLinked != GL_TRUE)
	{
		int log_length = 0;
		char log[1024];
		glGetProgramInfoLog((GLuint)program, 1024, &log_length, log);
		library->WriteLog("link", log, "link");
	}

	if (vertex_compiled != GL_TRUE || fragment_compiled != GL_TRUE || iProgramLinked != GL_TRUE)
	{
		TError("Shader compilation failed for shader " + m_sName + ". Check shaders.txt\n");

		Destroy();

		return false;
	}

	m_program = program;
	m_vshader = vshader;
	m_fshader = fshader;

	int num_uniforms;
	glGetProgramiv((GLuint)m_program, GL_ACTIVE_UNIFORMS, &num_uniforms);

	char uniform_name[256];
	GLsizei uniform_name_length;
	GLint uniform_size;
	GLenum uniform_type;

	TCheck(MAX_UNIFORMS < 100); // If this grows too much it may be time to substitute it with a different data type.

	for (int i = 0; i < num_uniforms; i++)
	{
		glGetActiveUniform((GLuint)m_program, i, sizeof(uniform_name), &uniform_name_length, &uniform_size, &uniform_type, uniform_name);

		UniformIndex k;
		for (k = (UniformIndex)0; k < MAX_UNIFORMS; k = (UniformIndex)(k+1))
		{
			if (strcmp(asset_shaders[index].uniform_names[k], uniform_name) == 0)
				break;
		}

		// TODO: This should be an assert.
		if (k == MAX_UNIFORMS)
			continue;

		m_uniforms[k] = (UniformIndex)glGetUniformLocation((GLuint)m_program, uniform_name);
	}

	return true;
}

void Shader::Destroy()
{
	glDetachShader((GLuint)m_program, (GLuint)m_vshader);
	glDetachShader((GLuint)m_program, (GLuint)m_fshader);
	glDeleteShader((GLuint)m_vshader);
	glDeleteShader((GLuint)m_fshader);
	glDeleteProgram((GLuint)m_program);
}
