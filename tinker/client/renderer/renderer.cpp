#include "renderer.h"

#include <GL3/gl3w.h>

#include "matrix.h"

#include "context.h"
#include "window.h"

#ifdef USE_SKYBOX
#include "skybox.h"
#endif

#define TError(x) TUnimplemented()

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

void Renderer::Initialize()
{
#ifdef USE_SKYBOX
	m_skybox_verts_vbo = LoadVertexDataIntoGL(asset_skybox_verts_size_bytes, &asset_skybox_verts[0]);
	m_skybox_indxs_vbo = LoadIndexDataIntoGL(asset_skybox_indxs_size_bytes, &asset_skybox_indxs[0]);

	glGenVertexArrays(1, &m_skybox_vao);
	glBindVertexArray(m_skybox_vao);

	glBindBuffer(GL_ARRAY_BUFFER, (GLuint)m_skybox_verts_vbo);
	glEnableVertexAttribArray((GLuint)m_shaders->m_position_attribute);
	glEnableVertexAttribArray((GLuint)m_shaders->m_texcoord_attributes[0]);
	glVertexAttribPointer((GLuint)m_shaders->m_position_attribute, 3, GL_FLOAT, false, asset_skybox_vert_stride_bytes, 0);
	glVertexAttribPointer((GLuint)m_shaders->m_texcoord_attributes[0], 2, GL_FLOAT, false, asset_skybox_vert_stride_bytes, BUFFER_OFFSET(asset_skybox_uv_offset_bytes));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_skybox_indxs_vbo);

	glBindVertexArray(0);
#endif

	glGenVertexArrays(1, &m_default_vao);
	glBindVertexArray(m_default_vao);

	glGenBuffers(1, &m_dynamic_mesh_vbo);
}

void Renderer::ClearDepth()
{
	glClear(GL_DEPTH_BUFFER_BIT);
}

void Renderer::ClearColor(color4 c)
{
	glClearColor((float)(c.r) / 255, (float)(c.g) / 255, (float)(c.b) / 255, (float)(c.a) / 255);
	glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::SetCamera(vec3 camera_position, vec3 camera_direction, vec3 camera_up, float camera_fov, float camera_near, float camera_far)
{
	m_camera_position = camera_position;
	m_camera_direction = camera_direction;
	m_camera_up = camera_up;
	m_camera_fov = camera_fov;
	m_camera_near = camera_near;
	m_camera_far = camera_far;
}

void Renderer::StartRendering(Context* c)
{
	float aspect_ratio = (float)m_window_data->m_width / (float)m_window_data->m_height;

	mat4 projection = mat4::ProjectPerspective(m_camera_fov, aspect_ratio, m_camera_near, m_camera_far);
	c->SetProjection(projection);
	m_projection = projection;

	mat4 modelview = mat4::ConstructCameraView(m_camera_position, m_camera_direction, m_camera_up);
	c->SetView(modelview);
	m_modelview = modelview;
}

void Renderer::FinishRendering(Context* /*c*/)
{
}

void Renderer::BeginRenderPrimitive(int drawmode)
{
	m_verts.clear();
	m_num_verts = 0;

	m_has_texcoord = false;
	m_has_normal = false;
	m_has_color = false;

	m_drawmode = drawmode;
}

Renderer::RenderFrame* Renderer::GetCurrentFrame()
{
	TAssert(m_num_renderframes);
	return &m_renderframes[m_num_renderframes - 1];
}

Renderer::RenderFrame* Renderer::PushRenderFrame()
{
	TAssert(m_num_renderframes < MAX_RENDERFRAMES);
	m_num_renderframes++;
	return new (&m_renderframes[m_num_renderframes - 1]) Renderer::RenderFrame();
}

Renderer::RenderFrame* Renderer::PopRenderFrame()
{
	TAssert(m_num_renderframes > 0);
	m_num_renderframes--;
	return &m_renderframes[m_num_renderframes - 1];
}

uint32 Renderer::LoadVertexDataIntoGL(size_t size_bytes, const float* verts)
{
	// If it's only floats doubles and the occasional int then it should always be a multiple of four bytes.
	TAssert(size_bytes % 4 == 0);

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, size_bytes, verts, GL_STATIC_DRAW);

	int size = 0;
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	if (size_bytes != size)
	{
		glDeleteBuffers(1, &vbo);
		TAssert(false);
		TError("CRenderer::LoadVertexDataIntoGL(): Data size is mismatch with input array\n");
		return 0;
	}

	return vbo;
}

uint32 Renderer::LoadIndexDataIntoGL(size_t size_bytes, const unsigned int* indxs)
{
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size_bytes, indxs, GL_STATIC_DRAW);

	int size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	if (size_bytes != size)
	{
		glDeleteBuffers(1, &vbo);
		TAssert(false);
		TError("CRenderer::LoadVertexDataIntoGL(): Data size is mismatch with input array\n");
		return 0;
	}

	return vbo;
}
