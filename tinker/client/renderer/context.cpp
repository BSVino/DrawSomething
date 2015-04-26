#include "context.h"

#include "tinker_gl.h"

#include "renderer.h"
#include "shaders.h"
#include "window.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

Context::Context(Renderer* renderer, bool inherit)
{
	m_renderer = renderer;

	m_frame = m_renderer->PushRenderFrame();

	if (inherit && m_renderer->m_num_renderframes > 1)
	{
		Renderer::RenderFrame* prev_frame = &m_renderer->m_renderframes[m_renderer->m_num_renderframes - 2];

		m_frame->m_projection = prev_frame->m_projection;
		m_frame->m_view = prev_frame->m_view;
		m_frame->m_transformations = prev_frame->m_transformations;

		m_frame->m_shader = prev_frame->m_shader;
	}
	else
	{
		m_frame->m_projection = mat4::s_identity;
		m_frame->m_view = mat4::s_identity;
		m_frame->m_transformations = mat4::s_identity;

		m_frame->m_shader = TInvalid(ShaderIndex);
	}
}

Context::~Context()
{
	TAssert(m_renderer->m_num_renderframes);

	m_frame = m_renderer->PopRenderFrame();

	if (m_renderer->m_num_renderframes)
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

void Context::SetProjection(const mat4& m)
{
	m_frame->m_projection = m;
	m_frame->m_projection_updated = false;
}

void Context::SetView(const mat4& m)
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

#define UNREASONABLE 999999

void Context::SetUniform(UniformIndex uniform, const vec3& value)
{
	TCheck(value.x < UNREASONABLE && value.x > -UNREASONABLE);
	TCheck(value.y < UNREASONABLE && value.y > -UNREASONABLE);
	TCheck(value.z < UNREASONABLE && value.z > -UNREASONABLE);
	glUniform3fv(m_renderer->m_shaders->m_shaders[m_frame->m_shader].m_uniforms[uniform], 1, &value.x);
}

void Context::SetUniform(UniformIndex uniform, const mat4& value)
{
	TCheck(value.m[0][0] < UNREASONABLE && value.m[0][0] > -UNREASONABLE);
	glUniformMatrix4fv(m_renderer->m_shaders->m_shaders[m_frame->m_shader].m_uniforms[uniform], 1, false, value);
}

void Context::BeginRenderTris()
{
	m_renderer->BeginRenderPrimitive(GL_TRIANGLES);
}

void Context::BeginRenderTriFan()
{
	m_renderer->BeginRenderPrimitive(GL_TRIANGLE_FAN);
}

void Context::BeginRenderTriStrip()
{
	m_renderer->BeginRenderPrimitive(GL_TRIANGLE_STRIP);
}

void Context::BeginRenderLines()
{
	m_renderer->BeginRenderPrimitive(GL_LINES);
}

void Context::BeginRenderLineLoop()
{
	m_renderer->BeginRenderPrimitive(GL_LINE_LOOP);
}

void Context::BeginRenderLineStrip()
{
	m_renderer->BeginRenderPrimitive(GL_LINE_STRIP);
}

void Context::BeginRenderDebugLines()
{
	BeginRenderLines();
}

void Context::BeginRenderPoints(float flSize)
{
	m_renderer->BeginRenderPrimitive(GL_POINTS);

	glPointSize(flSize);
}

void Context::TexCoord(float s, float t, int channel)
{
	TAssert(channel == 0); // Vertex() and EndRender() need updating if you want to use another channel.

	m_texcoords = vec2(s, t);

	TAssert(m_renderer->m_has_texcoord || !m_renderer->m_dynamic_mesh_floats);
	m_renderer->m_has_texcoord = true;
}

void Context::TexCoord(const vec2& v, int channel)
{
	TAssert(channel == 0); // Vertex() and EndRender() need updating if you want to use another channel.

	m_texcoords = v;

	TAssert(m_renderer->m_has_texcoord || !m_renderer->m_dynamic_mesh_floats);
	m_renderer->m_has_texcoord = true;
}

void Context::TexCoord(const vec3& v, int channel)
{
	TAssert(channel == 0); // Vertex() and EndRender() need updating if you want to use another channel.

	m_texcoords = vec2(v);

	TAssert(m_renderer->m_has_texcoord || !m_renderer->m_dynamic_mesh_floats);
	m_renderer->m_has_texcoord = true;
}

void Context::Normal(const vec3& v)
{
	m_normal = v;

	TAssert(m_renderer->m_has_normal || !m_renderer->m_dynamic_mesh_floats);
	m_renderer->m_has_normal = true;
}

void Context::Color(const color4& c)
{
	m_color = vec4(c);

	TAssert(m_renderer->m_has_color || !m_renderer->m_dynamic_mesh_floats);
	m_renderer->m_has_color = true;
}

void Context::Vertex(const vec3& v)
{
	m_renderer->PushDynamicMeshFloat(v.x);
	m_renderer->PushDynamicMeshFloat(v.y);
	m_renderer->PushDynamicMeshFloat(v.z);

	if (m_renderer->m_has_texcoord)
	{
		//for (size_t i = 0; i < MAX_TEXTURE_CHANNELS; i++)
		{
			m_renderer->PushDynamicMeshFloat(m_texcoords.x);
			m_renderer->PushDynamicMeshFloat(m_texcoords.y);
		}
	}

	if (m_renderer->m_has_normal)
	{
		m_renderer->PushDynamicMeshFloat(m_normal.x);
		m_renderer->PushDynamicMeshFloat(m_normal.y);
		m_renderer->PushDynamicMeshFloat(m_normal.z);
	}

	if (m_renderer->m_has_color)
	{
		m_renderer->PushDynamicMeshFloat(m_color.x);
		m_renderer->PushDynamicMeshFloat(m_color.y);
		m_renderer->PushDynamicMeshFloat(m_color.z);
		m_renderer->PushDynamicMeshFloat(m_color.w);
	}

	m_renderer->m_num_verts++;
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

	if (!m_frame->m_projection_updated)
		SetUniform(UNIFORM_PROJECTION, m_frame->m_projection);

	if (!m_frame->m_view_updated)
		SetUniform(UNIFORM_VIEW, m_frame->m_view);

	if (!m_frame->m_transform_updated)
		SetUniform(UNIFORM_GLOBAL, m_frame->m_transformations);

	m_frame->m_projection_updated = m_frame->m_view_updated = m_frame->m_transform_updated = true;

	glBindVertexArray(m_renderer->m_default_vao);

	glBindBuffer(GL_ARRAY_BUFFER, m_renderer->m_dynamic_mesh_vbo);
	glBufferData(GL_ARRAY_BUFFER, m_renderer->m_dynamic_mesh_floats * sizeof(float), m_renderer->m_verts, GL_STATIC_DRAW);

	int stride = 3 * sizeof(float);
	int position_offset = 0;
	int texcoord_offset = stride;
	int normal_offset = stride;
	int color_offset = stride;

	if (m_renderer->m_has_texcoord)
	{
		int uv_size = 2 * sizeof(float);
		stride += uv_size;
		normal_offset += uv_size;
		color_offset += uv_size;
	}

	if (m_renderer->m_has_normal)
	{
		int normal_size = 3 * sizeof(float);
		stride += normal_size;
		color_offset += normal_size;
	}

	if (m_renderer->m_has_color)
		stride += 4 * sizeof(float);

	TAssert(m_renderer->m_shaders->m_position_attribute != ~0);
	glEnableVertexAttribArray((GLuint)m_renderer->m_shaders->m_position_attribute);
	glVertexAttribPointer((GLuint)m_renderer->m_shaders->m_position_attribute, 3, GL_FLOAT, false, stride, BUFFER_OFFSET(position_offset));

	if (m_renderer->m_has_texcoord)
	{
		if (m_renderer->m_shaders->m_texcoord_attributes[0] != ~0)
		{
			glEnableVertexAttribArray((GLuint)m_renderer->m_shaders->m_texcoord_attributes[0]);
			glVertexAttribPointer((GLuint)m_renderer->m_shaders->m_texcoord_attributes[0], 2, GL_FLOAT, false, stride, BUFFER_OFFSET(texcoord_offset));
		}
	}

	if (m_renderer->m_has_normal && m_renderer->m_shaders->m_normal_attribute != ~0)
	{
		glEnableVertexAttribArray((GLuint)m_renderer->m_shaders->m_normal_attribute);
		glVertexAttribPointer((GLuint)m_renderer->m_shaders->m_normal_attribute, 3, GL_FLOAT, false, stride, BUFFER_OFFSET(normal_offset));
	}

	if (m_renderer->m_has_color && m_renderer->m_shaders->m_color_attribute != ~0)
	{
		glEnableVertexAttribArray((GLuint)m_renderer->m_shaders->m_color_attribute);
		glVertexAttribPointer((GLuint)m_renderer->m_shaders->m_color_attribute, 3, GL_FLOAT, true, stride, BUFFER_OFFSET(color_offset));
	}

	glDrawArrays(m_renderer->m_drawmode, 0, m_renderer->m_num_verts);

	glDisableVertexAttribArray((GLuint)m_renderer->m_shaders->m_position_attribute);
	for (size_t i = 0; i < MAX_TEXTURE_CHANNELS; i++)
	{
		if (m_renderer->m_shaders->m_texcoord_attributes[i] != ~0)
			glDisableVertexAttribArray((GLuint)m_renderer->m_shaders->m_texcoord_attributes[i]);
	}
	if (m_renderer->m_shaders->m_normal_attribute != ~0)
		glDisableVertexAttribArray((GLuint)m_renderer->m_shaders->m_normal_attribute);
	if (m_renderer->m_shaders->m_color_attribute != ~0)
		glDisableVertexAttribArray((GLuint)m_renderer->m_shaders->m_color_attribute);
}

