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

	base.SetCamera(local->m_position, AngleVector(eangle(local->m_looking.p, local->m_looking.y, 0)), vec3(0, 0, 1), 90, 0.01f, 1000);

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

	base.FinishRendering(&c);
}
