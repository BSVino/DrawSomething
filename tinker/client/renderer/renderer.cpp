#include "renderer.h"

#include <GL3/gl3w.h>

#include "matrix.h"

#include "context.h"
#include "window.h"

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

Renderer::RenderFrame* Renderer::GetCurrentFrame()
{
	TAssert(m_renderframes.size());

	return &m_renderframes.back();
}
