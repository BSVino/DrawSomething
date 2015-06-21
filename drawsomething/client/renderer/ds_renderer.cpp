#include "ds_renderer.h"

#include "client/renderer/context.h"
#include "client/renderer/skybox.h"

#include "../ds_client.h"

#include "tinker_gl.h"


#include "../../server/ds_server.h"

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

	c.UseShader(SHADER_LINE);

	c.SetUniform(UNIFORM_GLOBAL, mat4::s_identity);
	c.SetUniform(UNIFORM_CAMERA, local->m_position);
	c.SetUniform(UNIFORM_CAMERA_DIRECTION, base.m_camera_direction);

#if 0
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
#endif

#ifdef _DEBUG
	c.UseShader(SHADER_DEBUGLINE);
	c.SetUniform(UNIFORM_GLOBAL, mat4::s_identity);
	c.SetUniform(UNIFORM_CAMERA, local->m_position);
	c.SetUniform(UNIFORM_CAMERA_DIRECTION, base.m_camera_direction);

	color4 color_white(255, 255, 255, 255);
	for (int x = 0; x < 10; x++)
	{
		for (int y = 0; y < 10; y++)
		{
			for (int z = 0; z < 10; z++)
			{
				c.BeginRenderLineStrip();
					c.Color(color_white);
					c.Vertex(vec3(x-0.01f, (float)y, (float)z));
					c.Vertex(vec3(x+0.01f, (float)y, (float)z));
				c.EndRender();
				c.BeginRenderLineStrip();
					c.Color(color_white);
					c.Vertex(vec3((float)x, y-0.01f, (float)z));
					c.Vertex(vec3((float)x, y+0.01f, (float)z));
				c.EndRender();
				c.BeginRenderLineStrip();
					c.Color(color_white);
					c.Vertex(vec3((float)x, (float)y, z-0.01f));
					c.Vertex(vec3((float)x, (float)y, z+0.01f));
				c.EndRender();
			}
		}
	}

#if 1
	if (g_server_data)
	{
		color4 color_header_loaded(100, 100, 255, 255);
		for (int k = 0; k < NUM_CLIENT_BUCKETS; k++)
		{
			BucketHeader* header = &g_client_data->m_buckets.m_buckets_hash[k];

			if (!header->Valid())
				continue;

			BucketCoordinate* bc = &header->m_coordinates.m_bucket;

			c.BeginRenderLineStrip();
				c.Color(color_header_loaded);
				c.Vertex(vec3(bc->x+0.02f, (float)bc->y+0.02f, (float)bc->z+0.02f));
				c.Vertex(vec3(bc->x+0.98f, (float)bc->y+0.02f, (float)bc->z+0.02f));
				c.Vertex(vec3(bc->x+0.98f, (float)bc->y+0.98f, (float)bc->z+0.02f));
				c.Vertex(vec3(bc->x+0.02f, (float)bc->y+0.98f, (float)bc->z+0.02f));
				c.Vertex(vec3(bc->x+0.02f, (float)bc->y+0.02f, (float)bc->z+0.02f));
				c.Vertex(vec3(bc->x+0.02f, (float)bc->y+0.02f, (float)bc->z+0.98f));
				c.Vertex(vec3(bc->x+0.02f, (float)bc->y+0.98f, (float)bc->z+0.98f));
				c.Vertex(vec3(bc->x+0.98f, (float)bc->y+0.98f, (float)bc->z+0.98f));
				c.Vertex(vec3(bc->x+0.98f, (float)bc->y+0.02f, (float)bc->z+0.98f));
				c.Vertex(vec3(bc->x+0.02f, (float)bc->y+0.02f, (float)bc->z+0.98f));
			c.EndRender();
			c.BeginRenderLineStrip();
				c.Color(color_header_loaded);
				c.Vertex(vec3(bc->x+0.98f, (float)bc->y+0.02f, (float)bc->z+0.02f));
				c.Vertex(vec3(bc->x+0.98f, (float)bc->y+0.02f, (float)bc->z+0.98f));
			c.EndRender();
			c.BeginRenderLineStrip();
				c.Color(color_header_loaded);
				c.Vertex(vec3(bc->x+0.98f, (float)bc->y+0.98f, (float)bc->z+0.02f));
				c.Vertex(vec3(bc->x+0.98f, (float)bc->y+0.98f, (float)bc->z+0.98f));
			c.EndRender();
			c.BeginRenderLineStrip();
				c.Color(color_header_loaded);
				c.Vertex(vec3(bc->x+0.02f, (float)bc->y+0.98f, (float)bc->z+0.02f));
				c.Vertex(vec3(bc->x+0.02f, (float)bc->y+0.98f, (float)bc->z+0.98f));
			c.EndRender();
		}
	}

	color4 color_header_loaded(100, 255, 100, 255);

	for (int k = 0; k < NUM_SERVER_BUCKETS; k++)
	{
		BucketHeader* header = &g_server_data->m_buckets.m_buckets_hash[k];

		if (!header->Valid())
			continue;

		BucketCoordinate* bc = &header->m_coordinates.m_bucket;

		c.BeginRenderLineStrip();
			c.Color(color_header_loaded);
			c.Vertex(vec3(bc->x+0.01f, (float)bc->y+0.01f, (float)bc->z+0.01f));
			c.Vertex(vec3(bc->x+0.99f, (float)bc->y+0.01f, (float)bc->z+0.01f));
			c.Vertex(vec3(bc->x+0.99f, (float)bc->y+0.99f, (float)bc->z+0.01f));
			c.Vertex(vec3(bc->x+0.01f, (float)bc->y+0.99f, (float)bc->z+0.01f));
			c.Vertex(vec3(bc->x+0.01f, (float)bc->y+0.01f, (float)bc->z+0.01f));
			c.Vertex(vec3(bc->x+0.01f, (float)bc->y+0.01f, (float)bc->z+0.99f));
			c.Vertex(vec3(bc->x+0.01f, (float)bc->y+0.99f, (float)bc->z+0.99f));
			c.Vertex(vec3(bc->x+0.99f, (float)bc->y+0.99f, (float)bc->z+0.99f));
			c.Vertex(vec3(bc->x+0.99f, (float)bc->y+0.01f, (float)bc->z+0.99f));
			c.Vertex(vec3(bc->x+0.01f, (float)bc->y+0.01f, (float)bc->z+0.99f));
		c.EndRender();
		c.BeginRenderLineStrip();
			c.Color(color_header_loaded);
			c.Vertex(vec3(bc->x+0.99f, (float)bc->y+0.01f, (float)bc->z+0.01f));
			c.Vertex(vec3(bc->x+0.99f, (float)bc->y+0.01f, (float)bc->z+0.99f));
		c.EndRender();
		c.BeginRenderLineStrip();
			c.Color(color_header_loaded);
			c.Vertex(vec3(bc->x+0.99f, (float)bc->y+0.99f, (float)bc->z+0.01f));
			c.Vertex(vec3(bc->x+0.99f, (float)bc->y+0.99f, (float)bc->z+0.99f));
		c.EndRender();
		c.BeginRenderLineStrip();
			c.Color(color_header_loaded);
			c.Vertex(vec3(bc->x+0.01f, (float)bc->y+0.99f, (float)bc->z+0.01f));
			c.Vertex(vec3(bc->x+0.01f, (float)bc->y+0.99f, (float)bc->z+0.99f));
		c.EndRender();
	}
#endif
#endif

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

