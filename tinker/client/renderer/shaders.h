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

#ifndef DT_SHADERS_H
#define DT_SHADERS_H

#include <tvector.h>
#include <tmap.h>

#include <tstring.h>
#include <vector.h>
#include <matrix.h>

#include "assets.h"

#define MAX_TEXTURE_CHANNELS 2

struct Shader
{
	size_t m_vshader;
	size_t m_fshader;
	size_t m_program;

	size_t m_position_attribute;
	size_t m_normal_attribute;
	size_t m_tangent_attribute;
	size_t m_bitangent_attribute;
	size_t m_texcoord_attributes[MAX_TEXTURE_CHANNELS];
	size_t m_color_attribute;

	UniformIndex m_uniforms[MAX_UNIFORMS];

	Shader();

	bool Compile(ShaderIndex index, struct ShaderLibrary* library);
	void Destroy();
};

struct ShaderLibrary
{
	Shader m_shaders[MAX_SHADERS];

	tstring* m_header;
	tstring* m_functions;
	tstring* m_main;

	int m_samples;

	int m_compiled : 1;
	int m_log_needs_clearing : 1;

	ShaderLibrary();
	~ShaderLibrary();

	void Initialize(int samples);
	void Destroy();

	void CompileShaders();
	void DestroyShaders();

	ShaderIndex FindShader(char* name);

	void ClearLog();
	void WriteLog(const tstring& file, const char* log, const char* shader_text);
};

#endif
