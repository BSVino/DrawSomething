#pragma once

#include "color.h"
#include "vector.h"
#include "matrix.h"
#include "tvector.h"

#include "assets.h"

struct ShaderLibrary;

struct Renderer
{
	Matrix4x4 m_modelview;
	Matrix4x4 m_projection;

	Vector m_camera_position;
	Vector m_camera_direction;
	Vector m_camera_up;
	float m_camera_fov;
	float m_camera_near;
	float m_camera_far;

	// For building meshes
	tvector<Vector2D> m_tex_coords;
	tvector<Vector>   m_normals;
	tvector< ::Color> m_colors;
	tvector<Vector>   m_verts;

	struct RenderFrame
	{
		Matrix4x4 m_view;
		Matrix4x4 m_projection;
		Matrix4x4 m_transformations;

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
	tvector <RenderFrame> m_renderframes;

	ShaderLibrary*     m_shaders;
	struct WindowData* m_window_data;

	Renderer(ShaderLibrary* shaders, struct WindowData* window_data)
		: m_shaders(shaders), m_window_data(window_data)
	{
	}

	void ClearDepth();
	void ClearColor(Color c);

	void SetCamera(Vector camera_position, Vector camera_direction, Vector camera_up, float camera_fov, float camera_near, float camera_far);

	void StartRendering(struct Context* c);
	void FinishRendering(struct Context* c);

	RenderFrame* GetCurrentFrame();
};

