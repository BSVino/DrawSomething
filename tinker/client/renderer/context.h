#pragma once

#include "vector.h"
#include "shaders.h"
#include "renderer.h"

struct Context
{
	Renderer* m_renderer;
	Renderer::RenderFrame* m_frame;

	// For building meshes
	Vector   m_normal;
	Vector2D m_texcoords[MAX_TEXTURE_CHANNELS];
	::Color  m_color;
	int m_drawmode;
	int m_has_texcoord : 1;
	int m_has_normal : 1;
	int m_has_color : 1;

	Context(Renderer* renderer, bool inherit = true);
	~Context();

	void SetProjection(const Matrix4x4& m);
	void SetView(const Matrix4x4& m);

	inline Matrix4x4 GetProjection() { return m_frame->m_projection; }
	inline Matrix4x4 GetView() { return m_frame->m_view; }

	void UseShader(char* shader_name);
	void UseShader(ShaderIndex shader); // Can save on the program name lookup
	void SetUniform(UniformIndex uniform, const Matrix4x4& value);

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
	void TexCoord(const Vector2D& v, int channel = 0);
	void TexCoord(const DoubleVector2D& v, int channel = 0);
	void TexCoord(const Vector& v, int channel = 0);
	void TexCoord(const DoubleVector& v, int channel = 0);
	void Normal(const Vector& v);
	void Color(const ::Color& c);	// Per-attribute color
	void Vertex(const Vector& v);
	void EndRender();
};

