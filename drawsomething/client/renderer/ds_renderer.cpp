#include "ds_renderer.h"

#include "client/renderer/context.h"
#include "client/renderer/skybox.h"

#include "../ds_client.h"

#include <GL3/gl3w.h>

void DSRenderer::Draw()
{
	Artist* local = g_client_data->GetLocalArtist();

	if (!local)
		return;

	base.ClearDepth();

	base.SetCamera(local->m_position, AngleVector(eangle(local->m_looking.p, local->m_looking.y, 0)), vec3(0, 0, 1), g_client_data->m_local_artist.GetFOV(), 0.01f, 1000);

	Context c(&base);
	base.StartRendering(&c);

	RenderSkybox();

	c.UseShader(SHADER_MODEL);

	c.SetUniform(UNIFORM_CAMERA, local->m_position);

	c.BeginRenderTriFan();
		c.Color(color4(57, 138, 175, 255));

		c.Vertex(vec3(-10, -10, 0));
		c.Vertex(vec3(10, -10, 0));
		c.Vertex(vec3(10, 10, 0));
		c.Vertex(vec3(-10, 10, 0));
	c.EndRender();

	c.UseShader(SHADER_DEBUGLINE);

	for (int k = 0; k < g_client_data->m_num_strokes; k++)
	{
		c.BeginRenderLineStrip();
			int first_point = g_client_data->m_strokes[k].m_first;
			int max_point = g_client_data->m_strokes[k].m_first + g_client_data->m_strokes[k].m_size;
			for (int j = first_point; j < max_point; j++)
			{
				vec3 point = g_client_data->m_stroke_points[j];
				c.Vertex(point);
			}
		c.EndRender();
	}

	base.FinishRendering(&c);
}

void DSRenderer::RenderSkybox()
{
	Context c(&base, false);

	glDisable(GL_CULL_FACE);
	glBindVertexArray(base.m_skybox_vao);

	c.UseShader(SHADER_SKYBOX);

	c.SetUniform(UNIFORM_PROJECTION, base.m_projection);
	c.SetUniform(UNIFORM_VIEW, mat4::ConstructCameraView(vec3(0, 0, 0), base.m_camera_direction, base.m_camera_up));
	c.SetUniform(UNIFORM_GLOBAL, mat4::s_identity);

	glDrawElements(GL_TRIANGLES, asset_skybox_num_verts, GL_UNSIGNED_INT, nullptr);
}

