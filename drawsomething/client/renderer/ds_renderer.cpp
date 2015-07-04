#include "ds_renderer.h"

#include "client/renderer/context.h"
#include "client/renderer/skybox.h"

#include "../ds_client.h"

#include "tinker_gl.h"


#include "../../server/ds_server.h"

void DrawBox(Context& c, BucketCoordinate* bc, float margin, color4 color)
{
	c.BeginRenderLineStrip();
		c.Color(color);
		c.Vertex(vec3(bc->x+margin, (float)bc->y+margin, (float)bc->z+margin));
		c.Vertex(vec3(bc->x+1-margin, (float)bc->y+margin, (float)bc->z+margin));
		c.Vertex(vec3(bc->x+1-margin, (float)bc->y+1-margin, (float)bc->z+margin));
		c.Vertex(vec3(bc->x+margin, (float)bc->y+1-margin, (float)bc->z+margin));
		c.Vertex(vec3(bc->x+margin, (float)bc->y+margin, (float)bc->z+margin));
		c.Vertex(vec3(bc->x+margin, (float)bc->y+margin, (float)bc->z+1-margin));
		c.Vertex(vec3(bc->x+margin, (float)bc->y+1-margin, (float)bc->z+1-margin));
		c.Vertex(vec3(bc->x+1-margin, (float)bc->y+1-margin, (float)bc->z+1-margin));
		c.Vertex(vec3(bc->x+1-margin, (float)bc->y+margin, (float)bc->z+1-margin));
		c.Vertex(vec3(bc->x+margin, (float)bc->y+margin, (float)bc->z+1-margin));
	c.EndRender();
	c.BeginRenderLineStrip();
		c.Color(color);
		c.Vertex(vec3(bc->x+1-margin, (float)bc->y+margin, (float)bc->z+margin));
		c.Vertex(vec3(bc->x+1-margin, (float)bc->y+margin, (float)bc->z+1-margin));
	c.EndRender();
	c.BeginRenderLineStrip();
		c.Color(color);
		c.Vertex(vec3(bc->x+1-margin, (float)bc->y+1-margin, (float)bc->z+margin));
		c.Vertex(vec3(bc->x+1-margin, (float)bc->y+1-margin, (float)bc->z+1-margin));
	c.EndRender();
	c.BeginRenderLineStrip();
		c.Color(color);
		c.Vertex(vec3(bc->x+margin, (float)bc->y+1-margin, (float)bc->z+margin));
		c.Vertex(vec3(bc->x+margin, (float)bc->y+1-margin, (float)bc->z+1-margin));
	c.EndRender();
}

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

#ifdef _DEBUG
	// Server buckets are green
	color4 color_server(100, 255, 100, 255);

	if (g_server_data)
	{
		c.UseShader(SHADER_DEBUGLINE);
		c.SetUniform(UNIFORM_GLOBAL, mat4::s_identity);
		c.SetUniform(UNIFORM_CAMERA, local->m_position);
		c.SetUniform(UNIFORM_CAMERA_DIRECTION, base.m_camera_direction);

		for (int q = 0; q < NUM_SERVER_BUCKETS; q++)
		{
			BucketHeader* bucket = &g_server_data->m_buckets.m_buckets_hash[q];

			if (!bucket->Valid())
				continue;

			vec3 bucket_origin = vec3(bucket->m_coordinates.m_bucket.x, bucket->m_coordinates.m_bucket.y, bucket->m_coordinates.m_bucket.z);

			for (int k = 0; k < bucket->m_num_strokes; k++)
			{
				c.BeginRenderLineStrip();
					c.Color(color_server);
					VertexIndex first_vert = bucket->m_strokes[k].m_first_vertex;
					VertexIndex max_point = bucket->m_strokes[k].m_first_vertex + bucket->m_strokes[k].m_num_verts;
					for (int j = first_vert; j < max_point; j++)
					{
						vec3 point = bucket->m_verts[j] + bucket_origin;
						c.Vertex(point);
					}
				c.EndRender();
			}
		}
	}
#endif

	color4 color_white(255, 255, 255, 255);

	c.UseShader(SHADER_LINE);

	c.SetUniform(UNIFORM_GLOBAL, mat4::s_identity);
	c.SetUniform(UNIFORM_CAMERA, local->m_position);
	c.SetUniform(UNIFORM_CAMERA_DIRECTION, base.m_camera_direction);

	for (int q = 0; q < NUM_CLIENT_BUCKETS; q++)
	{
		BucketHeader* bucket = &g_client_data->m_buckets.m_buckets_hash[q];

		if (!bucket->Valid())
			continue;

		vec3 bucket_origin = vec3(bucket->m_coordinates.m_bucket.x, bucket->m_coordinates.m_bucket.y, bucket->m_coordinates.m_bucket.z);

		for (int k = 0; k < bucket->m_num_strokes; k++)
		{
			c.BeginRenderLineStrip();
				c.Color(color_white);
				VertexIndex first_vert = bucket->m_strokes[k].m_first_vertex;
				VertexIndex max_point = bucket->m_strokes[k].m_first_vertex + bucket->m_strokes[k].m_num_verts;
				for (int j = first_vert; j < max_point; j++)
				{
					vec3 point = bucket->m_verts[j] + bucket_origin;
					c.Vertex(point);
				}
			c.EndRender();
		}
	}

#ifdef _DEBUG
	c.UseShader(SHADER_DEBUGLINE);
	c.SetUniform(UNIFORM_GLOBAL, mat4::s_identity);
	c.SetUniform(UNIFORM_CAMERA, local->m_position);
	c.SetUniform(UNIFORM_CAMERA_DIRECTION, base.m_camera_direction);

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
	// Client buckets are blue
	color4 color_header_loaded(100, 100, 255, 255);
	for (int k = 0; k < NUM_CLIENT_BUCKETS; k++)
	{
		BucketHeader* header = &g_client_data->m_buckets.m_buckets_hash[k];

		if (!header->Valid())
			continue;

		BucketCoordinate* bc = &header->m_coordinates.m_bucket;

		DrawBox(c, bc, 0.02f, color_header_loaded);
	}

	if (g_server_data)
	{
		for (int k = 0; k < NUM_SERVER_BUCKETS; k++)
		{
			BucketHeader* header = &g_server_data->m_buckets.m_buckets_hash[k];

			if (!header->Valid())
				continue;

			BucketCoordinate* bc = &header->m_coordinates.m_bucket;

			DrawBox(c, bc, 0.01f, color_server);
		}
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

