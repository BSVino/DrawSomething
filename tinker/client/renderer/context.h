#pragma once

#include "vector.h"
#include "shaders.h"
#include "renderer.h"

struct Context
{
	Renderer* m_renderer;
	Renderer::RenderFrame* m_frame;

	// For building meshes
	vec3   m_normal;
	vec2   m_texcoords[MAX_TEXTURE_CHANNELS];
	color4 m_color;
	int m_drawmode;
	int m_has_texcoord : 1;
	int m_has_normal : 1;
	int m_has_color : 1;

	Context(Renderer* renderer, bool inherit = true);
	~Context();

	void SetProjection(const mat4& m);
	void SetView(const mat4& m);

	inline mat4 GetProjection() { return m_frame->m_projection; }
	inline mat4 GetView() { return m_frame->m_view; }

	void UseShader(char* shader_name);
	void UseShader(ShaderIndex shader); // Can save on the program name lookup
	void SetUniform(UniformIndex uniform, const mat4& value);

	// Immediate mode emulation
	void BeginRenderTris();
	void BeginRenderTriFan();
	void BeginRenderTriStrip();
	void BeginRenderLines();
	void BeginRenderLineLoop();
	void BeginRenderLineStrip();
	void BeginRenderPoints(float size = 1);
	void BeginRenderDebugLines();
	void TexCoord(float s, float t, int channel = 0);
	void TexCoord(const vec2& v, int channel = 0);
	void TexCoord(const vec3& v, int channel = 0);
	void Normal(const vec3& v);
	void Color(const color4& c);	// Per-attribute color
	void Vertex(const vec3& v);
	void EndRender();
};

