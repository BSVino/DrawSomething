#include "ds_renderer.h"

#include "client/renderer/context.h"

#include "../ds_client.h"

void DSRenderer::Draw()
{
	Artist* local = g_client_data->GetLocalArtist();

	if (!local)
		return;

	base.ClearColor(color4(78, 188, 239, 255));
	base.ClearDepth();

	base.SetCamera(local->m_position, AngleVector(eangle(local->m_looking.p, local->m_looking.y, 0)), vec3(0, 0, 1), g_client_data->m_local_artist.GetFOV(), 0.01f, 1000);

	Context c(&base);
	base.StartRendering(&c);

	c.UseShader(SHADER_MODEL);

	c.BeginRenderTriFan();
		c.Color(color4(57, 138, 175, 255));

		c.TexCoord(vec2(0, 1));
		c.Vertex(vec3(-10, -10, 0));
		c.TexCoord(vec2(1, 1));
		c.Vertex(vec3(10, -10, 0));
		c.TexCoord(vec2(1, 0));
		c.Vertex(vec3(10, 10, 0));
		c.TexCoord(vec2(0, 0));
		c.Vertex(vec3(-10, 10, 0));
	c.EndRender();

	for (int k = 0; k < g_client_data->m_num_strokes; k++)
	{
		c.BeginRenderLineStrip();
			c.Color(color4(255, 255, 255, 255));

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
