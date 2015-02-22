#pragma once

#include "color.h"
#include "vector.h"
#include "matrix.h"
#include "tvector.h"

#include "assets.h"

struct ShaderLibrary;

#define MAX_RENDERFRAMES 8

struct Renderer
{
	mat4 m_modelview;
	mat4 m_projection;

	vec3 m_camera_position;
	vec3 m_camera_direction;
	vec3 m_camera_up;
	float m_camera_fov;
	float m_camera_near;
	float m_camera_far;

	uint32 m_default_vao;

#ifdef USE_SKYBOX
	uint32 m_skybox_verts_vbo;
	uint32 m_skybox_indxs_vbo;
	uint32 m_skybox_vao;
#endif

	// For building meshes
	tvector<float> m_verts;
	int m_drawmode;
	int m_num_verts;
	int m_has_texcoord : 1;
	int m_has_normal : 1;
	int m_has_color : 1;
	uint32 m_dynamic_mesh_vbo;

	struct RenderFrame
	{
		mat4 m_view;
		mat4 m_projection;
		mat4 m_transformations;

		ShaderIndex m_shader;

		int m_projection_updated : 1;
		int m_view_updated : 1;
		int m_transform_updated : 1;

		RenderFrame()
		{
			m_projection_updated = false;
			m_view_updated = false;
			m_transform_updated = false;
		}
	};
	RenderFrame m_renderframes[MAX_RENDERFRAMES];
	uint32 m_num_renderframes;

	ShaderLibrary*     m_shaders;
	struct WindowData* m_window_data;

	Renderer(ShaderLibrary* shaders, struct WindowData* window_data)
		: m_shaders(shaders), m_window_data(window_data)
	{
		m_num_renderframes = 0;
	}

	void Initialize();

	void ClearDepth();
	void ClearColor(color4 c);

	void SetCamera(vec3 camera_position, vec3 camera_direction, vec3 camera_up, float camera_fov, float camera_near, float camera_far);

	void StartRendering(struct Context* c);
	void FinishRendering(struct Context* c);

	void RenderSkybox();

	void BeginRenderPrimitive(int drawmode);

	RenderFrame* GetCurrentFrame();
	RenderFrame* PushRenderFrame();
	RenderFrame* PopRenderFrame();

	uint32 LoadVertexDataIntoGL(size_t size_bytes, const float* verts);
	uint32 LoadIndexDataIntoGL(size_t size_bytes, const unsigned int* indxs);
};

