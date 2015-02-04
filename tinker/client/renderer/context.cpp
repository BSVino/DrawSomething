#include "context.h"

#include <GL3/gl3w.h>

#include "renderer.h"
#include "shaders.h"
#include "window.h"

Context::Context(Renderer* renderer, bool inherit)
{
	m_renderer = renderer;

	m_frame = &m_renderer->m_renderframes.push_back();

	if (inherit && m_renderer->m_renderframes.size() > 1)
	{
		Renderer::RenderFrame* prev_frame = &m_renderer->m_renderframes[m_renderer->m_renderframes.size() - 2];

		m_frame->m_projection = prev_frame->m_projection;
		m_frame->m_view = prev_frame->m_view;
		m_frame->m_transformations = prev_frame->m_transformations;

		m_frame->m_shader = prev_frame->m_shader;
	}
}

Context::~Context()
{
	TAssert(m_renderer->m_renderframes.size());

	m_renderer->m_renderframes.pop_back();

	if (m_renderer->m_renderframes.size())
	{
		if (m_frame->m_shader != TInvalid(ShaderIndex))
		{
			m_frame->m_projection_updated = false;
			m_frame->m_view_updated = false;
			m_frame->m_transform_updated = false;
		}
	}
	else
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		if (m_renderer)
			glViewport(0, 0, (GLsizei)m_renderer->m_window_data->m_width, (GLsizei)m_renderer->m_window_data->m_height);

		glUseProgram(0);

		glDisable(GL_BLEND);

		glDepthMask(true);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glDepthFunc(GL_LESS);

		glFrontFace(GL_CCW);
	}
}

void Context::SetProjection(const Matrix4x4& m)
{
	m_frame->m_projection = m;
	m_frame->m_projection_updated = false;
}

void Context::SetView(const Matrix4x4& m)
{
	m_frame->m_view = m;
	m_frame->m_view_updated = false;
}

void Context::UseShader(char* program_name)
{
	m_frame->m_shader = m_renderer->m_shaders->FindShader(program_name);

	if (*program_name)
		TCheck(m_frame->m_shader != SHADER_INVALID);

	UseShader(m_frame->m_shader);
}

void Context::UseShader(ShaderIndex shader)
{
	m_frame->m_shader = shader;

	if (m_frame->m_shader == SHADER_INVALID)
	{
		glUseProgram(0);
		return;
	}

	glUseProgram((GLuint)m_renderer->m_shaders->m_shaders[m_frame->m_shader].m_program);

	m_frame->m_projection_updated = false;
	m_frame->m_view_updated = false;
}

void Context::SetUniform(UniformIndex uniform, const Matrix4x4& value)
{
	glUniformMatrix4fv(m_renderer->m_shaders->m_shaders[m_frame->m_shader].m_uniforms[uniform], 1, false, value);
}

void Context::BeginRenderTris()
{
	m_renderer->m_tex_coords.clear();
	m_renderer->m_normals.clear();
	m_renderer->m_colors.clear();
	m_renderer->m_verts.clear();

	m_has_texcoord = false;
	m_has_normal = false;
	m_has_color = false;

	m_drawmode = GL_TRIANGLES;
}

void Context::BeginRenderTriFan()
{
	m_renderer->m_tex_coords.clear();
	m_renderer->m_normals.clear();
	m_renderer->m_colors.clear();
	m_renderer->m_verts.clear();

	m_has_texcoord = false;
	m_has_normal = false;
	m_has_color = false;

	m_drawmode = GL_TRIANGLE_FAN;
}

void Context::BeginRenderTriStrip()
{
	m_renderer->m_tex_coords.clear();
	m_renderer->m_normals.clear();
	m_renderer->m_colors.clear();
	m_renderer->m_verts.clear();

	m_has_texcoord = false;
	m_has_normal = false;
	m_has_color = false;

	m_drawmode = GL_TRIANGLE_STRIP;
}

void Context::BeginRenderLines()
{
	m_renderer->m_tex_coords.clear();
	m_renderer->m_normals.clear();
	m_renderer->m_colors.clear();
	m_renderer->m_verts.clear();

	m_has_texcoord = false;
	m_has_normal = false;
	m_has_color = false;

	m_drawmode = GL_LINES;
}

void Context::BeginRenderLineLoop()
{
	m_renderer->m_tex_coords.clear();
	m_renderer->m_normals.clear();
	m_renderer->m_colors.clear();
	m_renderer->m_verts.clear();

	m_has_texcoord = false;
	m_has_normal = false;
	m_has_color = false;

	m_drawmode = GL_LINE_LOOP;
}

void Context::BeginRenderLineStrip()
{
	m_renderer->m_tex_coords.clear();
	m_renderer->m_normals.clear();
	m_renderer->m_colors.clear();
	m_renderer->m_verts.clear();

	m_has_texcoord = false;
	m_has_normal = false;
	m_has_color = false;

	m_drawmode = GL_LINE_STRIP;
}

void Context::BeginRenderDebugLines()
{
	BeginRenderLines();
}

void Context::BeginRenderPoints(float flSize)
{
	m_renderer->m_tex_coords.clear();
	m_renderer->m_normals.clear();
	m_renderer->m_colors.clear();
	m_renderer->m_verts.clear();

	m_has_texcoord = false;
	m_has_normal = false;
	m_has_color = false;

	glPointSize(flSize);
	m_drawmode = GL_POINTS;
}

void Context::TexCoord(float s, float t, int channel)
{
	TAssert(channel == 0); // EndRender() needs updating if you want to use another channel.

	m_texcoords[channel] = Vector2D(s, t);

	m_has_texcoord = true;
}

void Context::TexCoord(const Vector2D& v, int channel)
{
	TAssert(channel == 0); // EndRender() needs updating if you want to use another channel.

	m_texcoords[channel] = v;

	m_has_texcoord = true;
}

void Context::TexCoord(const DoubleVector2D& v, int channel)
{
	TAssert(channel == 0); // EndRender() needs updating if you want to use another channel.

	m_texcoords[channel] = Vector2D(v);

	m_has_texcoord = true;
}

void Context::TexCoord(const Vector& v, int channel)
{
	TAssert(channel == 0); // EndRender() needs updating if you want to use another channel.

	m_texcoords[channel] = v;

	m_has_texcoord = true;
}

void Context::TexCoord(const DoubleVector& v, int channel)
{
	TAssert(channel == 0); // EndRender() needs updating if you want to use another channel.

	m_texcoords[channel] = DoubleVector2D(v);

	m_has_texcoord = true;
}

void Context::Normal(const Vector& v)
{
	m_normal = v;
	m_has_normal = true;
}

void Context::Color(const ::Color& c)
{
	m_color = c;
	m_has_color = true;
}

void Context::Vertex(const Vector& v)
{
	if (m_has_texcoord)
	{
		for (size_t i = 0; i < MAX_TEXTURE_CHANNELS; i++)
			m_renderer->m_tex_coords.push_back(m_texcoords[i]);
	}

	if (m_has_normal)
		m_renderer->m_normals.push_back(m_normal);

	if (m_has_color)
		m_renderer->m_colors.push_back(m_color);

	m_renderer->m_verts.push_back(v);
}

void Context::EndRender()
{
	bool valid_shader = m_frame->m_shader < MAX_SHADERS && m_frame->m_shader >= 0;
	TCheck(valid_shader);
	if (!valid_shader)
	{
		UseShader(SHADER_MODEL);
		if (m_frame->m_shader == SHADER_INVALID)
			return;
	}

	Shader* shader = &m_renderer->m_shaders->m_shaders[m_frame->m_shader];

	if (!m_frame->m_projection_updated)
		SetUniform(UNIFORM_PROJECTION, m_frame->m_projection);

	if (!m_frame->m_view_updated)
		SetUniform(UNIFORM_VIEW, m_frame->m_view);

	if (!m_frame->m_transform_updated)
		SetUniform(UNIFORM_GLOBAL, m_frame->m_transformations);

	m_frame->m_projection_updated = m_frame->m_view_updated = m_frame->m_transform_updated = true;

	if (m_has_texcoord)
	{
		if (shader->m_texcoord_attributes[0] != ~0)
		{
			glEnableVertexAttribArray((GLuint)shader->m_texcoord_attributes[0]);
			glVertexAttribPointer((GLuint)shader->m_texcoord_attributes[0], 2, GL_FLOAT, false, 0, m_renderer->m_tex_coords.data());
		}
	}

	if (m_has_normal && shader->m_normal_attribute != ~0)
	{
		glEnableVertexAttribArray((GLuint)shader->m_normal_attribute);
		glVertexAttribPointer((GLuint)shader->m_normal_attribute, 3, GL_FLOAT, false, 0, m_renderer->m_normals.data());
	}

	if (m_has_color && shader->m_color_attribute != ~0)
	{
		glEnableVertexAttribArray((GLuint)shader->m_color_attribute);
		glVertexAttribPointer((GLuint)shader->m_color_attribute, 3, GL_UNSIGNED_BYTE, true, sizeof(::Color), m_renderer->m_colors.data());
	}

	TAssert(shader->m_position_attribute != ~0);
	glEnableVertexAttribArray((GLuint)shader->m_position_attribute);
	glVertexAttribPointer((GLuint)shader->m_position_attribute, 3, GL_FLOAT, false, 0, m_renderer->m_verts.data());

	glDrawArrays(m_drawmode, 0, (GLsizei)m_renderer->m_verts.size());

	glDisableVertexAttribArray((GLuint)shader->m_position_attribute);
	for (size_t i = 0; i < MAX_TEXTURE_CHANNELS; i++)
	{
		if (shader->m_texcoord_attributes[i] != ~0)
			glDisableVertexAttribArray((GLuint)shader->m_texcoord_attributes[i]);
	}
	if (shader->m_normal_attribute != ~0)
		glDisableVertexAttribArray((GLuint)shader->m_normal_attribute);
	if (shader->m_color_attribute != ~0)
		glDisableVertexAttribArray((GLuint)shader->m_color_attribute);
}

